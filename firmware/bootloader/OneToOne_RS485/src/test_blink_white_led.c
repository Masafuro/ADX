/*
 * Milestone 4 Test Application: Blink White LED (PB3)
 * Target: ADX Core-D (ATtiny1616)
 * Flash Address: 0x0200 (512-byte offset, following Optiboot bootloader)
 *
 * Behavior:
 *   - PB3 (White LED) blinks at ~500ms intervals (1Hz heartbeat)
 *   - PB2 (Red LED) is explicitly held OFF
 */
#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 3333333UL // 20MHz internal oscillator with /6 prescaler
#endif

void _start(void) __attribute__((naked)) __attribute__((section(".text")));

void _start(void) {
    // Configure PB3 (White LED) as output
    PORTB.DIRSET = PIN3_bm;

    // Configure PB2 (Red LED) as output and ensure it is OFF
    PORTB.DIRSET = PIN2_bm;
    PORTB.OUTCLR = PIN2_bm;

    // Infinite heartbeat blink loop
    while (1) {
        PORTB.OUTSET = PIN3_bm; // White LED ON
        _delay_ms(500);
        PORTB.OUTCLR = PIN3_bm; // White LED OFF
        _delay_ms(500);
    }
}
