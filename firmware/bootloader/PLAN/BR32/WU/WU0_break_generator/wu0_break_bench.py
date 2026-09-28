#!/usr/bin/env python3
"""
WU-0: PC RS-485 Break Generation & Bus Stability Benchmark Tool
===============================================================
Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)

Features:
  1. Compares 3 Break Generation Techniques:
     - Method 1: Half-Baud Trick (Baud reduction to 9,600 bps)
     - Method 2: ser.send_break() (OS Hardware Break API)
     - Method 3: Normal Baud Zero Data (Single 0x00 frame)
  2. High-Capacity Bus Stability Long-Run Test (1,000+ iterations):
     - Accurate periodic polling scheduler with zero cumulative drift
     - Inter-Response Time (ΔT_resp) statistics: Mean, Min, Max, Range, Variance, Jitter (σ)
     - Round-Trip Time (RTT) statistics
     - Distribution Histogram (ASCII art)
     - Real-time progress bar & telemetry monitoring on COM21

Usage:
  python wu0_break_bench.py --port COM19 --debug-port COM21
  python wu0_break_bench.py --port COM19 --stability --count 1000 --interval 100
  python wu0_break_bench.py --port COM19 --bench
"""

import sys
import time
import argparse
import threading
import statistics
from typing import Optional, Tuple, Dict, List

try:
    import serial
except ImportError:
    print("[FATAL] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)


def high_precision_delay_us(us: float):
    """Accurate busy-wait loop for microsecond delays on Windows/Linux."""
    if us <= 0:
        return
    target = time.perf_counter() + (us / 1_000_000.0)
    while time.perf_counter() < target:
        pass


class DebugMonitor:
    """Listens to Core-D CH342K Soft-UART telemetry on COM21 @ 9,600 bps."""
    def __init__(self, port: Optional[str], baudrate: int = 9600):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.running = False
        self.thread: Optional[threading.Thread] = None
        self.silent = False

    def start(self):
        if not self.port:
            return
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.05
            )
            self.running = True
            self.thread = threading.Thread(target=self._read_loop, daemon=True)
            self.thread.start()
            print(f"[DEBUG-MON] Listening on {self.port} @ {self.baudrate} bps (Soft-UART PB4)")
        except Exception as e:
            print(f"[DEBUG-MON WARN] Could not open {self.port}: {e}")
            self.ser = None

    def _read_loop(self):
        line_buf = ""
        while self.running and self.ser and self.ser.is_open:
            try:
                data = self.ser.read(self.ser.in_waiting or 1)
                if not data:
                    continue
                text = data.decode("latin1", errors="replace")
                for ch in text:
                    if ch == '\r':
                        continue
                    if ch == '\n':
                        if line_buf.strip() and not self.silent:
                            now_str = time.strftime("%H:%M:%S")
                            print(f"\n  |-> [COM21 {now_str}] {line_buf}")
                            sys.stdout.write("> ")
                            sys.stdout.flush()
                        line_buf = ""
                    else:
                        line_buf += ch
            except Exception:
                break

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()


