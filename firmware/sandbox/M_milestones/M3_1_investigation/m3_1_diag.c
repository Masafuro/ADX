/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * Milestone 3-1 (M3-1): Advanced Post-Write Hardware State & Diagnostic Firmware
 * Target MCU: Microchip ATtiny1616-MNR on ADX Core-D
 *
 * Purpose:
 *   Investigate post-Flash-programming hardware behavior, NVMCTRL status,
 *   reset triggers (RSTFR), and provide full telemetry diagnostics to RS-485.
 *
 * Hardware Mapping:
 *   - Channel 1: RS-485 (USART0 Alternate Pins) @ 115,200 bps
 *       PA1: TXD, PA2: RXD, PA4: DE, PA7: /RE
 *   - Indicators:
 *       PB2: Red LED, PB3: White LED
 *   - Channel 2: Soft-UART PB4 (9600 bps optional mirror)
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "../../M_firmware/inc/mr32.h"

#define MR32_MY_NODE_ID       0x01
#define FLASH_PAGE_SIZE       64
#define CHUNK_DATA_SIZE       16
#define CHUNKS_PER_PAGE       4
#define APP_START_PAGE        16   // Page 16 = 0x0400 (Protection Boundary)
#define FLASH_TOTAL_PAGES     256

// Diagnostic Commands
#define CMD_BOOT_READ_CHUNK   0x12
#define CMD_DIAG_SYS_STATUS   0x20
#define CMD_DIAG_SOFT_RESET   0x22

// System Diagnostic State Tracking
static uint8_t s_boot_rstfr = 0;       // Reset Flag register at startup
static uint16_t s_write_count = 0;     // Number of physical Flash writes performed
static uint16_t s_last_written_page = 0xFFFF;
static uint16_t s_last_flash_crc = 0x0000;
static uint8_t s_last_nvmctrl_status = 0x00;

// Soft-UART for PB4 (CH342K Port B @ 9600 bps)
static inline void dbg_delay_bit(void) {
    _delay_loop_2(520); // 20MHz / 9600bps ≈ 2083 cycles
}

static void dbg_putch(char c) {
    uint8_t sreg = SREG;
    cli();
    VPORTB.OUT &= ~PIN4_bm; // Start bit
    dbg_delay_bit();
    for (uint8_t i = 0; i < 8; i++) {
        if (c & 0x01) {
            VPORTB.OUT |= PIN4_bm;
        } else {
            VPORTB.OUT &= ~PIN4_bm;
        }
        dbg_delay_bit();
        c >>= 1;
    }
    VPORTB.OUT |= PIN4_bm; // Stop bit
    dbg_delay_bit();
    dbg_delay_bit();
    SREG = sreg;
}

static void dbg_print(const char *str) {
    while (*str) {
        dbg_putch(*str++);
    }
}

// RS-485 Transceiver Controls
static inline void rs485_tx_start(void) {
    VPORTA.OUT |= (PIN4_bm | PIN7_bm);
    _delay_us(5);
}

static inline void putch(uint8_t ch) {
    while (!(USART0.STATUS & USART_DREIF_bm))
        ;
    USART0.TXDATAL = ch;
}

static inline void rs485_tx_end(void) {
    while (!(USART0.STATUS & USART_TXCIF_bm))
        ;
    USART0.STATUS = USART_TXCIF_bm;
    _delay_us(160); // T_guard (>= 150us)
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }
}

// Flash Memory Buffer & Drivers
static uint8_t sram_page_buffer[FLASH_PAGE_SIZE];

// Direct read from physical Flash mapped memory (0x4000 + offset)
static void flash_read_chunk(uint16_t page_idx, uint8_t chunk_idx, uint8_t *out_buf) {
    uint16_t offset = (page_idx << 6) + (chunk_idx << 4);
    const uint8_t *src = (const uint8_t *)(MAPPED_PROGMEM_START + offset);
    for (uint8_t i = 0; i < CHUNK_DATA_SIZE; i++) {
        out_buf[i] = *(src++);
    }
}

