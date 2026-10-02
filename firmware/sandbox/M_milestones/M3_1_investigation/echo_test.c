/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * M3-1 Pure RS-485 Physical Echo Diagnostic Sketch
 * Target MCU: Microchip ATtiny1616-MNR on ADX Core-D
 *
 * Description:
 *   Zero-protocol, zero-Flash, pure physical RS-485 mirror echo.
 *   Receives 32 bytes from USART0 (RS-485), and transmits the EXACT SAME 32 bytes back.
 *   Used to isolate whether framing/offset/phantom bytes originate in the physical
 *   transceiver/UART layer or in the upper MR32 protocol layer.
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define ECHO_FRAME_LEN 32

static inline void rs485_tx_start(void) {
    VPORTA.OUT |= (PIN4_bm | PIN7_bm); // DE=1, /RE=1 (Transmit Mode)
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
    _delay_us(160); // Guard time
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // DE=0, /RE=0 (Receive Mode)
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL; // Flush self-echo if any
    }
}

int main(void) {
    // 1. Clock: 20MHz internal
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

    // 2. Pins: PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=Red LED, PB3=White LED
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;              // TX IDLE HIGH (Mark)
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // Initial Receive Mode
    VPORTA.DIR &= ~PIN2_bm;             // RX as input

    VPORTB.DIR |= PIN2_bm | PIN3_bm;
    VPORTB.OUT &= ~(PIN2_bm | PIN3_bm);

    // 3. USART0 Alternate Pins (PA1=TX, PA2=RX)
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 4. USART0 115,200 bps (8N1)
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = 555;
    } else {
        USART0.BAUD = 694;
    }
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc | USART_SBMODE_1BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_NORMAL_gc | USART_RXEN_bm | USART_TXEN_bm;

    uint8_t buf[ECHO_FRAME_LEN];
    uint8_t idx = 0;

    // Blink White LED once on power-on to confirm fresh startup
    VPORTB.OUT |= PIN3_bm;
    _delay_ms(100);
    VPORTB.OUT &= ~PIN3_bm;

    while (1) {
        if (USART0.STATUS & USART_RXCIF_bm) {
            buf[idx++] = USART0.RXDATAL;

            if (idx >= ECHO_FRAME_LEN) {
                // 32 Bytes received! Echo back EXACT same 32 bytes!
                VPORTB.OUT ^= PIN2_bm; // Toggle Red LED on echo

                rs485_tx_start();
                for (uint8_t i = 0; i < ECHO_FRAME_LEN; i++) {
                    putch(buf[i]);
                }
                rs485_tx_end();

                idx = 0; // Reset index for next frame
            }
        }
    }
}
