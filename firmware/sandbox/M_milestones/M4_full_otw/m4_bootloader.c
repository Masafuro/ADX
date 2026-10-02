/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * Milestone 4 (M4): MR32 12KB Full OTW Flash Writer & Application Launcher
 * Target MCU: Microchip ATtiny1616-MNR (QFN-20) on ADX Core-D
 *
 * Memory Protection Layout (Experimental Base Design):
 *   - Pages 0..63 (0x0000 - 0x0FFF, 4,096 Bytes / 4KB): Bootloader Protected (LOCKED)
 *   - Pages 64..255 (0x1000 - 0x3FFF, 12,288 Bytes / 12KB): Application Space (WRITABLE)
 *
 * Hardware Mapping:
 *   - Channel 1: RS-485 (USART0 Alternate Pins) @ 115,200 bps
 *       PA1: TXD (PORTMUX USART0 Alternate)
 *       PA2: RXD
 *       PA4: SP485EEN DE  (Active HIGH)
 *       PA7: SP485EEN /RE (Active LOW)
 *   - Channel 2: PC Debug Monitor (Soft-UART) @ 9,600 bps
 *       PB4: TXD (CH342K Port B)
 *   - Indicators:
 *       PB2: Red LED (Toggles on physical Flash page write)
 *       PB3: White LED (Toggles on chunk activity)
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "../../M_firmware/inc/mr32.h"

#define MR32_MY_NODE_ID       0x01
#define FLASH_PAGE_SIZE       64
#define CHUNK_DATA_SIZE       16
#define CHUNKS_PER_PAGE       4
#define APP_START_PAGE        64   // Page 64 = 0x1000 (Protection Boundary: 4KB)
#define FLASH_TOTAL_PAGES     256

// =========================================================================
// Channel 2: Soft-UART Debug Telemetry (PB4 TX @ 9,600 bps, 20 MHz)
// =========================================================================
static inline void dbg_delay_bit(void) {
    _delay_loop_2(520); // 20MHz / 9600bps ≈ 2083 cycles
}

static void dbg_putch(char c) {
    uint8_t sreg = SREG;
    cli();
    VPORTB.OUT &= ~PIN4_bm; // Start bit
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
    VPORTB.OUT |= PIN4_bm;  // Stop bit
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

__attribute__((unused)) static void dbg_print_hex16(uint16_t val) {
    dbg_print_hex8((uint8_t)(val >> 8));
    dbg_print_hex8((uint8_t)(val & 0xFF));
}

// =========================================================================
// Channel 1: RS-485 Half-Duplex Operations (SP485EEN @ 115,200 bps)
// =========================================================================
static inline void rs485_tx_start(void) {
    VPORTA.OUT |= (PIN4_bm | PIN7_bm);
    _delay_us(10);
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
    _delay_us(160); // T_guard (>= 150us)
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }
}

// =========================================================================
// Hardware Flash Memory Operations (NVMCTRL Self-Programming)
// =========================================================================
static uint8_t sram_page_buffer[FLASH_PAGE_SIZE];

// Commit 64-byte SRAM buffer into physical Flash page
__attribute__((noinline))
static void nvm_commit_page_hw(uint16_t page_idx) {
    uint16_t page_addr = page_idx << 6; // page_idx * 64
    uint8_t *dest = (uint8_t *)(MAPPED_PROGMEM_START + page_addr);

    // Load 64 bytes into hardware page buffer
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        *(dest++) = sram_page_buffer[i];
    }

    // Trigger hardware page erase & write command
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);

    // Wait until hardware completion (approx 2.5ms)
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm))
        ;

    // 【安全規程】NVMCTRL を待機状態 (NONE) に完全復帰
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);
}

// Read 16 bytes directly from physical Flash
static void flash_read_chunk(uint16_t page_idx, uint8_t chunk_idx, uint8_t *out_buf) {
    uint16_t offset = (page_idx << 6) + (chunk_idx << 4);
    const uint8_t *src = (const uint8_t *)(MAPPED_PROGMEM_START + offset);
    for (uint8_t i = 0; i < CHUNK_DATA_SIZE; i++) {
        out_buf[i] = *(src++);
    }
}

// Calculate CRC16 of 64-byte physical Flash page
static uint16_t flash_calculate_page_crc(uint16_t page_idx) {
    uint16_t offset = page_idx << 6;
    const uint8_t *src = (const uint8_t *)(MAPPED_PROGMEM_START + offset);
    return mr32_calculate_crc(src, FLASH_PAGE_SIZE);
}

// Jump to User Application at 0x1000 (Word address 0x0800)
__attribute__((noreturn))
static void launch_user_app(void) {
    dbg_print("\r\n[BOOT] Shutting down bootloader services...\r\n");
    dbg_print("[BOOT] Jumping to User Application @ 0x1000...\r\n");
    _delay_ms(20);

    // Disable interrupts & reset USART0
    cli();
    USART0.CTRLB = 0;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm); // RS-485 RX mode

    // Indirect Jump to 0x1000 (Byte 0x1000 == Word 0x0800)
    __asm__ __volatile__(
        "clr r1\n\t"
        "ldi r30, 0x00\n\t"
        "ldi r31, 0x08\n\t" // Z = 0x0800
        "ijmp\n\t"
    );

    while (1)
        ; // Unreachable
}

