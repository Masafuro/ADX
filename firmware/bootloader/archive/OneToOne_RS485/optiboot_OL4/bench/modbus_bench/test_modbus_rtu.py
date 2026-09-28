#!/usr/bin/env python3
"""
MODBUS-RTU Style RS-485 High-Reliability Ping-Pong Benchmark
============================================================
Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)
Features:
  - Pure Standard Asynchronous UART @ 19,200 bps (8N1) - NO LIN, NO Break Trick!
  - Robust Silent-Interval (t3.5) frame delimitation and automatic recovery.
  - Standard MODBUS CRC-16 error checking.
  - Simultaneous dual-channel telemetry monitoring via COM21 (9,600 bps).

Usage:
  python test_modbus_rtu.py --port COM19 --debug-port COM21 --count 100
  python test_modbus_rtu.py --port COM19 --count 100
"""

import sys
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


def calc_modbus_crc16(data: bytes) -> int:
    """MODBUS RTU standard CRC-16 (Polynomial 0xA001, initial 0xFFFF)."""
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


class DebugListener:
    """Background listener for Core-D CH342K Soft-UART telemetry on COM21."""
    def __init__(self, port: str, baudrate: int = 9600, log_path: str = "com21_modbus.log"):
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
            self.f_log.write(f"\n--- MODBUS Session Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} ---\n")
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


class ModbusMaster:
    def __init__(self, port: str, baudrate: int = 19200, timeout: float = 0.10):
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

    def ping(self, seq: int) -> Tuple[bool, str, Optional[Dict], float, bytes]:
        """
        Sends MODBUS RTU Ping packet and awaits 8-byte response.
        Request:  [0x01, 0x03, seq_h, seq_l, crc_l, crc_h] (6 bytes)
        Response: [0x01, 0x03, seq_h, seq_l, count_h, count_l, crc_l, crc_h] (8 bytes)
        """
        # Ensure 3.5 character silent interval before frame (>1.8ms @ 19200)
        time.sleep(0.005)
        self.ser.reset_input_buffer()

        req_payload = bytes([0x01, 0x03, (seq >> 8) & 0xFF, seq & 0xFF])
        crc = calc_modbus_crc16(req_payload)
        packet = req_payload + bytes([crc & 0xFF, (crc >> 8) & 0xFF])

        t_start = time.perf_counter()
        self.ser.write(packet)
        self.ser.flush()

        resp = self.ser.read(8)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        if len(resp) < 8:
            return False, "TIMEOUT", None, rtt_ms, resp

        # Validate CRC
        calc_resp_crc = calc_modbus_crc16(resp[:6])
        recv_resp_crc = resp[6] | (resp[7] << 8)

        if calc_resp_crc != recv_resp_crc:
            return False, "CRC_ERR", {"calc_crc": calc_resp_crc, "recv_crc": recv_resp_crc}, rtt_ms, resp

        resp_seq = (resp[2] << 8) | resp[3]
        resp_count = (resp[4] << 8) | resp[5]

        if resp_seq != seq:
            return False, "SEQ_ERR", {"sent_seq": seq, "resp_seq": resp_seq}, rtt_ms, resp

        telemetry = {
            "seq": resp_seq,
            "slave_count": resp_count,
            "raw_rx": resp
        }
        return True, "PASS", telemetry, rtt_ms, resp


