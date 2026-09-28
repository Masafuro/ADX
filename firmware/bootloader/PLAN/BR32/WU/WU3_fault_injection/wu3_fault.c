/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-3: BR32 Fault Injection & Framing Resilience Probe
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
 *   - PB2: Red LED (Toggles on response transmission)
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

// BR32 Standard Status Codes (Specification Section 3.4)
#define STATUS_OK          0x00
#define STATUS_ERR_CRC     0x01
#define STATUS_ERR_TIMEOUT 0x02

// =========================================================================
// Channel 2: Soft-UART Debug Telemetry (PB4 TX @ 9,600 bps, 3.33 MHz)
// =========================================================================
static inline void dbg_delay_bit(void) {
    _delay_loop_2(86);
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
    while (*s) {
        dbg_putch(*s++);
    }
}

static void dbg_print_hex8(uint8_t val) {
    static const char hex[] = "0123456789ABCDEF";
    dbg_putch(hex[(val >> 4) & 0x0F]);
    dbg_putch(hex[val & 0x0F]);
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
    VPORTA.OUT |= (PIN4_bm | PIN7_bm);
    _delay_loop_2(42); // ~50us トランシーバ安定化
}

static inline void putch(uint8_t ch) {
    while (!(USART0.STATUS & USART_DREIF_bm))
        ;
    USART0.TXDATAL = ch;
}

static inline void rs485_tx_end(void) {
    while (!(USART0.STATUS & USART_TXCIF_bm))
        ;
    USART0.STATUS = USART_TXCIF_bm;

    _delay_loop_2(42); // ~50us バス解放マージン
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);

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
// Helper: Check if partial UID matches all zeros or my_uid
// =========================================================================
static bool is_target_accepted(const uint8_t *rx_buf, uint8_t rx_count, const uint8_t *my_uid) {
    if (rx_count < 2) {
        return true; // CMD だけ届いて切れた場合も OneToOne では応答
    }

    uint8_t uid_bytes_received = (rx_count >= 12) ? 10 : (rx_count - 2);

    bool all_zero = true;
    bool match_me = true;

    for (uint8_t i = 0; i < uid_bytes_received; i++) {
        if (rx_buf[2 + i] != 0x00) {
            all_zero = false;
        }
        if (rx_buf[2 + i] != my_uid[i]) {
            match_me = false;
        }
    }

    return (all_zero || match_me);
}