static uint16_t flash_calculate_page_crc(uint16_t page_idx) {
    uint16_t offset = page_idx << 6;
    const uint8_t *src = (const uint8_t *)(MAPPED_PROGMEM_START + offset);
    return mr32_calculate_crc(src, FLASH_PAGE_SIZE);
}

// Physical Flash Page Erase & Write with explicit NVMCTRL cleanups
__attribute__((noinline))
static void nvm_commit_page_hw(uint16_t page_idx) {
    uint16_t page_addr = page_idx << 6;
    uint8_t *dest = (uint8_t *)(MAPPED_PROGMEM_START + page_addr);

    // 1. Load 64 bytes into hardware page buffer
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        *(dest++) = sram_page_buffer[i];
    }

    // 2. Trigger hardware page erase & write command
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);

    // 3. Wait until hardware completion (approx 2.5ms)
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
        ;

    // 4. Capture NVM status and restore NVMCTRL to NONE state
    s_last_nvmctrl_status = NVMCTRL.STATUS;
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

    s_write_count++;
    s_last_written_page = page_idx;
}

typedef enum {
    STATE_WAIT_SYNC,
    STATE_WAIT_MAGIC,
    STATE_RECV_BODY
} rx_state_t;

int main(void) {
    // 0. Capture Reset Flag register immediately at startup
    s_boot_rstfr = RSTCTRL.RSTFR;
    RSTCTRL.RSTFR = s_boot_rstfr; // Clear flags by writing 1

    // 1. Clock: 20MHz internal
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

    // 2. Pin Directions
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR &= ~PIN2_bm;

    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT &= ~(PIN2_bm | PIN3_bm);
    VPORTB.OUT |= PIN4_bm; // Soft-UART IDLE HIGH

    // 3. USART0 Alternate Pins
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 4. USART0 115,200 bps
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = 555;
    } else {
        USART0.BAUD = 694;
    }
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc | USART_SBMODE_1BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_NORMAL_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 5. Diagnostics Banner
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D M3-1: Hardware Diag & Telemetry Suite\r\n");
    dbg_print("=======================================================\r\n");

    uint16_t current_page = 0xFFFF;
    uint8_t chunk_mask = 0;

    uint8_t rx_buf[MR32_FRAME_LEN];
    uint8_t tx_buf[MR32_FRAME_LEN];
    uint8_t rx_idx = 0;
    bool ignore_rest = false;
    rx_state_t state = STATE_WAIT_SYNC;

    while (1) {
        if (USART0.STATUS & USART_RXCIF_bm) {
            uint8_t b = USART0.RXDATAL;

            switch (state) {
                case STATE_WAIT_SYNC:
                    if (b == MR32_SYNC_BYTE) {
                        rx_buf[0] = b;
                        rx_idx = 1;
                        state = STATE_WAIT_MAGIC;
                    }
                    break;

                case STATE_WAIT_MAGIC:
                    if (b == MR32_MAGIC_BYTE) {
                        rx_buf[1] = b;
                        rx_idx = 2;
                        ignore_rest = false;
                        state = STATE_RECV_BODY;
                    } else {
                        state = STATE_WAIT_SYNC;
                    }
                    break;

                case STATE_RECV_BODY:
                    if (rx_idx == 2) {
                        if (b != MR32_MY_NODE_ID && b != MR32_ID_BROADCAST) {
                            ignore_rest = true;
                        }
                    }

                    if (!ignore_rest) {
                        rx_buf[rx_idx] = b;
                    }
                    rx_idx++;

                    if (rx_idx >= MR32_FRAME_LEN) {
                        if (!ignore_rest) {
                            uint16_t calc_crc = mr32_calculate_crc(&rx_buf[2], 28);
                            uint16_t recv_crc = (uint16_t)rx_buf[30] | ((uint16_t)rx_buf[31] << 8);

                            if (calc_crc == recv_crc) {
                                uint8_t cmd = rx_buf[4];
                                uint8_t src = rx_buf[3];
                                uint8_t seq = rx_buf[5];
                                uint8_t *payload = &rx_buf[6];

                                if (cmd == MR32_CMD_BOOT_PING) {
                                    // ----------------------------------------------------
                                    // PING: Basic Liveness & Target ID
                                    // ----------------------------------------------------
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_PING;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = 0x16; // MCU Type High
                                    tx_buf[8] = 0x16; // MCU Type Low (ATtiny1616)
                                    tx_buf[9] = 16;   // Flash KB
                                    tx_buf[10] = 64;  // Page Size
                                    for (uint8_t i = 11; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                } else if (cmd == MR32_CMD_BOOT_WRITE_CHUNK) {
                                    // ----------------------------------------------------
                                    // WRITE_CHUNK: 16B Buffer Accumulate & Commit
                                    // ----------------------------------------------------
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint8_t chunk_idx = payload[2];

                                    if (req_page < APP_START_PAGE || req_page >= FLASH_TOTAL_PAGES) {
                                        // Self-Programming Protection Guard
                                        tx_buf[0] = MR32_SYNC_BYTE;
                                        tx_buf[1] = MR32_MAGIC_BYTE;
                                        tx_buf[2] = src;
                                        tx_buf[3] = MR32_MY_NODE_ID;
                                        tx_buf[4] = MR32_CMD_BOOT_WRITE_CHUNK;
                                        tx_buf[5] = seq;

                                        tx_buf[6] = MR32_STATUS_ERR_PARAM; // 0x03
                                        tx_buf[7] = (uint8_t)(req_page & 0xFF);
                                        tx_buf[8] = (uint8_t)(req_page >> 8);
                                        tx_buf[9] = chunk_idx;
                                        for (uint8_t i = 10; i < 30; i++) {
                                            tx_buf[i] = 0x00;
                                        }

                                        uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                        tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                        tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                        rs485_tx_start();
                                        for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                            putch(tx_buf[i]);
                                        }
                                        rs485_tx_end();

                                    } else {
                                        if (req_page != current_page) {
                                            current_page = req_page;
                                            chunk_mask = 0;
                                            memset(sram_page_buffer, 0xFF, FLASH_PAGE_SIZE);
                                        }

                                        if (chunk_idx < CHUNKS_PER_PAGE) {
                                            uint8_t offset = (chunk_idx << 4);
                                            memcpy(&sram_page_buffer[offset], &payload[3], CHUNK_DATA_SIZE);
                                            chunk_mask |= (1 << chunk_idx);
                                            VPORTB.OUT ^= PIN3_bm; // White LED
                                        }

                                        uint8_t status = MR32_STATUS_OK;
                                        uint16_t flash_crc = 0;

                                        if (chunk_mask == 0x0F) {
                                            nvm_commit_page_hw(current_page);
                                            flash_crc = flash_calculate_page_crc(current_page);
                                            s_last_flash_crc = flash_crc;
                                            status = MR32_STATUS_PAGE_DONE;
                                            VPORTB.OUT ^= PIN2_bm; // Red LED
                                        }

                                        tx_buf[0] = MR32_SYNC_BYTE;
                                        tx_buf[1] = MR32_MAGIC_BYTE;
                                        tx_buf[2] = src;
                                        tx_buf[3] = MR32_MY_NODE_ID;
                                        tx_buf[4] = MR32_CMD_BOOT_WRITE_CHUNK;
                                        tx_buf[5] = seq;

                                        tx_buf[6] = status;
                                        tx_buf[7] = (uint8_t)(current_page & 0xFF);
                                        tx_buf[8] = (uint8_t)(current_page >> 8);
                                        tx_buf[9] = chunk_idx;
                                        tx_buf[10] = chunk_mask;
                                        tx_buf[11] = (uint8_t)(flash_crc & 0xFF);
                                        tx_buf[12] = (uint8_t)(flash_crc >> 8);
                                        for (uint8_t i = 13; i < 30; i++) {
                                            tx_buf[i] = 0x00;
                                        }

                                        uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                        tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                        tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                        rs485_tx_start();
                                        for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                            putch(tx_buf[i]);
                                        }
                                        rs485_tx_end();
                                    }

                                } else if (cmd == CMD_BOOT_READ_CHUNK) {
                                    // ----------------------------------------------------
                                    // READ_CHUNK: 16B Direct Physical Flash Readback
                                    // ----------------------------------------------------
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint8_t chunk_idx = payload[2];

                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = CMD_BOOT_READ_CHUNK;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = (uint8_t)(req_page & 0xFF);
                                    tx_buf[8] = (uint8_t)(req_page >> 8);
                                    tx_buf[9] = chunk_idx;

                                    if (chunk_idx < CHUNKS_PER_PAGE && req_page < FLASH_TOTAL_PAGES) {
                                        flash_read_chunk(req_page, chunk_idx, &tx_buf[10]);
                                    } else {
                                        memset(&tx_buf[10], 0xFF, CHUNK_DATA_SIZE);
                                    }

                                    uint16_t flash_crc = (req_page < FLASH_TOTAL_PAGES) ? flash_calculate_page_crc(req_page) : 0;
                                    tx_buf[26] = (uint8_t)(flash_crc & 0xFF);
                                    tx_buf[27] = (uint8_t)(flash_crc >> 8);
                                    tx_buf[28] = 0x0F;
                                    tx_buf[29] = 0x00;

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                } else if (cmd == MR32_CMD_BOOT_CRC_CHECK) {
                                    // ----------------------------------------------------
                                    // CRC_CHECK: Autonomous Page CRC Verification
                                    // ----------------------------------------------------
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint16_t flash_crc = (req_page < FLASH_TOTAL_PAGES) ? flash_calculate_page_crc(req_page) : 0xFFFF;

                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_CRC_CHECK;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = (uint8_t)(flash_crc & 0xFF);
                                    tx_buf[8] = (uint8_t)(flash_crc >> 8);
                                    tx_buf[9] = (uint8_t)(req_page & 0xFF);
                                    tx_buf[10] = (uint8_t)(req_page >> 8);
                                    for (uint8_t i = 11; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                } else if (cmd == CMD_DIAG_SYS_STATUS) {
                                    // ----------------------------------------------------
                                    // DIAG_SYS_STATUS (0x20): Hardware Telemetry Register Dump
                                    // ----------------------------------------------------
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = CMD_DIAG_SYS_STATUS;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = s_boot_rstfr;              // Startup RSTCTRL.RSTFR
                                    tx_buf[8] = NVMCTRL.STATUS;            // Current NVMCTRL.STATUS
                                    tx_buf[9] = s_last_nvmctrl_status;     // Post-write NVMCTRL.STATUS
                                    tx_buf[10] = (uint8_t)(s_write_count & 0xFF);
                                    tx_buf[11] = (uint8_t)(s_write_count >> 8);
                                    tx_buf[12] = (uint8_t)(s_last_written_page & 0xFF);
                                    tx_buf[13] = (uint8_t)(s_last_written_page >> 8);
                                    tx_buf[14] = (uint8_t)(s_last_flash_crc & 0xFF);
                                    tx_buf[15] = (uint8_t)(s_last_flash_crc >> 8);
                                    tx_buf[16] = CLKCTRL.MCLKSTATUS;       // Main Clock status
                                    tx_buf[17] = SREG;                     // Status Register
                                    for (uint8_t i = 18; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                } else if (cmd == CMD_DIAG_SOFT_RESET) {
                                    // ----------------------------------------------------
                                    // DIAG_SOFT_RESET (0x22): Request Clean Software Reset
                                    // ----------------------------------------------------
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = CMD_DIAG_SOFT_RESET;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    for (uint8_t i = 7; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                    _delay_ms(10); // Wait for packet to clear the line
                                    _PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRE_bm); // Trigger Soft Reset!
                                }
                            }
                        }
                        state = STATE_WAIT_SYNC;
                    }
                    break;
            }
        }
    }
}
