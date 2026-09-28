/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-1: BR32 32-Byte Fixed Frame Echo & Heartbeat Telemetry Probe
 * Target MCU: Microchip ATtiny1616-MNR (QFN-20) on ADX Core-D
 *
 * Channel 1: RS-485 (USART0 Alternate Pins) @ 19,200 bps
 *   - PA1: TXD (PORTMUX USART0 Alternate)
 *   - PA2: RXD
 *   - PA4: SP485EEN DE  (Active HIGH)
 *   - PA7: SP485EEN /RE (Active LOW)
 *
 * Channel 2: PC Debug Monitor (Soft-UART) @ 9,600 bps
 *   - PB4: TXD (Connected to CH342K Port B / COM21)
 *
 * Visual Indicator:
 *   - PB2: Red LED (Toggles on valid 32B frame round-trip)
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <stdbool.h>

#define BR32_FRAME_SIZE 32
#define BAUD_SETTING_16 0x02B5  // 19200 @ 16MHz/6
#define BAUD_SETTING_20 0x0364  // 19200 @ 20MHz/6

// =========================================================================
// Channel 2: Soft-UART Debug Telemetry (PB4 TX @ 9,600 bps, 3.33 MHz)
// =========================================================================
static inline void dbg_delay_bit(void) {
    // 3.33MHz / 9600bps ≈ 347 cycles.
    // _delay_loop_2(86) ≈ 344 cycles.
    _delay_loop_2(86);
}

static void dbg_putch(char c) {
    uint8_t sreg = SREG;
    cli();

    // Start bit (LOW)
    VPORTB.OUT &= ~PIN4_bm;
    dbg_delay_bit();

    // 8 Data bits (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        if (c & 0x01) {
            VPORTB.OUT |= PIN4_bm;
        } else {
            VPORTB.OUT &= ~PIN4_bm;
        }
        dbg_delay_bit();
        c >>= 1;
    }

    // Stop bit (HIGH)
    VPORTB.OUT |= PIN4_bm;
    dbg_delay_bit();
    dbg_delay_bit();

    SREG = sreg;
}

static void dbg_print(const char *s) {
    while (*s) {
        dbg_putch(*s++);
    }
}

