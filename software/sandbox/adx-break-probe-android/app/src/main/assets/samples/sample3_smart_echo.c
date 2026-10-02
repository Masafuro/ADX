/*
 * Sample 3: Smart Synchronized Breathing LED with Auto-Reboot to Bootloader
 * Target: ATtiny1616 on ADX Core-D @ 0x1000
 * SPDX-License-Identifier: MIT
 *
 * Feature:
 *   - Red (PB2) and White (PB3) LEDs breathe together (500ms).
 *   - Listens on RS-485 (USART0 @ 115,200 bps).
 *   - Automatically executes software reset to Bootloader when receiving MR32 Ping (0x10) or BMC Boot (0x01)!
 *   - Eliminates need for manual USB unplugging!
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdbool.h>

#define MR32_SYNC_BYTE   0x55
#define MR32_MAGIC_BYTE  0xAD
#define CMD_PING         0x10
#define CMD_BOOT         0x01

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

    // Pin configuration
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // RS-485 RX mode (DE=0, /RE=0)
    VPORTA.DIR &= ~PIN2_bm;

    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT &= ~(PIN2_bm | PIN3_bm);
    VPORTB.OUT |= PIN4_bm;

    // USART0 Alternate pins (PA1/PA2) @ 115,200 bps
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = 555;
    } else {
        USART0.BAUD = 694;
    }
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc | USART_SBMODE_1BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_NORMAL_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 起動直後の残余バッファを完全にフラッシュ
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }

    _delay_ms(100);
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   🌟 ADX SAMPLE 3: SMART AUTO-REBOOT ACTIVE! 🌟\r\n");
    dbg_print("   Both LEDs Breathe (500ms) | Listening on RS-485\r\n");
    dbg_print("   [MAGIC] Send Ping/Connect from PWA to Reboot!\r\n");
    dbg_print("=======================================================\r\n");

    // 起動後 1 秒間はジャンプ直後の余韻パケットによる誤リブートを防止するガード時間
    _delay_ms(500);
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }

    uint8_t rx_buf[6];
    uint8_t rx_idx = 0;
    uint32_t loop_count = 0;
    bool led_state = false;

    while (1) {
        // Check RS-485 for Reboot command
        if (USART0.STATUS & USART_RXCIF_bm) {
            uint8_t b = USART0.RXDATAL;

            if (rx_idx == 0) {
                if (b == MR32_SYNC_BYTE) {
                    rx_buf[0] = b;
                    rx_idx = 1;
                }
            } else if (rx_idx == 1) {
                if (b == MR32_MAGIC_BYTE) {
                    rx_buf[1] = b;
                    rx_idx = 2;
                } else {
                    rx_idx = 0;
                }
            } else {
                rx_buf[rx_idx++] = b;
                // Byte 2: DST, Byte 3: SRC, Byte 4: CMD
                if (rx_idx >= 5) {
                    uint8_t cmd = rx_buf[4];
                    // 明示的に Ping (0x10) または BMC Boot (0x01) を受領した時だけリブート！
                    if (cmd == CMD_PING || cmd == CMD_BOOT) {
                        dbg_print("\r\n[SMART-APP] Valid MR32 Ping/Boot Received! Resetting to Bootloader...\r\n");
                        _delay_ms(20);
                        _PROTECTED_WRITE(RSTCTRL.SWRR, 1); // Software Reset!
                        while (1) ;
                    }
                    rx_idx = 0;
                }
            }
        }

        _delay_ms(1);
        loop_count++;

        if (loop_count >= 500) {
            loop_count = 0;
            led_state = !led_state;
            if (led_state) {
                VPORTB.OUT |= PIN2_bm | PIN3_bm; // Both ON
            } else {
                VPORTB.OUT &= ~(PIN2_bm | PIN3_bm); // Both OFF
            }
        }
    }

    return 0;
}