// =========================================================================
// Main Program Loop
// =========================================================================
int main(void) {
    // 1. ピン初期化
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR &= ~PIN2_bm;

    VPORTB.DIR |= PIN2_bm | PIN4_bm;
    VPORTB.OUT &= ~PIN2_bm;
    VPORTB.OUT |= PIN4_bm;

    // 2. USART0 オルタネートピン (PA1:TX, PA2:RX)
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 3. USART0 LINAUTO 初期化
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = BAUD_SETTING_16;
    } else {
        USART0.BAUD = BAUD_SETTING_20;
    }
    USART0.DBGCTRL = 1;
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 4. スレーブ自身の 10 バイト SIGROW 読み出し
    uint8_t my_uid[10];
    const volatile uint8_t *sig_ptr = (const volatile uint8_t *)&SIGROW_SERNUM0;
    for (uint8_t i = 0; i < 10; i++) {
        my_uid[i] = sig_ptr[i];
    }

    // 5. Break 検出アーム
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    // 6. 起動案内
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D WU-3: BR32 Fault Injection Probe\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [DEVICE UID] 0x");
    for (uint8_t i = 0; i < 10; i++) {
        dbg_print_hex8(my_uid[i]);
    }
    dbg_print("\r\n [RULES] Incomplete Frame (<32B) -> STATUS_ERR_TIMEOUT (0x02)\r\n");
    dbg_print("         Corrupt CRC-16         -> STATUS_ERR_CRC (0x01)\r\n");
    dbg_print("         Valid 32-Byte Frame    -> STATUS_OK (0x00)\r\n");
    dbg_print(" Listening for BR32 Frames @ 19200 bps...\r\n");

    uint8_t rx_buf[BR32_FRAME_SIZE];
    uint8_t tx_buf[BR32_FRAME_SIZE];
    uint16_t transaction_count = 0;
    uint16_t idle_counter = 0;

    for (;;) {
        // --- 監視 1: 不正同期フィールドエラー ---
        if (USART0.STATUS & USART_ISFIF_bm) {
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 監視 2: Break + 0x55 (Sync) 完了後、Byte 0 (CMD) 受信 ---
        if (USART0.STATUS & USART_RXCIF_bm) {
            (void)USART0.RXDATAH;
            rx_buf[0] = USART0.RXDATAL;
            uint8_t rx_count = 1;

            // =========================================================
            // 時間枠受信ループ (最大 32 バイトまで吸い込む)
            // 1 バイト約 0.52ms。無受信タイムアウトは約 15〜18ms (12,000 ループ)
            // =========================================================
            while (rx_count < BR32_FRAME_SIZE) {
                uint16_t to_timer = 0;
                while (!(USART0.STATUS & USART_RXCIF_bm)) {
                    if (++to_timer > 12000) {
                        // 途中で送信が切れた（タイムアウト発生）
                        goto rx_window_done;
                    }
                }
                (void)USART0.RXDATAH;
                rx_buf[rx_count++] = USART0.RXDATAL;
            }

        rx_window_done:
            // =========================================================
            // ★ BR32 原則 6: 32バイト超過の検知（連鎖的衝突防止のための完全沈黙）
            // 32バイト受信完了後、直後（約 0.6ms: 1文字時間以上）に 33 バイト目が
            // 流れてきている場合、それは「32B固定長」ではなくノイズ・衝突・超過ゴミである。
            // 待って返信しようとすれば連鎖的衝突を招くため、原則 2 相当（BREAK なしゴミ）
            // として扱い、1 ビットも喋らず即座に完全沈黙して次の BREAK を待つ！
            // =========================================================
            if (rx_count == BR32_FRAME_SIZE) {
                _delay_loop_2(500); // ~0.6ms (1文字時間以上待機して 33 バイト目有無を確認)
                if (USART0.STATUS & USART_RXCIF_bm) {
                    dbg_print("[RULE 6 SILENCE] Overflow >32B detected! Aborting to prevent collision.\r\n");
                    while (USART0.STATUS & USART_RXCIF_bm) {
                        (void)USART0.RXDATAL;
                    }
                    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                    continue; // 1 ビットも喋らず沈黙！
                }
            }

            // =========================================================
            // 宛先照合ゲート (届いた範囲で検査)
            // =========================================================
            if (is_target_accepted(rx_buf, rx_count, my_uid)) {
                transaction_count++;

                uint8_t resp_status = STATUS_OK;

                if (rx_count < BR32_FRAME_SIZE) {
                    // ★ 欠損検知: 時間枠満了時点で 32 バイト未満！
                    resp_status = STATUS_ERR_TIMEOUT;
                } else {
                    // 32 バイト満額受信時の CRC 検証
                    uint16_t calc_crc = calc_crc16(rx_buf, BR32_FRAME_SIZE - 2);
                    uint16_t rx_crc = ((uint16_t)rx_buf[30] << 8) | rx_buf[31];
                    if (calc_crc != rx_crc) {
                        // ★ CRC 異常検知！
                        resp_status = STATUS_ERR_CRC;
                    }
                }

                // =====================================================
                // 仕様書 3.2 節完全準拠の 32 バイト返信フレーム生成
                // =====================================================
                memset(tx_buf, 0, BR32_FRAME_SIZE);
                tx_buf[0] = resp_status;                                // [0] STATUS
                tx_buf[1] = (rx_count > 1) ? rx_buf[1] : 0x00;          // [1] ECHO_SEQ
                tx_buf[2] = rx_count;                                   // [2] RX_COUNT (実受信バイト数！)
                tx_buf[3] = (resp_status == STATUS_OK) ? 0x01 : 0x00;  // [3] DEV_STATE
                // [4..13] スレーブ自身の生 SIGROW
                for (uint8_t i = 0; i < 10; i++) {
                    tx_buf[4 + i] = my_uid[i];
                }
                tx_buf[14] = (uint8_t)(transaction_count >> 8);
                tx_buf[15] = (uint8_t)(transaction_count & 0xFF);
                // [16..29] EXTRA (受信できたデータの先頭部分をエコー)
                uint8_t echo_len = (rx_count > 16) ? (rx_count - 16) : 0;
                if (echo_len > 14) echo_len = 14;
                for (uint8_t i = 0; i < echo_len; i++) {
                    tx_buf[16 + i] = rx_buf[15 + i];
                }
                // [30..31] 応答フレーム自体の CRC16
                uint16_t resp_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
                tx_buf[30] = (uint8_t)(resp_crc >> 8);
                tx_buf[31] = (uint8_t)(resp_crc & 0xFF);

                // =====================================================
                // RS-485 応答返信 (障害報告であっても堂々と 32B 即時送出！)
                // =====================================================
                rs485_tx_start();
                for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
                    putch(tx_buf[i]);
                }
                rs485_tx_end();

                // PB2 LED トグル
                VPORTB.IN |= PIN2_bm;

                // COM21 デバッグテレメトリ出力
                dbg_print("[TRANS #");
                dbg_print_dec16(transaction_count);
                dbg_print("] Status=0x");
                dbg_print_hex8(resp_status);
                dbg_print(" RX_COUNT=");
                dbg_print_dec16(rx_count);
                dbg_print(resp_status == STATUS_OK ? " [OK]\r\n" :
                          resp_status == STATUS_ERR_TIMEOUT ? " [TIMEOUT_FAULT]\r\n" : " [CRC_FAULT]\r\n");

            } else {
                // 届いた部分が他人 UID である場合は完全沈黙
                dbg_print("[SILENCE] Ignored foreign frame\r\n");
            }

            // 次の Break 待機を再アーム
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 待機ハートビート ---
        if (++idle_counter == 0) {
            VPORTB.IN |= PIN2_bm;
        }
    }
}
