/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * Milestone 4-1 (M4-1): Compact Bootloader (<1024 Bytes) Single Flash Page Write
 * Target MCU: Microchip ATtiny1616-MNR (QFN-20) on ADX Core-D
 *
 * Hard Constraint: Total binary size strictly <= 1024 Bytes (BOOTEND = 0x04)
 *                  Whole code resides within Pages 0..15 (0x0000 - 0x03FF).
 *                  Page 16 (0x0400 - 0x043F) is 100% clean application space.
 *
 * Channel: RS-485 (USART0 Alternate Pins) @ 19,200 bps
 *   - PA1: TXD (PORTMUX USART0 Alternate)
 *   - PA2: RXD
 *   - PA4: SP485EEN DE  (Active HIGH)
 *   - PA7: SP485EEN /RE (Active LOW)
 *
 * Visual Indicator:
 *   - PB2: Red LED (Toggles on frame activity & idle heartbeat)
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <stdbool.h>
#include <string.h>

#define BR32_FRAME_SIZE 32
#define BAUD_SETTING_16 0x02B5  // 19200 @ 16MHz/6
#define BAUD_SETTING_20 0x0364  // 19200 @ 20MHz/6

// BR32 Commands
#define CMD_IDENTIFY    0x01
#define CMD_WRITE_CHUNK 0x10
#define CMD_READ_CHUNK  0x20
#define CMD_BOOT_APP    0x30

// BR32 Status Codes
#define STATUS_OK          0x00
#define STATUS_ERR_CRC     0x01
#define STATUS_ERR_TIMEOUT 0x02
#define STATUS_ERR_PROTECT 0x03

// Memory Configuration
#define FLASH_PAGE_SIZE    64
#define FLASH_CHUNK_SIZE   16
#define FLASH_TOTAL_PAGES  256
#define APP_START_PAGE     16   // Page 16 = 0x0400

#define CHUNK_IDX_MASK     0x03
#define FLAG_COMMIT_PAGE   0x04

// Static RAM Buffers (Placed in SRAM 0x3800..)
static uint8_t page_buffer[FLASH_PAGE_SIZE];
static uint8_t rx_buf[BR32_FRAME_SIZE];
static uint8_t tx_buf[BR32_FRAME_SIZE];
static uint8_t my_uid[10];

// =========================================================================
// RS-485 Low-Level Byte Output
// =========================================================================
static inline void putch(uint8_t ch) {
    while (!(USART0.STATUS & USART_DREIF_bm))
        ;
    USART0.TXDATAL = ch;
}

// =========================================================================
// Unified 32-Byte Response Transmitter (Size-optimized single transmitter)
// =========================================================================
static void send_response(uint8_t status, uint8_t seq, uint8_t rx_cnt) {
    tx_buf[0] = status;
    tx_buf[1] = seq;
    tx_buf[2] = rx_cnt;
    for (uint8_t i = 0; i < 10; i++) {
        tx_buf[4 + i] = my_uid[i];
    }

    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < BR32_FRAME_SIZE - 2; i++) {
        crc = _crc_xmodem_update(crc, tx_buf[i]);
    }
    tx_buf[30] = (uint8_t)(crc >> 8);
    tx_buf[31] = (uint8_t)(crc & 0xFF);

    // DE=1, /RE=1 (Receiver disabled, Transmitter enabled)
    VPORTA.OUT |= (PIN4_bm | PIN7_bm);
    _delay_loop_2(42); // ~50us transceiver settle

    for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
        putch(tx_buf[i]);
    }

    // Wait until last bit completely leaves shift register
    while (!(USART0.STATUS & USART_TXCIF_bm))
        ;
    USART0.STATUS = USART_TXCIF_bm;

    _delay_loop_2(42); // ~50us line margin
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // Release bus to high-Z reception

    // Flush any loopback bytes
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }

    // Activity LED toggle
    VPORTB.IN |= PIN2_bm;

    // Re-arm LIN Break detect
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
}

// =========================================================================
// Flash Page Erase & Write (Executed strictly inside BOOT section < 0x0400)
// =========================================================================
__attribute__((noinline))
void nvm_commit_page_hw(uint16_t page_addr) {
    uint8_t *dest = (uint8_t *)(MAPPED_PROGMEM_START + page_addr);
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        *(dest++) = page_buffer[i];
    }
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
        ;
}

