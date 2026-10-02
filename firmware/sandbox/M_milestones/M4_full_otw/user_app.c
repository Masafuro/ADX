/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * Milestone 4 (M4): Test User Application (Loaded at 0x1000 / Page 64)
 * Target MCU: Microchip ATtiny1616-MNR on ADX Core-D
 *
 * Visual & Telemetry Indicators:
 *   - PB2 (Red LED) & PB3 (White LED): Alternating fast blink (150ms)
 *   - PB4 (Soft-UART TX @ 9600 bps, 20MHz): Periodic telemetry logs
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

// =========================================================================
// Soft-UART Telemetry (PB4 TX @ 9600 bps, 20 MHz)
// =========================================================================
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
    VPORTB.OUT |= PIN4_bm;  // Stop bit
    dbg_delay_bit();
    dbg_delay_bit();
    SREG = sreg;
}

static void dbg_print(const char *s) {
    while (*s) {
        dbg_putch(*s++);
    }
}

static void dbg_print_dec(uint32_t val) {
    char buf[11];
    char *p = &buf[10];
    *p = '\0';
    if (val == 0) {
        *(--p) = '0';
    } else {
        while (val > 0) {
            *(--p) = '0' + (val % 10);
            val /= 10;
        }
    }
    dbg_print(p);
}

// =========================================================================
// Application Main Entry Point (Address: 0x1000)
// =========================================================================
int main(void) {
    // 1. 全割り込み禁止・クロック確認
    cli();
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0); // 20 MHz

    // 2. ピン初期化 (PB2=赤LED, PB3=白LED, PB4=TX)
    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT |= PIN4_bm; // TX Idle HIGH

    // 3. 起動バナー送信 (Soft-UART)
    _delay_ms(50);
    dbg_print("\r\n\r\n");
    dbg_print("===============================================================\r\n");
    dbg_print("   🎉 ADX Core-D USER APPLICATION LAUNCHED SUCCESSFULLY! 🎉\r\n");
    dbg_print("===============================================================\r\n");
    dbg_print(" [ENTRY] Address: 0x1000 (Flash Page 64 / Application Space)\r\n");
    dbg_print(" [INDICATORS] PB2 (Red) & PB3 (White) alternating blink active\r\n");
    dbg_print(" [STATUS] Milestone 4 OTW Auto-Execution Verified!\r\n");
    dbg_print("===============================================================\r\n\r\n");

    uint32_t heartbeat = 0;
    bool phase = false;

    while (1) {
        // LED 交互点滅
        if (phase) {
            VPORTB.OUT |= PIN2_bm;
            VPORTB.OUT &= ~PIN3_bm;
        } else {
            VPORTB.OUT &= ~PIN2_bm;
            VPORTB.OUT |= PIN3_bm;
        }
        phase = !phase;

        _delay_ms(150);

        heartbeat++;
        if (heartbeat % 10 == 0) {
            dbg_print("[APP HEARTBEAT] Alive count: ");
            dbg_print_dec(heartbeat / 10);
            dbg_print("s | Running at 20MHz\r\n");
        }
    }

    return 0;
}
