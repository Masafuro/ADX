/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * Milestone 4 (M4): BR32 Single Flash Page Erase/Write & Non-blocking Isolation
 * Target MCU: Microchip ATtiny1616-MNR (QFN-20) on ADX Core-D
 *
 * Flash Target: Page 16 (0x0400 - 0x043F, 64 Bytes)
 * Bootloader Protection: Pages 0..15 (0x0000 - 0x03FF, 1KB) strictly protected
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
#include <avr/cpufunc.h>
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
#define CMD_BOOT_APP    0x30

// BR32 Standard Status Codes (Specification Section 3.4)
#define STATUS_OK          0x00
#define STATUS_ERR_CRC     0x01
#define STATUS_ERR_TIMEOUT 0x02
#define STATUS_ERR_PROTECT 0x03
#define STATUS_ERR_BUSY    0x04

// ATtiny1616 Flash Memory Configuration
#define FLASH_PAGE_SIZE    64
#define FLASH_CHUNK_SIZE   16
#define FLASH_TOTAL_PAGES  256  // 16 KB / 64 B
#define APP_START_PAGE     16   // Page 16 = 0x0400 (First 1KB protected)

// Chunk Flags Masks
#define CHUNK_IDX_MASK     0x03
#define FLAG_COMMIT_PAGE   0x04
#define FLAG_LAUNCH_APP    0x08

// SRAM Page Buffer (64 Bytes = 4 Chunks x 16 Bytes)
static uint8_t page_buffer[FLASH_PAGE_SIZE];
static uint8_t current_buffered_page = 0xFF;

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
// Hardwired Flash Commit Routine (Placed strictly inside BOOT Section < 0x0400)
// =========================================================================
__attribute__((noinline))
void nvm_commit_page_hw(uint16_t page_addr, const uint8_t *data) {
    uint8_t *dest = (uint8_t *)(MAPPED_PROGMEM_START + page_addr);
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        *(dest++) = data[i];
    }
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
        ;
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

    // ページバッファ初期化
    memset(page_buffer, 0xFF, sizeof(page_buffer));

    // 5. Break 検出アーム (原則 1)
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    // 6. 起動案内
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D Milestone 4: Flash Page Erase/Write Gate\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [DEVICE UID] 0x");
    for (uint8_t i = 0; i < 10; i++) {
        dbg_print_hex8(my_uid[i]);
    }
    dbg_print("\r\n [PROTECTION] Pages 0..15 (0x0000..0x03FF) STRICTLY PROTECTED\r\n");
    dbg_print(" [TARGET PAGE] Page 16 (0x0400..0x043F, 64 Bytes)\r\n");
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
            uint8_t page_idx = rx_buf[12];
            uint8_t chunk_flags = rx_buf[13];
            uint8_t chunk_idx = chunk_flags & CHUNK_IDX_MASK;
            bool do_flash_commit = false;

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
                // 【機能②】16B チャンク書き込み
                // 1. 安全保護領域チェック
                if (page_idx < APP_START_PAGE || page_idx >= FLASH_TOTAL_PAGES) {
                    tx_buf[0] = STATUS_ERR_PROTECT;
                    dbg_print("[SECURITY ALERT] Write blocked to protected page 0x");
                    dbg_print_hex8(page_idx);
                    dbg_print("\r\n");
                } else {
                    // 2. 正常領域: SRAM バッファに蓄積
                    current_buffered_page = page_idx;
                    memcpy(&page_buffer[chunk_idx * FLASH_CHUNK_SIZE], &rx_buf[14], FLASH_CHUNK_SIZE);

                    // エコーバック
                    memcpy(&tx_buf[14], &page_buffer[chunk_idx * FLASH_CHUNK_SIZE], FLASH_CHUNK_SIZE);

                    dbg_print("[CMD: WRITE_CHUNK] Page 0x");
                    dbg_print_hex8(page_idx);
                    dbg_print(" Chunk #");
                    dbg_print_dec16(chunk_idx);
                    dbg_print(" buffered in SRAM\r\n");

                    // 3. COMMIT_PAGE 指示の検出 (Bit 2 が立っている、または Chunk 3)
                    if (chunk_flags & FLAG_COMMIT_PAGE) {
                        do_flash_commit = true;
                    }
                }

            } else if (cmd == CMD_READ_CHUNK) {
                // 【機能③】物理 Flash メモリから直接 16B 読み出し
                if (page_idx >= FLASH_TOTAL_PAGES) {
                    tx_buf[0] = STATUS_ERR_PROTECT;
                    dbg_print("[ERR] Read page out of bounds: 0x");
                    dbg_print_hex8(page_idx);
                    dbg_print("\r\n");
                } else {
                    // 物理 Flash アドレス（MAPPED_PROGMEM_START = 0x8000）から直接読み出す！
                    uint16_t flash_offset = ((uint16_t)page_idx * FLASH_PAGE_SIZE) + ((uint16_t)chunk_idx * FLASH_CHUNK_SIZE);
                    const uint8_t *flash_ptr = (const uint8_t *)(MAPPED_PROGMEM_START + flash_offset);

                    memcpy(&tx_buf[14], flash_ptr, FLASH_CHUNK_SIZE);

                    dbg_print("[CMD: READ_CHUNK] Read 16B from Physical Flash [0x");
                    dbg_print_hex8((uint8_t)(flash_offset >> 8));
                    dbg_print_hex8((uint8_t)(flash_offset & 0xFF));
                    dbg_print("]\r\n");
                }
            }

            // [30..31] 応答フレーム CRC16
            uint16_t resp_crc = calc_crc16(tx_buf, BR32_FRAME_SIZE - 2);
            tx_buf[30] = (uint8_t)(resp_crc >> 8);
            tx_buf[31] = (uint8_t)(resp_crc & 0xFF);

            // =========================================================
            // ★ 最優先: RS-485 応答返信 (即時射出)
            // =========================================================
            rs485_tx_start();
            for (uint8_t i = 0; i < BR32_FRAME_SIZE; i++) {
                putch(tx_buf[i]);
            }
            rs485_tx_end(); // ここで TXCIF 待機 ＆ バス完全解放 (DE=0, /RE=0)

            // PB2 LED トグル
            VPORTB.IN |= PIN2_bm;

            // =========================================================
            // ★ 非同期分離: 送信完了後に Flash 消去書き込みを実行！
            // =========================================================
            if (do_flash_commit) {
                dbg_print("  |-> [ASYNC NVM] Committing Page 0x");
                dbg_print_hex8(page_idx);
                dbg_print(" (Flash Addr 0x");
                uint16_t p_addr = (uint16_t)page_idx * FLASH_PAGE_SIZE;
                dbg_print_hex8((uint8_t)(p_addr >> 8));
                dbg_print_hex8((uint8_t)(p_addr & 0xFF));
                dbg_print(") Erase & Write...\r\n");

                // ★ アプローチ 2: BOOT 領域（< 0x0400）に配置された専用ルーチンで物理書き込み！
                nvm_commit_page_hw(p_addr, page_buffer);

                dbg_print("  |-> [ASYNC NVM] Flash write complete! (Slack time ~158ms remaining)\r\n");
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
