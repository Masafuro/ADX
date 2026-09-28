#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT

WU-2: BR32 SIGROW Gate & UID Fuzzing / Silence Benchmark
Host-side tool for testing BR32 hardware-level UID filtering, complete bus silence,
and rapid self-healing under malicious/fuzzed traffic against ADX Core-D.

Key Test Suites:
  [Test 1] All-Zero (0x00*10) Broadcast / Self-Identification Probe -> PASS (Accept)
  [Test 2] Exact Match UID (Targeted Unicast) Probe                -> PASS (Accept)
  [Test 3] Malicious UID Silence Gate (Bit-flip, 0xFF*10, Fake, Garbage) -> PASS (100% Silence)
  [Test 4] Stress Fuzzing & Immediate Self-Healing (50~100 Cycles Fuzz -> Instant Recovery)
"""

import sys
import os
import time
import argparse
import threading
import random
from typing import Optional, Tuple, List, Dict, Any

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Please install via: pip install pyserial")
    sys.exit(1)


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
        self.last_log = ""

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
                if line:
                    self.last_log = line.strip()
                    if not self.silent:
                        sys.stdout.write(f"  |-> [{self.port}] {line}")
                        sys.stdout.flush()
            except Exception:
                break

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()


class BR32FuzzTester:
    """Master controller for BR32 SIGROW Gate and Fuzzing Benchmark."""

    def __init__(self, port: str = "COM19", baudrate: int = 19200):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.slave_uid: Optional[bytes] = None

    def open(self):
        print(f"[INIT] Opening RS-485 Serial Port {self.port} @ {self.baudrate} bps...")
        self.ser = serial.Serial(self.port, baudrate=self.baudrate, timeout=0.08)
        time.sleep(0.05)
        print(f"[INIT] Connected successfully to {self.port}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    @staticmethod
    def build_frame(cmd: int, seq: int, uid: bytes = b'\x00' * 10,
                    addr: int = 0x0000, payload: bytes = b'') -> bytes:
        """
        Construct a standard 32-byte BR32 master request frame with CRC-16.
        Format (32B):
          [0]    CMD (1B)
          [1]    SEQ (1B)
          [2..11] TARGET_SIGROW (10B)
          [12..13] ADDR (2B)
          [14]   LEN (1B)
          [15..29] DATA (15B)
          [30..31] CRC16 (2B)
        """
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
        """
        Parse and validate a standard 32-byte BR32 slave response frame (Specification 3.2).
        Format (32B):
          [0]    STATUS (0x00 = STATUS_OK)
          [1]    ECHO_SEQ
          [2]    RX_COUNT (32)
          [3]    DEV_STATE (0x01=Targeted, 0x00=Broadcast)
          [4..13] MY_SIGROW (10B Unique Serial ID)
          [14]   CUR_PAGE (Accepted Counter H)
          [15]   CUR_CHUNK (Accepted Counter L)
          [16..29] EXTRA (14B Payload Echo)
          [30..31] CRC16
        """
        if len(raw) != 32:
            return None

        calc_crc = crc16_ccitt(raw[:30])
        rx_crc = (raw[30] << 8) | raw[31]
        crc_ok = (calc_crc == rx_crc)

        uid_bytes = raw[4:14]
        uid_hex = " ".join(f"{b:02X}" for b in uid_bytes)

        return {
            "status": raw[0],
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

    def send_br32_frame(self, frame: bytes) -> Tuple[bool, float, bytes, Optional[Dict[str, Any]]]:
        """
        Sends a single 32-byte frame via Method 1 (Half-Baud Break + 0x55 Sync prefix).
        Returns: (success, rtt_ms, raw_rx, parsed_dict)
        """
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        # Step 1: Drop to half-baud and send 0x00 (LIN Break)
        self.ser.baudrate = self.baudrate // 2
        self.ser.write(b'\x00')
        self.ser.flush()

        # Step 2: Restore normal baudrate
        self.ser.baudrate = self.baudrate

        # Step 3: Send LIN Sync byte (0x55) followed by the 32-byte BR32 frame (Total: 33 bytes)
        self.ser.write(bytes([0x55]) + frame)
        self.ser.flush()

        # Step 4: Wait for 32-byte response from slave (timeout 80ms)
        rx = self.ser.read(32)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        parsed = self.parse_response(rx)
        success = (parsed is not None and parsed["crc_ok"] and parsed["seq"] == frame[1] and parsed["status"] == 0x00)

        return success, rtt_ms, rx, parsed

    def discover_slave_uid(self) -> Optional[bytes]:
        """Send an All-Zero broadcast frame to discover the slave's hardware UID."""
        print("\n[DISCOVERY] Probing bus with All-Zero UID to discover slave...")
        frame = self.build_frame(cmd=0x01, seq=0x01, uid=b'\x00' * 10, addr=0x0000, payload=b"DISCOVERY")
        ok, rtt, rx, parsed = self.send_br32_frame(frame)

        if ok and parsed:
            self.slave_uid = parsed["uid_bytes"]
            print(f"[DISCOVERY] Found Slave SIGROW UID: 0x{parsed['uid_hex']} (RTT: {rtt:.2f} ms)")
            return self.slave_uid
        else:
            print("[DISCOVERY] FAIL: No response from slave. Check wiring, power, and firmware.")
            return None

    def run_suite(self, fuzz_count: int = 50, interval_ms: float = 200.0, dbg_mon: Optional[DebugMonitor] = None):
        """
        Execute the comprehensive 4-Stage WU-2 Benchmark Suite:
          [1] All-Zero (Broadcast) Gate
          [2] Exact Match UID (Unicast) Gate
          [3] Malicious UID Silence Gate (4 types)
          [4] Stress Fuzzing & Rapid Self-Healing
        """
        print("\n" + "=" * 76)
        print("          WU-2: BR32 SIGROW GATE & UID FUZZING BENCHMARK SUITE")
        print("=" * 76)
        print(f" Target Port       : {self.port} @ {self.baudrate} bps")
        print(f" Test Interval     : {interval_ms:.1f} ms")
        print(f" Fuzzing Stress    : {fuzz_count} Malicious Frames")
        print("=" * 76)

        # Pre-requisite: Discover UID
        uid = self.discover_slave_uid()
        if not uid:
            print("\n[ABORT] Cannot proceed without discovering slave UID.")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # TEST 1: All-Zero UID Gate Verification (Should Respond)
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [TEST 1] All-Zero (0x00*10) Broadcast / Self-Identification Probe")
        print("          Expectation: Slave ACCEPTS and replies with 32B frame.")
        print("-" * 76)
        frame_zero = self.build_frame(cmd=0x01, seq=0x10, uid=b'\x00' * 10, payload=b"TEST1_ZERO")
        ok1, rtt1, rx1, p1 = self.send_br32_frame(frame_zero)
        if ok1 and p1:
            print(f"  -> RESULT: [PASS] Responded in {rtt1:.2f} ms | Status=0x{p1['status']:02X} DEV_STATE=0x{p1['dev_state']:02X}")
            print(f"     Slave UID Confirmed: 0x{p1['uid_hex']}")
        else:
            print(f"  -> RESULT: [FAIL] Expected response, got timeout / error! (RX={rx1.hex()})")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # TEST 2: Exact Match UID Gate Verification (Should Respond)
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [TEST 2] Exact Match UID (Targeted Unicast) Probe")
        print(f"          Target UID : 0x{' '.join(f'{b:02X}' for b in uid)}")
        print("          Expectation: Slave ACCEPTS and replies with 32B frame.")
        print("-" * 76)
        frame_exact = self.build_frame(cmd=0x01, seq=0x20, uid=uid, payload=b"TEST2_EXACT")
        ok2, rtt2, rx2, p2 = self.send_br32_frame(frame_exact)
        if ok2 and p2:
            print(f"  -> RESULT: [PASS] Responded in {rtt2:.2f} ms | Status=0x{p2['status']:02X} DEV_STATE=0x{p2['dev_state']:02X}")
        else:
            print(f"  -> RESULT: [FAIL] Expected response, got timeout / error! (RX={rx2.hex()})")
            return False

        time.sleep(interval_ms / 1000.0)

        # -------------------------------------------------------------
        # TEST 3: Malicious UID Silence Gate (Should Completely Ignore)
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(" [TEST 3] Malicious UID Silence Gate Verification (4 Attack Types)")
        print("          Expectation: Slave maintains 100% TOTAL SILENCE (0 bytes TX).")
        print("          Any returned byte or bus collision is a FAILURE.")
        print("-" * 76)

        # Generate attack variants
        # 3A: 1-Bit Inverted UID (flip bit 0 of last byte)
        uid_flip = bytearray(uid)
        uid_flip[-1] ^= 0x01
        uid_flip = bytes(uid_flip)

        # 3B: All 0xFF UID
        uid_ones = b'\xFF' * 10

        # 3C: Someone Else's Valid-looking UID
        uid_fake = bytes([0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01, 0x02, 0x03, 0x04, 0x05])

        # 3D: Random Noise UID
        uid_rand = os.urandom(10)

        attacks = [
            ("3-A: 1-Bit Inverted UID", uid_flip),
            ("3-B: All 0xFF UID      ", uid_ones),
            ("3-C: Foreign Device UID ", uid_fake),
            ("3-D: Random Garbage UID ", uid_rand),
        ]

        test3_all_pass = True
        for name, bad_uid in attacks:
            bad_frame = self.build_frame(cmd=0x01, seq=0x30, uid=bad_uid, payload=b"ATTACK_PROBE")
            ok, rtt, rx, p = self.send_br32_frame(bad_frame)
            bad_hex = " ".join(f"{b:02X}" for b in bad_uid)

            if len(rx) == 0:
                print(f"  [{name}] UID: 0x{bad_hex} -> [PASS] Perfect Silence (0 Bytes on Bus)")
            else:
                print(f"  [{name}] UID: 0x{bad_hex} -> [FAIL] Spoke unexpectedly! RX={rx.hex(' ')}")
                test3_all_pass = False
            time.sleep(interval_ms / 1000.0)

        if not test3_all_pass:
            print("\n[ABORT] TEST 3 Failed: Slave broke silence on unauthorized UID!")
            return False

        # -------------------------------------------------------------
        # TEST 4: Stress Fuzzing & Rapid Self-Healing Test
        # -------------------------------------------------------------
        print("\n" + "-" * 76)
        print(f" [TEST 4] Rapid Stress Fuzzing ({fuzz_count} Cycles) & Self-Healing Probe")
        print("          Step A: Blast randomized fuzzed UIDs at bus.")
        print("                  Slave must remain 100% silent throughout all attacks.")
        print("          Step B: Immediately send 1 valid frame.")
        print("                  Slave must instantly reply with ZERO lockup/deadlock.")
        print("-" * 76)

        fuzz_silence_count = 0
        fuzz_leaked_count = 0

        print(f"  Blasting {fuzz_count} fuzzed frames @ {interval_ms:.1f} ms interval...")
        for i in range(fuzz_count):
            rand_uid = os.urandom(10)
            # Ensure it is neither all-zero nor our slave UID
            if rand_uid == b'\x00' * 10 or rand_uid == uid:
                rand_uid = b'\xDE\xAD\xBE\xEF\x00\x00\x00\x00\x00\x01'

            f_fuzz = self.build_frame(cmd=0x01, seq=(i & 0xFF), uid=rand_uid, payload=b"FUZZ_STRESS")
            ok_f, rtt_f, rx_f, p_f = self.send_br32_frame(f_fuzz)

            if len(rx_f) == 0:
                fuzz_silence_count += 1
            else:
                fuzz_leaked_count += 1

            if (i + 1) % 10 == 0 or (i + 1) == fuzz_count:
                sys.stdout.write(f"\r  Progress: [{i + 1:3d}/{fuzz_count:3d}] | Silence: {fuzz_silence_count} | Leak: {fuzz_leaked_count}")
                sys.stdout.flush()

            time.sleep(interval_ms / 1000.0)

        print("")

        if fuzz_leaked_count > 0:
            print(f"  -> Step A: [FAIL] Leaked {fuzz_leaked_count} responses during fuzzing!")
            return False
        else:
            print(f"  -> Step A: [PASS] 100.0% Silence ({fuzz_silence_count}/{fuzz_count} frames ignored)")

        # Step B: Immediate Self-Healing Probe
        print("\n  Step B: Injecting 1x Targeted Frame immediately following fuzz attack...")
        heal_frame = self.build_frame(cmd=0x01, seq=0x99, uid=uid, payload=b"HEALING_CHECK!")
        ok_h, rtt_h, rx_h, p_h = self.send_br32_frame(heal_frame)

        if ok_h and p_h:
            print(f"  -> Step B: [PASS] INSTANT RECOVERY! RTT = {rtt_h:.2f} ms")
            print(f"             Slave accepted frame #0x{p_h['counter']:04X}, State=0x{p_h['dev_state']:02X}")
            print(f"             Zero deadlock. Complete self-healing proven.")
        else:
            print(f"  -> Step B: [FAIL] Slave did not recover! Deadlock / crash detected. (RX={rx_h.hex()})")
            return False

        # -------------------------------------------------------------
        # Final Summary
        # -------------------------------------------------------------
        print("\n" + "=" * 76)
        print("                   WU-2 FINAL BENCHMARK VERDICT")
        print("=" * 76)
        print("  [TEST 1] All-Zero Broadcast Gate       : PASS (100% Accepted)")
        print("  [TEST 2] Exact Match Targeted Gate     : PASS (100% Accepted)")
        print("  [TEST 3] Malicious UID Silence Gate    : PASS (100% Silence on 4 attack vectors)")
        print(f"  [TEST 4] Stress Fuzzing & Self-Healing : PASS ({fuzz_count}/{fuzz_count} Silence + Instant Recovery)")
        print("=" * 76)
        print("  OVERALL VERDICT: ★ GRADE A+ (BR32 SILENCE GATE CERTIFIED) ★")
        print("=" * 76 + "\n")
        return True

    def run_single(self, target_type: str = "valid", seq: int = 1):
        """Run a single test frame with specified target type."""
        uid = self.discover_slave_uid()
        if not uid and target_type != "zero":
            print("[ERROR] Cannot run without slave UID.")
            return

        if target_type == "zero":
            target_uid = b'\x00' * 10
            desc = "All-Zero Broadcast"
        elif target_type == "valid":
            target_uid = uid
            desc = "Exact Match Slave UID"
        elif target_type == "bitflip":
            target_uid = bytearray(uid)
            target_uid[-1] ^= 0x01
            target_uid = bytes(target_uid)
            desc = "1-Bit Inverted UID"
        elif target_type == "ones":
            target_uid = b'\xFF' * 10
            desc = "All 0xFF UID"
        elif target_type == "fake":
            target_uid = b'\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA'
            desc = "Fake Foreign UID"
        elif target_type == "random":
            target_uid = os.urandom(10)
            desc = "Random Garbage UID"
        else:
            print(f"[ERROR] Unknown target type: {target_type}")
            return

        print(f"\n[SINGLE PROBE] Type: {desc}")
        print(f"               Target UID: 0x{' '.join(f'{b:02X}' for b in target_uid)}")

        frame = self.build_frame(cmd=0x01, seq=seq, uid=target_uid, payload=b"SINGLE_PROBE")
        ok, rtt, rx, p = self.send_br32_frame(frame)

        if ok and p:
            print(f"[RECV] Status: PASS (Accepted & Valid Response)")
            print(f"       RTT: {rtt:.2f} ms | Slave UID: 0x{p['uid_hex']} | Status=0x{p['status']:02X}")
        elif len(rx) == 0:
            print(f"[RECV] Status: SILENCE (0 Bytes Received / Timeout after {rtt:.2f} ms)")
            print("       -> Slave correctly ignored unauthorized UID!")
        else:
            print(f"[RECV] Status: CORRUPT / UNEXPECTED (RX: {rx.hex(' ')})")


