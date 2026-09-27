#!/usr/bin/env python3
"""
LIN Break & LINAUTO Synchronization Benchmark Tool (v1.4 - Dual-Channel Telemetry Edition)
==========================================================================================
Target: ADX Core-D (ATtiny1616-MNR, SP485EEN)
Channel 1 (RS-485 under test): COM19 @ 19,200 bps
Channel 2 (Debug Telemetry):  COM21 @ 38,400 bps (CH342K Soft-UART on PB4)

Features:
  1. Microsecond-accurate Break + Sync + PID transmission over RS-485 (COM19).
  2. Simultaneous non-blocking telemetry capture from Core-D internal state (COM21).
  3. Real-time correlation of RS-485 Frame timeouts with internal USART0.STATUS, BAUD, and RXD pin levels.
  4. Automatic logging of full dual-channel session to file.

Usage:
  python test_lin_break.py --port COM19 --debug-port COM21 --count 30
  python test_lin_break.py --port COM19 --count 30
"""

import sys
import os
import time
import queue
import threading
import argparse
from datetime import datetime
from typing import Dict, List, Optional, Tuple

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)


def high_precision_delay_us(us: float):
    """Accurate busy-wait loop for microsecond delays on Windows/Linux."""
    if us <= 0:
        return
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


class DebugListener:
    """Background listener for Core-D CH342K Soft-UART telemetry on COM21."""
    def __init__(self, port: str, baudrate: int = 38400, log_path: str = "com21_telemetry.log"):
        self.port = port
        self.baudrate = baudrate
        self.log_path = log_path
        self.ser: Optional[serial.Serial] = None
        self.running = False
        self.thread: Optional[threading.Thread] = None
        self.msg_queue: queue.Queue = queue.Queue()
        self.f_log = None

    def start(self):
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
            self.f_log = open(self.log_path, "a", encoding="utf-8")
            self.f_log.write(f"\n--- Benchmark Session Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} ---\n")
            self.f_log.flush()
            self.thread = threading.Thread(target=self._worker, daemon=True)
            self.thread.start()
            print(f"[DEBUG] Telemetry listener started on {self.port} @ {self.baudrate} bps -> {self.log_path}")
        except Exception as e:
            print(f"[WARN] Could not open debug port {self.port}: {e}")
            self.ser = None

    def _worker(self):
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
                        if line_buf:
                            now_str = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                            msg = f"[{now_str}] {line_buf}"
                            self.msg_queue.put(msg)
                            if self.f_log:
                                self.f_log.write(msg + "\n")
                                self.f_log.flush()
                            line_buf = ""
                    else:
                        line_buf += ch
            except Exception:
                break

    def get_messages(self) -> List[str]:
        """Fetch all queued messages received since last call."""
        msgs = []
        while not self.msg_queue.empty():
            try:
                msgs.append(self.msg_queue.get_nowait())
            except queue.Empty:
                break
        return msgs

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass
        if self.f_log:
            try:
                self.f_log.close()
            except Exception:
                pass


