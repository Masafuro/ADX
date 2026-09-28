/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-5-1 Diagnostic Probe Firmware (Transparent Skeleton Sniffer)
 * Target MCU: Microchip ATtiny1616-MNR on ADX Core-D
 *
 * Purpose:
 *   Capture raw serial bytes received after LIN Break + Sync,
 *   expose exact byte counts and raw buffer contents via BOTH:
 *   1. RS-485 32-Byte Diagnostic Response Frame (to Browser WebSerial)
 *   2. Soft-UART Telemetry @ 9,600 bps on PB4 (to COM21)
 *
 * Channel 1: RS-485 (USART0 Alternate Pins) @ 19,200 bps
 *   - PA1: TXD (PORTMUX USART0 Alternate)
 *   - PA2: RXD
 *   - PA4: SP485EEN DE  (Active HIGH)
 *   - PA7: SP485EEN /RE (Active LOW)
 *
 * Channel 2: PC Debug Monitor (Soft-UART) @ 9,600 bps
 *   - PB4: TXD (Connected to CH342K Port B / COM21)
 *
 * Visual Indicator:
 *   - PB2: Red LED (Toggles on frame capture & idle heartbeat)
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

// =========================================================================
// Channel 2: Soft-UART Debug Telemetry (PB4 TX @ 9,600 bps, 3.33 MHz)
// =========================================================================
static inline void dbg_delay_bit(void) {
    _delay_loop_2(86); // ~344 cycles @ 3.33 MHz
}

static void dbg_putch(char c) {
    uint8_t sreg = SREG;
    cli();

    // Start bit (LOW)
    VPORTB.OUT &= ~PIN4_bm;
    dbg_delay_bit();

    // 8 Data bits (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        if (c & 0x01) {
            VPORTB.OUT |= PIN4_bm;
        } else {
            VPORTB.OUT &= ~PIN4_bm;
        }
        dbg_delay_bit();
        c >>= 1;
    }

    // Stop bit (HIGH)
    VPORTB.OUT |= PIN4_bm;
    dbg_delay_bit();
    dbg_delay_bit();

    SREG = sreg;
}

static void dbg_print(const char *s) {
    while (*s) {
        dbg_putch(*s++);
    }
}

static void dbg_print_hex8(uint8_t val) {
    static const char hex[] = "0123456789ABCDEF";
    dbg_putch(hex[(val >> 4) & 0x0F]);
    dbg_putch(hex[val & 0x0F]);
}

static void dbg_print_dec16(uint16_t val) {
    char buf[6];
    char *p = &buf[5];
    *p = '\0';
    if (val == 0) {
        *--p = '0';
    } else {
        while (val > 0) {
            *--p = '0' + (val % 10);
            val /= 10;
        }
    }
    dbg_print(p);
}

// =========================================================================
// Channel 1: RS-485 Low-Level Byte Output
// =========================================================================
static inline void putch(uint8_t ch) {
    while (!(USART0.STATUS & USART_DREIF_bm))
        ;
    USART0.TXDATAL = ch;
}

// Static Buffers
static uint8_t rx_buf[64];
static uint8_t tx_buf[BR32_FRAME_SIZE];
static uint8_t my_uid[10];

