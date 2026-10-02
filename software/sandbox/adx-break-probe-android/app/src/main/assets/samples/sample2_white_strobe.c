/*
 * Sample 2: White LED High-Speed Strobe Flasher
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

int main(void) {
    cli();
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

    // PB2: Red LED, PB3: White LED, PB4: Soft-UART
    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT |= PIN2_bm;  // Red LED SOLID ON
    VPORTB.OUT |= PIN4_bm;  // TX Idle HIGH

    _delay_ms(50);
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ⚡ ADX SAMPLE 2: WHITE STROBE ACTIVE! ⚡\r\n");
    dbg_print("   Flash Address: 0x1000 | Red LED SOLID ON\r\n");
    dbg_print("=======================================================\r\n");

    uint16_t strobe_count = 0;

    while (1) {
        // High-speed white flash
        VPORTB.OUT |= PIN3_bm;
        _delay_ms(30);
        VPORTB.OUT &= ~PIN3_bm;
        _delay_ms(170);

        strobe_count++;
        if (strobe_count % 10 == 0) {
            dbg_print("[STROBE] Pulse count: 10 pulses completed.\r\n");
        }
    }

    return 0;
}
