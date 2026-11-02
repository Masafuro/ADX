#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb_config.h"
#include "tusb.h"
#include "sysex_protocol.h"

// LED ピン (Pico H オンボード LED)
#define LED_PIN             25

// バッファサイズ定義
#define SYSEX_RX_BUF_SIZE   1024
#define SYSEX_TX_BUF_SIZE   1024

static uint8_t rx_buffer[SYSEX_RX_BUF_SIZE];
static size_t rx_index = 0;
static bool in_sysex = false;

static uint8_t tx_buffer[SYSEX_TX_BUF_SIZE];

// LED 点滅用
static uint32_t blink_interval_ms = 1000;
static uint32_t last_blink_time = 0;

void led_task(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_blink_time >= blink_interval_ms) {
        last_blink_time = now;
        gpio_xor_mask(1ul << LED_PIN);
    }
}

// SysEx パケット送信ヘルパー
void send_sysex_packet(uint8_t resp_cmd, uint8_t seq, const uint8_t *raw_payload, size_t payload_len, uint8_t status) {
    size_t tx_idx = 0;

    tx_buffer[tx_idx++] = MIDI_SYSEX_START;         // 0xF0
    tx_buffer[tx_idx++] = MIDI_MANUFACTURER_EXP;    // 0x7D
    tx_buffer[tx_idx++] = resp_cmd & 0x7F;          // レスポンスコマンド (0x41, 0x42 etc.)
    tx_buffer[tx_idx++] = seq & 0x7F;               // シーケンス番号
    tx_buffer[tx_idx++] = status & 0x7F;            // 実行ステータス (STATUS_OK等)

    // ペイロード長 (7bit x 2)
    tx_buffer[tx_idx++] = (payload_len >> 7) & 0x7F;
    tx_buffer[tx_idx++] = payload_len & 0x7F;

    // ペイロードを 7-to-8 パッキングして格納
    if (raw_payload && payload_len > 0) {
        size_t packed_len = sysex_pack_7to8(raw_payload, payload_len, &tx_buffer[tx_idx]);
        tx_idx += packed_len;
    }

    // チェックサム計算 (F0, F7を除くヘッダ〜ペイロード)
    uint8_t csum = sysex_calc_checksum(&tx_buffer[1], tx_idx - 1);
    tx_buffer[tx_idx++] = csum;

    tx_buffer[tx_idx++] = MIDI_SYSEX_END;           // 0xF7

    // TinyUSB MIDI ストリーム送信 (Cable 0)
    tud_midi_stream_write(0, tx_buffer, tx_idx);

    // 通信発生時に LED を即座にトグル
    gpio_xor_mask(1ul << LED_PIN);
}

// 受信した SysEx パケットの処理
void process_sysex(const uint8_t *packet, size_t len) {
    // 最小パケット長: F0, 7D, Cmd, Seq, LenH, LenL, Csum, F7 = 8バイト
    if (len < 8) return;

    if (packet[0] != MIDI_SYSEX_START || packet[len - 1] != MIDI_SYSEX_END) return;
    if (packet[1] != MIDI_MANUFACTURER_EXP) return; // メーカーID不一致

    uint8_t cmd  = packet[2];
    uint8_t seq  = packet[3];
    uint16_t declared_len = (packet[4] << 7) | packet[5];

    // チェックサム検証
    uint8_t expected_csum = packet[len - 2];
    uint8_t calc_csum = sysex_calc_checksum(&packet[1], len - 3);

    if (expected_csum != calc_csum) {
        // チェックサムエラー応答
        send_sysex_packet(cmd | RESP_FLAG, seq, NULL, 0, STATUS_ERR_CHECKSUM);
        return;
    }

    const uint8_t *packed_payload = &packet[6];
    size_t packed_payload_len = len - 8; // (ヘッダ6バイト + Csum1バイト + F7 1バイト)

    // ペイロードのアンパック
    static uint8_t raw_payload[512];
    size_t unpacked_len = 0;
    if (packed_payload_len > 0) {
        unpacked_len = sysex_unpack_7to8(packed_payload, packed_payload_len, raw_payload);
    }

    if (unpacked_len != declared_len) {
        send_sysex_packet(cmd | RESP_FLAG, seq, NULL, 0, STATUS_ERR_PAYLOAD_LEN);
        return;
    }

    // コマンド分岐
    switch (cmd) {
        case CMD_PING: {
            // PING 応答: デバイス名とファームウェアバージョン文字列を返す
            const char *pong_info = "ADX RPR4 Smart Probe | HW:Pico-H+2CH-RS485 | FW:v0.1.0-predev";
            send_sysex_packet(RESP_PONG, seq, (const uint8_t*)pong_info, strlen(pong_info), STATUS_OK);
            break;
        }

        case CMD_ECHO: {
            // 受信データをそのままエコーバック
            send_sysex_packet(RESP_ECHO, seq, raw_payload, unpacked_len, STATUS_OK);
            break;
        }

        default:
            // 不明なコマンド
            send_sysex_packet(cmd | RESP_FLAG, seq, NULL, 0, STATUS_ERR_INVALID_CMD);
            break;
    }
}

// MIDI 受信タスク
void midi_task(void) {
    while (tud_midi_available()) {
        uint8_t ch;
        uint32_t read_bytes = tud_midi_stream_read(&ch, 1);
        if (read_bytes == 0) break;

        if (ch == MIDI_SYSEX_START) {
            in_sysex = true;
            rx_index = 0;
            rx_buffer[rx_index++] = ch;
        } else if (in_sysex) {
            if (rx_index < SYSEX_RX_BUF_SIZE) {
                rx_buffer[rx_index++] = ch;
            } else {
                // バッファ溢れ
                in_sysex = false;
                rx_index = 0;
            }

            if (ch == MIDI_SYSEX_END) {
                in_sysex = false;
                process_sysex(rx_buffer, rx_index);
                rx_index = 0;
            }
        }
    }
}

int main(void) {
    // Pico 基本初期化
    stdio_init_all();

    // LED 設定
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1); // 起動時点灯

    // TinyUSB デバイス初期化 (Roothub Port 0)
    tusb_init();
    tud_init(BOARD_TUD_RHPORT);

    while (1) {
        tud_task(); // TinyUSB デバイスイベントループ
        midi_task();
        led_task();
    }

    return 0;
}

