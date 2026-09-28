/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-4: BR32 Virtual SRAM Page Painter Sandbox (No Flash Risk)
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
 *   - PB2: Red LED (Toggles on chunk operation)
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

// BR32 Standard Commands (Specification Section 3.3)
#define CMD_IDENTIFY    0x01
#define CMD_WRITE_CHUNK 0x10
#define CMD_READ_CHUNK  0x20

// BR32 Standard Status Codes (Specification Section 3.4)
#define STATUS_OK          0x00
#define STATUS_ERR_CRC     0x01
#define STATUS_ERR_TIMEOUT 0x02

// Virtual SRAM Buffer (64 Bytes = 4 Chunks x 16 Bytes, Flash is NEVER touched)
static uint8_t virtual_sram[64];

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
// CRC-16 Calculation (XMODEM: Poly 0x1021, Init 0xFFFF, MSB-first)
// =========================================================================
static uint16_t calc_crc16(const uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc = _crc_xmodem_update(crc, data[i]);
    }
    return crc;
}

// =========================================================================
// Helper: Check if target UID matches all zeros or my_uid
// =========================================================================
static bool is_target_accepted(const uint8_t *target_uid, const uint8_t *my_uid) {
    bool all_zero = true;
    bool match_me = true;

    for (uint8_t i = 0; i < 10; i++) {
        if (target_uid[i] != 0x00) {
            all_zero = false;
        }
        if (target_uid[i] != my_uid[i]) {
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

    // 仮想 SRAM を初期文字列でクリア ("ADX BR32 VIRTUAL SRAM INITIALIZED ...")
    memset(virtual_sram, 0xAA, sizeof(virtual_sram));

    // 5. Break 検出アーム (原則 1)
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    // 6. 起動案内
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D WU-4: Virtual SRAM Page Painter Probe\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [DEVICE UID] 0x");
    for (uint8_t i = 0; i < 10; i++) {
        dbg_print_hex8(my_uid[i]);
    }
    dbg_print("\r\n [BUFFER] 64 Bytes Virtual SRAM (No Flash Write)\r\n");
    dbg_print(" [COMMANDS] 0x01=IDENTIFY, 0x10=WRITE_CHUNK, 0x20=READ_CHUNK\r\n");
    dbg_print(" Listening for BR32 Frames @ 19200 bps...\r\n");

    uint8_t rx_buf[BR32_FRAME_SIZE];
    uint8_t tx_buf[BR32_FRAME_SIZE];
    uint16_t op_count = 0;
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

            // 時間枠受信ループ (最大 32 バイト吸い込む)
            while (rx_count < BR32_FRAME_SIZE) {
                uint16_t to_timer = 0;
                while (!(USART0.STATUS & USART_RXCIF_bm)) {
                    if (++to_timer > 12000) {
                        goto rx_window_done;
                    }
                }
                (void)USART0.RXDATAH;
                rx_buf[rx_count++] = USART0.RXDATAL;
            }

        rx_window_done:
            // =========================================================
            // ★ BR32 原則 6: 32バイト超過検知 (連鎖衝突防止の完全沈黙)
            // =========================================================
            if (rx_count == BR32_FRAME_SIZE) {
                _delay_loop_2(500); // ~0.6ms
                if (USART0.STATUS & USART_RXCIF_bm) {
                    dbg_print("[RULE 6 SILENCE] Overflow >32B detected! Aborting.\r\n");
                    while (USART0.STATUS & USART_RXCIF_bm) {
                        (void)USART0.RXDATAL;
                    }
                    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                    continue; // 完全沈黙
                }
            }

            // =========================================================
            // 宛先照合ゲート (原則 2: 宛先不一致は完全沈黙)
            // =========================================================
            if (rx_count < 12 || !is_target_accepted(&rx_buf[2], my_uid)) {
                // 宛先が自分宛てでも全0でもない、または宛先領域すら届いていない場合は沈黙
                dbg_print("[GATE SILENCE] Foreign or truncated target\r\n");
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                continue;
            }

            op_count++;

            // =========================================================
            // 整合性チェック (原則 3 & 4)
            // =========================================================
            if (rx_count < BR32_FRAME_SIZE) {
                // 原則 3: 32B 未満で切断
                memset(tx_buf, 0, BR32_FRAME_SIZE);
                tx_buf[0] = STATUS_ERR_TIMEOUT;
                tx_buf[1] = (rx_count > 1) ? rx_buf[1] : 0x00;
                tx_buf[2] = rx_count;
                for (uint8_t i = 0; i < 10; i++) tx_buf[4 + i] = my_uid[i];
                uint16_t err_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
                tx_buf[30] = (uint8_t)(err_crc >> 8);
                tx_buf[31] = (uint8_t)(err_crc & 0xFF);

                rs485_tx_start();
                for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) putch(tx_buf[i]);
                rs485_tx_end();

                dbg_print("[ERR] Timeout incomplete frame\r\n");
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                continue;
            }

            uint16_t calc_crc = calc_crc16(rx_buf, BR32_FRAME_SIZE - 2);
            uint16_t rx_crc = ((uint16_t)rx_buf[30] << 8) | rx_buf[31];
            if (calc_crc != rx_crc) {
                // 原則 4: CRC 不一致
                memset(tx_buf, 0, BR32_FRAME_SIZE);
                tx_buf[0] = STATUS_ERR_CRC;
                tx_buf[1] = rx_buf[1];
                tx_buf[2] = BR32_FRAME_SIZE;
                for (uint8_t i = 0; i < 10; i++) tx_buf[4 + i] = my_uid[i];
                uint16_t err_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
                tx_buf[30] = (uint8_t)(err_crc >> 8);
                tx_buf[31] = (uint8_t)(err_crc & 0xFF);

                rs485_tx_start();
                for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) putch(tx_buf[i]);
                rs485_tx_end();

                dbg_print("[ERR] CRC mismatch\r\n");
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                continue;
            }

            // =========================================================
            // 原則 5: コマンド処理 (IDENTIFY, WRITE_CHUNK, READ_CHUNK)
            // =========================================================
            uint8_t cmd = rx_buf[0];
            uint8_t chunk_idx = rx_buf[13] & 0x03; // Bit 0..1: Chunk index (0..3)

            memset(tx_buf, 0, BR32_FRAME_SIZE);
            tx_buf[0] = STATUS_OK;
            tx_buf[1] = rx_buf[1]; // ECHO_SEQ
            tx_buf[2] = BR32_FRAME_SIZE;
            tx_buf[3] = 0x01 | (chunk_idx << 4); // DEV_STATE (READY | chunk_idx)
            for (uint8_t i = 0; i < 10; i++) {
                tx_buf[4 + i] = my_uid[i];
            }

            if (cmd == CMD_IDENTIFY) {
                // 【機能①】自己同定
                dbg_print("[CMD: IDENTIFY] Responding UID\r\n");

            } else if (cmd == CMD_WRITE_CHUNK) {
                // 【機能②】16B データを SRAM 仮想ページバッファ (0..63) に書き込み
                memcpy(&virtual_sram[chunk_idx * 16], &rx_buf[14], 16);

                // 書き込んだ 16B を応答フレーム [14..29] にエコーバック
                memcpy(&tx_buf[14], &virtual_sram[chunk_idx * 16], 16);

                dbg_print("[CMD: WRITE_CHUNK #");
                dbg_print_dec16(chunk_idx);
                dbg_print("] 16B saved to SRAM\r\n");

            } else if (cmd == CMD_READ_CHUNK) {
                // 【機能③】SRAM 仮想ページバッファ (0..63) から 16B 読み出し
                memcpy(&tx_buf[14], &virtual_sram[chunk_idx * 16], 16);

                dbg_print("[CMD: READ_CHUNK #");
                dbg_print_dec16(chunk_idx);
                dbg_print("] 16B read from SRAM\r\n");
            }

            // [30..31] 応答フレーム CRC16
            uint16_t resp_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
            tx_buf[30] = (uint8_t)(resp_crc >> 8);
            tx_buf[31] = (uint8_t)(resp_crc & 0xFF);

            // =========================================================
            // 最優先: RS-485 応答返信 (即時射出)
            // =========================================================
            rs485_tx_start();
            for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
                putch(tx_buf[i]);
            }
            rs485_tx_end();

            // PB2 LED トグル
            VPORTB.IN |= PIN2_bm;

            // 次の Break 待機を再アーム
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }

        // --- 待機ハートビート ---
        if (++idle_counter == 0) {
            VPORTB.IN |= PIN2_bm;
        }
    }
}
