#!/usr/bin/env python3
"""
WU-1: MR32 Protocol Benchmark & Fast Reject Verification Suite (Python 3)
SPDX-License-Identifier: MIT

Usage:
  python3 wu1_fast_reject_bench.py --port /dev/ttyUSB0 [--cycles 500] [--baud 115200]
  (Windows: python wu1_fast_reject_bench.py --port COM19)
"""

import sys
import time
import math
import random
import argparse
import struct
from typing import Optional, Tuple, List

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] 'pyserial' is not installed. Please run: pip install pyserial")
    sys.exit(1)

# Import common MR32 packet utility
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../../tools")))
try:
    from mr32_packet import (
        build_mr32_frame, parse_mr32_frame,
        MR32_FRAME_LEN, CMD_BOOT_PING, STATUS_OK
    )
except ImportError:
    # Fallback standalone implementation if path resolution fails
    def calculate_crc16(data: bytes) -> int:
        crc = 0xFFFF
        for b in data:
            crc ^= (b << 8)
            for _ in range(8):
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF if (crc & 0x8000) else (crc << 1) & 0xFFFF
        return crc

    def build_mr32_frame(dst_id: int, src_id: int, cmd: int, seq_num: int, payload: bytes = b"") -> bytes:
        payload = payload[:24] + b"\x00" * max(0, 24 - len(payload))
        hdr = struct.pack("BBBBBB", 0x55, 0xAD, dst_id, src_id, cmd, seq_num)
        crc = calculate_crc16(hdr[2:] + payload)
        return hdr + payload + struct.pack("<H", crc)

    def parse_mr32_frame(frame: bytes):
        if len(frame) != 32 or frame[0] != 0x55 or frame[1] != 0xAD:
            return False, None, "Invalid frame"
        calc_crc = calculate_crc16(frame[2:30])
        recv_crc = struct.unpack("<H", frame[30:32])[0]
        if calc_crc != recv_crc:
            return False, None, "CRC mismatch"
        return True, {
            "dst_id": frame[2], "src_id": frame[3], "cmd": frame[4], "seq_num": frame[5],
            "payload": frame[6:30], "crc16": recv_crc
        }, "OK"
    CMD_BOOT_PING = 0x10
    STATUS_OK = 0x00
    MR32_FRAME_LEN = 32


