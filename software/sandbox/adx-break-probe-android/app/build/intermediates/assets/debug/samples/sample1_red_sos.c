/*
 * Sample 1: Red LED Morse SOS Flasher
 * Target: ATtiny1616 on ADX Core-D @ 0x1000
 * SPDX-License-Identifier: MIT
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>

static inline void dbg_delay_bit(void) {
    _delay_loop_2(520);
}

static void dbg_putch(char c) {
    uint8_t sreg = SREG;
    cli();
    VPORTB.OUT &= ~PIN4_bm;
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
    VPORTB.OUT |= PIN4_bm;
    dbg_delay_bit();
    dbg_delay_bit();
    SREG = sreg;
}

static void dbg_print(const char *s) {
    while (*s) dbg_putch(*s++);
}

static void red_dot(void) {
    VPORTB.OUT |= PIN2_bm;
    _delay_ms(120);
    VPORTB.OUT &= ~PIN2_bm;
    _delay_ms(120);
}

static void red_dash(void) {
    VPORTB.OUT |= PIN2_bm;
    _delay_ms(360);
    VPORTB.OUT &= ~PIN2_bm;
    _delay_ms(120);
}

int main(void) {
    cli();
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

    // PB2: Red LED, PB3: White LED, PB4: Soft-UART
    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT &= ~PIN3_bm; // White LED OFF
    VPORTB.OUT |= PIN4_bm;  // TX Idle HIGH

    _delay_ms(50);
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   🚨 ADX SAMPLE 1: RED LED MORSE SOS ACTIVE! 🚨\r\n");
    dbg_print("   Flash Address: 0x1000 | White LED OFF\r\n");
    dbg_print("=======================================================\r\n");

    while (1) {
        dbg_print("[SOS] Sending Morse SOS (... --- ...)\r\n");

        // ... (S)
        red_dot(); red_dot(); red_dot();
        _delay_ms(240);

        // --- (O)
        red_dash(); red_dash(); red_dash();
        _delay_ms(240);

        // ... (S)
        red_dot(); red_dot(); red_dot();
        _delay_ms(1200);
    }

    return 0;
}
