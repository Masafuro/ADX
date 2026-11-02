#include "ln485_driver.h"
#include "pico/time.h"

bool ln485_tx_init(ln485_tx_t *dev, PIO pio, uint sm, uint pin_tx, uint baud) {
    if (!dev) return false;

    dev->pio = pio;
    dev->sm = sm;
    dev->pin_tx = pin_tx;
    dev->baud = baud;

    if (!pio_can_add_program(pio, &ln485_tx_program)) {
        return false;
    }

    dev->offset = pio_add_program(pio, &ln485_tx_program);
    ln485_tx_program_init(pio, sm, dev->offset, pin_tx, baud);

    return true;
}

void ln485_tx_send_break(ln485_tx_t *dev) {
    // bit 0 を 1 にして FIFO にプッシュ (BREAK コマンド)
    pio_sm_put_blocking(dev->pio, dev->sm, (uint32_t)LN485_CMD_BREAK);
}

void ln485_tx_send_byte(ln485_tx_t *dev, uint8_t b) {
    // bit 0 = 0 (通常UART), bit 1..8 にデータバイトを格納
    uint32_t val = ((uint32_t)b << 1) & 0x1FE;
    pio_sm_put_blocking(dev->pio, dev->sm, val);
}

void ln485_tx_send_frame(ln485_tx_t *dev, const uint8_t *data, size_t len, bool with_break) {
    if (with_break) {
        ln485_tx_send_break(dev);
        ln485_tx_send_byte(dev, LN485_SYNC_BYTE);
    }

    if (data && len > 0) {
        for (size_t i = 0; i < len; i++) {
            ln485_tx_send_byte(dev, data[i]);
        }
    }
}

uint8_t ln485_calc_checksum(uint8_t pid, const uint8_t *data, size_t len, bool enhanced) {
    uint16_t sum = enhanced ? pid : 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
        if (sum > 0xFF) sum -= 0xFF;
    }
    return (uint8_t)(~sum);
}

void ln485_tx_send_standard_frame(ln485_tx_t *dev, uint8_t pid, const uint8_t payload[LN485_PAYLOAD_SIZE]) {
    // 1. LIN BREAK
    ln485_tx_send_break(dev);

    // 2. SYNC (0x55)
    ln485_tx_send_byte(dev, LN485_SYNC_BYTE);

    // 3. PID / CMD
    ln485_tx_send_byte(dev, pid);

    // 4. Data Payload (8 Bytes)
    for (int i = 0; i < LN485_PAYLOAD_SIZE; i++) {
        ln485_tx_send_byte(dev, payload[i]);
    }

    // 5. Enhanced Checksum (PID + 8B Data)
    uint8_t chk = ln485_calc_checksum(pid, payload, LN485_PAYLOAD_SIZE, true);
    ln485_tx_send_byte(dev, chk);
}

void ln485_tx_wait_idle(ln485_tx_t *dev) {
    // 1. TX FIFO が空になるのを待つ
    while (!pio_sm_is_tx_fifo_empty(dev->pio, dev->sm)) {
        tight_loop_contents();
    }

    // 2. 最後の Stop bit 送出（約1〜2文字分）が完了するまで待機
    // 115200bps の場合、1文字 ≈ 87us
    sleep_us(180);
}
