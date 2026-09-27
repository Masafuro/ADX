#!/usr/bin/env python3
"""
LIN Break & LINAUTO Synchronization Benchmark Tool (v1.1)
=========================================================
Target: ADX Core-D (ATtiny1616-MNR, SP485EEN)
Communicates over RS-485 via USB-UART adapter (e.g. COM19).

Features:
  1. STAGE 1: Power-on / Reset auto-detection (with interactive prompt).
  2. High-precision timing via time.perf_counter() for Windows microsecond accuracy.
  3. Supports both Optiboot_OL4 (4B response) and lin_break_probe (11B telemetry).
  4. Parameter sweep & ranking across Break duration and Delimiter/Switch delays.

Usage:
  python test_lin_break.py --port COM19 --count 50
  python test_lin_break.py --port COM19 --mode os_break --count 50
  python test_lin_break.py --port COM19 --sweep
"""

import sys
import os
import time
import argparse
from typing import Dict, List, Optional, Tuple

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)


def high_precision_delay_us(us: float):
    """Accurate busy-wait loop for sub-millisecond delays on Windows/Linux."""
    target = time.perf_counter() + (us / 1_000_000.0)
    while time.perf_counter() < target:
        pass


def crc16_ccitt(data: bytes, initial: int = 0xFFFF) -> int:
    crc = initial
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