class LinBreakTester:
    def __init__(self, port: str, baudrate: int = 19200, timeout: float = 0.15):
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
        mode: str = "baud_trick",
        pid: int = 0x80,
        delimiter_delay_us: float = 0.0,
        switch_delay_us: float = 0.0
    ) -> Tuple[bool, str, Optional[Dict], float, bytes]:
        """
        Sends Break + Sync(0x55) + PID, and analyzes slave response.
        Returns: (success, target_type, telemetry_dict, rtt_ms, raw_rx)
        """
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        # 1. Break via Half-Baud (9600 bps: 10 bits = 1041.7us total)
        self.ser.baudrate = self.baudrate // 2
        self.ser.write(b'\x00')
        self.ser.flush()

        if delimiter_delay_us > 0:
            high_precision_delay_us(delimiter_delay_us)

        # 2. Switch back to normal baudrate (19200 bps)
        self.ser.baudrate = self.baudrate
        if switch_delay_us > 0:
            high_precision_delay_us(switch_delay_us)

        # 3. Send Sync (0x55) + PID
        self.ser.write(bytes([0x55, pid]))
        self.ser.flush()

        # 4. Receive Slave Response via Standard LN-485 Frame Header
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

    def wait_for_power_on(self, max_wait_sec: float = 30.0, dbg_listener: Optional[DebugListener] = None) -> bool:
        """STAGE 1: Probe loop waiting for Core-D power on or reset."""
        print(f"[STAGE 1] Waiting for Core-D power on/reset (up to {max_wait_sec:.1f}s at {self.baudrate} bps)...")
        print(">>> POWER ON OR RESET CORE-D NOW <<<")

        t_start = time.perf_counter()
        probe_count = 0
        last_print = 0.0
        while time.perf_counter() - t_start < max_wait_sec:
            probe_count += 1
            ok, target_type, telem, rtt, raw = self.send_break_and_frame(
                mode="baud_trick",
                pid=0x80,
                delimiter_delay_us=0.0,
                switch_delay_us=0.0
            )

            # Check debug telemetry
            if dbg_listener:
                msgs = dbg_listener.get_messages()
                for m in msgs:
                    print(f"      |-> [COM21] {m}")

            if ok:
                elapsed = time.perf_counter() - t_start
                print(f"\n[STAGE 1: PASS] Power-on detected in {elapsed:.2f}s (probe #{probe_count}, RTT={rtt:.1f}ms)!")
                if target_type == "PROBE":
                    print(f"  [TARGET] lin_break_probe active! AutoBAUD=0x{telem['auto_baud']:04X} ({telem['auto_baud']})")
                elif target_type == "OPTIBOOT":
                    print(f"  [TARGET] Optiboot_OL4 active (4-byte PING response).")
                time.sleep(0.150)
                return True

            if time.perf_counter() - last_print >= 1.0:
                last_print = time.perf_counter()
                elapsed = int(time.perf_counter() - t_start)
                rx_info = f" (last rx: {len(raw)}B)" if raw else ""
                print(f"  ... probing #{probe_count} ({elapsed}s / {int(max_wait_sec)}s){rx_info} ...")

            time.sleep(0.04)

        print(f"\n[STAGE 1: FAIL] No response from Core-D after {max_wait_sec:.1f}s.")
        return False


def run_benchmark(
    port: str,
    baudrate: int = 19200,
    count: int = 30,
    delimiter_delay_us: float = 0.0,
    switch_delay_us: float = 0.0,
    slot_interval_ms: float = 40.0,
    skip_stage1: bool = False,
    debug_port: Optional[str] = None,
    debug_baud: int = 38400
) -> float:
    print("=" * 72)
    print(f" LIN Break & LINAUTO Benchmark @ {baudrate} bps (count={count})")
    print(f" Port={port}, Break=9600bps (1042us), Delimiter=104us")
    if debug_port:
        print(f" Dual-Channel Mode: Debug Telemetry on {debug_port} @ {debug_baud} bps")
    print(f" Timing: delim_delay={delimiter_delay_us:.0f}us, switch_delay={switch_delay_us:.0f}us, interval={slot_interval_ms:.0f}ms")
    print("=" * 72)

    dbg_listener = None
    if debug_port:
        dbg_listener = DebugListener(port=debug_port, baudrate=debug_baud)
        dbg_listener.start()

    tester = LinBreakTester(port=port, baudrate=baudrate)
    try:
        tester.open()
    except Exception as e:
        print(f"[ERROR] Failed to open {port}: {e}")
        if dbg_listener:
            dbg_listener.stop()
        return 0.0

    if not skip_stage1:
        if not tester.wait_for_power_on(max_wait_sec=30.0, dbg_listener=dbg_listener):
            tester.close()
            if dbg_listener:
                dbg_listener.stop()
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
            mode="baud_trick",
            pid=0x80,
            delimiter_delay_us=delimiter_delay_us,
            switch_delay_us=switch_delay_us
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
                if isfif_diff > 0: err_str += f"+ISFIF(+{isfif_diff})"

                print(f"{i:>4} | {'PROBE':>8} | {'PASS':>8} | {rtt:>8.2f} | {telem['frame_count']:>7} | {cur_isfif:>7} | 0x{telem['auto_baud']:04X}   | {err_str:>10}")
            else:
                print(f"{i:>4} | {'OPTIBOOT':>8} | {'PASS':>8} | {rtt:>8.2f} | {'-':>7} | {'-':>7} | {'-':>8} | 4B PING OK")
        else:
            timeout_count += 1
            raw_str = raw.hex() if raw else 'empty'
            print(f"{i:>4} | {target_type:>8} | {'FAIL':>8} | {rtt:>8.2f} | {'-':>7} | {'-':>7} | {'-':>8} | rx={len(raw)}B ({raw_str})")

        # Display any debug telemetry received from COM21 for this trial
        if dbg_listener:
            time.sleep(0.01) # Short settle for UART
            msgs = dbg_listener.get_messages()
            for m in msgs:
                print(f"      |-> [COM21] {m}")

        time.sleep(slot_interval_ms / 1000.0)

    tester.close()
    if dbg_listener:
        time.sleep(0.2)
        # Flush remaining messages
        msgs = dbg_listener.get_messages()
        for m in msgs:
            print(f"      |-> [COM21] {m}")
        dbg_listener.stop()

    # Summary Statistics
    print("\n" + "=" * 72)
    print(f" BENCHMARK RESULTS SUMMARY (@ {baudrate} bps):")
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
        print(f" Ideal 20MHz BAUD   : 0x02B6 (694)")
    print("=" * 72)
    return success_rate


