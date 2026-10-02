#!/usr/bin/env python3
"""
Milestone 3 (M3): MR32 Physical Flash Page (64B) Write & Verification Suite
SPDX-License-Identifier: MIT

Usage:
  python3 m3_flash_bench.py --port /dev/ttyUSB0 [--page 20]
  (Windows: python m3_flash_bench.py --port COM22 --page 20)
"""

import sys
import time
import struct
import argparse
from typing import Optional, Tuple, List, Dict, Any, Union

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] 'pyserial' is not installed. Please run: pip install pyserial")
    sys.exit(1)

# =========================================================================
# Standalone MR32 Protocol Definitions & Helpers
# =========================================================================
MR32_SYNC_BYTE       = 0x55
MR32_MAGIC_BYTE      = 0xAD
MR32_FRAME_LEN       = 32
MR32_HEADER_LEN      = 6
MR32_PAYLOAD_LEN     = 24
MR32_CRC_LEN         = 2

CMD_BOOT_PING        = 0x10
CMD_BOOT_WRITE_CHUNK = 0x11
CMD_BOOT_READ_CHUNK  = 0x12
CMD_BOOT_CRC_CHECK   = 0x13

STATUS_OK            = 0x00
STATUS_ERR_PARAM     = 0x03
STATUS_PAGE_DONE     = 0x10


def calculate_crc16_ccitt(data: Union[bytes, bytearray], init_val: int = 0xFFFF) -> int:
    """Calculate CRC-16-CCITT (Polynomial: 0x1021, Initial: 0xFFFF)."""
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
    """Build a 32-byte fixed-length MR32 frame."""
    if len(payload) < MR32_PAYLOAD_LEN:
        padded_payload = payload + b"\x00" * (MR32_PAYLOAD_LEN - len(payload))
    else:
        padded_payload = payload[:MR32_PAYLOAD_LEN]

    header = struct.pack("BBBBBB", MR32_SYNC_BYTE, MR32_MAGIC_BYTE, dst_id, src_id, cmd, seq_num)
    body = header[2:] + padded_payload
    crc = calculate_crc16_ccitt(body)

    frame = header + padded_payload + struct.pack("<H", crc)
    return frame


def parse_mr32_frame(frame: bytes) -> Tuple[bool, Optional[Dict[str, Any]], str]:
    """Parse and validate a 32-byte MR32 frame."""
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


