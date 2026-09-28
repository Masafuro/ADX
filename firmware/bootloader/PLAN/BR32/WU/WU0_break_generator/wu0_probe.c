/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-0: PC RS-485 Break Generation Probe Firmware
 * Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)
 *
 * Hardware Pin Connections:
 *   PA1: USART0 TXD (Alternate pin via PORTMUX)
 *   PA2: USART0 RXD (Alternate pin, LINAUTO Slave)
 *   PA4: SP485EEN DE  (Driver Enable, Active HIGH via VPORTA)
 *   PA7: SP485EEN /RE (Receiver Enable, Active LOW via VPORTA)
 *   PB2: Onboard RED LED (Status indicator, Active HIGH)
 *   PB4: Soft-UART TXD (9,600 bps, 8N1) -> PC COM21 (CH342K Port B)
 */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay_basic.h>

#define BAUD_RATE 19200L
#define BAUD_SETTING_16 (((16000000UL / 6) * 64) / (16L * BAUD_RATE))
#define BAUD_SETTING_20 (((20000000UL / 6) * 64) / (16L * BAUD_RATE))

static uint16_t break_count = 0;
static uint16_t isfif_count = 0;

// =========================================================================
// Channel 2: CH342K Independent Soft-UART TX on PB4 @ 9,600 bps
// F_CPU = 3.333333 MHz (OSC20M / 6).
// 1 bit = 104.167 us = 347.2 cycles.
// _delay_loop_2(n) consumes 4 * n cycles.
// =========================================================================
static void dbg_putch(uint8_t c) {
    // Start bit: LOW
    VPORTB.OUT &= ~PIN4_bm;
    _delay_loop_2(86); // 86 * 4 = 344 cycles (~103.2 us)

    // 8 Data bits (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        if (c & 0x01) {
            VPORTB.OUT |= PIN4_bm;
        } else {
            VPORTB.OUT &= ~PIN4_bm;
        }
        c >>= 1;
        _delay_loop_2(83); // 332 cycles + loop overhead (~15 cycles) ≒ 347 cycles
    }

    // Stop bit: HIGH
    VPORTB.OUT |= PIN4_bm;
    _delay_loop_2(86); // 344 cycles (~103.2 us)
}

static void dbg_print(const char *str) {
    uint8_t safety = 128;
    while (*str && safety--) {
        dbg_putch((uint8_t)*str++);
    }
}

static void dbg_print_hex8(uint8_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    dbg_putch(hex_chars[(val >> 4) & 0x0F]);
    dbg_putch(hex_chars[val & 0x0F]);
}

static void dbg_print_hex16(uint16_t val) {
    dbg_print_hex8((uint8_t)(val >> 8));
    dbg_print_hex8((uint8_t)(val & 0xFF));
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
// Channel 1: RS-485 Half-Duplex Operations (SP485EEN)
// =========================================================================
static inline void rs485_tx_start(void) {
    // DE=1 (PA4), /RE=1 (PA7: 自爆エコー完全遮断)
    VPORTA.OUT |= (PIN4_bm | PIN7_bm);
    _delay_loop_2(42); // ~50us トランシーバ安定化
}

static inline void putch(uint8_t ch) {
    while (!(USART0.STATUS & USART_DREIF_bm))
        ;
    USART0.TXDATAL = ch;
}

static inline void rs485_tx_end(void) {
    // 最後のストップビット送出完了を厳密に待機 (TXCIF)
    while (!(USART0.STATUS & USART_TXCIF_bm))
        ;
    USART0.STATUS = USART_TXCIF_bm;

    _delay_loop_2(42); // ~50us バス解放マージン
    // DE=0, /RE=0 (受信モード復帰)
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);

    // 受信 FIFO の過渡エコーをフラッシュ
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }
}

// =========================================================================
// Main Program Loop
// =========================================================================
int main(void) {
    // 1. ピン入出力設定
    //    PA1(TXD)=OUT HIGH, PA2(RXD)=IN
    //    PA4(DE)=OUT LOW, PA7(/RE)=OUT LOW (受信モード初期化・浮遊電荷防止)
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR &= ~PIN2_bm;

    //    PB2(LED)=OUT LOW, PB4(Soft-UART TX)=OUT HIGH
    VPORTB.DIR |= PIN2_bm | PIN4_bm;
    VPORTB.OUT &= ~PIN2_bm;
    VPORTB.OUT |= PIN4_bm;

    // 2. USART0 をオルタネートピン (PA1:TXD, PA2:RXD) にリダイレクト
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 3. USART0 LINAUTO モード初期化
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = BAUD_SETTING_16;
    } else {
        USART0.BAUD = BAUD_SETTING_20;
    }
    USART0.DBGCTRL = 1;
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 4. Break 検出をアーム (WFB=1)
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    // 5. 起動案内を COM21 (9600 bps) に出力
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D WU-0: RS-485 Break Generation Probe\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [INIT] Initial BAUD=0x"); dbg_print_hex16(USART0.BAUD);
    dbg_print(" STATUS=0x"); dbg_print_hex8(USART0.STATUS);
    dbg_print("\r\n Listening for PC Break + 0x55 (Sync) on COM19 @ 19200 bps...\r\n");

    uint16_t idle_counter = 0;

    for (;;) {
        // --- 監視 1: 不正同期フィールドエラー (ISFIF) ---
        if (USART0.STATUS & USART_ISFIF_bm) {
            isfif_count++;
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
            dbg_print("[SYNC_ERR] ISFIF=1 (Inconsistent Sync Edge) BAUD=0x");
            dbg_print_hex16(USART0.BAUD);
            dbg_print("\r\n");
        }

        // --- 監視 2: Break + 0x55 + 1Byte 受信完了 (RXCIF) ---
        if (USART0.STATUS & USART_RXCIF_bm) {
            break_count++;
            (void)USART0.RXDATAH;
            uint8_t payload_byte = USART0.RXDATAL;

            // 赤色 LED (PB2) をトグル点滅
            VPORTB.IN |= PIN2_bm;

            // COM21 テレメトリ出力
            dbg_print("[BREAK #");
            dbg_print_dec16(break_count);
            dbg_print(" OK] AutoBAUD=0x");
            dbg_print_hex16(USART0.BAUD);
            dbg_print(" Byte=0x");
            dbg_print_hex8(payload_byte);
            dbg_print(" ST=0x");
            dbg_print_hex8(USART0.STATUS);
            dbg_print("\r\n");

            // RS-485 (COM19) へ 4 バイトの確認応答を返信 [0x55, 0xAA, B_CNT_H, B_CNT_L]
            rs485_tx_start();
            putch(0x55);
            putch(0xAA);
            putch((uint8_t)(break_count >> 8));
            putch((uint8_t)(break_count & 0xFF));
            putch(payload_byte); // 受信したバイトのエコーバック
            rs485_tx_end();

            // 再び Break 待機 (WFB=1) をアーム
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 監視 3: アイドルハートビート (~1秒周期で LED を点滅) ---
        if (++idle_counter == 0) {
            VPORTB.IN |= PIN2_bm; // PB2 トグル
        }
    }
}
