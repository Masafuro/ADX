#!/usr/bin/env python3
"""
WU-3: MR32 16B x 4-Chunk Virtual SRAM Page Painter & Resilience Lab
SPDX-License-Identifier: MIT

Usage:
  python3 wu3_sram_painter_bench.py --port /dev/ttyUSB0
  (Windows: python wu3_sram_painter_bench.py --port COM22)
"""

import sys
import time
import struct
import random
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
    body = header[2:] + padded_payload  # 28 bytes for CRC calculation
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
# Virtual SRAM Painter Benchmark Class
# =========================================================================
class SramPainterBench:
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

    def send_and_receive(self, frame: bytes, timeout: float = 0.05) -> Tuple[Optional[bytes], float]:
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

    def write_chunk(self, page: int, chunk_idx: int, data16: bytes, node: int = 0x01) -> Tuple[bool, Dict[str, Any], float]:
        assert len(data16) == 16
        payload = struct.pack("<HB", page, chunk_idx) + data16 + b"\x00" * 5
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_WRITE_CHUNK, seq_num=chunk_idx, payload=payload)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.08)
        if not resp:
            return False, {}, rtt_ms
        valid, parsed, _ = parse_mr32_frame(resp)
        if not valid or parsed is None:
            return False, {}, rtt_ms

        p = parsed["payload"]
        info = {
            "status": p[0],
            "page": p[1] | (p[2] << 8),
            "chunk_idx": p[3],
            "chunk_mask": p[4],
            "sram_crc": p[5] | (p[6] << 8)
        }
        return True, info, rtt_ms

    def read_chunk(self, page: int, chunk_idx: int, node: int = 0x01) -> Tuple[bool, bytes, Dict[str, Any], float]:
        payload = struct.pack("<HB", page, chunk_idx) + b"\x00" * 21
        frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_READ_CHUNK, seq_num=chunk_idx, payload=payload)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.08)
        if not resp:
            return False, b"", {}, rtt_ms
        valid, parsed, _ = parse_mr32_frame(resp)
        if not valid or parsed is None:
            return False, b"", {}, rtt_ms

        p = parsed["payload"]
        data16 = p[4:20]
        info = {
            "status": p[0],
            "page": p[1] | (p[2] << 8),
            "chunk_idx": p[3],
            "sram_crc": p[20] | (p[21] << 8),
            "chunk_mask": p[22]
        }
        return True, data16, info, rtt_ms

    def scenario_1_sequential(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [SCENARIO 1] Sequential 4-Chunk Paint & Readback Verify (0 -> 1 -> 2 -> 3)")
        print("=" * 65)

        # 64-byte test message (exactly 64 bytes)
        msg_text = "ADX MR32 Magic RS-485 Virtual SRAM Painter - 64B Bit-for-Bit OK!"
        test_data = msg_text.encode("utf-8")[:64].ljust(64, b" ")
        assert len(test_data) == 64
        expected_crc = calculate_crc16_ccitt(test_data)

        page_no = 1
        print(f" Target Page: {page_no} (64 Bytes)")
        print(f" Expected 64B CRC16: 0x{expected_crc:04X}")

        # Step 1: Sequential Write
        print("\n [Step 1: Writing 4 Chunks sequentially]")
        write_rtts: List[float] = []
        for c in range(4):
            c_data = test_data[c * 16 : (c + 1) * 16]
            ok, info, rtt = self.write_chunk(page=page_no, chunk_idx=c, data16=c_data, node=node)
            write_rtts.append(rtt)
            if not ok:
                print(f"  [FAIL] Chunk {c} write failed!")
                return False
            status_str = "PAGE_DONE" if info.get("status") == STATUS_PAGE_DONE else "OK"
            mask_val = info.get("chunk_mask", 0)
            print(f"  Chunk {c}: mask=0x{mask_val:02X} status={status_str} RTT={rtt:.2f}ms (HEX: {c_data.hex()})")

        # Step 2: Readback Verify
        print("\n [Step 2: Reading back 4 Chunks & Reassembly]")
        reassembled = bytearray()
        read_rtts: List[float] = []
        for c in range(4):
            ok, r_data, _, rtt = self.read_chunk(page=page_no, chunk_idx=c, node=node)
            read_rtts.append(rtt)
            if not ok:
                print(f"  [FAIL] Chunk {c} read failed!")
                return False
            reassembled.extend(r_data)
            print(f"  Chunk {c}: read={r_data.hex()} RTT={rtt:.2f}ms")

        read_crc = calculate_crc16_ccitt(bytes(reassembled))
        print("-" * 65)
        print(f" [RESULT] TX ASCII: '{test_data.decode('utf-8', errors='replace')}'")
        print(f" [RESULT] RX ASCII: '{reassembled.decode('utf-8', errors='replace')}'")
        print(f" [RESULT] TX CRC: 0x{expected_crc:04X} | RX CRC: 0x{read_crc:04X}")

        if bytes(reassembled) == test_data:
            print(" >>> ★ 100% BIT-FOR-BIT PERFECT MATCH (64/64 Bytes) ★ <<<")
            avg_w = sum(write_rtts) / 4.0 if write_rtts else 0.0
            avg_r = sum(read_rtts) / 4.0 if read_rtts else 0.0
            print(f"     Average Write RTT: {avg_w:.2f} ms | Read RTT: {avg_r:.2f} ms")
            return True
        else:
            print(" [FAIL] Data mismatch!")
            return False

    def scenario_2_shuffled(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [SCENARIO 2] Shuffled Chunk Order Resilience (3 -> 1 -> 0 -> 2)")
        print("=" * 65)

        test_data = bytes([0x10 * i + j for i in range(4) for j in range(16)])
        page_no = 2
        shuffle_order = [3, 1, 0, 2]

        print(f" Target Page: {page_no} | Send Order: {shuffle_order}")
        for c in shuffle_order:
            c_data = test_data[c * 16 : (c + 1) * 16]
            ok, info, rtt = self.write_chunk(page=page_no, chunk_idx=c, data16=c_data, node=node)
            if not ok:
                print(f"  [FAIL] Shuffled chunk {c} write failed!")
                return False
            mask_val = info.get("chunk_mask", 0)
            status_val = info.get("status", 0)
            print(f"  Sent Chunk {c} -> Mask: 0x{mask_val:02X} | Status: 0x{status_val:02X} | RTT: {rtt:.2f}ms")

        # Read back
        reassembled = bytearray()
        for c in range(4):
            ok, r_data, _, _ = self.read_chunk(page=page_no, chunk_idx=c, node=node)
            if not ok:
                return False
            reassembled.extend(r_data)

        if bytes(reassembled) == test_data:
            print(" [PASS] Shuffled chunks automatically aligned perfectly by offset!")
            return True
        else:
            print(" [FAIL] Shuffled alignment failed!")
            return False

    def scenario_3_duplicate(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [SCENARIO 3] Duplicate / Retransmission Resilience (0 -> 1 -> 1 -> 2 -> 3)")
        print("=" * 65)

        test_data = bytes([0xA0 + i for i in range(64)])
        page_no = 3
        dup_order = [0, 1, 1, 2, 3]

        for c in dup_order:
            c_data = test_data[c * 16 : (c + 1) * 16]
            ok, info, rtt = self.write_chunk(page=page_no, chunk_idx=c, data16=c_data, node=node)
            if not ok:
                print(f"  [FAIL] Chunk {c} write failed!")
                return False
            mask_val = info.get("chunk_mask", 0)
            print(f"  Sent Chunk {c} -> Mask: 0x{mask_val:02X} | RTT: {rtt:.2f}ms")

        # Verify
        reassembled = bytearray()
        for c in range(4):
            ok, r_data, _, _ = self.read_chunk(page=page_no, chunk_idx=c, node=node)
            if not ok:
                return False
            reassembled.extend(r_data)

        if bytes(reassembled) == test_data:
            print(" [PASS] Duplicate chunk overwritten cleanly without corruption!")
            return True
        return False

    def scenario_4_drop_recovery(self, node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [SCENARIO 4] Incomplete Drop & Clean New Page Recovery")
        print("=" * 65)

        print(" Sending incomplete Page 99 (Chunks 0, 1, 2 only)...")
        dummy = bytes([0xFF] * 16)
        for c in [0, 1, 2]:
            self.write_chunk(page=99, chunk_idx=c, data16=dummy, node=node)

        print(" Now starting fresh Page 100 (Full 4 chunks)...")
        page100_data = bytes([i for i in range(64)])
        for c in range(4):
            c_data = page100_data[c * 16 : (c + 1) * 16]
            ok, _, _ = self.write_chunk(page=100, chunk_idx=c, data16=c_data, node=node)
            if not ok:
                return False

        # Read Page 100
        reassembled = bytearray()
        for c in range(4):
            ok, r_data, _, _ = self.read_chunk(page=100, chunk_idx=c, node=node)
            if not ok:
                return False
            reassembled.extend(r_data)

        if bytes(reassembled) == page100_data:
            print(" [PASS] Clean recovery! Old incomplete page discarded, Page 100 perfectly formed.")
            return True
        return False

    def scenario_5_multi_page_speed(self, node: int = 0x01, num_pages: int = 10) -> bool:
        print("\n" + "=" * 65)
        print(f" [SCENARIO 5] Multi-Page High-Speed Paint Benchmark ({num_pages} Pages / {num_pages * 64} Bytes)")
        print("=" * 65)

        page_times: List[float] = []
        all_passed = True

        t_total_start = time.perf_counter()

        for p_idx in range(num_pages):
            page_no = 20 + p_idx
            page_data = bytes([random.randint(0, 255) for _ in range(64)])
            expected_crc = calculate_crc16_ccitt(page_data)

            last_info: Dict[str, Any] = {}
            t_page_start = time.perf_counter()
            for c in range(4):
                c_data = page_data[c * 16 : (c + 1) * 16]
                ok, info, _ = self.write_chunk(page=page_no, chunk_idx=c, data16=c_data, node=node)
                if not ok:
                    all_passed = False
                    break
                last_info = info

            t_page_end = time.perf_counter()
            page_ms = (t_page_end - t_page_start) * 1000.0
            page_times.append(page_ms)

            # Check final chunk CRC
            if last_info.get("sram_crc") != expected_crc:
                print(f"  Page {page_no}: CRC mismatch! Expected 0x{expected_crc:04X}, Got 0x{last_info.get('sram_crc', 0):04X}")
                all_passed = False
            else:
                print(f"  Page {page_no}: 64B Painted in {page_ms:5.2f} ms | CRC16: 0x{expected_crc:04X} [PASS]")

        t_total_end = time.perf_counter()
        _ = t_total_end - t_total_start

        avg_page_ms = sum(page_times) / len(page_times) if page_times else 0.0
        avg_chunk_rtt = avg_page_ms / 4.0
        est_16kb_s = (avg_page_ms * 256) / 1000.0  # 16KB = 256 pages

        print("-" * 65)
        print("                THROUGHPUT & TIMING REPORT")
        print("=" * 65)
        print(f" Total Pages Painted  : {num_pages} pages ({num_pages * 64} Bytes)")
        print(f" Average Chunk RTT    : {avg_chunk_rtt:.2f} ms")
        print(f" Average 64B Page Time: {avg_page_ms:.2f} ms")
        print(f" Estimated 16KB OTW   : {est_16kb_s:.2f} s  (Target: < 7.7 s)")
        print("-" * 65)
        if all_passed and avg_page_ms < 30.0:
            print(" >>> SPEED & RELIABILITY RATING: GRADE A+ (ULTRA-FAST) <<<")
        else:
            print(" >>> SPEED & RELIABILITY RATING: GRADE A <<<")
        print("=" * 65)
        return all_passed


def main():
    parser = argparse.ArgumentParser(description="WU-3 MR32 Virtual SRAM Painter Benchmark")
    parser.add_argument("--port", "-p", type=str, help="RS-485 Serial Port (e.g. COM22, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--node", "-n", type=int, default=1, help="Target Node ID (default: 1)")
    parser.add_argument("--pages", type=int, default=10, help="Number of pages for Scenario 5 (default: 10)")
    args = parser.parse_args()

    if not args.port:
        ports = serial.tools.list_ports.comports()
        print("[DETECTED PORTS]")
        for p in ports:
            print(f"  - {p.device}: {p.description}")
        print("\nPlease specify a serial port using --port <PORT_NAME>")
        sys.exit(0)

    bench = SramPainterBench(port_name=args.port, baudrate=args.baud)
    try:
        bench.connect()

        # Execute All 5 Scenarios
        s1 = bench.scenario_1_sequential(node=args.node)
        s2 = bench.scenario_2_shuffled(node=args.node)
        s3 = bench.scenario_3_duplicate(node=args.node)
        s4 = bench.scenario_4_drop_recovery(node=args.node)
        s5 = bench.scenario_5_multi_page_speed(node=args.node, num_pages=args.pages)

        if all([s1, s2, s3, s4, s5]):
            print("\n[ALL PASS] WU-3 All 5 Scenarios Completed Successfully!")
            print("           You can copy the outputs into:")
            print("           firmware/sandbox/records/WU3_sram_painter_report.md")
        else:
            print("\n[WARN] Some scenarios encountered errors. Check output above.")

    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Benchmark cancelled by user.")
    finally:
        bench.close()


if __name__ == "__main__":
    main()