def main():
    parser = argparse.ArgumentParser(
        description="WU-2: BR32 SIGROW Gate & UID Fuzzing / Silence Benchmark",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Run complete 4-stage benchmark suite (Recommended):
  python wu2_fuzz_bench.py --port COM19 --debug-port COM21 --suite

  # Run suite with 100 fuzz cycles:
  python wu2_fuzz_bench.py --port COM19 --debug-port COM21 --suite --fuzz-count 100

  # Run single probe with 1-bit modified UID:
  python wu2_fuzz_bench.py --port COM19 --debug-port COM21 --single --target bitflip
        """
    )
    parser.add_argument("--port", type=str, default="COM19", help="RS-485 serial port (default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 baudrate (default: 19200)")
    parser.add_argument("--debug-port", type=str, default="COM21", help="Core-D Soft-UART debug port (default: COM21)")
    parser.add_argument("--suite", action="store_true", help="Run complete 4-stage WU-2 test suite")
    parser.add_argument("--fuzz-count", type=int, default=50, help="Number of stress fuzzing cycles in Test 4 (default: 50)")
    parser.add_argument("--interval", type=float, default=200.0, help="Packet interval in ms (default: 200.0)")
    parser.add_argument("--single", action="store_true", help="Run a single probe")
    parser.add_argument("--target", type=str, default="valid", choices=["zero", "valid", "bitflip", "ones", "fake", "random"],
                        help="Target UID type for single probe")

    args = parser.parse_args()

    # Start Debug Monitor
    dbg_mon = None
    if args.debug_port:
        dbg_mon = DebugMonitor(port=args.debug_port, baudrate=9600)
        dbg_mon.start()

    tester = BR32FuzzTester(port=args.port, baudrate=args.baud)

    try:
        tester.open()
        if args.suite:
            tester.run_suite(fuzz_count=args.fuzz_count, interval_ms=args.interval, dbg_mon=dbg_mon)
        elif args.single:
            tester.run_single(target_type=args.target)
        else:
            # Default action: run suite
            tester.run_suite(fuzz_count=args.fuzz_count, interval_ms=args.interval, dbg_mon=dbg_mon)
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