# =========================================================================
# Milestone 3 Flash Tester Class
# =========================================================================
class M3FlashBench:
    def __init__(self, port_name: str, baudrate: int = 115200, timeout: float = 0.1):
        self.port_name = port_name
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser: Optional[serial.Serial] = None

    def connect(self):
        print(f"[INIT] Opening RS-485 port {self.port_name} @ {self.baudrate} bps (8N1)...")
        self.ser = serial.Serial(
            port=self.port_name,
            baudrate=self.baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=self.timeout
        )
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        time.sleep(0.05)
        print("[INIT] Serial port opened successfully.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("[INFO] Port closed.")

    def send_and_receive(self, frame: bytes, timeout: float = 0.08) -> Tuple[Optional[bytes], float]:
        if not self.ser:
            return None, 0.0

        self.ser.reset_input_buffer()
        t_start = time.perf_counter()
        self.ser.write(frame)
        self.ser.flush()

        resp = b""
        deadline = t_start + timeout
        while len(resp) < MR32_FRAME_LEN and time.perf_counter() < deadline:
            chunk = self.ser.read(MR32_FRAME_LEN - len(resp))
            if chunk:
                resp += chunk

        t_end = time.perf_counter()
        rtt_ms = (t_end - t_start) * 1000.0

        if len(resp) == MR32_FRAME_LEN:
            return resp, rtt_ms
        return None, rtt_ms

    def ping(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [TEST 1] Single Ping Check")
        print("=" * 65)
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=1)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.1)
        if not resp:
            print("[FAIL] No Ping response from target!")
            return False
        valid, parsed, _ = parse_mr32_frame(resp)
        if not valid or parsed is None:
            print("[FAIL] Corrupted Ping response!")
            return False

        p = parsed["payload"]
        mcu_id = (p[1] << 8) | p[2]
        flash_kb = p[3]
        page_b = p[4]
        print(f" [PASS] Ping OK in {rtt_ms:.2f} ms | MCU: 0x{mcu_id:04X} (ATtiny{mcu_id:x}), Flash: {flash_kb}KB, Page: {page_b}B")
        return True

    def test_bootloader_protection(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [TEST 2] Bootloader Protection Guard (Attempt Write to Page 0)")
        print("=" * 65)

        dummy16 = b"\x00" * 16
        payload = struct.pack("<HB", 0, 0) + dummy16 + b"\x00" * 5
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_WRITE_CHUNK, seq_num=1, payload=payload)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.1)

        if not resp:
            print("[FAIL] No response for protected write attempt!")
            return False
        valid, parsed, msg = parse_mr32_frame(resp)
        if not valid or parsed is None:
            print(f"[FAIL] Protection response invalid: {msg} (RX: {resp.hex() if resp else 'None'})")
            return False

        status = parsed["payload"][0]
        if status == STATUS_ERR_PARAM:
            print(f" [PASS] Self-Programming Guard Activated! Target Page 0 safely REJECTED with STATUS_ERR_PARAM (0x03) in {rtt_ms:.2f} ms.")
            return True
        else:
            print(f" [FAIL] Protected page was NOT rejected! Status: 0x{status:02X}")
            return False

    def write_flash_page(self, page_no: int, test_data: bytes, node: int = 0x01) -> Tuple[bool, float, int]:
        print("\n" + "=" * 65)
        print(f" [TEST 3] Physical Flash Page Write (Page {page_no} / 64 Bytes)")
        print("=" * 65)
        assert len(test_data) == 64
        expected_crc = calculate_crc16_ccitt(test_data)
        print(f" Target Physical Page : {page_no} (Address: 0x{page_no * 64:04X} - 0x{(page_no + 1) * 64 - 1:04X})")
        print(f" Expected 64B CRC16   : 0x{expected_crc:04X}")

        t_start = time.perf_counter()
        write_rtts: List[float] = []

        last_info: Dict[str, Any] = {}
        for c in range(4):
            c_data = test_data[c * 16 : (c + 1) * 16]
            payload = struct.pack("<HB", page_no, c) + c_data + b"\x00" * 5
            frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_WRITE_CHUNK, seq_num=c, payload=payload)
            resp, rtt = self.send_and_receive(frame, timeout=0.1)
            write_rtts.append(rtt)

            if not resp:
                print(f"  [FAIL] Chunk {c} write timed out!")
                return False, 0.0, 0
            valid, parsed, msg = parse_mr32_frame(resp)
            if not valid or parsed is None:
                print(f"  [FAIL] Chunk {c} write response invalid: {msg} (RX: {resp.hex() if resp else 'None'})")
                return False, 0.0, 0

            p = parsed["payload"]
            info = {
                "status": p[0],
                "chunk_idx": p[3],
                "chunk_mask": p[4],
                "flash_crc": p[5] | (p[6] << 8)
            }
            last_info = info
            status_str = "PAGE_DONE (NVM Flash Written!)" if info["status"] == STATUS_PAGE_DONE else "OK"
            print(f"  Chunk {c}: mask=0x{info['chunk_mask']:02X} status={status_str} RTT={rtt:.2f}ms")

        t_end = time.perf_counter()
        total_page_ms = (t_end - t_start) * 1000.0

        if last_info.get("status") == STATUS_PAGE_DONE:
            reported_crc = last_info.get("flash_crc", 0)
            print(f"\n [PASS] Physical Flash Page {page_no} committed in {total_page_ms:.2f} ms!")
            print(f"        Flash Hardware Readback CRC16: 0x{reported_crc:04X} (Expected: 0x{expected_crc:04X})")
            if reported_crc == expected_crc:
                print("        CRC16 Match Confirmed immediately by MCU!")
                return True, total_page_ms, reported_crc
            else:
                print("        [WARN] CRC16 mismatch on page commit response!")
                return False, total_page_ms, reported_crc

        return False, total_page_ms, 0

    def verify_readback_flash(self, page_no: int, test_data: bytes, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(f" [TEST 4] Physical Flash Readback & Bit-for-Bit Verification")
        print("=" * 65)

        reassembled = bytearray()
        read_rtts: List[float] = []

        for c in range(4):
            payload = struct.pack("<HB", page_no, c) + b"\x00" * 21
            frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_READ_CHUNK, seq_num=c, payload=payload)
            resp, rtt = self.send_and_receive(frame, timeout=0.1)
            read_rtts.append(rtt)

            if not resp:
                print(f"  [FAIL] Chunk {c} read timed out!")
                return False
            valid, parsed, msg = parse_mr32_frame(resp)
            if not valid or parsed is None:
                print(f"  [FAIL] Chunk {c} read response invalid: {msg} (RX: {resp.hex() if resp else 'None'})")
                return False

            p = parsed["payload"]
            chunk_data = p[4:20]
            reassembled.extend(chunk_data)
            print(f"  Chunk {c}: Read 16B in {rtt:.2f}ms (HEX: {chunk_data.hex()})")

        read_crc = calculate_crc16_ccitt(bytes(reassembled))
        expected_crc = calculate_crc16_ccitt(test_data)

        print("-" * 65)
        print(f" [RESULT] TX ASCII: '{test_data.decode('utf-8', errors='replace')}'")
        print(f" [RESULT] RX ASCII: '{reassembled.decode('utf-8', errors='replace')}'")
        print(f" [RESULT] TX CRC: 0x{expected_crc:04X} | Physical Flash CRC: 0x{read_crc:04X}")

        if bytes(reassembled) == test_data:
            print(" >>> ★ 100% BIT-FOR-BIT PERFECT MATCH ON PHYSICAL FLASH (64/64 Bytes) ★ <<<")
            return True
        else:
            print(" [FAIL] Data mismatch between transmitted data and physical Flash!")
            return False

    def verify_flash_crc_check_cmd(self, page_no: int, expected_crc: int, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(f" [TEST 5] Hardware CRC Check Command (CMD_BOOT_CRC_CHECK: 0x13)")
        print("=" * 65)

        payload = struct.pack("<H", page_no) + b"\x00" * 22
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_CRC_CHECK, seq_num=1, payload=payload)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.1)

        if not resp:
            print("[FAIL] No CRC check response from target!")
            return False
        valid, parsed, msg = parse_mr32_frame(resp)
        if not valid or parsed is None:
            print(f" [FAIL] CRC check response invalid: {msg} (RX: {resp.hex() if resp else 'None'})")
            return False

        p = parsed["payload"]
        reported_crc = p[1] | (p[2] << 8)
        ret_page = p[3] | (p[4] << 8)

        print(f" Page: {ret_page} | Reported Flash CRC: 0x{reported_crc:04X} in {rtt_ms:.2f} ms")
        if reported_crc == expected_crc:
            print(" [PASS] Flash CRC Command Matched Expected Checksum Exactly!")
            return True
        else:
            print(f" [FAIL] CRC mismatch: Expected 0x{expected_crc:04X}, Got 0x{reported_crc:04X}")
            return False