class LinBreakTester:
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 0.06):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser: Optional[serial.Serial] = None

    def open(self):
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

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def send_break_and_frame(
        self,
        mode: str,
        pid: int = 0x80,
        break_duration_ms: float = 0.25,
        delimiter_delay_ms: float = 1.0,
        switch_delay_ms: float = 1.0
    ) -> Tuple[bool, str, Optional[Dict], float, bytes]:
        """
        Sends Break + Sync(0x55) + PID, and analyzes slave response.
        Auto-detects:
          - lin_break_probe (11B telemetry packet)
          - Optiboot_OL4 (4B PING status packet)
        Returns: (success, target_type, telemetry_dict, rtt_ms, raw_rx)
        """
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        if mode == "baud_trick":
            # Half-baud trick: switch to 57600, send 0x00, switch to 115200, send frame
            self.ser.baudrate = self.baudrate // 2
            self.ser.write(b'\x00')
            self.ser.flush()
            if delimiter_delay_ms > 0:
                high_precision_delay_us(delimiter_delay_ms * 1000.0)

            self.ser.baudrate = self.baudrate
            if switch_delay_ms > 0:
                high_precision_delay_us(switch_delay_ms * 1000.0)

            self.ser.write(bytes([0x55, pid]))
            self.ser.flush()

        elif mode == "baud_trick_safe":
            # 57600bps with 2 STOPBITS to guarantee Delimiter >= 2 Tbits (34.7us)
            self.ser.stopbits = serial.STOPBITS_TWO
            self.ser.baudrate = self.baudrate // 2
            self.ser.write(b'\x00')
            self.ser.flush()
            if delimiter_delay_ms > 0:
                high_precision_delay_us(delimiter_delay_ms * 1000.0)

            self.ser.stopbits = serial.STOPBITS_ONE
            self.ser.baudrate = self.baudrate
            if switch_delay_ms > 0:
                high_precision_delay_us(switch_delay_ms * 1000.0)

            self.ser.write(bytes([0x55, pid]))
            self.ser.flush()

        elif mode == "os_break":
            # OS Break: No baudrate switching -> Zero USB PLL glitch
            self.ser.break_condition = True
            high_precision_delay_us(break_duration_ms * 1000.0)
            self.ser.break_condition = False
            if delimiter_delay_ms > 0:
                high_precision_delay_us(delimiter_delay_ms * 1000.0)

            self.ser.write(bytes([0x55, pid]))
            self.ser.flush()

        else:
            raise ValueError(f"Unknown mode: {mode}")

        # 3. Receive Slave Response via Standard LN-485 Frame Header
        # Read [Status (1B), Length (1B)]
        hdr = self.ser.read(2)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        if len(hdr) < 2:
            return False, "TIMEOUT", None, rtt_ms, hdr

        status = hdr[0]
        length = hdr[1]
        rest = self.ser.read(length + 2)
        raw_rx = hdr + rest

        if len(rest) < length + 2:
            return False, "INCOMPLETE", None, rtt_ms, raw_rx

        payload = rest[:length]
        rx_crc = (rest[length] << 8) | rest[length + 1]

        # 1. Probe Diagnostic Packet (Length == 7: [PID, frame_count(2B), isfif_count(2B), auto_baud(2B)])
        if status == 0x00 and length == 7:
            calc_crc = crc16_ccitt(payload)
            if calc_crc != rx_crc:
                return False, "CRC_ERR", {"crc_err": True, "calc": calc_crc, "recv": rx_crc}, rtt_ms, raw_rx

            pid_resp    = payload[0]
            frame_count = (payload[1] << 8) | payload[2]
            isfif_count = (payload[3] << 8) | payload[4]
            auto_baud   = (payload[5] << 8) | payload[6]

            telemetry = {
                "type": "probe",
                "status": status,
                "pid": pid_resp,
                "frame_count": frame_count,
                "isfif_count": isfif_count,
                "auto_baud": auto_baud,
                "ferr": False,
                "perr": False,
                "bufovf": False
            }
            return True, "PROBE", telemetry, rtt_ms, raw_rx

        # 2. Standard Optiboot_OL4 PING Packet (Length == 0)
        if status == 0x00 and length == 0:
            telemetry = {
                "type": "optiboot",
                "status": status,
                "len": 0
            }
            return True, "OPTIBOOT", telemetry, rtt_ms, raw_rx

        # 3. Other unexpected status
        return False, f"STATUS_0x{status:02X}", None, rtt_ms, raw_rx

    def wait_for_power_on(self, max_wait_sec: float = 30.0) -> bool:
        """STAGE 1: Probe loop waiting for Core-D power on or reset (exact match to ol4_flasher)."""
        print(f"[STAGE 1] Waiting for Core-D power on/reset (up to {max_wait_sec:.1f}s)...")
        print(">>> POWER ON OR RESET CORE-D NOW <<<")

        t_start = time.perf_counter()
        probe_count = 0
        last_print = 0.0
        while time.perf_counter() - t_start < max_wait_sec:
            probe_count += 1
            # Exact ol4_flasher parameters: 0ms delays for baud_trick
            ok, target_type, telem, rtt, raw = self.send_break_and_frame(
                mode="baud_trick",
                pid=0x80,
                delimiter_delay_ms=0.0,
                switch_delay_ms=0.0
            )
            if ok:
                elapsed = time.perf_counter() - t_start
                print(f"\n[STAGE 1: PASS] Power-on detected in {elapsed:.2f}s (probe #{probe_count}, RTT={rtt:.1f}ms)!")
                if target_type == "PROBE":
                    print(f"  [TARGET] lin_break_probe active! Initial USART0.BAUD=0x{telem['auto_baud']:04X}")
                elif target_type == "OPTIBOOT":
                    print(f"  [TARGET] Optiboot_OL4 active (4-byte PING response).")
                time.sleep(0.150)
                return True

            # Print heartbeat dots every 1 second
            if time.perf_counter() - last_print >= 1.0:
                last_print = time.perf_counter()
                elapsed = int(time.perf_counter() - t_start)
                rx_info = f" (last rx: {len(raw)}B)" if raw else ""
                print(f"  ... probing #{probe_count} ({elapsed}s / {int(max_wait_sec)}s){rx_info} ...")

            time.sleep(0.02)

        print(f"\n[STAGE 1: FAIL] No response from Core-D after {max_wait_sec:.1f}s.")
        return False


