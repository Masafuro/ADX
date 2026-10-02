/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-3: MR32 16B x 4-Chunk Virtual SRAM Page Painter Lab
 * Target MCU: Microchip ATtiny1616-MNR (QFN-20) on ADX Core-D
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
 *       PB2: Red LED (Toggles on 4-chunk 64B page completion)
 *       PB3: White LED (Toggles on each chunk received)
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "../../M_firmware/inc/mr32.h"

#define MR32_MY_NODE_ID       0x01
#define VIRTUAL_PAGE_SIZE     64
#define CHUNK_DATA_SIZE       16
#define CHUNKS_PER_PAGE       4

#define CMD_BOOT_READ_CHUNK   0x12

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
// Main Loop & Virtual SRAM Painter Logic
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
    dbg_print("   ADX Core-D WU-3: MR32 Virtual SRAM Page Painter\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED, PB3=ACT\r\n");
    dbg_print(" [DEVICE] ATtiny1616-MNR (Flash: 16KB, Page: 64B)\r\n");
    dbg_print(" [NODE ID] 0x"); dbg_print_hex8(MR32_MY_NODE_ID);
    dbg_print("\r\n [MR32] 115,200 bps 8N1, Magic=0x55 0xAD\r\n");
    dbg_print(" Virtual SRAM Buffer (64 Bytes) Ready.\r\n");
    dbg_print(" Listening for MR32 Chunk Frames...\r\n");

    // 仮想 SRAM ページバッファ (64 Bytes)
    uint8_t virtual_sram[VIRTUAL_PAGE_SIZE];
    memset(virtual_sram, 0, VIRTUAL_PAGE_SIZE);
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
                            ignore_rest = true; // Fast Reject 他ノード宛て
                        }
                    }

                    if (!ignore_rest) {
                        rx_buf[rx_idx] = b;
                    }
                    rx_idx++;

                    if (rx_idx >= MR32_FRAME_LEN) {
                        if (!ignore_rest) {
                            // CRC 照合
                            uint16_t calc_crc = mr32_calculate_crc(&rx_buf[2], 28);
                            uint16_t recv_crc = (uint16_t)rx_buf[30] | ((uint16_t)rx_buf[31] << 8);

                            if (calc_crc == recv_crc) {
                                uint8_t cmd = rx_buf[4];
                                uint8_t src = rx_buf[3];
                                uint8_t seq = rx_buf[5];
                                uint8_t *payload = &rx_buf[6];

                                if (cmd == MR32_CMD_BOOT_WRITE_CHUNK) {
                                    // チャンク書き込み処理
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint8_t chunk_idx = payload[2];

                                    // 新規ページ検出時はバッファ初期化
                                    if (req_page != current_page) {
                                        current_page = req_page;
                                        chunk_mask = 0;
                                        memset(virtual_sram, 0, VIRTUAL_PAGE_SIZE);
                                    }

                                    // チャンク格納 (オフセット = chunk_idx << 4)
                                    if (chunk_idx < CHUNKS_PER_PAGE) {
                                        uint8_t offset = (chunk_idx << 4);
                                        memcpy(&virtual_sram[offset], &payload[3], CHUNK_DATA_SIZE);
                                        chunk_mask |= (1 << chunk_idx);
                                        VPORTB.OUT ^= PIN3_bm; // 白LED トグル
                                    }

                                    // 64B 完成判定
                                    uint8_t status = MR32_STATUS_OK;
                                    uint16_t sram_crc = 0;
                                    if (chunk_mask == 0x0F) {
                                        // 全 4 チャンク揃った！
                                        status = MR32_STATUS_PAGE_DONE;
                                        sram_crc = mr32_calculate_crc(virtual_sram, VIRTUAL_PAGE_SIZE);
                                        VPORTB.OUT ^= PIN2_bm; // 赤LED トグル (ページ完了)
                                    }

                                    // 32B 応答パケット作成
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_WRITE_CHUNK;
                                    tx_buf[5] = seq;

                                    // Payload (24B)
                                    tx_buf[6] = status;
                                    tx_buf[7] = (uint8_t)(current_page & 0xFF);
                                    tx_buf[8] = (uint8_t)(current_page >> 8);
                                    tx_buf[9] = chunk_idx;
                                    tx_buf[10] = chunk_mask;
                                    tx_buf[11] = (uint8_t)(sram_crc & 0xFF);
                                    tx_buf[12] = (uint8_t)(sram_crc >> 8);
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

                                } else if (cmd == CMD_BOOT_READ_CHUNK) {
                                    // チャンク読み戻し処理 (ベリファイ用)
                                    uint16_t req_page = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
                                    uint8_t chunk_idx = payload[2];

                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = CMD_BOOT_READ_CHUNK;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = (uint8_t)(req_page & 0xFF);
                                    tx_buf[8] = (uint8_t)(req_page >> 8);
                                    tx_buf[9] = chunk_idx;

                                    // SRAM から 16 バイト読み出し
                                    if (chunk_idx < CHUNKS_PER_PAGE) {
                                        uint8_t offset = (chunk_idx << 4);
                                        memcpy(&tx_buf[10], &virtual_sram[offset], CHUNK_DATA_SIZE);
                                    } else {
                                        memset(&tx_buf[10], 0, CHUNK_DATA_SIZE);
                                    }

                                    // 64B バッファ全体の CRC16
                                    uint16_t sram_crc = mr32_calculate_crc(virtual_sram, VIRTUAL_PAGE_SIZE);
                                    tx_buf[26] = (uint8_t)(sram_crc & 0xFF);
                                    tx_buf[27] = (uint8_t)(sram_crc >> 8);
                                    tx_buf[28] = chunk_mask;
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
                                    // 64B バッファ CRC 照合
                                    uint16_t sram_crc = mr32_calculate_crc(virtual_sram, VIRTUAL_PAGE_SIZE);

                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;
                                    tx_buf[3] = MR32_MY_NODE_ID;
                                    tx_buf[4] = MR32_CMD_BOOT_CRC_CHECK;
                                    tx_buf[5] = seq;

                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = (uint8_t)(sram_crc & 0xFF);
                                    tx_buf[8] = (uint8_t)(sram_crc >> 8);
                                    tx_buf[9] = chunk_mask;
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

                                } else if (cmd == MR32_CMD_BOOT_PING) {
                                    // Ping 応答
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
                        state = STATE_WAIT_SYNC;
                    }
                    break;
            }
        }
    }
}
