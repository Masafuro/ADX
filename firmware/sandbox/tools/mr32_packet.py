#!/usr/bin/env python3
"""
ADX MR32 Protocol Packet Utility (Python 3)
SPDX-License-Identifier: MIT

32-byte Fixed-Length Protocol for MR32:
  [0]     : SYNC (0x55)
  [1]     : MAGIC (0xAD)
  [2]     : DST_ID
  [3]     : SRC_ID
  [4]     : PKT_CMD
  [5]     : SEQ_NUM
  [6..29] : PAYLOAD (24 Bytes)
  [30]    : CRC16 LSB
  [31]    : CRC16 MSB
"""

import struct
from typing import Tuple, Optional

# Protocol Constants
MR32_SYNC_BYTE  = 0x55
MR32_MAGIC_BYTE = 0xAD
MR32_FRAME_LEN  = 32
MR32_HEADER_LEN = 6
MR32_PAYLOAD_LEN= 24
MR32_CRC_LEN    = 2

# Standard Commands (memo/ADX_FIELD_NETWORK_SPECIFICATION_PROPOSAL.md)
CMD_BMC_ENTER_BOOT  = 0x01
CMD_BMC_BOOT_DONE   = 0x02
CMD_BOOT_PING       = 0x10
CMD_BOOT_WRITE_CHUNK= 0x11
CMD_BOOT_CRC_CHECK  = 0x13
CMD_APP_TELEMETRY   = 0x40

# Response Status Codes
STATUS_OK           = 0x00
STATUS_ERR_CRC      = 0x01
STATUS_ERR_TIMEOUT  = 0x02
STATUS_ERR_PARAM    = 0x03


def calculate_crc16_ccitt(data: bytes, init_val: int = 0xFFFF) -> int:
    """
    Calculate CRC-16-CCITT (Polynomial: 0x1021, Normal, Initial: 0xFFFF).
    Target: Byte 2 (DST_ID) to Byte 29 (end of PAYLOAD), total 28 bytes.
    """
    crc = init_val
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def build_mr32_frame(dst_id: int, src_id: int, cmd: int, seq_num: int, payload: bytes = b"") -> bytes:
    """
    Build a 32-byte fixed-length MR32 frame.
    Payload is padded with zeros or truncated to exactly 24 bytes.
    """
    if len(payload) < MR32_PAYLOAD_LEN:
        payload = payload + b"\x00" * (MR32_PAYLOAD_LEN - len(payload))
    else:
        payload = payload[:MR32_PAYLOAD_LEN]

    header = struct.pack("BBBBBB", MR32_SYNC_BYTE, MR32_MAGIC_BYTE, dst_id, src_id, cmd, seq_num)
    body = header[2:] + payload  # 28 bytes for CRC calculation
    crc = calculate_crc16_ccitt(body)

    # Frame: SYNC(1) + MAGIC(1) + DST(1) + SRC(1) + CMD(1) + SEQ(1) + PAYLOAD(24) + CRC_LSB(1) + CRC_MSB(1)
    frame = header + payload + struct.pack("<H", crc)
    assert len(frame) == MR32_FRAME_LEN
    return frame


def parse_mr32_frame(frame: bytes) -> Tuple[bool, Optional[dict], str]:
    """
    Parse and validate a 32-byte MR32 frame.
    Returns: (is_valid, parsed_dict, error_message)
    """
    if len(frame) != MR32_FRAME_LEN:
        return False, None, f"Invalid length: expected 32, got {len(frame)}"

    if frame[0] != MR32_SYNC_BYTE:
        return False, None, f"Invalid SYNC byte: 0x{frame[0]:02X} != 0x55"

    if frame[1] != MR32_MAGIC_BYTE:
        return False, None, f"Invalid MAGIC byte: 0x{frame[1]:02X} != 0xAD"

    expected_crc = calculate_crc16_ccitt(frame[2:30])
    received_crc = struct.unpack("<H", frame[30:32])[0]
    if expected_crc != received_crc:
        return False, None, f"CRC mismatch: expected 0x{expected_crc:04X}, got 0x{received_crc:04X}"

    parsed = {
        "sync": frame[0],
        "magic": frame[1],
        "dst_id": frame[2],
        "src_id": frame[3],
        "cmd": frame[4],
        "seq_num": frame[5],
        "payload": frame[6:30],
        "crc16": received_crc,
    }
    return True, parsed, "OK"


if __name__ == "__main__":
    # Self-test
    test_payload = b"ADX_MR32_TEST_PAYLOAD!!"
    frame = build_mr32_frame(dst_id=0x01, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=1, payload=test_payload)
    print(f"[Self-Test] Built Frame (hex): {frame.hex()}")
    ok, parsed, err = parse_mr32_frame(frame)
    assert ok, f"Self-test failed: {err}"
    print(f"[Self-Test] Parsed Frame: CMD=0x{parsed['cmd']:02X}, DST=0x{parsed['dst_id']:02X}, CRC=0x{parsed['crc16']:04X}")
    print("[Self-Test] All MR32 frame checks passed successfully.")
