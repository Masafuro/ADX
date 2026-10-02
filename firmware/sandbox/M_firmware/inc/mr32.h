/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * MR32 Protocol Definitions for AVR Microcontrollers (ATtiny1616 / ATtiny412)
 *
 * 32-Byte Fixed-Length Frame:
 *   [0]     : SYNC (0x55)
 *   [1]     : MAGIC (0xAD)
 *   [2]     : DST_ID
 *   [3]     : SRC_ID
 *   [4]     : PKT_CMD
 *   [5]     : SEQ_NUM
 *   [6..29] : PAYLOAD (24 Bytes)
 *   [30]    : CRC16 LSB
 *   [31]    : CRC16 MSB
 */

#ifndef MR32_H_
#define MR32_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- MR32 Frame Constants --- */
#define MR32_FRAME_LEN          32
#define MR32_HEADER_LEN         6
#define MR32_PAYLOAD_LEN        24
#define MR32_CRC_LEN            2

#define MR32_SYNC_BYTE          0x55
#define MR32_MAGIC_BYTE         0xAD

/* --- Node ID Definitions --- */
#define MR32_ID_HOST            0x00
#define MR32_ID_DEFAULT_NODE    0x01
#define MR32_ID_BROADCAST       0xFF

/* --- Standard Commands (Byte 4: PKT_CMD) --- */
/* 0x00 - 0x0F: BMC Base Management */
#define MR32_CMD_BMC_ENTER_BOOT 0x01
#define MR32_CMD_BMC_BOOT_DONE  0x02

/* 0x10 - 0x1F: MCU Bootloader */
#define MR32_CMD_BOOT_PING      0x10
#define MR32_CMD_BOOT_WRITE_CHUNK 0x11
#define MR32_CMD_BOOT_READ_CHUNK  0x12
#define MR32_CMD_BOOT_CRC_CHECK 0x13
#define MR32_CMD_BOOT_APP_EXEC  0x14

/* 0x40 - 0x7F: Messaging & Telemetry */
#define MR32_CMD_APP_TELEMETRY  0x40

/* --- Status Codes (Byte 0 of Response Payload) --- */
#define MR32_STATUS_OK          0x00
#define MR32_STATUS_ERR_CRC     0x01
#define MR32_STATUS_ERR_TIMEOUT 0x02
#define MR32_STATUS_ERR_PARAM   0x03
#define MR32_STATUS_PAGE_DONE   0x10

/* --- MR32 Frame Struct --- */
typedef struct __attribute__((packed)) {
    uint8_t sync;                       /* 0x55 */
    uint8_t magic;                      /* 0xAD */
    uint8_t dst_id;                     /* Destination Node ID */
    uint8_t src_id;                     /* Source Node ID */
    uint8_t cmd;                        /* Command Code */
    uint8_t seq_num;                    /* Sequence Number */
    uint8_t payload[MR32_PAYLOAD_LEN];  /* 24 Bytes Payload */
    uint8_t crc_lsb;                    /* CRC16-CCITT LSB */
    uint8_t crc_msb;                    /* CRC16-CCITT MSB */
} mr32_frame_t;

/* --- Fast CRC-16-CCITT Routine (Polynomial: 0x1021, Initial: 0xFFFF) --- */
static inline uint16_t mr32_crc16_update(uint16_t crc, uint8_t data) {
    crc ^= (uint16_t)data << 8;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ 0x1021;
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

static inline uint16_t mr32_calculate_crc(const uint8_t *buffer, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc = mr32_crc16_update(crc, buffer[i]);
    }
    return crc;
}

#ifdef __cplusplus
}
#endif

#endif /* MR32_H_ */
