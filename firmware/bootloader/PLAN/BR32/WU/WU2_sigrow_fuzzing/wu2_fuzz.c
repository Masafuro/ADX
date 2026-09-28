/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-2: BR32 SIGROW Gate & UID Fuzzing / Silence Probe
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
 *   - PB2: Red LED (Toggles on accepted frame response)
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <stdbool.h>
#include <string.h>

#define BR32_FRAME_SIZE 32
#define BAUD_SETTING_16 0x02B5  // 19200 @ 16MHz/6
#define BAUD_SETTING_20 0x0364  // 19200 @ 20MHz/6

// Status codes
#define STATUS_OK          0x00
#define STATUS_ERR_CRC     0x01
#define STATUS_ERR_TIMEOUT 0x02

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
// Helper: Check if target UID is all zeros (OneToOne / Broadcast)
// =========================================================================
static bool is_all_zero(const uint8_t *uid, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (uid[i] != 0x00) {
            return false;
        }
    }
    return true;
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
    dbg_print("   ADX Core-D WU-2: BR32 SIGROW Gate & Silence Probe\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [DEVICE UID] 0x");
    for (uint8_t i = 0; i < 10; i++) {
        dbg_print_hex8(my_uid[i]);
    }
    dbg_print("\r\n [GATE RULES] Accept: All 0x00 OR Match UID. Other: 100% SILENCE.\r\n");
    dbg_print(" Listening for BR32 Frames @ 19200 bps...\r\n");

    uint8_t rx_buf[BR32_FRAME_SIZE];
    uint8_t tx_buf[BR32_FRAME_SIZE];
    uint16_t accepted_count = 0;
    uint16_t silent_count = 0;
    uint16_t crc_error_count = 0;
    uint16_t timeout_count = 0;
    uint16_t idle_counter = 0;

    for (;;) {
        // --- 監視 1: 不正同期フィールドエラー (ISFIF) ---
        if (USART0.STATUS & USART_ISFIF_bm) {
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 監視 2: Break + 0x55 (Sync) 完了後、最初のフレームバイト (CMD) 受信 (RXCIF) ---
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
                // CRC-16-CCITT (XMODEM) 検証 (先頭 30 バイト [0..29] の CRC を計算)
                uint16_t calc_crc = calc_crc16(rx_buf, BR32_FRAME_SIZE - 2);
                uint16_t rx_crc = ((uint16_t)rx_buf[30] << 8) | rx_buf[31];

                if (calc_crc == rx_crc) {
                    // =========================================================
                    // ★ WU-2 核心: SIGROW 宛先照合ゲート (Address Filtering)
                    // Master Frame Offset:
                    //   [0]: CMD
                    //   [1]: SEQ
                    //   [2..11]: TARGET_SIGROW (10 Bytes)
                    // =========================================================
                    const uint8_t *target_uid = &rx_buf[2];
                    bool is_broadcast = is_all_zero(target_uid, 10);
                    bool is_for_me    = (memcmp(target_uid, my_uid, 10) == 0);

                    if (is_broadcast || is_for_me) {
                        // --- ゲート通過: 自分宛て または 全0x00 自己同定 ---
                        accepted_count++;

                        // 仕様書 3.2 節完全準拠の 32 バイト返信フレーム生成
                        tx_buf[0] = STATUS_OK;                      // [0] STATUS
                        tx_buf[1] = rx_buf[1];                      // [1] ECHO_SEQ
                        tx_buf[2] = BR32_FRAME_SIZE;               // [2] RX_COUNT (32)
                        tx_buf[3] = is_for_me ? 0x01 : 0x00;        // [3] DEV_STATE (0x01=Targeted, 0x00=Broadcast)
                        // [4..13] スレーブ自身の生 SIGROW 10 バイト
                        for (uint8_t i = 0; i < 10; i++) {
                            tx_buf[4 + i] = my_uid[i];
                        }
                        tx_buf[14] = (uint8_t)(accepted_count >> 8);   // [14] CUR_PAGE (ここでは受諾カウンタ上位)
                        tx_buf[15] = (uint8_t)(accepted_count & 0xFF); // [15] CUR_CHUNK (受諾カウンタ下位)
                        // [16..29] EXTRA (受信ペイロード rx_buf[15..28] をエコーバック)
                        for (uint8_t i = 0; i < 14; i++) {
                            tx_buf[16 + i] = rx_buf[15 + i];
                        }
                        // [30..31] 応答フレームの CRC-16
                        uint16_t resp_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
                        tx_buf[30] = (uint8_t)(resp_crc >> 8);
                        tx_buf[31] = (uint8_t)(resp_crc & 0xFF);

                        // =====================================================
                        // 最優先: RS-485 応答返信 (遅延ゼロ即時射出)
                        // =====================================================
                        rs485_tx_start();
                        for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
                            putch(tx_buf[i]);
                        }
                        rs485_tx_end();

                        // PB2 赤色 LED トグル (応答完了)
                        VPORTB.IN |= PIN2_bm;

                        // COM21 テレメトリ出力 (返信完了後)
                        dbg_print("[ACCEPTED #");
                        dbg_print_dec16(accepted_count);
                        dbg_print("] Match=");
                        dbg_print(is_for_me ? "TARGETED" : "ALL_ZERO");
                        dbg_print(" SEQ=0x");
                        dbg_print_hex8(rx_buf[1]);
                        dbg_print("\r\n");

                    } else {
                        // =====================================================
                        // ★ ゲート遮断: 他人の UID / 不正 UID / ゴミデータ
                        // 1 ビットも喋らず完全沈黙！ (DE=0, /RE=0 受信状態維持)
                        // =====================================================
                        silent_count++;

                        // COM21 デバッグモニタにのみ沈黙記録を報告 (RS-485 回線は無音)
                        dbg_print("[SILENCE #");
                        dbg_print_dec16(silent_count);
                        dbg_print("] Ignored Target UID=0x");
                        for (uint8_t i = 0; i < 10; i++) {
                            dbg_print_hex8(target_uid[i]);
                        }
                        dbg_print("\r\n");
                    }

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