def run_benchmark(
    port: str,
    baudrate: int = 19200,
    count: int = 100,
    interval_ms: float = 20.0,
    debug_port: Optional[str] = None,
    debug_baud: int = 9600
):
    print("=" * 72)
    print(f" MODBUS-RTU Style RS-485 Benchmark @ {baudrate} bps (count={count})")
    print(f" Port={port}, Silent Interval=5ms, Slot Interval={interval_ms}ms")
    if debug_port:
        print(f" Dual-Channel Mode: Debug Telemetry on {debug_port} @ {debug_baud} bps")
    print("=" * 72)

    dbg = None
    if debug_port:
        dbg = DebugListener(port=debug_port, baudrate=debug_baud)
        dbg.start()

    master = ModbusMaster(port=port, baudrate=baudrate)
    try:
        master.open()
    except Exception as e:
        print(f"[ERROR] Failed to open {port}: {e}")
        if dbg:
            dbg.stop()
        return

    # Check connectivity with 3 initial pings
    print("[INFO] Testing initial ping-pong connectivity...")
    init_ok = False
    for trial in range(5):
        ok, status, telem, rtt, raw = master.ping(seq=0x1000 + trial)
        if ok:
            print(f"  [PASS] Initial handshake OK (RTT={rtt:.2f}ms, Slave Count={telem['slave_count']})")
            init_ok = True
            break
        time.sleep(0.05)

    if not init_ok:
        print("[WARN] Slave did not respond to initial pings. Proceeding with benchmark anyway...")

    success_count = 0
    fail_count = 0
    rtt_list = []

    print(f"{'#':>4} | {'Status':>8} | {'RTT(ms)':>8} | {'Seq':>7} | {'SlaveCnt':>9} | {'Info':>12}")
    print("-" * 72)

    for i in range(1, count + 1):
        ok, status, telem, rtt, raw = master.ping(seq=i)
        rtt_list.append(rtt)

        if ok and telem:
            success_count += 1
            print(f"{i:>4} | {'PASS':>8} | {rtt:>8.2f} | 0x{telem['seq']:04X} | {telem['slave_count']:>9} | OK")
        else:
            fail_count += 1
            raw_str = raw.hex() if raw else 'empty'
            print(f"{i:>4} | {status:>8} | {rtt:>8.2f} | {'-':>7} | {'-':>9} | rx={len(raw)}B ({raw_str})")

        # Telemetry messages from COM21
        if dbg:
            time.sleep(0.005)
            msgs = dbg.get_messages()
            for m in msgs:
                print(f"      |-> [COM21] {m}")

        time.sleep(interval_ms / 1000.0)

    master.close()
    if dbg:
        time.sleep(0.1)
        for m in dbg.get_messages():
            print(f"      |-> [COM21] {m}")
        dbg.stop()

    print("\n" + "=" * 72)
    print(f" MODBUS-RTU BENCHMARK SUMMARY (@ {baudrate} bps):")
    print("=" * 72)
    success_rate = (success_count / count) * 100.0
    print(f" Total Trials       : {count}")
    print(f" Successful Frames  : {success_count} ({success_rate:.1f}%)")
    print(f" Failures / Timeouts: {fail_count}")

    if rtt_list:
        pass_rtts = [rtt_list[idx] for idx in range(len(rtt_list)) if idx < success_count]
        if pass_rtts:
            print(f" PASS RTT Min/Avg/Max: {min(pass_rtts):.2f} ms / {sum(pass_rtts)/len(pass_rtts):.2f} ms / {max(pass_rtts):.2f} ms")
    print("=" * 72)


def main():
    parser = argparse.ArgumentParser(description="MODBUS-RTU Style RS-485 Benchmark")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial port under test (default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 Baud rate (default: 19200)")
    parser.add_argument("--debug-port", default=None, help="CH342K Debug Telemetry port, e.g. COM21 (default: None)")
    parser.add_argument("--debug-baud", type=int, default=9600, help="Debug telemetry baud rate (default: 9600)")
    parser.add_argument("--count", type=int, default=100, help="Number of benchmark trials (default: 100)")
    parser.add_argument("--interval-ms", type=float, default=20.0, help="Slot interval in ms (default: 20.0)")

    args = parser.parse_args()

    run_benchmark(
        port=args.port,
        baudrate=args.baud,
        count=args.count,
        interval_ms=args.interval_ms,
        debug_port=args.debug_port,
        debug_baud=args.debug_baud
    )


if __name__ == "__main__":
    main()