int main(void) {
    // 1. Pin Configuration (PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB4=SoftTX, PB2=LED)
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // Receiver enabled, Transmitter disabled
    VPORTA.DIR &= ~PIN2_bm;

    VPORTB.DIR |= PIN4_bm | PIN2_bm;
    VPORTB.OUT |= PIN4_bm;  // Soft-UART Idle HIGH
    VPORTB.OUT &= ~PIN2_bm; // LED OFF

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

    // 5. Initial Greeting on Soft-UART (COM21)
    dbg_print("\r\n==================================================\r\n");
    dbg_print(" ADX Core-D WU5-1 Transparent Diagnostic Probe\r\n");
    dbg_print(" UID: ");
    for (uint8_t i = 0; i < 10; i++) {
        dbg_print_hex8(my_uid[i]);
        dbg_putch(' ');
    }
    dbg_print("\r\n Listening on RS-485 @ 19,200 bps (LINAUTO mode)...\r\n");
    dbg_print("==================================================\r\n");

    // 6. Arm Break Detection
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    uint16_t diag_seq = 0;
    uint16_t idle_counter = 0;

    for (;;) {
        // Guard 1: Reset on Inconsistent Sync Field
        if (USART0.STATUS & USART_ISFIF_bm) {
            dbg_print("[WARN] ISFIF Triggered - Resetting WFB\r\n");
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // Guard 2: Break + Sync Received -> Capture incoming stream
        if (USART0.STATUS & USART_RXCIF_bm) {
            uint8_t initial_status = USART0.STATUS;
            (void)USART0.RXDATAH;
            rx_buf[0] = USART0.RXDATAL;
            uint8_t rx_count = 1;

            // Greedily capture all incoming bytes until bus goes idle (~18ms timeout)
            while (rx_count < sizeof(rx_buf)) {
                uint16_t to_timer = 0;
                while (!(USART0.STATUS & USART_RXCIF_bm)) {
                    if (++to_timer > 15000) {
                        goto rx_stream_done;
                    }
                }
                (void)USART0.RXDATAH;
                rx_buf[rx_count++] = USART0.RXDATAL;
            }

        rx_stream_done:
            diag_seq++;

            // =============================================================
            // A. Format RS-485 Diagnostic Response Frame (32 Bytes)
            // =============================================================
            // [0] STATUS = 0xAA (DIAGNOSTIC_FRAME_MARKER)
            // [1] RX_COUNT (exact number of bytes captured, e.g. 31, 32, 33)
            // [2] FIRST_BYTE (rx_buf[0])
            // [3] SECOND_BYTE (rx_buf[1])
            // [4..13] SLAVE_UID (10 Bytes)
            // [14..29] RAW_DUMP (First 16 bytes captured in rx_buf)
            // [30..31] CRC-16-CCITT
            memset(tx_buf, 0, BR32_FRAME_SIZE);
            tx_buf[0] = 0xAA;
            tx_buf[1] = rx_count;
            tx_buf[2] = (rx_count > 0) ? rx_buf[0] : 0xEE;
            tx_buf[3] = (rx_count > 1) ? rx_buf[1] : 0xEE;
            for (uint8_t i = 0; i < 10; i++) {
                tx_buf[4 + i] = my_uid[i];
            }
            uint8_t dump_len = (rx_count < 16) ? rx_count : 16;
            for (uint8_t i = 0; i < dump_len; i++) {
                tx_buf[14 + i] = rx_buf[i];
            }

            uint16_t crc = 0xFFFF;
            for (uint8_t i = 0; i < BR32_FRAME_SIZE - 2; i++) {
                crc = _crc_xmodem_update(crc, tx_buf[i]);
            }
            tx_buf[30] = (uint8_t)(crc >> 8);
            tx_buf[31] = (uint8_t)(crc & 0xFF);

            // Transmit RS-485 Response
            VPORTA.OUT |= (PIN4_bm | PIN7_bm);
            _delay_loop_2(42);
            for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
                putch(tx_buf[i]);
            }
            while (!(USART0.STATUS & USART_TXCIF_bm))
                ;
            USART0.STATUS = USART_TXCIF_bm;
            _delay_loop_2(42);
            VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // Release bus

            // Activity LED Toggle
            VPORTB.IN |= PIN2_bm;

            // =============================================================
            // B. Transmit Detailed Telemetry to Soft-UART (COM21)
            // =============================================================
            dbg_print("\r\n[DIAG #");
            dbg_print_dec16(diag_seq);
            dbg_print("] RX_COUNT=");
            dbg_print_dec16(rx_count);
            dbg_print(" bytes | InitStatus=0x");
            dbg_print_hex8(initial_status);
            dbg_print(" | BAUD=0x");
            dbg_print_hex8((uint8_t)(USART0.BAUD >> 8));
            dbg_print_hex8((uint8_t)(USART0.BAUD & 0xFF));
            dbg_print("\r\n  Raw Stream Dump (first ");
            dbg_print_dec16(rx_count);
            dbg_print(" bytes):\r\n  ");
            for (uint8_t i = 0; i < rx_count; i++) {
                dbg_print_hex8(rx_buf[i]);
                dbg_putch(' ');
                if ((i + 1) % 16 == 0 && (i + 1) < rx_count) {
                    dbg_print("\r\n  ");
                }
            }
            dbg_print("\r\n");

            // Flush any loopback bytes & re-arm LINAUTO
            while (USART0.STATUS & USART_RXCIF_bm) {
                (void)USART0.RXDATAL;
            }
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // Heartbeat LED
        if (++idle_counter == 0) {
            VPORTB.IN |= PIN2_bm;
        }
    }
}