def run_benchmark(
    port: str,
    mode: str,
    count: int = 50,
    break_duration_ms: float = 0.25,
    delimiter_delay_ms: float = 1.0,
    switch_delay_ms: float = 1.0,
    skip_stage1: bool = False
):
    print("=" * 72)
    print(f" LIN Break & LINAUTO Benchmark: mode={mode}, count={count}")
    print(f" Port={port}, Baud=115200")
    print(f" Parameters: break={break_duration_ms:.2f}ms, delim_delay={delimiter_delay_ms:.2f}ms, switch_delay={switch_delay_ms:.2f}ms")
    print("=" * 72)

    tester = LinBreakTester(port=port)
    try:
        tester.open()
    except Exception as e:
        print(f"[ERROR] Failed to open {port}: {e}")
        return 0.0

    if not skip_stage1:
        if not tester.wait_for_power_on(max_wait_sec=30.0):
            tester.close()
            return 0.0

    success_count = 0
    timeout_count = 0
    prev_isfif = None
    rtt_list = []
    baud_list = []
    isfif_inc_count = 0

    print(f"{'#':>4} | {'Target':>8} | {'Status':>8} | {'RTT(ms)':>8} | {'Frame#':>7} | {'ISFIF#':>7} | {'AutoBAUD':>8} | {'Info':>10}")
    print("-" * 72)

    for i in range(1, count + 1):
        ok, target_type, telem, rtt, raw = tester.send_break_and_frame(
            mode=mode,
            pid=0x80,
            break_duration_ms=break_duration_ms,
            delimiter_delay_ms=delimiter_delay_ms,
            switch_delay_ms=switch_delay_ms
        )

        rtt_list.append(rtt)

        if ok and telem:
            success_count += 1
            if telem["type"] == "probe":
                cur_isfif = telem["isfif_count"]
                isfif_diff = 0
                if prev_isfif is not None:
                    isfif_diff = cur_isfif - prev_isfif
                    if isfif_diff > 0:
                        isfif_inc_count += isfif_diff
                prev_isfif = cur_isfif

                baud_list.append(telem["auto_baud"])

                err_str = "OK"
                if telem["ferr"]: err_str += "+FERR"
                if telem["perr"]: err_str += "+PERR"
                if telem["bufovf"]: err_str += "+OVF"
                if isfif_diff > 0: err_str += f"+ISFIF(+{isfif_diff})"

                print(f"{i:>4} | {'PROBE':>8} | {'PASS':>8} | {rtt:>8.2f} | {telem['frame_count']:>7} | {cur_isfif:>7} | 0x{telem['auto_baud']:04X}   | {err_str:>10}")
            else:
                # Optiboot_OL4 4-byte response
                print(f"{i:>4} | {'OPTIBOOT':>8} | {'PASS':>8} | {rtt:>8.2f} | {'-':>7} | {'-':>7} | {'-':>8} | 4B PING OK")
        else:
            timeout_count += 1
            raw_str = raw.hex() if raw else 'empty'
            print(f"{i:>4} | {target_type:>8} | {'FAIL':>8} | {rtt:>8.2f} | {'-':>7} | {'-':>7} | {'-':>8} | rx={len(raw)}B ({raw_str})")

        time.sleep(0.025)

    tester.close()

    # Summary Statistics
    print("\n" + "=" * 72)
    print(" BENCHMARK RESULTS SUMMARY:")
    print("=" * 72)
    success_rate = (success_count / count) * 100.0
    print(f" Total Trials       : {count}")
    print(f" Successful Frames  : {success_count} ({success_rate:.1f}%)")
    print(f" Timeouts / Fails   : {timeout_count}")
    print(f" Total ISFIF Errors : {isfif_inc_count}")

    if rtt_list:
        print(f" RTT Min / Avg / Max: {min(rtt_list):.2f} ms / {sum(rtt_list)/len(rtt_list):.2f} ms / {max(rtt_list):.2f} ms")

    if baud_list:
        avg_baud = sum(baud_list) / len(baud_list)
        print(f" USART0.BAUD Min/Avg/Max: 0x{min(baud_list):04X} / 0x{int(avg_baud):04X} ({avg_baud:.1f}) / 0x{max(baud_list):04X}")
        print(f" Ideal 20MHz BAUD   : 0x0485 (1157)")
    print("=" * 72)
    return success_rate


