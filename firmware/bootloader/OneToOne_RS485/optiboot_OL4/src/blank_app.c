/*
 * Minimal Blank Test Application for ADX Core-D (ATtiny1616)
 * Placed at application start address 0x0400 (BOOTEND = 0x04).
 */
#include <avr/io.h>

void _start(void) __attribute__((naked)) __attribute__((section(".text")));

void _start(void) {
    // Ensure PB2 (Red LED) is kept off in the application
    PORTB.DIRSET = PIN2_bm;
    PORTB.OUTCLR = PIN2_bm;

    // Infinite loop representing a blank idle application
    while (1) {
        __asm__ __volatile__("nop\n");
    }
}