// =========================================================================
// Main Compact Bootloader Loop
// =========================================================================
int main(void) {
    // 1. Pin Configuration (PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED)
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR &= ~PIN2_bm;

    VPORTB.DIR |= PIN2_bm;
    VPORTB.OUT &= ~PIN2_bm;

    // 2. Route USART0 to Alternate Pins (PA1, PA2)
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 3. USART0 LINAUTO Hardware Break Detection Setup
    USART0.BAUD = ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) ? BAUD_SETTING_16 : BAUD_SETTING_20;
    USART0.DBGCTRL = 1;
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 4. Read 10-Byte Internal Factory Serial (SIGROW)
    const volatile uint8_t *sig_ptr = (const volatile uint8_t *)&SIGROW_SERNUM0;
    for (uint8_t i = 0; i < 10; i++) {
        my_uid[i] = sig_ptr[i];
    }

    // 5. Initialize SRAM Page Buffer
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        page_buffer[i] = 0xFF;
    }

    // 6. Arm Break Detection (Principle 1)
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    uint16_t idle_counter = 0;

    for (;;) {
        // Guard 1: Inconsistent Sync Field Error Reset
        if (USART0.STATUS & USART_ISFIF_bm) {
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // Guard 2: Break + 0x55 Sync Received -> Read Frame Byte 0
        if (USART0.STATUS & USART_RXCIF_bm) {
            (void)USART0.RXDATAH;
            rx_buf[0] = USART0.RXDATAL;
            uint8_t rx_count = 1;

            // 32-Byte Timed Window Receiver Loop
            while (rx_count < BR32_FRAME_SIZE) {
                uint16_t to_timer = 0;
                while (!(USART0.STATUS & USART_RXCIF_bm)) {
                    if (++to_timer > 12000) {
                        goto rx_window_done;
                    }
                }
                (void)USART0.RXDATAH;
                rx_buf[rx_count++] = USART0.RXDATAL;
            }

        rx_window_done:
            // Principle 6: Silence on Overflow >32 Bytes (Cascading Collision Prevention)
            if (rx_count == BR32_FRAME_SIZE) {
                _delay_loop_2(500); // ~0.6ms
                if (USART0.STATUS & USART_RXCIF_bm) {
                    while (USART0.STATUS & USART_RXCIF_bm) {
                        (void)USART0.RXDATAL;
                    }
                    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                    continue;
                }
            }

            // Principle 2: Gate Check (Destination Matching)
            if (rx_count < 12) {
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                continue;
            }
            bool is_broadcast = true;
            bool is_for_me = true;
            for (uint8_t i = 0; i < 10; i++) {
                if (rx_buf[2 + i] != 0x00) is_broadcast = false;
                if (rx_buf[2 + i] != my_uid[i]) is_for_me = false;
            }
            if (!is_broadcast && !is_for_me) {
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                continue; // Complete Silence
            }

            // Timeout error
            if (rx_count < BR32_FRAME_SIZE) {
                for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) tx_buf[i] = 0;
                send_response(STATUS_ERR_TIMEOUT, rx_buf[1], rx_count);
                continue;
            }

            // Principle 4: CRC-16 Verification
            uint16_t calc_crc = 0xFFFF;
            for (uint8_t i = 0; i < BR32_FRAME_SIZE - 2; i++) {
                calc_crc = _crc_xmodem_update(calc_crc, rx_buf[i]);
            }
            uint16_t rx_crc = ((uint16_t)rx_buf[30] << 8) | rx_buf[31];
            if (calc_crc != rx_crc) {
                for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) tx_buf[i] = 0;
                send_response(STATUS_ERR_CRC, rx_buf[1], BR32_FRAME_SIZE);
                continue;
            }

            // Principle 5: Standard Command Processing
            uint8_t cmd = rx_buf[0];
            uint8_t page_idx = rx_buf[12];
            uint8_t chunk_flags = rx_buf[13];
            uint8_t chunk_idx = chunk_flags & CHUNK_IDX_MASK;
            bool do_commit = false;

            for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) tx_buf[i] = 0;
            tx_buf[3] = 0x01 | (chunk_idx << 4); // DEV_STATE (READY | chunk_idx)

            if (cmd == CMD_WRITE_CHUNK) {
                if (page_idx < APP_START_PAGE || page_idx >= FLASH_TOTAL_PAGES) {
                    tx_buf[0] = STATUS_ERR_PROTECT;
                } else {
                    memcpy(&page_buffer[chunk_idx * FLASH_CHUNK_SIZE], &rx_buf[14], FLASH_CHUNK_SIZE);
                    memcpy(&tx_buf[14], &page_buffer[chunk_idx * FLASH_CHUNK_SIZE], FLASH_CHUNK_SIZE);
                    if (chunk_flags & FLAG_COMMIT_PAGE) {
                        do_commit = true;
                    }
                }
            } else if (cmd == CMD_READ_CHUNK) {
                if (page_idx >= FLASH_TOTAL_PAGES) {
                    tx_buf[0] = STATUS_ERR_PROTECT;
                } else {
                    uint16_t flash_offset = ((uint16_t)page_idx * FLASH_PAGE_SIZE) + ((uint16_t)chunk_idx * FLASH_CHUNK_SIZE);
                    memcpy(&tx_buf[14], (const void *)(MAPPED_PROGMEM_START + flash_offset), FLASH_CHUNK_SIZE);
                }
            }

            // Immediate RS-485 Response Transmission
            send_response(tx_buf[0], rx_buf[1], BR32_FRAME_SIZE);

            // Asynchronous Response Isolation: Commit to Flash AFTER ACK transmission
            if (do_commit) {
                nvm_commit_page_hw((uint16_t)page_idx * FLASH_PAGE_SIZE);
            }
        }

        // Heartbeat LED
        if (++idle_counter == 0) {
            VPORTB.IN |= PIN2_bm;
        }
    }
}
