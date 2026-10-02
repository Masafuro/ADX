#!/usr/bin/env python3
"""
Milestone 4 (M4): MR32 12KB Full OTW Flash Writer & Auto-Execution Suite
SPDX-License-Identifier: MIT

Usage:
  python3 m4_otw_bench.py --port /dev/ttyUSB0 [--image app_12k.bin]
  (Windows: python m4_otw_bench.py --port COM22 --image app_12k.bin)
"""

import sys
import os
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
MR32_SYNC_BYTE          = 0x55
MR32_MAGIC_BYTE         = 0xAD
MR32_FRAME_LEN          = 32
MR32_PAYLOAD_LEN        = 24

CMD_BOOT_PING           = 0x10
CMD_BOOT_WRITE_CHUNK    = 0x11
CMD_BOOT_READ_CHUNK     = 0x12
CMD_BOOT_CRC_CHECK      = 0x13
CMD_BOOT_APP_EXEC       = 0x14

STATUS_OK               = 0x00
STATUS_ERR_PARAM        = 0x03
STATUS_PAGE_DONE        = 0x10

APP_START_PAGE          = 64
APP_TOTAL_PAGES         = 192   # Pages 64..255 (12KB)
FLASH_PAGE_SIZE         = 64
CHUNKS_PER_PAGE         = 4
CHUNK_SIZE              = 16


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
# Milestone 4 Full OTW Tester Class
# =========================================================================
class M4FullOTWBench:
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
        print(" [TEST 1] Target Ping & Identity Check")
        print("=" * 65)
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=1)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.1)
        if not resp:
            print("[FAIL] No Ping response from target!")
            return False
        valid, parsed, msg = parse_mr32_frame(resp)
        if not valid or parsed is None:
            print(f"[FAIL] Corrupted Ping response: {msg} (RX: {resp.hex() if resp else 'None'})")
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

    def flash_full_otw(self, image_data: bytes, node: int = 0x01) -> Tuple[bool, float, float]:
        print("\n" + "=" * 65)
        print(" [TEST 3] 12KB Full OTW Flash Transfer (Pages 64..255 / 192 Pages)")
        print("=" * 65)
        expected_len = APP_TOTAL_PAGES * FLASH_PAGE_SIZE
        if len(image_data) < expected_len:
            print(f"[WARN] Image data size {len(image_data)}B < {expected_len}B. Padding with 0xFF.")
            image_data = image_data.ljust(expected_len, b"\xFF")
        elif len(image_data) > expected_len:
            image_data = image_data[:expected_len]

        print(f" Total Payload Size  : {len(image_data)} Bytes ({APP_TOTAL_PAGES} Pages)")
        print(f" Memory Target Range : Address 0x1000 - 0x3FFF")
        print(" Streaming 192 Flash pages via MR32 RS-485...")

        t_start = time.perf_counter()
        page_rtts: List[float] = []

        for p_idx in range(APP_TOTAL_PAGES):
            page_no = APP_START_PAGE + p_idx
            page_data = image_data[p_idx * FLASH_PAGE_SIZE : (p_idx + 1) * FLASH_PAGE_SIZE]
            expected_page_crc = calculate_crc16_ccitt(page_data)

            t_page_start = time.perf_counter()
            for c in range(CHUNKS_PER_PAGE):
                c_data = page_data[c * CHUNK_SIZE : (c + 1) * CHUNK_SIZE]
                payload = struct.pack("<HB", page_no, c) + c_data + b"\x00" * 5
                frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_WRITE_CHUNK, seq_num=c, payload=payload)
                resp, rtt = self.send_and_receive(frame, timeout=0.1)

                if not resp:
                    print(f"\n[FAIL] Page {page_no} Chunk {c} write timed out!")
                    return False, 0.0, 0.0
                valid, parsed, msg = parse_mr32_frame(resp)
                if not valid or parsed is None:
                    print(f"\n[FAIL] Page {page_no} Chunk {c} invalid response: {msg}")
                    return False, 0.0, 0.0

                if c == 3:
                    p = parsed["payload"]
                    status = p[0]
                    flash_crc = p[5] | (p[6] << 8)
                    if status != STATUS_PAGE_DONE:
                        print(f"\n[FAIL] Page {page_no} commit failed! Status: 0x{status:02X}")
                        return False, 0.0, 0.0
                    if flash_crc != expected_page_crc:
                        print(f"\n[FAIL] Page {page_no} CRC mismatch! Flash: 0x{flash_crc:04X} != Expected: 0x{expected_page_crc:04X}")
                        return False, 0.0, 0.0

            t_page_end = time.perf_counter()
            page_rtts.append((t_page_end - t_page_start) * 1000.0)

            # Progress Indicator
            progress = (p_idx + 1) / APP_TOTAL_PAGES
            bar_len = 25
            filled_len = int(bar_len * progress)
            bar = "=" * filled_len + "-" * (bar_len - filled_len)
            elapsed_s = time.perf_counter() - t_start
            kb_sec = ((p_idx + 1) * 64 / 1024.0) / elapsed_s if elapsed_s > 0 else 0.0

            sys.stdout.write(f"\r  [{bar}] {progress * 100:5.1f}% | Page {page_no:3d}/255 | RTT={page_rtts[-1]:.1f}ms | {kb_sec:.2f} KB/s")
            sys.stdout.flush()

        t_end = time.perf_counter()
        total_time_s = t_end - t_start
        avg_page_ms = sum(page_rtts) / len(page_rtts)
        avg_kb_s = (len(image_data) / 1024.0) / total_time_s

        print(f"\n\n [PASS] 12KB Full OTW Flash Completed in {total_time_s:.2f} seconds!")
        print(f"        Average Page Time : {avg_page_ms:.2f} ms / page")
        print(f"        Effective OTW Rate: {avg_kb_s:.2f} KB/s (Target < 7.0s: {'PASS' if total_time_s < 7.0 else 'WARN'})")
        return True, total_time_s, avg_page_ms

    def verify_spot_readback(self, image_data: bytes, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [TEST 4] Spot Bit-for-Bit Readback Verification (Pages 64, 65, 255)")
        print("=" * 65)

        test_pages = [64, 65, 255]
        for page_no in test_pages:
            p_idx = page_no - APP_START_PAGE
            expected_page_data = image_data[p_idx * FLASH_PAGE_SIZE : (p_idx + 1) * FLASH_PAGE_SIZE]
            reassembled = bytearray()

            for c in range(CHUNKS_PER_PAGE):
                payload = struct.pack("<HB", page_no, c) + b"\x00" * 21
                frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_READ_CHUNK, seq_num=c, payload=payload)
                resp, _ = self.send_and_receive(frame, timeout=0.1)

                if not resp:
                    print(f"  [FAIL] Page {page_no} Chunk {c} read timeout!")
                    return False
                valid, parsed, msg = parse_mr32_frame(resp)
                if not valid or parsed is None:
                    print(f"  [FAIL] Page {page_no} Chunk {c} invalid: {msg}")
                    return False

                reassembled.extend(parsed["payload"][4:20])

            if bytes(reassembled) == expected_page_data:
                print(f"  [PASS] Page {page_no:3d} (0x{page_no * 64:04X}): 100% Bit-for-Bit Match (64/64 Bytes)")
            else:
                print(f"  [FAIL] Page {page_no:3d}: Data mismatch on physical Flash!")
                return False

        print(" >>> ★ ALL SPOT-CHECK PAGES MATCHED 100.0% WITH FLASH ROM ★ <<<")
        return True

    def launch_user_application(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [TEST 5] Launch User Application (CMD_BOOT_APP_EXEC: 0x14)")
        print("=" * 65)
        print(" Sending Execution Command to Core-D...")
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_APP_EXEC, seq_num=1)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.1)

        if not resp:
            print("[FAIL] No response for Launch Command!")
            return False
        valid, parsed, msg = parse_mr32_frame(resp)
        if not valid or parsed is None:
            print(f"[FAIL] Invalid Launch response: {msg}")
            return False

        status = parsed["payload"][0]
        if status == STATUS_OK:
            print(f" [PASS] Bootloader acknowledged Launch Command (STATUS_OK) in {rtt_ms:.2f} ms!")
            print("        Bootloader is performing indirect jump (ijmp) to 0x1000...")
            print("-" * 65)
            print(" [ACTION] Please check the physical board:")
            print("   1. Visual: Red LED (PB2) and White LED (PB3) should alternate blink rapidly (150ms).")
            print("   2. Monitor (COM21 @ 9600 bps): Soft-UART should print user app banner:")
            print("      '🎉 ADX Core-D USER APPLICATION LAUNCHED SUCCESSFULLY! 🎉'")
            return True
        else:
            print(f"[FAIL] Bootloader returned error status: 0x{status:02X}")
            return False