// =========================================================================
// Main Loop & MR32 Flash Protocol Handler
// =========================================================================
typedef enum {
    STATE_WAIT_SYNC,
    STATE_WAIT_MAGIC,
    STATE_RECV_BODY
} rx_state_t;

int main(void) {
    // 1. クロック設定: 内部 20MHz 駆動
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

    // 2. ピン入出力設定
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR &= ~PIN2_bm;

    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT &= ~(PIN2_bm | PIN3_bm);
    VPORTB.OUT |= PIN4_bm;

    // 3. USART0 オルタネートピン設定
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 4. USART0 115,200 bps
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = 555;
    } else {
        USART0.BAUD = 694;
    }
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc | USART_SBMODE_1BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_NORMAL_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 5. 起動案内
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D M4: MR32 Full OTW Writer & App Launcher\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED, PB3=ACT\r\n");
    dbg_print(" [DEVICE] ATtiny1616-MNR (Flash: 16KB, Page: 64B)\r\n");
    dbg_print(" [PROTECT] Pages 0..63 (0x0000 - 0x0FFF, 4KB) LOCKED\r\n");
    dbg_print(" [TARGET] Application Space (Pages 64..255, 12KB) WRITABLE\r\n");
    dbg_print(" [APP JUMP] Target Address: 0x1000 (ijmp 0x0800)\r\n");
    dbg_print(" [MR32] 115,200 bps 8N1, Magic=0x55 0xAD\r\n");
    dbg_print(" Ready for Full OTW Programming...\r\n");

    uint16_t current_page = 0xFFFF;
    uint8_t chunk_mask = 0;

    uint8_t rx_buf[MR32_FRAME_LEN];
    uint8_t tx_buf[MR32_FRAME_LEN];
    uint8_t rx_idx = 0;
    bool ignore_rest = false;
    rx_state_t state = STATE_WAIT_SYNC;

    while (1) {
        if (USART0.STATUS & USART_RXCIF_bm) {
            uint8_t b = USART0.RXDATAL;

            switch (state) {
                case STATE_WAIT_SYNC:
                    if (b == MR32_SYNC_BYTE) {
                        rx_buf[0] = b;
                        rx_idx = 1;
                        state = STATE_WAIT_MAGIC;
                    }
                    break;

                case STATE_WAIT_MAGIC:
                    if (b == MR32_MAGIC_BYTE) {
                        rx_buf[1] = b;
                        rx_idx = 2;
                        ignore_rest = false;
                        state = STATE_RECV_BODY;
                    } else {
                        state = STATE_WAIT_SYNC;
                    }
                    break;

                case STATE_RECV_BODY:
                    if (rx_idx == 2) {
                        if (b != MR32_MY_NODE_ID && b != MR32_ID_BROADCAST) {
                            ignore_rest = true;
                        }
                    }

                    if (!ignore_rest) {
                        rx_buf[rx_idx] = b;
                    }
                    rx_idx++;

                    if (rx_idx >= MR32_FRAME_LEN) {
                        if (!ignore_rest) {
                            uint16_t calc_crc = mr32_calculate_crc(&rx_buf[2], 28);
                            uint16_t recv_crc = (uint16_t)rx_buf[30] | ((uint16_t)rx_buf[31] << 8);

                            if (calc_crc == recv_crc) {
                                uint8_t cmd = rx_buf[4];
                                uint8_t src = rx_buf[3];
                                uint8_t seq = rx_buf[5];
                                uint8_t *payload = &rx_buf[6];

                                if (cmd == MR32_CMD_BOOT_WRITE_CHUNK) {
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint8_t chunk_idx = payload[2];

                                    // 自爆防止ガード: ブートローダー保護領域 (Pages 0..63) への書込を拒絶
                                    if (req_page < APP_START_PAGE || req_page >= FLASH_TOTAL_PAGES) {
                                        tx_buf[0] = MR32_SYNC_BYTE;
                                        tx_buf[1] = MR32_MAGIC_BYTE;
                                        tx_buf[2] = src;
                                        tx_buf[3] = MR32_MY_NODE_ID;
                                        tx_buf[4] = MR32_CMD_BOOT_WRITE_CHUNK;
                                        tx_buf[5] = seq;

                                        tx_buf[6] = MR32_STATUS_ERR_PARAM; // 0x03 (Protected)
                                        tx_buf[7] = (uint8_t)(req_page & 0xFF);
                                        tx_buf[8] = (uint8_t)(req_page >> 8);
                                        tx_buf[9] = chunk_idx;
                                        for (uint8_t i = 10; i < 30; i++) {
                                            tx_buf[i] = 0x00;
                                        }

                                        uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                        tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                        tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                        rs485_tx_start();
                                        for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                            putch(tx_buf[i]);
                                        }
                                        rs485_tx_end();

                                    } else {
                                        // 正常アプリケーション領域 (Pages 64..255)
                                        if (req_page != current_page) {
                                            current_page = req_page;
                                            chunk_mask = 0;
                                            memset(sram_page_buffer, 0xFF, FLASH_PAGE_SIZE);
                                        }

                                        if (chunk_idx < CHUNKS_PER_PAGE) {
                                            uint8_t offset = (chunk_idx << 4);
                                            memcpy(&sram_page_buffer[offset], &payload[3], CHUNK_DATA_SIZE);
                                            chunk_mask |= (1 << chunk_idx);
                                            VPORTB.OUT ^= PIN3_bm; // 白LED トグル
                                        }

                                        uint8_t status = MR32_STATUS_OK;
                                        uint16_t flash_crc = 0;

                                        if (chunk_mask == 0x0F) {
                                            // ★ 4チャンク揃った瞬間に物理 Flash 書き込み実行！ ★
                                            nvm_commit_page_hw(current_page);

                                            // 物理 Flash から直接 CRC16 を再計算して完全性を検証
                                            flash_crc = flash_calculate_page_crc(current_page);
                                            status = MR32_STATUS_PAGE_DONE;
                                            VPORTB.OUT ^= PIN2_bm; // 赤LED トグル (Flash 書込完了)
                                        }

                                        tx_buf[0] = MR32_SYNC_BYTE;
                                        tx_buf[1] = MR32_MAGIC_BYTE;
                                        tx_buf[2] = src;
                                        tx_buf[3] = MR32_MY_NODE_ID;
                                        tx_buf[4] = MR32_CMD_BOOT_WRITE_CHUNK;
                                        tx_buf[5] = seq;

                                        tx_buf[6] = status;
                                        tx_buf[7] = (uint8_t)(current_page & 0xFF);
                                        tx_buf[8] = (uint8_t)(current_page >> 8);
                                        tx_buf[9] = chunk_idx;
                                        tx_buf[10] = chunk_mask;
                                        tx_buf[11] = (uint8_t)(flash_crc & 0xFF);
                                        tx_buf[12] = (uint8_t)(flash_crc >> 8);
                                        for (uint8_t i = 13; i < 30; i++) {
                                            tx_buf[i] = 0x00;
                                        }

                                        uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                        tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                        tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                        rs485_tx_start();
                                        for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                            putch(tx_buf[i]);
                                        }
                                        rs485_tx_end();
                                    }

                                } else if (cmd == MR32_CMD_BOOT_READ_CHUNK) {
                                    // 物理 Flash から 16 バイト読み出し
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint8_t chunk_idx = payload[2];

                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_READ_CHUNK;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = (uint8_t)(req_page & 0xFF);
                                    tx_buf[8] = (uint8_t)(req_page >> 8);
                                    tx_buf[9] = chunk_idx;

                                    if (chunk_idx < CHUNKS_PER_PAGE && req_page < FLASH_TOTAL_PAGES) {
                                        flash_read_chunk(req_page, chunk_idx, &tx_buf[10]);
                                    } else {
                                        memset(&tx_buf[10], 0xFF, CHUNK_DATA_SIZE);
                                    }

                                    uint16_t flash_crc = (req_page < FLASH_TOTAL_PAGES) ? flash_calculate_page_crc(req_page) : 0;
                                    tx_buf[26] = (uint8_t)(flash_crc & 0xFF);
                                    tx_buf[27] = (uint8_t)(flash_crc >> 8);
                                    tx_buf[28] = 0x0F;
                                    tx_buf[29] = 0x00;

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                } else if (cmd == MR32_CMD_BOOT_CRC_CHECK) {
                                    // 物理 Flash ページの CRC16 照合
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint16_t flash_crc = (req_page < FLASH_TOTAL_PAGES) ? flash_calculate_page_crc(req_page) : 0xFFFF;

                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_CRC_CHECK;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = (uint8_t)(flash_crc & 0xFF);
                                    tx_buf[8] = (uint8_t)(flash_crc >> 8);
                                    tx_buf[9] = (uint8_t)(req_page & 0xFF);
                                    tx_buf[10] = (uint8_t)(req_page >> 8);
                                    for (uint8_t i = 11; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                } else if (cmd == MR32_CMD_BOOT_APP_EXEC) {
                                    // ★ ユーザーアプリケーション起動コマンド ★
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_APP_EXEC;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    for (uint8_t i = 7; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();

                                    // レスポンス送信完了後、ユーザーアプリ (0x1000) へ自動ジャンプ！
                                    launch_user_app();

                                } else if (cmd == MR32_CMD_BOOT_PING) {
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_PING;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = 0x16;
                                    tx_buf[8] = 0x16;
                                    tx_buf[9] = 16;
                                    tx_buf[10] = 64;
                                    for (uint8_t i = 11; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();
                                }
                            }
                        }
                        rx_idx = 0;
                        state = STATE_WAIT_SYNC;
                    }
                    break;
            }
        }
    }
}
