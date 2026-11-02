#include "sysex_protocol.h"
#include <string.h>

/**
 * 7-to-8 パッキングアルゴリズム:
 * 7バイトの8bitデータ [d0..d6] を 8バイトの7bitデータ [msb_byte, b0..b6] に変換する。
 * msb_byte の bit 6..0 に d0..d6 の MSB を配置する。
 * 端数（7バイト未満）がある場合、余剰分の MSB も msb_byte に格納され、
 * 有効バイト数分だけ b0..b(rem-1) が続く。
 */
size_t sysex_pack_7to8(const uint8_t *raw, size_t raw_len, uint8_t *packed) {
    size_t out_idx = 0;
    size_t in_idx = 0;

    while (in_idx < raw_len) {
        size_t chunk_len = raw_len - in_idx;
        if (chunk_len > 7) {
            chunk_len = 7;
        }

        uint8_t msb_byte = 0;
        for (size_t i = 0; i < chunk_len; i++) {
            if (raw[in_idx + i] & 0x80) {
                msb_byte |= (uint8_t)(1 << (6 - i));
            }
        }

        packed[out_idx++] = msb_byte & 0x7F;

        for (size_t i = 0; i < chunk_len; i++) {
            packed[out_idx++] = raw[in_idx + i] & 0x7F;
        }

        in_idx += chunk_len;
    }

    return out_idx;
}

/**
 * 7-to-8 アンパッキングアルゴリズム:
 * 8バイトの7bitデータ [msb_byte, b0..b6] から 7バイトの8bitデータ [d0..d6] を復元する。
 */
size_t sysex_unpack_7to8(const uint8_t *packed, size_t packed_len, uint8_t *raw) {
    size_t in_idx = 0;
    size_t out_idx = 0;

    while (in_idx < packed_len) {
        uint8_t msb_byte = packed[in_idx++];
        size_t chunk_len = packed_len - in_idx;
        if (chunk_len > 7) {
            chunk_len = 7;
        }

        for (size_t i = 0; i < chunk_len; i++) {
            uint8_t b = packed[in_idx++];
            if (msb_byte & (1 << (6 - i))) {
                b |= 0x80;
            }
            raw[out_idx++] = b;
        }
    }

    return out_idx;
}

/**
 * 7bit チェックサム計算 (XOR)
 */
uint8_t sysex_calc_checksum(const uint8_t *data, size_t len) {
    uint8_t csum = 0;
    for (size_t i = 0; i < len; i++) {
        csum ^= (data[i] & 0x7F);
    }
    return csum & 0x7F;
}

