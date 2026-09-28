/*
 * Milestone 5 RS-485 Serial Monitor & Echo Test Application
 * Target: ADX Core-D (ATtiny1616)
 * Flash Address: 0x0200 (512-byte offset, following Optiboot bootloader)
 *
 * Hardware Map:
 *   - PB3: White LED (Output, Toggles every 1s)
 *   - PB2: Red LED (Output, Held OFF)
 *   - PA1: USART0 TX (Output to RS-485 DI)
 *   - PA2: USART0 RX (Input from RS-485 RO)
 *   - PA7: RS-485 RE/DE (Output, HIGH=TX, LOW=RX)
 *
 * Behavior:
 *   - 115200 bps 8N1 communication
 *   - Transmits heartbeat text every 1000ms
 *   - Echoes back any received character sent from Web Serial Monitor!
 */
#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 3333333UL // 20MHz internal oscillator with /6 prescaler
#endif

#define BAUD_RATE 115200UL
#define USART_BAUD_VALUE ((uint16_t)((4UL * F_CPU) / BAUD_RATE))

static void usart_init(void) {
    // PA1 (TX) as Output
    PORTA.DIRSET = PIN1_bm;
    // PA7 (RS-485 RE/DE) as Output, Default LOW (Receive mode)
    PORTA.DIRSET = PIN7_bm;
    PORTA.OUTCLR = PIN7_bm;

    USART0.BAUD = USART_BAUD_VALUE;
    USART0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;
}

static void usart_putc(char c) {
    PORTA.OUTSET = PIN7_bm; // RE/DE = HIGH (TX)
    _delay_us(8);

    while (!(USART0.STATUS & USART_DREIF_bm));
    USART0.TXDATAL = c;

    while (!(USART0.STATUS & USART_TXCIF_bm));
    USART0.STATUS |= USART_TXCIF_bm;

    _delay_us(8);
    PORTA.OUTCLR = PIN7_bm; // RE/DE = LOW (RX)
}

static void usart_puts(const char *str) {
    PORTA.OUTSET = PIN7_bm; // RE/DE = HIGH (TX)
    _delay_us(8);

    while (*str) {
        while (!(USART0.STATUS & USART_DREIF_bm));
        USART0.TXDATAL = *str++;
    }

    while (!(USART0.STATUS & USART_TXCIF_bm));
    USART0.STATUS |= USART_TXCIF_bm;

    _delay_us(8);
    PORTA.OUTCLR = PIN7_bm; // RE/DE = LOW (RX)
}

void _start(void) __attribute__((naked)) __attribute__((section(".text")));

void _start(void) {
    // Configure LEDs: PB3 (White), PB2 (Red)
    PORTB.DIRSET = PIN3_bm | PIN2_bm;
    PORTB.OUTCLR = PIN2_bm;
    PORTB.OUTSET = PIN3_bm;

    usart_init();

    usart_puts("\r\n=========================================\r\n");
    usart_puts("  ADX Core-D RS-485 Serial Monitor Ready!\r\n");
    usart_puts("  Baudrate: 115200 bps | LED: PB3 (White)\r\n");
    usart_puts("=========================================\r\n");

    uint16_t count = 0;

    while (1) {
        PORTB.OUTTGL = PIN3_bm;

        usart_puts("[ADX Core-D] Heartbeat packet #");

        uint16_t n = ++count;
        char numbuf[8];
        int i = 0;
        if (n == 0) {
            numbuf[i++] = '0';
        } else {
            char temp[8];
            int ti = 0;
            while (n > 0) {
                temp[ti++] = '0' + (n % 10);
                n /= 10;
            }
            while (ti > 0) numbuf[i++] = temp[--ti];
        }
        numbuf[i] = '\0';
        usart_puts(numbuf);

        if (PORTB.IN & PIN3_bm) {
            usart_puts(" | White LED: ON\r\n");
        } else {
            usart_puts(" | White LED: OFF\r\n");
        }

        // Check for incoming character across ~1000ms delay in 50ms slices
        for (int slice = 0; slice < 20; slice++) {
            if (USART0.STATUS & USART_RXCIF_bm) {
                char rx = USART0.RXDATAL;
                usart_puts(">>> Echo received: '");
                usart_putc(rx);
                usart_puts("'\r\n");
            }
            _delay_ms(50);
        }
    }
}