static void dbg_print_hex8(uint8_t val) {
    static const char hex[] = "0123456789ABCDEF";
    dbg_putch(hex[(val >> 4) & 0x0F]);
    dbg_putch(hex[val & 0x0F]);
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
// CRC-16 Calculation (XMODEM / CCITT-FALSE: Poly 0x1021, Init 0xFFFF)
// =========================================================================
static uint16_t calc_crc16(const uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc = _crc_xmodem_update(crc, data[i]);
    }
    return crc;
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

    // 4. スレーブ自身の 10 バイト SIGROW (Unique Serial ID) を読み出し
    uint8_t my_uid[10];
    const volatile uint8_t *sig_ptr = (const volatile uint8_t *)&SIGROW_SERNUM0;
    for (uint8_t i = 0; i < 10; i++) {
        my_uid[i] = sig_ptr[i];
    }

    // 5. Break 検出をアーム (WFB=1)
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    // 6. 起動案内を COM21 (9600 bps) に出力
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D WU-1: BR32 32-Byte Frame Echo Probe\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [DEVICE UID] 0x");
    for (uint8_t i = 0; i < 10; i++) {
        dbg_print_hex8(my_uid[i]);
    }
    dbg_print("\r\n [INIT] BAUD=0x"); dbg_print_hex16(USART0.BAUD);
    dbg_print(" STATUS=0x"); dbg_print_hex8(USART0.STATUS);
    dbg_print("\r\n Listening for BR32 32-Byte Frames @ 19200 bps...\r\n");

    uint8_t rx_buf[BR32_FRAME_SIZE];
    uint8_t tx_buf[BR32_FRAME_SIZE];
    uint16_t frame_count = 0;
    uint16_t crc_error_count = 0;
    uint16_t timeout_count = 0;
    uint16_t idle_counter = 0;

    for (;;) {
        // --- 監視 1: 不正同期フィールドエラー (ISFIF) ---
        if (USART0.STATUS & USART_ISFIF_bm) {
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 監視 2: Break + 0x55 (Sync) 完了後、最初のフレームバイト (CMD) 受信 (RXCIF) ---
        // 注意: LINAUTO ハードウェアは 0x55 (Sync) をボーレート校正に消費し破棄する。
        //       したがって最初に RXCIF が立った時点で読めるデータは、フレームの Byte 0 (CMD) である！
        if (USART0.STATUS & USART_RXCIF_bm) {
            (void)USART0.RXDATAH;
            rx_buf[0] = USART0.RXDATAL; // Byte 0: CMD

            bool frame_complete = true;

            // 残り 31 バイト (Byte 1..31) をタイムアウト付きループで受信
            // 19200 bps の 1 バイトは約 0.52ms。
            // 3.33MHz で 15,000 ループ ≈ 18ms のタイムアウト窓を設定。
            for (uint8_t b = 1; b < BR32_FRAME_SIZE; b++) {
                uint16_t to_timer = 0;
                while (!(USART0.STATUS & USART_RXCIF_bm)) {
                    if (++to_timer > 15000) {
                        frame_complete = false;
                        timeout_count++;
                        break;
                    }
                }
                if (!frame_complete) {
                    break;
                }
                (void)USART0.RXDATAH;
                rx_buf[b] = USART0.RXDATAL;
            }

            if (frame_complete) {
                // CRC-16-CCITT 検証 (先頭 30 バイト [0..29] の CRC を計算)
                uint16_t calc_crc = calc_crc16(rx_buf, BR32_FRAME_SIZE - 2);
                uint16_t rx_crc = ((uint16_t)rx_buf[30] << 8) | rx_buf[31];

                if (calc_crc == rx_crc) {
                    frame_count++;

                    // 仕様書 3.2 節準拠の 32 バイト応答フレームを生成
                    tx_buf[0] = 0x00;                           // [0] STATUS (0x00 = STATUS_OK)
                    tx_buf[1] = rx_buf[1];                      // [1] ECHO_SEQ (受信した SEQ)
                    tx_buf[2] = BR32_FRAME_SIZE;               // [2] RX_COUNT (正常受信 32 バイト)
                    // [3..12] スレーブ自身の生 SIGROW 10 バイト
                    for (uint8_t i = 0; i < 10; i++) {
                        tx_buf[3 + i] = my_uid[i];
                    }
                    tx_buf[13] = (uint8_t)(frame_count >> 8);   // [13] ADDR_H (通信カウンタ H)
                    tx_buf[14] = (uint8_t)(frame_count & 0xFF); // [14] ADDR_L (通信カウンタ L)
                    tx_buf[15] = rx_buf[14];                    // [15] LEN (受信フレームの [14] をエコー)
                    // [16..29] 受信データ 14 バイト (rx_buf[15..28]) をそのままエコーバック
                    for (uint8_t i = 0; i < 14; i++) {
                        tx_buf[16 + i] = rx_buf[15 + i];
                    }
                    // [30..31] 応答フレームの CRC-16-CCITT
                    uint16_t resp_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
                    tx_buf[30] = (uint8_t)(resp_crc >> 8);
                    tx_buf[31] = (uint8_t)(resp_crc & 0xFF);

                    // =====================================================
                    // 最優先: RS-485 応答返信 (遅延ゼロで即時射出！)
                    // =====================================================
                    rs485_tx_start();
                    for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
                        putch(tx_buf[i]);
                    }
                    rs485_tx_end();

                    // PB2 赤色 LED をチカッとトグル
                    VPORTB.IN |= PIN2_bm;

                    // =====================================================
                    // 返信完了後に COM21 テレメトリ出力 (RTT に影響なし！)
                    // =====================================================
                    dbg_print("[BR32 #");
                    dbg_print_dec16(frame_count);
                    dbg_print(" OK] SEQ=0x");
                    dbg_print_hex8(rx_buf[1]);
                    dbg_print(" CMD=0x");
                    dbg_print_hex8(rx_buf[0]);
                    dbg_print(" CRC=0x");
                    dbg_print_hex16(calc_crc);
                    dbg_print("\r\n");

                } else {
                    crc_error_count++;
                    dbg_print("[CRC_ERR] Exp=0x");
                    dbg_print_hex16(calc_crc);
                    dbg_print(" Got=0x");
                    dbg_print_hex16(rx_crc);
                    dbg_print("\r\n");
                }
            } else {
                dbg_print("[TIMEOUT] Incomplete frame\r\n");
            }

            // 次の Break 待機 (WFB=1) を再アーム
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 監視 3: アイドルハートビート (待機中は約 1 秒周期で LED 点滅) ---
        if (++idle_counter == 0) {
            VPORTB.IN |= PIN2_bm;
        }
    }
}