def main():
    parser = argparse.ArgumentParser(description="M3 MR32 Physical Flash Page Write & Verification Suite")
    parser.add_argument("--port", "-p", type=str, help="RS-485 Serial Port (e.g. COM22, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--node", "-n", type=int, default=1, help="Target Node ID (default: 1)")
    parser.add_argument("--page", type=int, default=64, help="Target Application Flash Page (default: 64)")
    args = parser.parse_args()

    if not args.port:
        ports = serial.tools.list_ports.comports()
        print("[DETECTED PORTS]")
        for p in ports:
            print(f"  - {p.device}: {p.description}")
        print("\nPlease specify a serial port using --port <PORT_NAME>")
        sys.exit(0)

    bench = M3FlashBench(port_name=args.port, baudrate=args.baud)
    try:
        bench.connect()

        # Step 1: Ping
        if not bench.ping(node=args.node):
            sys.exit(1)

        # Step 2: Protection Check
        time.sleep(0.02)
        t2_ok = bench.test_bootloader_protection(node=args.node)

        # Step 3: Write Page
        time.sleep(0.02)
        msg_text = "ADX MR32 Milestone 3: Real Physical Flash Page Write is OK 2026!"
        test_data = msg_text.encode("utf-8")[:64].ljust(64, b" ")
        expected_crc = calculate_crc16_ccitt(test_data)

        t3_ok, page_ms, reported_crc = bench.write_flash_page(page_no=args.page, test_data=test_data, node=args.node)

        # Step 4: Readback Verify
        t4_ok = False
        if t3_ok:
            time.sleep(0.02)
            t4_ok = bench.verify_readback_flash(page_no=args.page, test_data=test_data, node=args.node)

        # Step 5: Flash CRC Check Command
        t5_ok = False
        if t3_ok:
            time.sleep(0.02)
            t5_ok = bench.verify_flash_crc_check_cmd(page_no=args.page, expected_crc=expected_crc, node=args.node)

        print("\n" + "=" * 65)
        print("                 MILESTONE 3 FINAL SUMMARY")
        print("=" * 65)
        print(f" Test 1 (Ping Check)           : PASS")
        print(f" Test 2 (Bootloader Protection): {'PASS' if t2_ok else 'FAIL'}")
        print(f" Test 3 (Physical Flash Write) : {'PASS' if t3_ok else 'FAIL'} ({page_ms:.2f} ms)")
        print(f" Test 4 (Bit-for-Bit Readback) : {'PASS' if t4_ok else 'FAIL'}")
        print(f" Test 5 (Hardware Flash CRC)   : {'PASS' if t5_ok else 'FAIL'}")
        est_16kb_s = (page_ms * 256) / 1000.0
        print(f" Estimated 16KB Full OTW Time  : {est_16kb_s:.2f} s")
        print("-" * 65)

        if all([t2_ok, t3_ok, t4_ok, t5_ok]):
            print(" >>> ★ MILESTONE 3: GRADE A+ (OFFICIALLY PASSED) ★ <<<")
            print(" You can copy the outputs into:")
            print(" firmware/sandbox/records/M3_single_page_report.md")
        else:
            print(" >>> MILESTONE 3: SOME TESTS FAILED <<<")
        print("=" * 65)

    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Benchmark cancelled by user.")
    finally:
        bench.close()


if __name__ == "__main__":
    main()
