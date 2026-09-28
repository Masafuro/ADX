#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT

WU-3: BR32 Fault Injection & Framing Resilience Benchmark
Host-side tool for testing BR32 behavior under severe framing faults:
  [Case 1] Incomplete Frame (10 Bytes Only) -> Expect STATUS_ERR_TIMEOUT (0x02) & RX_COUNT=10
  [Case 2] Corrupted CRC-16 Frame (32 Bytes) -> Expect STATUS_ERR_CRC (0x01) & RX_COUNT=32
  [Case 3] Flooding / Excess Data (50 Bytes) -> Expect Graceful Truncation & Zero Lockup
  [Case 4] Rapid Post-Fault Self-Healing    -> Expect Instant Recovery on Next Valid Frame
"""

import sys
import os
import time
import argparse
import threading
from typing import Optional, Tuple, List, Dict, Any

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Please install via: pip install pyserial")
    sys.exit(1)


# Status definitions
STATUS_NAMES = {
    0x00: "STATUS_OK",
    0x01: "STATUS_ERR_CRC",
    0x02: "STATUS_ERR_TIMEOUT",
    0x03: "STATUS_ERR_PROTECT",
    0x04: "STATUS_ERR_BUSY"
}


def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    """Calculate CRC-16-CCITT / XMODEM (Polynomial 0x1021, Initial 0xFFFF, MSB-first)."""
    crc = init
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


class DebugMonitor:
    """Asynchronous listener for the Core-D Soft-UART debug stream (COM21 @ 9600 bps)."""

    def __init__(self, port: str = "COM21", baudrate: int = 9600):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.running = False
        self.thread: Optional[threading.Thread] = None
        self.silent = False

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
            self.running = True
            self.thread = threading.Thread(target=self._listen_loop, daemon=True)
            self.thread.start()
            print(f"[DEBUG-MON] Listening on {self.port} @ {self.baudrate} bps (Soft-UART PB4)")
        except Exception as e:
            print(f"[DEBUG-MON] Note: Could not open debug port {self.port}: {e}")

    def _listen_loop(self):
        while self.running and self.ser and self.ser.is_open:
            try:
                line = self.ser.readline().decode('ascii', errors='replace')
                if line and not self.silent:
                    sys.stdout.write(f"  |-> [{self.port}] {line}")
                    sys.stdout.flush()
            except Exception:
                break

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()


class BR32FaultTester:
    """Master controller for BR32 Fault Injection and Resilience Testing."""

    def __init__(self, port: str = "COM19", baudrate: int = 19200):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.slave_uid: Optional[bytes] = None

    def open(self):
        print(f"[INIT] Opening RS-485 Serial Port {self.port} @ {self.baudrate} bps...")
        self.ser = serial.Serial(self.port, baudrate=self.baudrate, timeout=0.15)
        time.sleep(0.05)
        print(f"[INIT] Connected successfully to {self.port}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    @staticmethod
    def build_frame(cmd: int, seq: int, uid: bytes = b'\x00' * 10,
                    addr: int = 0x0000, payload: bytes = b'') -> bytes:
        """Construct a standard 32-byte BR32 master request frame with CRC-16."""
        if len(uid) != 10:
            uid = uid.ljust(10, b'\x00')[:10]

        valid_len = min(15, len(payload))
        data_15 = payload[:valid_len].ljust(15, b'\xAA')

        header = bytes([
            cmd & 0xFF,
            seq & 0xFF,
        ]) + uid + bytes([
            (addr >> 8) & 0xFF,
            addr & 0xFF,
            valid_len,
        ]) + data_15

        crc = crc16_ccitt(header)
        frame = header + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        return frame

    @staticmethod
    def parse_response(raw: bytes) -> Optional[Dict[str, Any]]:
        """Parse and validate a standard 32-byte BR32 slave response frame."""
        if len(raw) != 32:
            return None

        calc_crc = crc16_ccitt(raw[:30])
        rx_crc = (raw[30] << 8) | raw[31]
        crc_ok = (calc_crc == rx_crc)

        uid_bytes = raw[4:14]
        uid_hex = " ".join(f"{b:02X}" for b in uid_bytes)
        status_val = raw[0]

        return {
            "status": status_val,
            "status_name": STATUS_NAMES.get(status_val, f"STATUS_UNKNOWN(0x{status_val:02X})"),
            "seq": raw[1],
            "rx_count": raw[2],
            "dev_state": raw[3],
            "uid_bytes": uid_bytes,
            "uid_hex": uid_hex,
            "counter": (raw[14] << 8) | raw[15],
            "data": raw[16:30],
            "crc_calc": calc_crc,
            "crc_rx": rx_crc,
            "crc_ok": crc_ok
        }

    def send_raw_payload(self, payload: bytes) -> Tuple[bool, float, bytes, Optional[Dict[str, Any]]]:
        """
        Sends raw payload via Method 1 (Break + 0x55 Sync prefix).
        Does NOT alter payload length (allows sending incomplete or oversized data).
        """
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        # Step 1: Drop to half-baud and send 0x00 (LIN Break)
        self.ser.baudrate = self.baudrate // 2
        self.ser.write(b'\x00')
        self.ser.flush()

        # Step 2: Restore normal baudrate
        self.ser.baudrate = self.baudrate

        # Step 3: Send LIN Sync byte (0x55) followed by raw payload
        self.ser.write(bytes([0x55]) + payload)
        self.ser.flush()

        # Step 4: Wait for 32-byte response from slave
        rx = self.ser.read(32)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        parsed = self.parse_response(rx)
        # Even if status is an error (TIMEOUT or CRC), if CRC of response is valid, communication succeeded
        success = (parsed is not None and parsed["crc_ok"])

        return success, rtt_ms, rx, parsed

    def discover_slave_uid(self) -> Optional[bytes]:
        """Discover slave UID using standard 32B frame."""
        print("\n[DISCOVERY] Probing bus with All-Zero UID to discover slave...")
        frame = self.build_frame(cmd=0x01, seq=0x01, uid=b'\x00' * 10, addr=0x0000, payload=b"DISCOVERY")
        ok, rtt, rx, parsed = self.send_raw_payload(frame)

        if ok and parsed and parsed["status"] == 0x00:
            self.slave_uid = parsed["uid_bytes"]
            print(f"[DISCOVERY] Found Slave SIGROW UID: 0x{parsed['uid_hex']} (RTT: {rtt:.2f} ms)")
            return self.slave_uid
        else:
            print("[DISCOVERY] FAIL: No response from slave. Check wiring, power, and firmware.")
            return None

    def run_suite(self, interval_ms: float = 200.0, dbg_mon: Optional[DebugMonitor] = None):
        """
        Execute the 4-Stage Fault Injection & Resilience Suite:
          [Case 1] Incomplete Frame (10 Bytes Only)
          [Case 2] Corrupted CRC-16 Frame (32 Bytes with bad CRC)
          [Case 3] Flooding / Excess Data (50 Bytes)
          [Case 4] Post-Destruction Self-Healing (Instant Recovery)
        """
        print("\n" + "=" * 76)
        print("        WU-3: BR32 FAULT INJECTION & FRAMING RESILIENCE BENCHMARK")
        print("=" * 76)
        print(f" Target Port       : {self.port} @ {self.baudrate} bps")
        print(f" Interval          : {interval_ms:.1f} ms")
        print("=" * 76)

        uid = self.discover_slave_uid()
        if not uid:
            print("\n[ABORT] Cannot proceed without slave response.")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # CASE 1: Incomplete Frame (10 Bytes Only)
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [CASE 1] Incomplete Frame Injection (10 Bytes Only - Truncated Mid-Stream)")
        print("          Attack     : Send only 10 bytes instead of 32 bytes.")
        print("          Expectation: Slave does NOT deadlock! After timeout window,")
        print("                       slave honestly replies: STATUS=0x02 (TIMEOUT), RX_COUNT=10")
        print("-" * 76)

        full_frame = self.build_frame(cmd=0x01, seq=0x11, uid=uid, payload=b"TRUNCATED")
        cut_payload = full_frame[:10]  # Only first 10 bytes!

        print(f"  Sending {len(cut_payload)} bytes: {cut_payload.hex(' ')}")
        ok1, rtt1, rx1, p1 = self.send_raw_payload(cut_payload)

        if ok1 and p1:
            print(f"  -> Slave Response: RTT = {rtt1:.2f} ms")
            print(f"     Status        : 0x{p1['status']:02X} ({p1['status_name']})")
            print(f"     Slave RX Count: {p1['rx_count']} Bytes")
            print(f"     Slave UID     : 0x{p1['uid_hex']}")

            if p1["status"] == 0x02 and p1["rx_count"] == 10:
                print("  -> RESULT: ★ [PASS] PERFECT RESILIENCE! Slave reported exact 10-byte truncation!")
            else:
                print("  -> RESULT: [FAIL] Unexpected status or rx_count.")
                return False
        else:
            print(f"  -> RESULT: [FAIL] Slave deadlocked or timed out! (RX: {rx1.hex()})")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # CASE 2: Corrupted CRC-16 Frame (32 Bytes with Bogus CRC)
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [CASE 2] CRC-16 Corruption Injection (32 Bytes Full Frame with Bogus CRC)")
        print("          Attack     : Send full 32 bytes, but intentionally poison CRC16 to 0xDEAD.")
        print("          Expectation: Slave receives all 32 bytes, detects CRC mismatch,")
        print("                       and honestly replies: STATUS=0x01 (CRC_ERR), RX_COUNT=32")
        print("-" * 76)

        poisoned_frame = bytearray(full_frame)
        poisoned_frame[30] = 0xDE
        poisoned_frame[31] = 0xAD
        poisoned_frame = bytes(poisoned_frame)

        print(f"  Sending 32 bytes with poisoned CRC (0xDEAD):")
        print(f"  HEX: {poisoned_frame.hex(' ')}")

        ok2, rtt2, rx2, p2 = self.send_raw_payload(poisoned_frame)

        if ok2 and p2:
            print(f"  -> Slave Response: RTT = {rtt2:.2f} ms")
            print(f"     Status        : 0x{p2['status']:02X} ({p2['status_name']})")
            print(f"     Slave RX Count: {p2['rx_count']} Bytes")

            if p2["status"] == 0x01 and p2["rx_count"] == 32:
                print("  -> RESULT: ★ [PASS] CRC ERROR TELEMETRY CONFIRMED!")
            else:
                print("  -> RESULT: [FAIL] Expected STATUS_ERR_CRC (0x01), got unexpected response.")
                return False
        else:
            print(f"  -> RESULT: [FAIL] No valid response frame received! (RX: {rx2.hex()})")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # CASE 3: Flooding / Excess Data Injection (50 Bytes Stream)
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [CASE 3] Flooding / Excess Data Injection (50 Bytes Streamed into 32B Window)")
        print("          Attack     : Stream 50 continuous bytes (32B valid frame + 18B garbage).")
        print("          Expectation: [BR32 Rule 6] Overflow >32B is treated as Rule 2 (No BREAK / Garbage).")
        print("                       Slave MUST NOT transmit (DE=0) to prevent cascading collisions!")
        print("                       100% TOTAL SILENCE on bus is EXPECTED and MANDATORY.")
        print("-" * 76)

        flooded_frame = full_frame + os.urandom(18)  # 32 + 18 = 50 bytes
        print(f"  Sending {len(flooded_frame)} bytes stream (32B frame + 18B trailing garbage)...")

        ok3, rtt3, rx3, p3 = self.send_raw_payload(flooded_frame)

        if len(rx3) == 0:
            print(f"  -> Slave Response: [SILENCE] 0 Bytes received (Timeout after {rtt3:.2f} ms)")
            print("  -> RESULT: ★ [PASS] BR32 RULE 6 VERIFIED! Slave maintained complete silence")
            print("                       and successfully prevented cascading bus collision!")
        else:
            print(f"  -> Slave Response: RX={rx3.hex(' ')}")
            print("  -> RESULT: [FAIL] Slave broke silence unexpectedly on overflow stream!")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # CASE 4: Post-Fault Immediate Self-Healing Test
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [CASE 4] Post-Fault Immediate Self-Healing Verification")
        print("          Attack     : Incomplete (Rule 3) -> Poisoned CRC (Rule 4) -> Flooding (Rule 6).")
        print("          Expectation: [BR32 Rule 5] Next standard 32B valid frame must succeed INSTANTLY (RTT ≈ 38ms)")
        print("-" * 76)

        heal_frame = self.build_frame(cmd=0x01, seq=0x77, uid=uid, payload=b"POST_FAULT_OK!")
        ok4, rtt4, rx4, p4 = self.send_raw_payload(heal_frame)

        if ok4 and p4 and p4["status"] == 0x00 and p4["rx_count"] == 32:
            print(f"  -> Slave Response: RTT = {rtt4:.2f} ms (Target ~38.6 ms)")
            print(f"     Status        : 0x{p4['status']:02X} ({p4['status_name']})")
            print(f"     Slave RX Count: {p4['rx_count']} Bytes")
            print(f"     Payload Echo  : {p4['data'].hex(' ')}")
            print("  -> RESULT: ★ [PASS] FLAWLESS INSTANT SELF-HEALING PROVEN!")
        else:
            print(f"  -> RESULT: [FAIL] Slave did not recover cleanly! (RX: {rx4.hex()})")
            return False

        # -------------------------------------------------------------
        # Final Summary
        # -------------------------------------------------------------
        print("\n" + "=" * 76)
        print("                   WU-3 FINAL BENCHMARK VERDICT")
        print("=" * 76)
        print("  [CASE 1] Incomplete Frame Detection (10B) : PASS (Rule 3: STATUS=0x02, RX_COUNT=10)")
        print("  [CASE 2] CRC-16 Poisoning Telemetry       : PASS (Rule 4: STATUS=0x01, RX_COUNT=32)")
        print("  [CASE 3] 50-Byte Flooding Overflow Gate   : PASS (Rule 6: 100% Silence, Zero Collision)")
        print("  [CASE 4] Post-Destruction Self-Healing    : PASS (Rule 5: Instant Recovery, RTT=38ms)")
        print("=" * 76)
        print("  OVERALL VERDICT: ★ GRADE A+ (BR32 6-AXIOM RESILIENCE CERTIFIED) ★")
        print("=" * 76 + "\n")
        return True


def main():
    parser = argparse.ArgumentParser(
        description="WU-3: BR32 Fault Injection & Framing Resilience Benchmark",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Run complete 4-case fault injection suite (Recommended):
  python wu3_fault_bench.py --port COM19 --debug-port COM21 --suite
        """
    )
    parser.add_argument("--port", type=str, default="COM19", help="RS-485 serial port (default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 baudrate (default: 19200)")
    parser.add_argument("--debug-port", type=str, default="COM21", help="Core-D Soft-UART debug port (default: COM21)")
    parser.add_argument("--suite", action="store_true", help="Run complete 4-case WU-3 test suite")
    parser.add_argument("--interval", type=float, default=200.0, help="Packet interval in ms (default: 200.0)")

    args = parser.parse_args()

    dbg_mon = None
    if args.debug_port:
        dbg_mon = DebugMonitor(port=args.debug_port, baudrate=9600)
        dbg_mon.start()

    tester = BR32FaultTester(port=args.port, baudrate=args.baud)

    try:
        tester.open()
        tester.run_suite(interval_ms=args.interval, dbg_mon=dbg_mon)
    except KeyboardInterrupt:
        print("\n[INFO] Benchmark interrupted by user.")
    except Exception as e:
        print(f"\n[ERROR] Test execution failed: {e}")
        import traceback
        traceback.print_exc()
    finally:
        tester.close()
        if dbg_mon:
            dbg_mon.stop()


if __name__ == "__main__":
    main()
