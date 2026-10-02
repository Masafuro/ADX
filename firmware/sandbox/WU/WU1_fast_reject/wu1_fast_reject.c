/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * WU-1: MR32 32-Byte Fixed-Length Frame Echo & Fast Reject Lab
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
 *       PB2: Red LED (Toggles on valid 32B frame round-trip)
 *       PB3: White LED (Fast Reject / Activity indicator)
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "../../M_firmware/inc/mr32.h"

#define MR32_MY_NODE_ID 0x01

// =========================================================================
// Channel 2: Soft-UART Debug Telemetry (PB4 TX @ 9,600 bps, 20 MHz)
// =========================================================================
static inline void dbg_delay_bit(void) {
    // 20MHz / 9600bps ≈ 2083 cycles.
    // _delay_loop_2(520) ≈ 2080 cycles.
    _delay_loop_2(520);
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

__attribute__((unused)) static void dbg_print_dec16(uint16_t val) {
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
// Channel 1: RS-485 Half-Duplex Operations (SP485EEN @ 115,200 bps)
// =========================================================================
static inline void rs485_tx_start(void) {
    // DE=1 (PA4), /RE=1 (PA7: 自爆受信エコー完全遮断)
    VPORTA.OUT |= (PIN4_bm | PIN7_bm);
    _delay_us(10); // トランシーバ安定化
}

static inline void putch(uint8_t ch) {
    while (!(USART0.STATUS & USART_DREIF_bm))
        ;
    USART0.TXDATAL = ch;
}

static inline void rs485_tx_end(void) {
    // 最後のストップビット送出完了を待機 (TXCIF)
    while (!(USART0.STATUS & USART_TXCIF_bm))
        ;
    USART0.STATUS = USART_TXCIF_bm;

    // ガード時間 T_guard (>= 150us)
    _delay_us(160);

    // DE=0, /RE=0 (受信モード復帰)
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);

    // 受信 FIFO の過渡エコーをフラッシュ
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }
}

// =========================================================================
// Main Loop & Fast Reject State Machine
// =========================================================================
typedef enum {
    STATE_WAIT_SYNC,    // Waiting for 0x55
    STATE_WAIT_MAGIC,   // Waiting for 0xAD
    STATE_RECV_BODY     // Receiving Bytes 2..31
} rx_state_t;