class Mr32Benchmarker:
    def __init__(self, port_name: str, baudrate: int = 115200, timeout: float = 0.1):
        self.port_name = port_name
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

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

    def flush(self):
        self.ser.reset_input_buffer()

    def send_and_receive(self, frame: bytes, timeout: float = 0.05) -> Tuple[Optional[bytes], float]:
        """Send 32B frame and wait for 32B response. Returns (response_bytes, rtt_ms)."""
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()
        self.ser.write(frame)
        self.ser.flush()

        # Read 32 bytes with deadline
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

    def test_single_ping(self, target_node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(f" [TEST 1] Single Ping Check (Target Node: 0x{target_node:02X})")
        print("=" * 65)
        frame = build_mr32_frame(dst_id=target_node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=1)
        resp, rtt_ms = self.send_and_receive(frame, timeout=0.1)

        if not resp:
            print(f"[FAIL] No response received from Node 0x{target_node:02X} (Timeout: {rtt_ms:.2f}ms)")
            return False

        valid, parsed, msg = parse_mr32_frame(resp)
        if not valid:
            print(f"[FAIL] Received invalid frame: {msg}")
            return False

        p = parsed["payload"]
        status = p[0]
        mcu_id = (p[1] << 8) | p[2]
        flash_kb = p[3]
        page_b = p[4]
        ping_cnt = p[5] | (p[6] << 8)

        print(f" [OK] Response received in {rtt_ms:.2f} ms!")
        print(f"      Status       : 0x{status:02X} ({'STATUS_OK' if status == 0 else 'ERROR'})")
        print(f"      MCU ID       : 0x{mcu_id:04X} (ATtiny{mcu_id:x})")
        print(f"      Flash Size   : {flash_kb} KB")
        print(f"      Page Size    : {page_b} Bytes")
        print(f"      Ping Counter : {ping_cnt}")
        print(f"      CRC-16       : 0x{parsed['crc16']:04X} (Verified OK)")
        return True

    def test_fast_reject(self, target_node: int = 0x01) -> bool:
        print("\n" + "=" * 65)
        print(" [TEST 2] Fast Reject & Silence Suite (Noise & Fuzzing Immunity)")
        print("=" * 65)
        all_passed = True

        # TC-1: Leading Noise (Bytes != 0x55)
        print(" TC-1: Sending 100 bytes of non-0x55 noise (Expect 100% Silence)...", end=" ", flush=True)
        self.flush()
        noise = bytes([random.choice([b for b in range(256) if b != 0x55]) for _ in range(100)])
        self.ser.write(noise)
        self.ser.flush()
        time.sleep(0.03)
        rx = self.ser.read(100)
        if len(rx) == 0:
            print("[PASS] Silence confirmed (0 bytes returned)")
        else:
            print(f"[FAIL] Leaked {len(rx)} bytes: {rx.hex()}")
            all_passed = False

        # TC-2: Bad Magic (0x55 followed by non-0xAD)
        print(" TC-2: Sending bad magic (0x55 0x00 ... Expect 100% Silence)...", end=" ", flush=True)
        self.flush()
        bad_magic = b"\x55\x00" + bytes([0xAA] * 30)
        self.ser.write(bad_magic)
        self.ser.flush()
        time.sleep(0.03)
        rx = self.ser.read(100)
        if len(rx) == 0:
            print("[PASS] Silence confirmed (0 bytes returned)")
        else:
            print(f"[FAIL] Leaked {len(rx)} bytes: {rx.hex()}")
            all_passed = False

        # TC-3: Other Node Destination (DST_ID = 0x02 != 0x01)
        print(" TC-3: Sending frame for Node 0x02 (Expect 100% Silence)...", end=" ", flush=True)
        self.flush()
        other_frame = build_mr32_frame(dst_id=0x02, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=10)
        self.ser.write(other_frame)
        self.ser.flush()
        time.sleep(0.03)
        rx = self.ser.read(100)
        if len(rx) == 0:
            print("[PASS] Silence confirmed (0 bytes returned)")
        else:
            print(f"[FAIL] Leaked {len(rx)} bytes: {rx.hex()}")
            all_passed = False

        # TC-4: Corrupted CRC
        print(" TC-4: Sending frame with inverted CRC (Expect 100% Silence)...", end=" ", flush=True)
        self.flush()
        good_frame = bytearray(build_mr32_frame(dst_id=target_node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=20))
        good_frame[30] ^= 0xFF  # Corrupt CRC
        good_frame[31] ^= 0xFF
        self.ser.write(bytes(good_frame))
        self.ser.flush()
        time.sleep(0.03)
        rx = self.ser.read(100)
        if len(rx) == 0:
            print("[PASS] Silence confirmed (0 bytes returned)")
        else:
            print(f"[FAIL] Leaked {len(rx)} bytes: {rx.hex()}")
            all_passed = False

        # TC-5: Fuzzing Recovery (1,000 random bytes -> Instant Ping recovery)
        print(" TC-5: Fuzzing with 1,000 random bytes -> Verify instant recovery...", end=" ", flush=True)
        self.flush()
        fuzz = bytes([random.randint(0, 255) for _ in range(1000)])
        self.ser.write(fuzz)
        self.ser.flush()
        time.sleep(0.05)
        self.flush()

        # Follow with immediate valid ping
        ping_frame = build_mr32_frame(dst_id=target_node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=99)
        resp, rtt_ms = self.send_and_receive(ping_frame, timeout=0.1)
        if resp and len(resp) == MR32_FRAME_LEN:
            print(f"[PASS] MCU healed instantly! Responded in {rtt_ms:.2f} ms")
        else:
            print("[FAIL] MCU failed to recover after fuzzing!")
            all_passed = False

        return all_passed

    def test_long_run_jitter(self, target_node: int = 0x01, cycles: int = 500, interval_ms: float = 20.0):
        print("\n" + "=" * 65)
        print(f" [TEST 3] Long-Run Stability & Jitter Test ({cycles} Cycles @ {self.baudrate} bps)")
        print(f"          Interval: {interval_ms:.1f} ms | Frame: 32B TX / 32B RX")
        print("=" * 65)

        rtt_list: List[float] = []
        success_count = 0
        timeout_count = 0

        t_bench_start = time.perf_counter()

        for seq in range(1, cycles + 1):
            frame = build_mr32_frame(dst_id=target_node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=(seq & 0xFF))
            resp, rtt_ms = self.send_and_receive(frame, timeout=0.08)

            if resp and len(resp) == MR32_FRAME_LEN:
                valid, _, _ = parse_mr32_frame(resp)
                if valid:
                    success_count += 1
                    rtt_list.append(rtt_ms)
                else:
                    timeout_count += 1
            else:
                timeout_count += 1

            if seq % max(1, cycles // 20) == 0 or seq == cycles:
                pct = (seq / cycles) * 100
                print(f" Progress: [{('=' * int(pct // 4)).ljust(25)}] {seq}/{cycles} ({pct:.1f}%) | Last RTT: {rtt_ms:.2f}ms", end="\r")

            time.sleep(interval_ms / 1000.0)

        t_bench_end = time.perf_counter()
        total_time = t_bench_end - t_bench_start
        print("\n" + "-" * 65)

        if not rtt_list:
            print("[FAIL] No valid responses collected during benchmark.")
            return

        # Statistical Calculations
        n = len(rtt_list)
        mean_rtt = sum(rtt_list) / n
        min_rtt = min(rtt_list)
        max_rtt = max(rtt_list)
        variance = sum((x - mean_rtt) ** 2 for x in rtt_list) / n
        jitter_sigma = math.sqrt(variance)

        print("                   MR32 STABILITY STATISTICAL REPORT")
        print("=" * 65)
        print(f" Total Probes Sent   : {cycles}")
        print(f" Successful Responses: {success_count} ({success_count / cycles * 100:.2f}%)")
        print(f" Failed / Timed Out  : {timeout_count} ({timeout_count / cycles * 100:.2f}%)")
        print(f" Total Elapsed Time  : {total_time:.2f} s")
        print("-" * 65)
        print(f" [1] 往復遅延 (RTT: Round-Trip Latency) [32B TX / 32B RX]")
        print(f"     ・平均値 (Mean RTT)  : {mean_rtt:6.2f} ms")
        print(f"     ・最小値 (Min RTT)   : {min_rtt:6.2f} ms")
        print(f"     ・最大値 (Max RTT)   : {max_rtt:6.2f} ms")
        print(f"     ・変動幅 (Range)     : {max_rtt - min_rtt:6.2f} ms")
        print(f"     ・標準偏差 (Jitter σ): {jitter_sigma:6.2f} ms  ★バス決定性指標")
        print("-" * 65)

        # Grade Assessment
        if success_count == cycles and jitter_sigma < 1.0:
            grade = "GRADE A+ (ROCK SOLID: Ultra-Low Jitter, Deterministic Sync)"
        elif success_count / cycles >= 0.99 and jitter_sigma < 2.5:
            grade = "GRADE A  (EXCELLENT: Highly Reliable)"
        elif success_count / cycles >= 0.95:
            grade = "GRADE B  (ACCEPTABLE: Occasional Jitter/Loss)"
        else:
            grade = "GRADE F  (UNSTABLE: High Loss/Jitter)"

        print(f" >>> OVERALL BUS STABILITY: {grade} <<<")
        print("=" * 65)


def list_serial_ports():
    ports = serial.tools.list_ports.comports()
    print("[DETECTED PORTS]")
    for p in ports:
        print(f"  - {p.device}: {p.description} (VID:PID = {p.vid:04X}:{p.pid:04X} if p.vid else 'N/A')")


def main():
    parser = argparse.ArgumentParser(description="WU-1 MR32 Benchmark & Fast Reject Test Suite")
    parser.add_argument("--port", "-p", type=str, help="RS-485 Serial Port (e.g. COM19, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--node", "-n", type=int, default=1, help="Target Node ID (default: 1)")
    parser.add_argument("--cycles", "-c", type=int, default=300, help="Cycles for stability test (default: 300)")
    parser.add_argument("--interval", "-i", type=float, default=20.0, help="Interval in ms (default: 20.0)")
    parser.add_argument("--list", "-l", action="store_true", help="List available serial ports")
    args = parser.parse_args()

    if args.list or not args.port:
        list_serial_ports()
        if not args.port:
            print("\nPlease specify a serial port using --port <PORT_NAME>")
            sys.exit(0)

    bench = Mr32Benchmarker(port_name=args.port, baudrate=args.baud)
    try:
        bench.connect()

        # Step 1: Single Ping
        if not bench.test_single_ping(target_node=args.node):
            print("\n[ABORT] Single ping failed. Please verify hardware wiring, baudrate, and node ID.")
            sys.exit(1)

        # Step 2: Fast Reject & Fuzzing
        fast_reject_ok = bench.test_fast_reject(target_node=args.node)

        # Step 3: Stability & Jitter
        bench.test_long_run_jitter(target_node=args.node, cycles=args.cycles, interval_ms=args.interval)

        print("\n[COMPLETE] WU-1 Verification Finished. You can copy the statistical report above into:")
        print("           firmware/sandbox/records/WU1_fast_reject_report.md")

    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Benchmark cancelled by user.")
    finally:
        bench.close()


if __name__ == "__main__":
    main()
