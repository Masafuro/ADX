/*
 * Milestone 5 Test Application: Alternate Blink Red (PB2) and White (PB3) LEDs
 * Target: ADX Core-D (ATtiny1616)
 * Flash Address: 0x0200 (512-byte offset, following Optiboot bootloader)
 *
 * Behavior:
 *   - PB2 (Red LED) and PB3 (White LED) alternate blinking every 200ms
 *   - Provides unmistakable visual confirmation that flash contents were changed!
 */
#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 3333333UL // 20MHz internal oscillator with /6 prescaler
#endif

void _start(void) __attribute__((naked)) __attribute__((section(".text")));

void _start(void) {
    // Configure PB2 (Red LED) and PB3 (White LED) as outputs
    PORTB.DIRSET = PIN2_bm | PIN3_bm;

    // Infinite alternate blink loop
    while (1) {
        // Red ON, White OFF
        PORTB.OUTSET = PIN2_bm;
        PORTB.OUTCLR = PIN3_bm;
        _delay_ms(200);

        // Red OFF, White ON
        PORTB.OUTCLR = PIN2_bm;
        PORTB.OUTSET = PIN3_bm;
        _delay_ms(200);
    }
}