int main(void) {
    // 1. クロック設定: 内部高周波オシレータの分周を解除して 20MHz 駆動
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

    // 2. ピン入出力設定
    //    PA1(TXD)=OUT HIGH, PA2(RXD)=IN
    //    PA4(DE)=OUT LOW, PA7(/RE)=OUT LOW (受信モード初期化)
    VPORTA.DIR |= PIN1_bm | PIN4_bm | PIN7_bm;
    VPORTA.OUT |= PIN1_bm;
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR &= ~PIN2_bm;

    //    PB2(赤LED)=OUT LOW, PB3(白LED)=OUT LOW, PB4(Soft-UART TX)=OUT HIGH
    VPORTB.DIR |= PIN2_bm | PIN3_bm | PIN4_bm;
    VPORTB.OUT &= ~(PIN2_bm | PIN3_bm);
    VPORTB.OUT |= PIN4_bm;

    // 3. USART0 をオルタネートピン (PA1:TXD, PA2:RXD) にリダイレクト
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 4. USART0 ボーレート設定: 115,200 bps
    //    F_CPU = 20MHz: BAUD = (64 * 20000000) / (16 * 115200) ≈ 694 (0x02B6)
    //    F_CPU = 16MHz: BAUD = (64 * 16000000) / (16 * 115200) ≈ 555 (0x022B)
    if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
        USART0.BAUD = 555;
    } else {
        USART0.BAUD = 694;
    }
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc | USART_SBMODE_1BIT_gc;
    USART0.CTRLA = 0;
    USART0.CTRLB = USART_RXMODE_NORMAL_gc | USART_RXEN_bm | USART_TXEN_bm;

    // 5. 起動メッセージ出力
    dbg_print("\r\n=======================================================\r\n");
    dbg_print("   ADX Core-D WU-1: MR32 Fast Reject & Echo Probe\r\n");
    dbg_print("=======================================================\r\n");
    dbg_print(" [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED\r\n");
    dbg_print(" [DEVICE] ATtiny1616-MNR (Flash: 16KB, Page: 64B)\r\n");
    dbg_print(" [NODE ID] 0x"); dbg_print_hex8(MR32_MY_NODE_ID);
    dbg_print("\r\n [MR32] 115,200 bps 8N1, Magic=0x55 0xAD\r\n");
    dbg_print(" Listening for MR32 Frames...\r\n");

    uint8_t rx_buf[MR32_FRAME_LEN];
    uint8_t tx_buf[MR32_FRAME_LEN];
    uint8_t rx_idx = 0;
    bool ignore_rest = false;
    rx_state_t state = STATE_WAIT_SYNC;

    uint16_t ping_count = 0;
    uint16_t fast_reject_noise_count = 0;
    uint16_t fast_reject_other_node = 0;
    uint16_t crc_error_count = 0;

    // メイン受信ループ (ポーリング & 超高速破棄)
    while (1) {
        if (USART0.STATUS & USART_RXCIF_bm) {
            uint8_t b = USART0.RXDATAL;

            switch (state) {
                case STATE_WAIT_SYNC:
                    if (b == MR32_SYNC_BYTE) {
                        rx_buf[0] = b;
                        rx_idx = 1;
                        state = STATE_WAIT_MAGIC;
                    } else {
                        // 【Fast Reject 1】0x55 以外は 1 クロックで破棄
                        fast_reject_noise_count++;
                    }
                    break;

                case STATE_WAIT_MAGIC:
                    if (b == MR32_MAGIC_BYTE) {
                        rx_buf[1] = b;
                        rx_idx = 2;
                        ignore_rest = false;
                        state = STATE_RECV_BODY;
                    } else {
                        // 【Fast Reject 2】0xAD でなければ即リセット
                        fast_reject_noise_count++;
                        state = STATE_WAIT_SYNC;
                    }
                    break;

                case STATE_RECV_BODY:
                    // Byte 2 は DST_ID
                    if (rx_idx == 2) {
                        if (b != MR32_MY_NODE_ID && b != MR32_ID_BROADCAST) {
                            // 【Fast Reject 3】他ノード宛てはバッファリングせず読み飛ばし
                            ignore_rest = true;
                            fast_reject_other_node++;
                        }
                    }

                    if (!ignore_rest) {
                        rx_buf[rx_idx] = b;
                    }
                    rx_idx++;

                    // 32バイト満了判定
                    if (rx_idx >= MR32_FRAME_LEN) {
                        if (!ignore_rest) {
                            // CRC-16-CCITT 照合 (Byte 2 .. Byte 29 の 28 バイト)
                            uint16_t calc_crc = mr32_calculate_crc(&rx_buf[2], 28);
                            uint16_t recv_crc = (uint16_t)rx_buf[30] | ((uint16_t)rx_buf[31] << 8);

                            if (calc_crc == recv_crc) {
                                // 正常フレーム受領
                                uint8_t cmd = rx_buf[4];
                                uint8_t src = rx_buf[3];
                                uint8_t seq = rx_buf[5];

                                if (cmd == MR32_CMD_BOOT_PING) {
                                    ping_count++;
                                    VPORTB.OUT ^= PIN2_bm; // PB2 赤LED トグル

                                    // 32B 応答パケット作成
                                    tx_buf[0] = MR32_SYNC_BYTE;
                                    tx_buf[1] = MR32_MAGIC_BYTE;
                                    tx_buf[2] = src;              // DST: 要求元 (Host=0)
                                    tx_buf[3] = MR32_MY_NODE_ID;  // SRC: 自機 (0x01)
                                    tx_buf[4] = MR32_CMD_BOOT_PING;
                                    tx_buf[5] = seq;

                                    // ペイロード (24B)
                                    tx_buf[6] = MR32_STATUS_OK;
                                    tx_buf[7] = 0x16;             // MCU 型番 0x1616
                                    tx_buf[8] = 0x16;
                                    tx_buf[9] = 16;               // Flash サイズ 16 KB
                                    tx_buf[10] = 64;              // ページサイズ 64 B
                                    tx_buf[11] = (uint8_t)(ping_count & 0xFF);
                                    tx_buf[12] = (uint8_t)(ping_count >> 8);

                                    // 未使用領域ゼロパディング
                                    for (uint8_t i = 13; i < 30; i++) {
                                        tx_buf[i] = 0x00;
                                    }

                                    // CRC16 計算付与
                                    uint16_t resp_crc = mr32_calculate_crc(&tx_buf[2], 28);
                                    tx_buf[30] = (uint8_t)(resp_crc & 0xFF);
                                    tx_buf[31] = (uint8_t)(resp_crc >> 8);

                                    // RS-485 送信
                                    rs485_tx_start();
                                    for (uint8_t i = 0; i < MR32_FRAME_LEN; i++) {
                                        putch(tx_buf[i]);
                                    }
                                    rs485_tx_end();
                                }
                            } else {
                                // CRC エラー: 完全沈黙
                                crc_error_count++;
                            }
                        }
                        // 受信完了、初期状態へ復帰
                        state = STATE_WAIT_SYNC;
                    }
                    break;
            }
        }
    }
}