class BreakBenchTester:
    def __init__(self, port: str, baudrate: int = 19200, timeout: float = 0.2):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser: Optional[serial.Serial] = None

    def open(self):
        print(f"[INIT] Opening RS-485 Serial Port {self.port} @ {self.baudrate} bps...")
        self.ser = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=self.timeout
        )
        time.sleep(0.05)
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        print(f"[INIT] Connected successfully to {self.port}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def send_method1_half_baud(self, test_byte: int = 0xA5) -> Tuple[bool, float, bytes, int]:
        """Method 1: Half-Baud Trick (Baud reduction -> 0x00 -> Restore -> 0x55 + Byte)"""
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        # Step 1: Drop to half-baud and send 0x00 (Generates ~18-20 bit times of LOW)
        self.ser.baudrate = self.baudrate // 2
        self.ser.write(b'\x00')
        self.ser.flush()

        # Step 2: Restore normal baudrate
        self.ser.baudrate = self.baudrate

        # Step 3: Send Sync (0x55) and Test Payload Byte
        self.ser.write(bytes([0x55, test_byte]))
        self.ser.flush()

        # Step 4: Wait for 5-byte slave response [0x55, 0xAA, CountH, CountL, EchoByte]
        rx = self.ser.read(5)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        slave_count = -1
        if len(rx) == 5 and rx[0] == 0x55 and rx[1] == 0xAA:
            slave_count = (rx[2] << 8) | rx[3]

        success = (len(rx) == 5 and rx[0] == 0x55 and rx[1] == 0xAA and rx[4] == test_byte)
        return success, rtt_ms, rx, slave_count

    def send_method2_send_break(self, test_byte: int = 0xA5, duration: float = 0.001) -> Tuple[bool, float, bytes, int]:
        """Method 2: pyserial send_break() API (SetCommBreak / ClearCommBreak)"""
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        self.ser.send_break(duration=duration)
        self.ser.write(bytes([0x55, test_byte]))
        self.ser.flush()

        rx = self.ser.read(5)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        slave_count = -1
        if len(rx) == 5 and rx[0] == 0x55 and rx[1] == 0xAA:
            slave_count = (rx[2] << 8) | rx[3]

        success = (len(rx) == 5 and rx[0] == 0x55 and rx[1] == 0xAA and rx[4] == test_byte)
        return success, rtt_ms, rx, slave_count

    def send_method3_zero_data(self, test_byte: int = 0xA5) -> Tuple[bool, float, bytes, int]:
        """Method 3: Normal Baudrate Zero Byte (b'\x00\x55' atomic write)"""
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        self.ser.write(bytes([0x00, 0x55, test_byte]))
        self.ser.flush()

        rx = self.ser.read(5)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        slave_count = -1
        if len(rx) == 5 and rx[0] == 0x55 and rx[1] == 0xAA:
            slave_count = (rx[2] << 8) | rx[3]

        success = (len(rx) == 5 and rx[0] == 0x55 and rx[1] == 0xAA and rx[4] == test_byte)
        return success, rtt_ms, rx, slave_count

    def run_benchmark(self, count: int = 30):
        print("\n" + "=" * 76)
        print(f" RUNNING COMPARATIVE BENCHMARK: {count} Iterations per Method")
        print("=" * 76)

        methods = [
            ("Method 1 (Half-Baud Trick)", self.send_method1_half_baud),
            ("Method 2 (send_break API)",  self.send_method2_send_break),
            ("Method 3 (Normal 0x00 Byte)", self.send_method3_zero_data),
        ]

        results = {}

        for name, fn in methods:
            print(f"\n[TESTING] {name} ({count} probes)...")
            success_count = 0
            rtts = []

            for i in range(1, count + 1):
                test_val = (0xA0 + (i & 0x0F)) & 0xFF
                ok, rtt, rx, _ = fn(test_val)
                if ok:
                    success_count += 1
                    rtts.append(rtt)
                    sys.stdout.write(".")
                else:
                    sys.stdout.write("x")
                sys.stdout.flush()
                time.sleep(0.05)

            rate = (success_count / count) * 100.0
            avg_rtt = statistics.mean(rtts) if rtts else 0.0
            jitter = statistics.stdev(rtts) if len(rtts) > 1 else 0.0
            min_rtt = min(rtts) if rtts else 0.0
            max_rtt = max(rtts) if rtts else 0.0

            results[name] = {
                "success": success_count,
                "rate": rate,
                "avg_rtt": avg_rtt,
                "jitter": jitter,
                "min_rtt": min_rtt,
                "max_rtt": max_rtt
            }

        print("\n\n" + "=" * 76)
        print("                       BENCHMARK COMPARISON REPORT")
        print("=" * 76)
        print(f" {'Method':<28} | {'Success':<9} | {'Rate':<7} | {'Avg RTT':<9} | {'Jitter σ':<9}")
        print("-" * 76)
        for name, r in results.items():
            print(f" {name:<28} | {r['success']:>2}/{count:<6} | {r['rate']:>5.1f}% | {r['avg_rtt']:>6.2f}ms | {r['jitter']:>6.2f}ms")
        print("=" * 76)

        best = max(results.items(), key=lambda x: (x[1]['rate'], -x[1]['jitter']))
        print(f"\n>>> RECOMMENDED BREAK GENERATOR: {best[0]} (Success: {best[1]['rate']:.1f}%, Jitter: {best[1]['jitter']:.2f}ms) <<<\n")

    def run_stability_test(self, count: int = 1000, interval_ms: float = 100.0, dbg_mon: Optional[DebugMonitor] = None):
        """
        High-Capacity Bus Stability Long-Run Test.
        Sends periodic probes at exact interval_ms and measures:
          - Inter-Response Time (ΔT_resp) statistics (Mean, Min, Max, Range, Variance, Jitter)
          - Round-Trip Time (RTT) statistics
          - Sequence continuity
          - Histogram distribution
        """
        if dbg_mon:
            dbg_mon.silent = True  # Suppress individual lines to keep console clean

        interval_sec = interval_ms / 1000.0
        print("\n" + "=" * 76)
        print("       BR32 BUS STABILITY LONG-RUN TEST (BREAK HEARTBEAT TELEMETRY)")
        print("=" * 76)
        print(f" Target Probes     : {count:,} cycles")
        print(f" Planned Interval  : {interval_ms:.2f} ms ({1000.0/interval_ms:.1f} Hz)")
        print(f" Method            : Method 1 (Half-Baud Trick)")
        print(f" Estimated Duration: {(count * interval_sec):.1f} seconds")
        print("=" * 76 + "\n")

        success_count = 0
        failure_count = 0
        rtts: List[float] = []
        resp_times: List[float] = []
        inter_intervals: List[float] = []
        slave_counts: List[int] = []

        t_base = time.perf_counter()
        t_next = t_base

        progress_step = max(1, count // 50)

        sys.stdout.write(" Progress: [")
        sys.stdout.flush()

        for i in range(1, count + 1):
            test_val = (0x10 + (i & 0x7F)) & 0xFF
            t_send_start = time.perf_counter()

            ok, rtt, rx, scnt = self.send_method1_half_baud(test_val)
            t_recv = time.perf_counter()

            if ok:
                success_count += 1
                rtts.append(rtt)
                resp_times.append(t_recv)
                if len(resp_times) >= 2:
                    delta_ms = (resp_times[-1] - resp_times[-2]) * 1000.0
                    inter_intervals.append(delta_ms)
                if scnt >= 0:
                    slave_counts.append(scnt)
            else:
                failure_count += 1

            if i % progress_step == 0:
                sys.stdout.write("=")
                sys.stdout.flush()

            # Drift-free scheduler: calculate next target tick
            t_next = t_base + (i * interval_sec)
            sleep_duration = t_next - time.perf_counter()
            if sleep_duration > 0:
                time.sleep(sleep_duration)

        sys.stdout.write("] Done!\n\n")

        if dbg_mon:
            dbg_mon.silent = False

        # --- Comprehensive Statistical Analysis ---
        success_rate = (success_count / count) * 100.0

        # RTT Stats
        rtt_mean = statistics.mean(rtts) if rtts else 0.0
        rtt_min = min(rtts) if rtts else 0.0
        rtt_max = max(rtts) if rtts else 0.0
        rtt_variance = statistics.variance(rtts) if len(rtts) > 1 else 0.0
        rtt_stdev = statistics.stdev(rtts) if len(rtts) > 1 else 0.0

        # Inter-Response Interval Stats (ΔT_resp)
        if inter_intervals:
            int_mean = statistics.mean(inter_intervals)
            int_min = min(inter_intervals)
            int_max = max(inter_intervals)
            int_range = int_max - int_min
            int_variance = statistics.variance(inter_intervals) if len(inter_intervals) > 1 else 0.0
            int_stdev = statistics.stdev(inter_intervals) if len(inter_intervals) > 1 else 0.0
        else:
            int_mean = int_min = int_max = int_range = int_variance = int_stdev = 0.0

        # Sequence Check
        seq_miss = 0
        if len(slave_counts) >= 2:
            for k in range(1, len(slave_counts)):
                if slave_counts[k] != slave_counts[k - 1] + 1:
                    seq_miss += 1

        # Rating Grade
        if success_rate >= 99.9 and int_stdev <= 1.0:
            rating = "GRADE A+ (ROCK SOLID: Ultra-Low Jitter, Flawless Sync)"
        elif success_rate >= 99.0 and int_stdev <= 2.0:
            rating = "GRADE A  (EXCELLENT: Production-Ready Stability)"
        elif success_rate >= 95.0 and int_stdev <= 5.0:
            rating = "GRADE B  (ACCEPTABLE: Tolerable Jitter)"
        else:
            rating = "GRADE C  (DEGRADED: Noise / Stub Reflection Detected)"

        # --- Print Detailed Report ---
        print("=" * 76)
        print("                  BR32 BUS STABILITY STATISTICAL REPORT")
        print("=" * 76)
        print(f" Total Probes Sent   : {count:,}")
        print(f" Successful Responses: {success_count:,} ({success_rate:.2f}%)")
        print(f" Failed / Timed Out  : {failure_count:,} ({(failure_count/count)*100.0:.2f}%)")
        print(f" Slave Sequence Drops: {seq_miss} drops")
        print("-" * 76)
        print(" [1] 応答間隔統計 (Inter-Response Interval: ΔT_resp) [Target: " + f"{interval_ms:.2f} ms]")
        print(f"     ・平均値 (Mean)      : {int_mean:>7.2f} ms  (理想値との差: {abs(int_mean - interval_ms):+.2f} ms)")
        print(f"     ・最小値 (Min)       : {int_min:>7.2f} ms")
        print(f"     ・最大値 (Max)       : {int_max:>7.2f} ms")
        print(f"     ・変動幅 (Range)     : {int_range:>7.2f} ms  (Max - Min)")
        print(f"     ・分散   (Variance σ²): {int_variance:>7.4f} ms²")
        print(f"     ・標準偏差 (Jitter σ): {int_stdev:>7.2f} ms  ★バス健全性コア指標")
        print("-" * 76)
        print(" [2] 往復遅延統計 (Round-Trip Latency: RTT)")
        print(f"     ・平均値 (Mean)      : {rtt_mean:>7.2f} ms")
        print(f"     ・最小値 (Min)       : {rtt_min:>7.2f} ms")
        print(f"     ・最大値 (Max)       : {rtt_max:>7.2f} ms")
        print(f"     ・分散   (Variance σ²): {rtt_variance:>7.4f} ms²")
        print(f"     ・標準偏差 (Jitter σ): {rtt_stdev:>7.2f} ms")
        print("-" * 76)

        # --- ASCII Art Histogram ---
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
        """
        Optimal Interval Sweeper / Optimizer.
        Sweeps polling intervals from start_ms to end_ms with step_ms increments.
        Measures Jitter (σ), Inter-interval Range, RTT, and Success Rate for each point.
        Identifies the Golden Operating Interval with minimum jitter.
        """
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
        print("          BR32 BUS OPTIMAL INTERVAL OPTIMIZER & SWEEPER")
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
                test_val = (0x30 + (p & 0x7F)) & 0xFF
                ok, rtt, rx, _ = self.send_method1_half_baud(test_val)
                t_recv = time.perf_counter()

                if ok:
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

        # --- Report & Bathtub Curve ---
        print("\n" + "=" * 76)
        print("                 INTERVAL vs JITTER PROFILE REPORT")
        print("=" * 76)
        print(f" {'Interval':<10} | {'Rate':<6} | {'Avg RTT':<8} | {'Jitter σ':<9} | {'Range ΔT':<9} | Stability Curve (Low σ = Better)")
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
    print("\n--- [WU-0 Interactive Control Menu] ---")
    print("  [1] Send Single Break via Method 1 (Half-Baud Trick)")
    print("  [2] Send Single Break via Method 2 (send_break API)")
    print("  [3] Send Single Break via Method 3 (Normal Baud 0x00)")
    print("  [S] Run High-Capacity Stability Test (1,000 cycles @ interval)")
    print("  [O] Run Optimal Interval Optimizer (Sweep & find minimum jitter)")
    print("  [B] Run Comparative Benchmark (30 iterations per method)")
    print("  [Q] Exit")
    print("---------------------------------------")


def main():
    parser = argparse.ArgumentParser(description="WU-0: PC RS-485 Break Generation & Stability Benchmark")
    parser.add_argument("--port", default="COM19", help="RS-485 Port (default: COM19)")
    parser.add_argument("--debug-port", default="COM21", help="Debug Soft-UART Port (default: COM21)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 Baudrate (default: 19200)")
    parser.add_argument("--bench", action="store_true", help="Run 3-method comparative benchmark")
    parser.add_argument("--stability", action="store_true", help="Run stability long-run test")
    parser.add_argument("--optimize", "--sweep", action="store_true", help="Run Optimal Interval Optimizer sweep")
    parser.add_argument("--sweep-start", type=float, default=80.0, help="Sweep start interval ms (default: 80.0)")
    parser.add_argument("--sweep-end", type=float, default=400.0, help="Sweep end interval ms (default: 400.0)")
    parser.add_argument("--sweep-step", type=float, default=20.0, help="Sweep step interval ms (default: 20.0)")
    parser.add_argument("--sweep-count", type=int, default=50, help="Probes per sweep step (default: 50)")
    parser.add_argument("--count", type=int, default=1000, help="Test cycles for stability test (default: 1000)")
    parser.add_argument("--interval", type=float, default=100.0, help="Polling interval in ms (default: 100.0)")
    args = parser.parse_args()

    dbg = DebugMonitor(args.debug_port)
    dbg.start()

    bench = BreakBenchTester(args.port, baudrate=args.baud)
    try:
        bench.open()
    except Exception as e:
        print(f"[FATAL] Cannot open RS-485 port {args.port}: {e}")
        dbg.stop()
        sys.exit(1)

    try:
        if args.bench:
            bench.run_benchmark(30)
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
                print("[TEST] Sending Break via Method 1 (Half-Baud)...")
                ok, rtt, rx, scnt = bench.send_method1_half_baud(0x11)
                hex_rx = rx.hex(' ') if rx else '<NO DATA>'
                res_str = "PASS" if ok else "FAIL"
                print(f"  -> [{res_str}] RTT={rtt:.2f}ms, SlaveCount={scnt}, RX: {hex_rx}")

            elif cmd == '2':
                print("[TEST] Sending Break via Method 2 (send_break API)...")
                ok, rtt, rx, scnt = bench.send_method2_send_break(0x22)
                hex_rx = rx.hex(' ') if rx else '<NO DATA>'
                res_str = "PASS" if ok else "FAIL"
                print(f"  -> [{res_str}] RTT={rtt:.2f}ms, SlaveCount={scnt}, RX: {hex_rx}")

            elif cmd == '3':
                print("[TEST] Sending Break via Method 3 (Zero Byte)...")
                ok, rtt, rx, scnt = bench.send_method3_zero_data(0x33)
                hex_rx = rx.hex(' ') if rx else '<NO DATA>'
                res_str = "PASS" if ok else "FAIL"
                print(f"  -> [{res_str}] RTT={rtt:.2f}ms, SlaveCount={scnt}, RX: {hex_rx}")

            elif cmd == 'S':
                bench.run_stability_test(count=args.count, interval_ms=args.interval, dbg_mon=dbg)
                print_menu()

            elif cmd == 'O':
                bench.run_interval_sweep(start_ms=args.sweep_start, end_ms=args.sweep_end,
                                         step_ms=args.sweep_step, probes_per_step=args.sweep_count, dbg_mon=dbg)
                print_menu()

            elif cmd == 'B':
                bench.run_benchmark(30)
                print_menu()

            elif cmd in ('Q', 'EXIT'):
                print("[INFO] Exiting WU-0 Bench.")
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
