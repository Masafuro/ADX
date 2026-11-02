#ifndef SYSEX_PROTOCOL_H
#define SYSEX_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// MIDI SysEx 定数
#define MIDI_SYSEX_START            0xF0
#define MIDI_SYSEX_END              0xF7
#define MIDI_MANUFACTURER_EXP       0x7D  // Non-commercial / Educational / Experimental

// コマンド定義 (Host -> RPR4: 0x01〜0x7F, MSB=0)
#define CMD_PING                    0x01  // Ping要求 (ファーム情報取得)
#define CMD_ECHO                    0x02  // エコーバック要求
#define CMD_WRITE_PAGE              0x03  // Flashページ書き込み要求 (M4用)
#define CMD_READ_PAGE               0x04  // Flashページ読み出し要求 (M4用)
#define CMD_RESET_TARGET            0x05  // ターゲットリセット/BREAK送出 (M2/M4用)

// レスポンス定義 (RPR4 -> Host: 0x00〜0x7F 内で表現)
#define RESP_FLAG                   0x40  // レスポンス識別フラグ (bit 6)
#define RESP_PONG                   (CMD_PING | RESP_FLAG)        // 0x41
#define RESP_ECHO                   (CMD_ECHO | RESP_FLAG)        // 0x42
#define RESP_WRITE_PAGE             (CMD_WRITE_PAGE | RESP_FLAG)   // 0x43
#define RESP_READ_PAGE              (CMD_READ_PAGE | RESP_FLAG)    // 0x44
#define RESP_RESET_TARGET           (CMD_RESET_TARGET | RESP_FLAG) // 0x45

// ステータスコード (7bit)
#define STATUS_OK                   0x00
#define STATUS_ERR_CHECKSUM         0x01
#define STATUS_ERR_INVALID_CMD      0x02
#define STATUS_ERR_PAYLOAD_LEN      0x03
#define STATUS_ERR_TARGET_TIMEOUT   0x04
#define STATUS_ERR_TARGET_NAK       0x05

// 7-to-8 パッキング関数
size_t sysex_pack_7to8(const uint8_t *raw, size_t raw_len, uint8_t *packed);

// 7-to-8 アンパッキング関数
size_t sysex_unpack_7to8(const uint8_t *packed, size_t packed_len, uint8_t *raw);

// 7bit チェックサム計算 (XOR)
uint8_t sysex_calc_checksum(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // SYSEX_PROTOCOL_H

