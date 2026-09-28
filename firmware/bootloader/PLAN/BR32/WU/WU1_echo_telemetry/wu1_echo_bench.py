#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT

WU-1: BR32 32-Byte Fixed Frame Echo & Heartbeat Telemetry Benchmark
Host-side tool for testing BR32 protocol frame round-trips against ADX Core-D.

Features:
  1. Full BR32 32-Byte Frame serialization with CRC-16-CCITT (Poly 0x1021, Init 0xFFFF).
  2. LIN Break injection via Method 1 (Half-Baud Trick).
  3. Real-time extraction and display of Slave's 10-byte SIGROW Unique ID.
  4. High-Capacity Bus Stability Long-Run Test with ΔT_resp jitter statistics & histogram.
  5. Optimal Interval Sweeper / Optimizer.
  6. Dual-Port Support: RS-485 (COM19) + Soft-UART Debug Monitor (COM21).
"""

import sys
import time
import argparse
import threading
import statistics
from typing import Optional, Tuple, List, Dict, Any

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Please install via: pip install pyserial")
    sys.exit(1)


def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    """Calculate CRC-16-CCITT (Polynomial 0x1021, Initial 0xFFFF)."""
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


class BR32EchoTester:
    """Master controller for BR32 32-Byte Fixed Frame Echo Testing."""

    def __init__(self, port: str = "COM19", baudrate: int = 19200):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None

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
        Format:
          [0]    CMD (1B)
          [1]    SEQ (1B)
          [2..11] TARGET_SIGROW (10B)
          [12]   ADDR_H (1B)
          [13]   ADDR_L (1B)
          [14]   LEN (1B)
          [15..29] DATA (15B)
          [30..31] CRC16 (2B)
        """
        if len(uid) != 10:
            uid = uid.ljust(10, b'\x00')[:10]

        valid_len = min(15, len(payload))
        data_15 = payload[:valid_len].ljust(15, b'\xAA')

        header = bytes([
            cmd & 0xFF,         # [0] CMD
            seq & 0xFF,         # [1] SEQ
        ]) + uid + bytes([      # [2..11] TARGET_SIGROW (10B)
            (addr >> 8) & 0xFF, # [12] ADDR_H
            addr & 0xFF,        # [13] ADDR_L
            valid_len,          # [14] LEN
        ]) + data_15            # [15..29] DATA (15B)

        crc = crc16_ccitt(header)
        frame = header + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        return frame

    @staticmethod
    def parse_response(raw: bytes) -> Optional[Dict[str, Any]]:
        """
        Parse and validate a standard 32-byte BR32 slave response frame.
        Format:
          [0]    STATUS (0x00 = STATUS_OK)
          [1]    ECHO_SEQ
          [2]    RX_COUNT (32)
          [3..12] SLAVE_SIGROW (10B Unique Serial ID)
          [13..14] ADDR / Frame Counter
          [15]   LEN
          [16..29] ECHO_DATA (14B)
          [30..31] CRC16
        """
        if len(raw) != 32:
            return None

        calc_crc = crc16_ccitt(raw[:30])
        rx_crc = (raw[30] << 8) | raw[31]
        crc_ok = (calc_crc == rx_crc)

        uid_bytes = raw[3:13]
        uid_hex = " ".join(f"{b:02X}" for b in uid_bytes)

        return {
            "status": raw[0],
            "seq": raw[1],
            "rx_count": raw[2],
            "uid_bytes": uid_bytes,
            "uid_hex": uid_hex,
            "addr": (raw[13] << 8) | raw[14],
            "len": raw[15],
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

        # Step 4: Wait for 32-byte response from slave
        rx = self.ser.read(32)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        parsed = self.parse_response(rx)
        success = (parsed is not None and parsed["crc_ok"] and parsed["seq"] == frame[1] and parsed["status"] == 0x00)

        return success, rtt_ms, rx, parsed

    def run_single_probe(self, seq: int = 1, test_payload: bytes = b"ADX_CORE_D_OK!"):
        """Sends a single BR32 frame and dumps the parsed slave response."""
        frame = self.build_frame(cmd=0x01, seq=seq, uid=b'\x00' * 10, addr=0x1234, payload=test_payload)
        print(f"\n[SEND] 32-Byte Frame (CMD=0x01, SEQ={seq}):")
        print(f"       HEX: {frame.hex(' ')}")

        ok, rtt, rx, parsed = self.send_br32_frame(frame)
        print(f"\n[RECV] Status: {'PASS (100% CRC Valid)' if ok else 'FAIL / TIMEOUT'}")
        print(f"       RTT: {rtt:.2f} ms")

        if rx:
            print(f"       Raw RX (32B): {rx.hex(' ')}")
        if parsed:
            print(f"       --------------------------------------------------------")
            print(f"       Slave SIGROW UID : 0x{parsed['uid_hex']}")
            print(f"       Response Status  : 0x{parsed['status']:02X} (0x00=OK)")
            print(f"       Response SEQ     : {parsed['seq']}")
            print(f"       Slave RX Count   : {parsed['rx_count']} Bytes")
            print(f"       Slave Frame Count: {parsed['addr']}")
            print(f"       Payload Len      : {parsed['len']}")
            print(f"       Payload Data     : {parsed['data'].hex(' ')} ('{parsed['data'][:parsed['len']].decode('ascii', errors='replace')}')")
            print(f"       CRC-16           : 0x{parsed['crc_rx']:04X} ({'OK' if parsed['crc_ok'] else 'MISMATCH'})")
            print(f"       --------------------------------------------------------")
        else:
            print("       <No valid 32-byte frame received>")

    def run_stability_test(self, count: int = 1000, interval_ms: float = 200.0, dbg_mon: Optional[DebugMonitor] = None):
        """
        High-Capacity Bus Stability Long-Run Test with full 32-byte frames.
        Measures Inter-Response Jitter, RTT statistics, sequence drops, and histogram.
        """
        if dbg_mon:
            dbg_mon.silent = True

        interval_sec = interval_ms / 1000.0
        print("\n" + "=" * 76)
        print("       BR32 BUS STABILITY LONG-RUN TEST (32-BYTE FULL FRAME TELEMETRY)")
        print("=" * 76)
        print(f" Target Probes     : {count:,} cycles")
        print(f" Planned Interval  : {interval_ms:.2f} ms ({1000.0/interval_ms:.1f} Hz)")
        print(f" Frame Size        : 32 Bytes TX / 32 Bytes RX (Fixed)")
        print(f" Method            : Method 1 (Half-Baud Trick)")
        print(f" Estimated Duration: {(count * interval_sec):.1f} seconds (~{(count * interval_sec)/60.0:.1f} minutes)")
        print("=" * 76 + "\n")

        success_count = 0
        failure_count = 0
        rtts: List[float] = []
        resp_times: List[float] = []
        inter_intervals: List[float] = []
        slave_uids = set()
        slave_counts: List[int] = []

        t_base = time.perf_counter()
        progress_step = max(1, count // 50)

        sys.stdout.write(" Progress: [")
        sys.stdout.flush()

        for i in range(1, count + 1):
            seq = i & 0xFF
            payload = bytes([(0xA0 + (i + k)) & 0xFF for k in range(14)])
            frame = self.build_frame(cmd=0x01, seq=seq, uid=b'\x00' * 10, addr=i, payload=payload)

            ok, rtt, rx, parsed = self.send_br32_frame(frame)
            t_recv = time.perf_counter()

            if ok and parsed:
                success_count += 1
                rtts.append(rtt)
                resp_times.append(t_recv)
                slave_uids.add(parsed["uid_hex"])
                slave_counts.append(parsed["addr"])
                if len(resp_times) >= 2:
                    inter_intervals.append((resp_times[-1] - resp_times[-2]) * 1000.0)
            else:
                failure_count += 1

            if i % progress_step == 0:
                sys.stdout.write("=")
                sys.stdout.flush()

            t_next = t_base + (i * interval_sec)
            sleep_duration = t_next - time.perf_counter()
            if sleep_duration > 0:
                time.sleep(sleep_duration)

        sys.stdout.write("] Done!\n\n")

        if dbg_mon:
            dbg_mon.silent = False

        # --- Comprehensive Statistical Analysis ---
        success_rate = (success_count / count) * 100.0
        rtt_mean = statistics.mean(rtts) if rtts else 0.0
        rtt_min = min(rtts) if rtts else 0.0
        rtt_max = max(rtts) if rtts else 0.0
        rtt_stdev = statistics.stdev(rtts) if len(rtts) > 1 else 0.0

        if inter_intervals:
            int_mean = statistics.mean(inter_intervals)
            int_min = min(inter_intervals)
            int_max = max(inter_intervals)
            int_range = int_max - int_min
            int_variance = statistics.variance(inter_intervals) if len(inter_intervals) > 1 else 0.0
            int_stdev = statistics.stdev(inter_intervals) if len(inter_intervals) > 1 else 0.0
        else:
            int_mean = int_min = int_max = int_range = int_variance = int_stdev = 0.0

        seq_miss = 0
        if len(slave_counts) >= 2:
            for k in range(1, len(slave_counts)):
                if slave_counts[k] != slave_counts[k - 1] + 1:
                    seq_miss += 1

        if success_rate >= 99.9 and int_stdev <= 1.0:
            rating = "GRADE A+ (ROCK SOLID: Ultra-Low Jitter, Flawless Sync)"
        elif success_rate >= 99.0 and int_stdev <= 2.0:
            rating = "GRADE A  (EXCELLENT: Production-Ready Stability)"
        elif success_rate >= 95.0 and int_stdev <= 5.0:
            rating = "GRADE B  (ACCEPTABLE: Tolerable Jitter)"
        else:
            rating = "GRADE C  (DEGRADED: High Jitter or Loss Detected)"

        print("=" * 76)
        print("                  BR32 BUS STABILITY STATISTICAL REPORT")
        print("=" * 76)
        print(f" Total Probes Sent   : {count:,}")
        print(f" Successful Responses: {success_count:,} ({success_rate:.2f}%)")
        print(f" Failed / Timed Out  : {failure_count:,} ({(failure_count/count)*100.0:.2f}%)")
        print(f" Slave Sequence Drops: {seq_miss} drops")
        if slave_uids:
            print(f" Identified Slaves   : {len(slave_uids)} device(s) [UID: {', '.join(slave_uids)}]")
        print("-" * 76)
        print(f" [1] 応答間隔統計 (Inter-Response Interval: ΔT_resp) [Target: {interval_ms:.2f} ms]")
        print(f"     ・平均値 (Mean)      : {int_mean:>7.2f} ms  (理想値との差: {int_mean - interval_ms:>+5.2f} ms)")
        print(f"     ・最小値 (Min)       : {int_min:>7.2f} ms")
        print(f"     ・最大値 (Max)       : {int_max:>7.2f} ms")
        print(f"     ・変動幅 (Range)     : {int_range:>7.2f} ms  (Max - Min)")
        print(f"     ・分散   (Variance σ²): {int_variance:>7.4f} ms²")
        print(f"     ・標準偏差 (Jitter σ): {int_stdev:>7.2f} ms  ★バス健全性コア指標")
        print("-" * 76)
        print(" [2] 往復遅延統計 (Round-Trip Latency: RTT) [32 Bytes TX / 32 Bytes RX]")
        print(f"     ・平均値 (Mean)      : {rtt_mean:>7.2f} ms")
        print(f"     ・最小値 (Min)       : {rtt_min:>7.2f} ms")
        print(f"     ・最大値 (Max)       : {rtt_max:>7.2f} ms")
        print(f"     ・標準偏差 (Jitter σ): {rtt_stdev:>7.2f} ms")
        print("-" * 76)

        if len(inter_intervals) >= 10:
            print(" [3] 応答間隔分布ヒストグラム (Interval Distribution Histogram)")
            num_bins = 10
            bin_width = max(0.1, (int_max - int_min) / num_bins)
            bins = [0] * num_bins
            for val in inter_intervals:
                idx = min(int((val - int_min) / bin_width), num_bins - 1)
                bins[idx] += 1

            max_count = max(bins) if bins else 1
            for b in range(num_bins):
                b_start = int_min + b * bin_width
                b_end = b_start + bin_width
                bar_len = int((bins[b] / max_count) * 35)
                bar_str = "█" * bar_len
                pct = (bins[b] / len(inter_intervals)) * 100.0
                print(f"     [{b_start:>6.2f} ~ {b_end:>6.2f} ms]: {bins[b]:>4} ({pct:>5.1f}%) {bar_str}")
            print("-" * 76)

        print(f" >>> OVERALL BUS STABILITY: {rating} <<<")
        print("=" * 76 + "\n")

    def run_interval_sweep(self, start_ms: float = 80.0, end_ms: float = 400.0, step_ms: float = 20.0,
                           probes_per_step: int = 50, dbg_mon: Optional[DebugMonitor] = None):
        """Sweeps polling intervals with full 32-byte frames and evaluates jitter profile."""
        if dbg_mon:
            dbg_mon.silent = True

        intervals = []
        cur = start_ms
        while cur <= end_ms + 0.001:
            intervals.append(round(cur, 1))
            cur += step_ms

        total_probes = len(intervals) * probes_per_step
        est_sec = sum((iv / 1000.0) * probes_per_step for iv in intervals)

        print("\n" + "=" * 76)
        print("       BR32 32-BYTE FRAME OPTIMAL INTERVAL SWEEPER & PROFILER")
        print("=" * 76)
        print(f" Sweep Range     : {start_ms:.1f} ms ~ {end_ms:.1f} ms (Step: {step_ms:.1f} ms)")
        print(f" Test Plan       : {len(intervals)} steps × {probes_per_step} probes = {total_probes:,} total probes")
        print(f" Estimated Time  : ~{est_sec:.1f} seconds (~{est_sec/60:.1f} minutes)")
        print("=" * 76 + "\n")

        results = []

        for idx, iv_ms in enumerate(intervals, 1):
            iv_sec = iv_ms / 1000.0
            sys.stdout.write(f" [{idx:02d}/{len(intervals):02d}] Interval {iv_ms:>5.1f} ms: [")
            sys.stdout.flush()

            success_cnt = 0
            rtts = []
            resp_times = []
            deltas = []

            t_base = time.perf_counter()
            prog_step = max(1, probes_per_step // 20)

            for p in range(1, probes_per_step + 1):
                seq = p & 0xFF
                payload = bytes([(0x30 + p) & 0xFF] * 14)
                frame = self.build_frame(cmd=0x01, seq=seq, uid=b'\x00' * 10, addr=p, payload=payload)

                ok, rtt, rx, parsed = self.send_br32_frame(frame)
                t_recv = time.perf_counter()

                if ok and parsed:
                    success_cnt += 1
                    rtts.append(rtt)
                    resp_times.append(t_recv)
                    if len(resp_times) >= 2:
                        deltas.append((resp_times[-1] - resp_times[-2]) * 1000.0)

                if p % prog_step == 0:
                    sys.stdout.write("=")
                    sys.stdout.flush()

                t_next = t_base + (p * iv_sec)
                sleep_dur = t_next - time.perf_counter()
                if sleep_dur > 0:
                    time.sleep(sleep_dur)

            rate = (success_cnt / probes_per_step) * 100.0
            avg_rtt = statistics.mean(rtts) if rtts else 0.0
            jitter = statistics.stdev(deltas) if len(deltas) > 1 else 0.0
            delta_range = (max(deltas) - min(deltas)) if len(deltas) > 1 else 0.0

            sys.stdout.write(f"] Rate:{rate:>5.1f}%, σ:{jitter:>5.2f}ms, Range:{delta_range:>5.1f}ms\n")
            sys.stdout.flush()

            results.append({
                "interval": iv_ms,
                "rate": rate,
                "avg_rtt": avg_rtt,
                "jitter": jitter,
                "range": delta_range
            })

        if dbg_mon:
            dbg_mon.silent = False

        print("\n" + "=" * 76)
        print("          INTERVAL vs JITTER PROFILE REPORT (32-BYTE BR32 FRAMES)")
        print("=" * 76)
        print(f" {'Interval':<10} | {'Rate':<6} | {'Avg RTT':<8} | {'Jitter σ':<9} | {'Range ΔT':<9} | Stability Curve")
        print("-" * 76)

        valid_results = [r for r in results if r['rate'] >= 90.0]
        best = min(valid_results, key=lambda x: (x['jitter'], x['range'])) if valid_results else results[0]
        max_jitter = max((r['jitter'] for r in results), default=1.0)
        max_jitter = max(max_jitter, 0.001)

        for r in results:
            bar_len = int((r['jitter'] / max_jitter) * 20)
            bar_str = "█" * max(1, bar_len)
            is_best = "★ GOLDEN OPTIMUM" if r['interval'] == best['interval'] else ""
            print(f" {r['interval']:>6.1f} ms  | {r['rate']:>5.1f}% | {r['avg_rtt']:>6.2f}ms | {r['jitter']:>7.2f}ms | {r['range']:>7.2f}ms | {bar_str:<20} {is_best}")

        print("=" * 76)
        print(f" >>> RECOMMENDED GOLDEN INTERVAL: {best['interval']:.1f} ms (Jitter σ: {best['jitter']:.2f} ms, Range: {best['range']:.2f} ms) <<<")
        print("=" * 76 + "\n")


def print_menu():
    print("\n--- [WU-1 BR32 32-Byte Frame Interactive Menu] ---")
    print("  [1] Send Single 32-Byte Frame (Query Slave SIGROW UID)")
    print("  [S] Run High-Capacity Stability Test (1,000 cycles @ interval)")
    print("  [O] Run Optimal Interval Sweeper (Sweep 80ms ~ 400ms)")
    print("  [Q] Exit")
    print("--------------------------------------------------")


def main():
    parser = argparse.ArgumentParser(description="WU-1: BR32 32-Byte Frame Echo & Telemetry Benchmark")
    parser.add_argument("--port", default="COM19", help="RS-485 Port (default: COM19)")
    parser.add_argument("--debug-port", default="COM21", help="Debug Soft-UART Port (default: COM21)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 Baudrate (default: 19200)")
    parser.add_argument("--single", action="store_true", help="Send single 32-byte frame and dump response")
    parser.add_argument("--stability", action="store_true", help="Run stability long-run test")
    parser.add_argument("--optimize", "--sweep", action="store_true", help="Run Optimal Interval Optimizer sweep")
    parser.add_argument("--sweep-start", type=float, default=80.0, help="Sweep start interval ms (default: 80.0)")
    parser.add_argument("--sweep-end", type=float, default=400.0, help="Sweep end interval ms (default: 400.0)")
    parser.add_argument("--sweep-step", type=float, default=20.0, help="Sweep step interval ms (default: 20.0)")
    parser.add_argument("--sweep-count", type=int, default=50, help="Probes per sweep step (default: 50)")
    parser.add_argument("--count", type=int, default=1000, help="Test cycles for stability test (default: 1000)")
    parser.add_argument("--interval", type=float, default=200.0, help="Polling interval in ms (default: 200.0)")
    args = parser.parse_args()

    dbg = DebugMonitor(args.debug_port)
    dbg.start()

    bench = BR32EchoTester(args.port, baudrate=args.baud)
    try:
        bench.open()
    except Exception as e:
        print(f"[FATAL] Cannot open RS-485 port {args.port}: {e}")
        dbg.stop()
        sys.exit(1)

    try:
        if args.single:
            bench.run_single_probe()
            return

        if args.stability:
            bench.run_stability_test(count=args.count, interval_ms=args.interval, dbg_mon=dbg)
            return

        if args.optimize:
            bench.run_interval_sweep(start_ms=args.sweep_start, end_ms=args.sweep_end,
                                     step_ms=args.sweep_step, probes_per_step=args.sweep_count, dbg_mon=dbg)
            return

        print_menu()
        sys.stdout.write("> ")
        sys.stdout.flush()

        while True:
            cmd = sys.stdin.readline().strip().upper()
            if not cmd:
                continue

            if cmd == '1':
                bench.run_single_probe()

            elif cmd == 'S':
                bench.run_stability_test(count=args.count, interval_ms=args.interval, dbg_mon=dbg)
                print_menu()

            elif cmd == 'O':
                bench.run_interval_sweep(start_ms=args.sweep_start, end_ms=args.sweep_end,
                                         step_ms=args.sweep_step, probes_per_step=args.sweep_count, dbg_mon=dbg)
                print_menu()

            elif cmd in ('Q', 'EXIT'):
                print("[INFO] Exiting WU-1 Bench.")
                break
            else:
                print(f"[UNKNOWN] Invalid command '{cmd}'")
                print_menu()

            sys.stdout.write("> ")
            sys.stdout.flush()

    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user.")
    finally:
        bench.close()
        dbg.stop()


if __name__ == "__main__":
    main()