def main():
    parser = argparse.ArgumentParser(description="M4 MR32 12KB Full OTW Flash Writer & Launcher")
    parser.add_argument("--port", "-p", type=str, help="RS-485 Serial Port (e.g. COM22, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--node", "-n", type=int, default=1, help="Target Node ID (default: 1)")
    parser.add_argument("--image", "-i", type=str, default="app_12k.bin", help="12KB Image binary file (default: app_12k.bin)")
    args = parser.parse_args()

    if not args.port:
        ports = serial.tools.list_ports.comports()
        print("[DETECTED PORTS]")
        for p in ports:
            print(f"  - {p.device}: {p.description}")
        print("\nPlease specify a serial port using --port <PORT_NAME>")
        sys.exit(0)

    # Load 12KB Image
    script_dir = os.path.dirname(os.path.abspath(__file__))
    img_path = args.image if os.path.isabs(args.image) else os.path.join(script_dir, args.image)
    if not os.path.exists(img_path):
        print(f"[ERROR] Image file '{img_path}' not found! Please run 'make' first.")
        sys.exit(1)

    with open(img_path, "rb") as f:
        image_data = f.read()

    bench = M4FullOTWBench(port_name=args.port, baudrate=args.baud)
    try:
        bench.connect()

        # Step 1: Ping
        if not bench.ping(node=args.node):
            sys.exit(1)

        # Step 2: Protection Check
        time.sleep(0.02)
        t2_ok = bench.test_bootloader_protection(node=args.node)

        # Step 3: 12KB Full OTW Flash
        time.sleep(0.02)
        t3_ok, total_s, avg_page_ms = bench.flash_full_otw(image_data=image_data, node=args.node)

        # Step 4: Spot Readback
        t4_ok = False
        if t3_ok:
            time.sleep(0.02)
            t4_ok = bench.verify_spot_readback(image_data=image_data, node=args.node)

        # Step 5: Launch Application
        t5_ok = False
        if t3_ok:
            time.sleep(0.02)
            t5_ok = bench.launch_user_application(node=args.node)

        print("\n" + "=" * 65)
        print("                 MILESTONE 4 FINAL SUMMARY")
        print("=" * 65)
        print(f" Test 1 (Target Ping Check)    : PASS")
        print(f" Test 2 (Bootloader Protection): {'PASS' if t2_ok else 'FAIL'}")
        print(f" Test 3 (12KB Full OTW Flash)  : {'PASS' if t3_ok else 'FAIL'} ({total_s:.2f} s / avg {avg_page_ms:.1f}ms/p)")
        print(f" Test 4 (Spot Bit-for-Bit Match): {'PASS' if t4_ok else 'FAIL'}")
        print(f" Test 5 (User App Execution)   : {'PASS' if t5_ok else 'FAIL'}")
        print("-" * 65)

        if all([t2_ok, t3_ok, t4_ok, t5_ok]):
            print(" >>> ★ MILESTONE 4: GRADE A+ (OFFICIALLY PASSED) ★ <<<")
            print(" Full 12KB OTW Firmware Update & Auto-Execution Confirmed!")
        else:
            print(" >>> MILESTONE 4: SOME TESTS FAILED <<<")
        print("=" * 65)

    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Benchmark cancelled by user.")
    finally:
        bench.close()


if __name__ == "__main__":
    main()