def run_sweep(port: str, baudrate: int = 19200, debug_port: Optional[str] = None, debug_baud: int = 38400):
    """Parameter sweep across delay configurations at 19,200 bps."""
    print("\n" + "#" * 72)
    print(f" AUTOMATIC PARAMETER SWEEP BENCHMARK @ {baudrate} bps")
    print("#" * 72)

    tester = LinBreakTester(port=port, baudrate=baudrate)
    tester.open()
    connected = tester.wait_for_power_on(max_wait_sec=30.0)
    tester.close()
    if not connected:
        return

    configs = [
        # delim_us, switch_us, interval_ms, label
        (   0.0,  0.0, 40.0, "baud_trick (standard 1050us physical completion wait)"),
        (1050.0, 50.0, 40.0, "baud_trick (1050us completion + 50us switch recovery)"),
        (1100.0,  0.0, 40.0, "baud_trick (1100us completion wait)"),
        (1050.0,  0.0, 60.0, "baud_trick (relaxed 60ms slot interval)"),
    ]

    sweep_results = []
    trials_per_config = 30

    for d_us, s_us, int_ms, label in configs:
        print(f"\n---> Testing Config: {label}")
        rate = run_benchmark(
            port=port,
            baudrate=baudrate,
            count=trials_per_config,
            delimiter_delay_us=d_us,
            switch_delay_us=s_us,
            slot_interval_ms=int_ms,
            skip_stage1=True,
            debug_port=debug_port,
            debug_baud=debug_baud
        )
        sweep_results.append((label, rate))
        time.sleep(0.1)

    print("\n" + "=" * 72)
    print(f" SWEEP RESULTS RANKING (@ {baudrate} bps):")
    print("=" * 72)
    for label, rate in sweep_results:
        bar = "█" * int(rate / 5)
        print(f" {rate:5.1f}% | {bar:<20} | {label}")
    print("=" * 72)


def main():
    parser = argparse.ArgumentParser(description="LIN Break & LINAUTO Benchmark Tool (19200 bps)")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial port under test (default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 Baud rate (default: 19200)")
    parser.add_argument("--debug-port", default=None, help="CH342K Debug Telemetry port, e.g. COM21 (default: None)")
    parser.add_argument("--debug-baud", type=int, default=38400, help="Debug telemetry baud rate (default: 38400)")
    parser.add_argument("--count", type=int, default=30, help="Number of benchmark trials (default: 30)")
    parser.add_argument("--delim-us", type=float, default=0.0, help="Delimiter delay in microseconds (default: 0.0)")
    parser.add_argument("--switch-us", type=float, default=0.0, help="Switch recovery delay in microseconds (default: 0.0)")
    parser.add_argument("--interval-ms", type=float, default=40.0, help="Slot interval in ms (default: 40.0)")
    parser.add_argument("--sweep", action="store_true", help="Run automated parameter sweep")
    parser.add_argument("--skip-stage1", action="store_true", help="Skip power-on wait loop")

    args = parser.parse_args()

    if args.sweep:
        run_sweep(port=args.port, baudrate=args.baud, debug_port=args.debug_port, debug_baud=args.debug_baud)
    else:
        run_benchmark(
            port=args.port,
            baudrate=args.baud,
            count=args.count,
            delimiter_delay_us=args.delim_us,
            switch_delay_us=args.switch_us,
            slot_interval_ms=args.interval_ms,
            skip_stage1=args.skip_stage1,
            debug_port=args.debug_port,
            debug_baud=args.debug_baud
        )


if __name__ == "__main__":
    main()
