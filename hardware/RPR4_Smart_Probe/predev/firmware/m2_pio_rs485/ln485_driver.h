#ifndef LN485_DRIVER_H
#define LN485_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hardware/pio.h"
#include "ln485.pio.h"

#ifdef __cplusplus
extern "C" {
#endif

// LN-485 定数
// コマンドエンコーディング (bit 0 がコマンド識別フラグ)
#define LN485_CMD_BREAK             0x001  // bit 0 = 1 (BREAK コマンド)
#define LN485_SYNC_BYTE             0x55   // LIN / LN-485 同期バイト

typedef struct {
    PIO pio;
    uint sm;
    uint offset;
    uint pin_tx;
    uint baud;
} ln485_tx_t;

// LN-485 送信ドライバ初期化
bool ln485_tx_init(ln485_tx_t *dev, PIO pio, uint sm, uint pin_tx, uint baud);

// LIN BREAK 単体送出 (13 bit Dominant Low + 1 bit Break Delimiter)
void ln485_tx_send_break(ln485_tx_t *dev);

// 1バイト通常 UART 送出 (bit 0 = 0, bit 1..8 = data)
void ln485_tx_send_byte(ln485_tx_t *dev, uint8_t b);

// LN-485 フレーム送出 (オプションで BREAK + SYNC を先頭に付与)
void ln485_tx_send_frame(ln485_tx_t *dev, const uint8_t *data, size_t len, bool with_break);

// 送信完了待機 (FIFO空 ＋ シフトレジスタ排出)
void ln485_tx_wait_idle(ln485_tx_t *dev);

#ifdef __cplusplus
}
#endif

#endif // LN485_DRIVER_H
