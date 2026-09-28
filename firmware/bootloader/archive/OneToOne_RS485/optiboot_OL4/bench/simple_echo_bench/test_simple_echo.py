#!/usr/bin/env python3
"""
Simple RS-485 Byte Echo Benchmark Tool (9600 bps)
==================================================
Sends characters/strings to Core-D over RS-485 (COM19 @ 9600 bps)
and checks for immediate hardware echo.
Simultaneously monitors CH342K internal debug telemetry on COM21 @ 9600 bps.

Usage:
  python test_simple_echo.py --port COM19 --debug-port COM21
  python test_simple_echo.py --port COM19 --debug-port COM21 --count 20
  python test_simple_echo.py --port COM19 --debug-port COM21 --interactive
"""

import sys
import time
import queue
import threading
import argparse
from datetime import datetime
from typing import Optional, List

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)


class DebugListener:
    """Background listener for Core-D CH342K Soft-UART telemetry on COM21."""
    def __init__(self, port: str, baudrate: int = 9600, log_path: str = "com21_echo.log"):
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
            self.f_log.write(f"\n--- Echo Session Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} ---\n")
            self.f_log.flush()
            self.thread = threading.Thread(target=self._worker, daemon=True)
            self.thread.start()
            print(f"[DEBUG] Telemetry listener active on {self.port} @ {self.baudrate} bps -> {self.log_path}")
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
                time.sleep(0.2)
                continue

    def get_messages(self) -> List[str]:
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


def run_auto_test(port: str, debug_port: Optional[str] = None, count: int = 20):
    print("=" * 70)
    print(f" Simple RS-485 Byte Echo Test @ 9600 bps (Trials={count})")
    print(f" Port={port}, 8N1, Single-Byte Ping-Pong")
    if debug_port:
        print(f" Debug Telemetry on {debug_port} @ 9600 bps")
    print("=" * 70)

    dbg = None
    if debug_port:
        dbg = DebugListener(port=debug_port, baudrate=9600)
        dbg.start()

    try:
        ser = serial.Serial(
            port=port,
            baudrate=9600,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.3
        )
    except Exception as e:
        print(f"[ERROR] Failed to open {port}: {e}")
        if dbg:
            dbg.stop()
        return

    time.sleep(0.1)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    success = 0
    test_chars = [b'A', b'B', b'C', b'X', b'Y', b'Z', b'1', b'2', b'3', b'#']

    print(f"{'#':>4} | {'Sent':>6} | {'Status':>8} | {'Echoed':>8} | {'RTT(ms)':>8} | {'Info':>10}")
    print("-" * 70)

    for i in range(1, count + 1):
        ch_to_send = test_chars[(i - 1) % len(test_chars)]
        
        ser.reset_input_buffer()
        t_start = time.perf_counter()
        
        # Send 1 byte over RS-485
        ser.write(ch_to_send)
        ser.flush()

        # Await 1 byte echo
        echo = ser.read(1)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        if echo == ch_to_send:
            success += 1
            print(f"{i:>4} | {ch_to_send.decode():>6} | {'PASS':>8} | {echo.decode():>8} | {rtt_ms:>8.2f} | OK")
        elif len(echo) > 0:
            print(f"{i:>4} | {ch_to_send.decode():>6} | {'MISMATCH':>8} | {echo.hex():>8} | {rtt_ms:>8.2f} | Byte mismatch")
        else:
            print(f"{i:>4} | {ch_to_send.decode():>6} | {'TIMEOUT':>8} | {'-':>8} | {rtt_ms:>8.2f} | No echo (0B)")

        # Poll COM21 telemetry
        if dbg:
            time.sleep(0.01)
            msgs = dbg.get_messages()
            for m in msgs:
                print(f"      |-> [COM21] {m}")

        time.sleep(0.1) # 100ms interval between trials

    ser.close()
    if dbg:
        time.sleep(0.1)
        for m in dbg.get_messages():
            print(f"      |-> [COM21] {m}")
        dbg.stop()

    print("\n" + "=" * 70)
    rate = (success / count) * 100.0
    print(f" RESULTS: {success}/{count} echoed successfully ({rate:.1f}%)")
    print("=" * 70)


def run_interactive(port: str, debug_port: Optional[str] = None):
    print("=" * 70)
    print(f" Interactive RS-485 Terminal @ 9600 bps")
    print(f" Type any text and press Enter to send to Core-D.")
    print(" Type 'quit' to exit.")
    print("=" * 70)

    dbg = None
    if debug_port:
        dbg = DebugListener(port=debug_port, baudrate=9600)
        dbg.start()

    ser = serial.Serial(port=port, baudrate=9600, timeout=0.5)
    time.sleep(0.05)

    try:
        while True:
            text = input("\nSend > ")
            if text.strip() == "quit":
                break
            if not text:
                continue

            for ch in text.encode():
                ser.write(bytes([ch]))
                ser.flush()
                echo = ser.read(1)
                if echo:
                    echo_char = chr(echo[0]) if 32 <= echo[0] <= 126 else f"0x{echo[0]:02X}"
                    print(f"  Echo: '{echo_char}'", end="", flush=True)
                else:
                    print("  [TIMEOUT]", end="", flush=True)
                time.sleep(0.01)
            print()

            if dbg:
                time.sleep(0.02)
                for m in dbg.get_messages():
                    print(f"  [COM21] {m}")

    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        if dbg:
            dbg.stop()


def main():
    parser = argparse.ArgumentParser(description="Simple RS-485 Byte Echo Benchmark (9600 bps)")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial port (default: COM19)")
    parser.add_argument("--debug-port", default=None, help="CH342K Debug Telemetry port (default: None)")
    parser.add_argument("--count", type=int, default=20, help="Number of automatic echo trials (default: 20)")
    parser.add_argument("--interactive", action="store_true", help="Interactive keyboard mode")

    args = parser.parse_args()

    if args.interactive:
        run_interactive(port=args.port, debug_port=args.debug_port)
    else:
        run_auto_test(port=args.port, debug_port=args.debug_port, count=args.count)


if __name__ == "__main__":
    main()