def run_sweep(port: str):
    """Parameter sweep with initial Power-On detection."""
    print("\n" + "#" * 72)
    print(" AUTOMATIC PARAMETER SWEEP BENCHMARK (with STAGE 1 Handshake)")
    print("#" * 72)

    tester = LinBreakTester(port=port)
    tester.open()
    connected = tester.wait_for_power_on(max_wait_sec=30.0)
    tester.close()
    if not connected:
        return

    configs = [
        ("baud_trick", 0.0, 0.0, 0.0, "baud_trick (original flasher: 0ms delay)"),
        ("baud_trick", 0.0, 0.5, 0.5, "baud_trick (delim=0.5ms, switch=0.5ms)"),
        ("baud_trick", 0.0, 1.0, 1.0, "baud_trick (delim=1.0ms, switch=1.0ms)"),
        ("baud_trick_safe", 0.0, 0.5, 0.5, "baud_trick_safe (2 stopbits, 0.5ms/0.5ms)"),
        ("baud_trick_safe", 0.0, 1.0, 1.0, "baud_trick_safe (2 stopbits, 1.0ms/1.0ms)"),
        ("os_break", 0.15, 0.2, 0.0, "os_break (break=150us, delim=0.2ms)"),
        ("os_break", 0.25, 0.5, 0.0, "os_break (break=250us, delim=0.5ms)"),
        ("os_break", 0.25, 1.0, 0.0, "os_break (break=250us, delim=1.0ms)"),
        ("os_break", 0.50, 1.0, 0.0, "os_break (break=500us, delim=1.0ms)"),
    ]

    sweep_results = []
    trials_per_config = 30

    for mode, b_ms, d_ms, s_ms, label in configs:
        print(f"\n---> Testing Config: {label}")
        rate = run_benchmark(
            port=port,
            mode=mode,
            count=trials_per_config,
            break_duration_ms=b_ms,
            delimiter_delay_ms=d_ms,
            switch_delay_ms=s_ms,
            skip_stage1=True
        )
        sweep_results.append((label, rate))
        time.sleep(0.1)

    print("\n" + "=" * 72)
    print(" SWEEP RESULTS RANKING:")
    print("=" * 72)
    for label, rate in sweep_results:
        bar = "█" * int(rate / 5)
        print(f" {rate:5.1f}% | {bar:<20} | {label}")
    print("=" * 72)


def main():
    parser = argparse.ArgumentParser(description="LIN Break & LINAUTO Benchmark Tool for Optiboot_OL4")
    parser.add_argument("--port", default="COM19", help="Serial port (default: COM19)")
    parser.add_argument("--mode", default="baud_trick", choices=["baud_trick", "baud_trick_safe", "os_break"],
                        help="Break generation mode")
    parser.add_argument("--count", type=int, default=50, help="Number of benchmark trials (default: 50)")
    parser.add_argument("--break-time", type=float, default=0.25, help="Break LOW duration for os_break in ms (default: 0.25)")
    parser.add_argument("--delimiter-delay", type=float, default=1.0, help="Delimiter delay in ms (default: 1.0)")
    parser.add_argument("--switch-delay", type=float, default=1.0, help="Baudrate switch recovery delay in ms (default: 1.0)")
    parser.add_argument("--sweep", action="store_true", help="Run automated parameter sweep across all modes")
    parser.add_argument("--skip-stage1", action="store_true", help="Skip power-on wait loop")

    args = parser.parse_args()

    if args.sweep:
        run_sweep(port=args.port)
    else:
        run_benchmark(
            port=args.port,
            mode=args.mode,
            count=args.count,
            break_duration_ms=args.break_time,
            delimiter_delay_ms=args.delimiter_delay,
            switch_delay_ms=args.switch_delay,
            skip_stage1=args.skip_stage1
        )


if __name__ == "__main__":
    main()
