#!/usr/bin/env python3
"""
Robust RS-485 Packet Benchmark @ 9,600 bps (Step 2)
Tests 8-byte structured packet roundtrip reliability between PC and ADX Core-D.

Channel 1 (COM19): RS-485 Bus @ 9600 bps
Channel 2 (COM21): Independent Debug Telemetry @ 9600 bps

Packet Protocol (8 Bytes):
  [0] STX (0x02)
  [1] SEQ (0x00 - 0xFF)
  [2] CMD / RESP (0x01=PING, 0x02=ECHO, 0x03=STATS / 0x06=ACK, 0x15=NAK_CHK)
  [3] DATA0
  [4] DATA1
  [5] DATA2
  [6] CHECKSUM = sum(bytes[0:6]) & 0xFF
  [7] ETX (0x03)
"""

import sys
import time
import argparse
import random
import threading
from typing import Optional, Tuple
import serial

PKT_STX = 0x02
PKT_ETX = 0x03
PKT_LEN = 8

CMD_PING      = 0x01
CMD_ECHO      = 0x02
CMD_GET_STATS = 0x03

RESP_ACK        = 0x06
RESP_NAK_CHKSUM = 0x15
RESP_NAK_CMD    = 0x16

class TelemetryListener:
    def __init__(self, port: str, baudrate: int = 9600, log_path: str = "com21_packet.log"):
        self.port = port
        self.baudrate = baudrate
        self.log_path = log_path
        self.running = False
        self.thread = None
        self.ser = None
        self.lock = threading.Lock()
        self.recent_lines = []

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
            self.running = True
            self.thread = threading.Thread(target=self._run, daemon=True)
            self.thread.start()
            print(f"[DEBUG] Telemetry listener active on {self.port} @ {self.baudrate} bps -> {self.log_path}")
        except Exception as e:
            print(f"[WARN] Failed to open telemetry port {self.port}: {e}")

    def _run(self):
        with open(self.log_path, "a", encoding="utf-8", errors="replace") as f:
            buf = bytearray()
            while self.running:
                try:
                    data = self.ser.read(64)
                    if data:
                        buf.extend(data)
                        while b"\n" in buf:
                            line_raw, buf = buf.split(b"\n", 1)
                            line = line_raw.decode("ascii", errors="replace").strip("\r")
                            now_str = time.strftime("%H:%M:%S") + f".{int(time.time()*1000)%1000:03d}"
                            entry = f"[{now_str}] {line}"
                            f.write(entry + "\n")
                            f.flush()
                            with self.lock:
                                self.recent_lines.append(entry)
                                if len(self.recent_lines) > 50:
                                    self.recent_lines.pop(0)
                except Exception:
                    break

    def get_and_clear_recent(self):
        with self.lock:
            lines = list(self.recent_lines)
            self.recent_lines.clear()
            return lines

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass


def build_packet(seq: int, cmd: int, d0: int, d1: int, d2: int) -> bytes:
    seq &= 0xFF
    cmd &= 0xFF
    d0 &= 0xFF
    d1 &= 0xFF
    d2 &= 0xFF
    chk = (PKT_STX + seq + cmd + d0 + d1 + d2) & 0xFF
    return bytes([PKT_STX, seq, cmd, d0, d1, d2, chk, PKT_ETX])


def parse_packet(pkt: bytes) -> Tuple[bool, str, dict]:
    if len(pkt) != PKT_LEN:
        return False, f"Length mismatch (expected {PKT_LEN}, got {len(pkt)})", {}

    stx, seq, code, d0, d1, d2, chk, etx = pkt

    if stx != PKT_STX:
        return False, f"Invalid STX (0x{stx:02X} != 0x{PKT_STX:02X})", {}

    if etx != PKT_ETX:
        return False, f"Invalid ETX (0x{etx:02X} != 0x{PKT_ETX:02X})", {}

    expected_chk = (stx + seq + code + d0 + d1 + d2) & 0xFF
    if chk != expected_chk:
        return False, f"Checksum error (expected 0x{expected_chk:02X}, got 0x{chk:02X})", {}

    info = {
        "stx": stx,
        "seq": seq,
        "code": code,
        "d0": d0,
        "d1": d1,
        "d2": d2,
        "chk": chk,
        "etx": etx
    }
    return True, "OK", info


def run_benchmark(port: str, debug_port: Optional[str], count: int, interval: float, timeout: float):
    print("=" * 72)
    print(f" RS-485 Packet Benchmark @ 9600 bps (Trials={count})")
    print(f" Port={port}, 8N1, 8-Byte Fixed Frame (STX=0x02, ETX=0x03)")
    print(f" Packet Interval={interval*1000:.1f}ms, Read Timeout={timeout*1000:.1f}ms")
    if debug_port:
        print(f" Dual-Channel Mode: Debug Telemetry on {debug_port} @ 9600 bps")
    print("=" * 72)

    telemetry = None
    if debug_port:
        telemetry = TelemetryListener(debug_port, 9600)
        telemetry.start()

    time.sleep(0.3)

    try:
        ser = serial.Serial(
            port=port,
            baudrate=9600,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=timeout,
            write_timeout=1.0
        )
    except Exception as e:
        print(f"[FATAL] Cannot open test port {port}: {e}")
        if telemetry:
            telemetry.stop()
        sys.exit(1)

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    print(f"\n{'#':>4} | {'Cmd':>4} | {'Seq':>3} | {'Status':>8} | {'RTT(ms)':>7} | {'Resp':>4} | {'Data':>8} | {'Info'}")
    print("-" * 72)

    pass_count = 0
    fail_count = 0
    timeout_count = 0
    chksum_err_count = 0
    rtts = []

    for trial in range(1, count + 1):
        seq = trial & 0xFF

        # Alternate between CMD_PING and CMD_ECHO with random payload
        if trial % 2 == 1:
            cmd = CMD_PING
            d0, d1, d2 = trial & 0xFF, 0x11, 0x22
            cmd_name = "PING"
        else:
            cmd = CMD_ECHO
            d0 = random.randint(0, 255)
            d1 = random.randint(0, 255)
            d2 = random.randint(0, 255)
            cmd_name = "ECHO"

        tx_pkt = build_packet(seq, cmd, d0, d1, d2)

        ser.reset_input_buffer()

        t_start = time.perf_counter()
        ser.write(tx_pkt)
        ser.flush()

        rx_data = ser.read(PKT_LEN)
        t_rtt = (time.perf_counter() - t_start) * 1000.0

        status = "FAIL"
        resp_code_str = "--"
        data_str = "--------"
        info_str = ""

        if len(rx_data) == 0:
            status = "TIMEOUT"
            timeout_count += 1
            info_str = "No response"
        elif len(rx_data) < PKT_LEN:
            status = "SHORT"
            fail_count += 1
            info_str = f"Incomplete ({len(rx_data)}/{PKT_LEN} bytes: {rx_data.hex()})"
        else:
            valid, reason, parsed = parse_packet(rx_data)
            if not valid:
                status = "BAD_PKT"
                chksum_err_count += 1
                info_str = reason
            else:
                resp_code = parsed["code"]
                resp_code_str = f"0x{resp_code:02X}"
                data_str = f"{parsed['d0']:02X} {parsed['d1']:02X} {parsed['d2']:02X}"

                if parsed["seq"] != seq:
                    status = "SEQ_ERR"
                    fail_count += 1
                    info_str = f"Seq mismatch (sent {seq}, got {parsed['seq']})"
                elif resp_code != RESP_ACK:
                    status = "NAK"
                    fail_count += 1
                    info_str = f"Slave returned NAK (0x{resp_code:02X})"
                else:
                    if cmd == CMD_ECHO:
                        if (parsed["d0"], parsed["d1"], parsed["d2"]) == (d0, d1, d2):
                            status = "PASS"
                            pass_count += 1
                            rtts.append(t_rtt)
                            info_str = "Echo matched"
                        else:
                            status = "DATA_ERR"
                            fail_count += 1
                            info_str = "Payload mismatch"
                    else:
                        status = "PASS"
                        pass_count += 1
                        rtts.append(t_rtt)
                        info_str = f"Pong OK (slave_cnt=0x{parsed['d2']:02X})"

        rtt_str = f"{t_rtt:7.2f}" if status != "TIMEOUT" else "      -"
        print(f"{trial:4d} | {cmd_name:>4} | {seq:3d} | {status:>8} | {rtt_str} | {resp_code_str:>4} | {data_str} | {info_str}")

        if telemetry:
            dbg_lines = telemetry.get_and_clear_recent()
            for line in dbg_lines:
                print(f"      |-> [COM21] {line}")

        time.sleep(interval)

    ser.close()
    if telemetry:
        time.sleep(0.1)
        dbg_lines = telemetry.get_and_clear_recent()
        for line in dbg_lines:
            print(f"      |-> [COM21] {line}")
        telemetry.stop()

    print("\n" + "=" * 72)
    print(" BENCHMARK RESULTS SUMMARY")
    print("=" * 72)
    total_valid = pass_count
    succ_rate = (total_valid / count) * 100.0
    print(f" Total Trials       : {count}")
    print(f" Successful Packets : {total_valid} ({succ_rate:.1f}%)")
    print(f" Timeouts           : {timeout_count}")
    print(f" Errors             : {fail_count + chksum_err_count}")

    if rtts:
        min_rtt = min(rtts)
        max_rtt = max(rtts)
        avg_rtt = sum(rtts) / len(rtts)
        jitter = max_rtt - min_rtt
        print(f" RTT Statistics     : Min={min_rtt:.2f}ms, Avg={avg_rtt:.2f}ms, Max={max_rtt:.2f}ms, Jitter={jitter:.2f}ms")
    print("=" * 72)


def main():
    parser = argparse.ArgumentParser(description="Core-D RS-485 Packet Benchmark @ 9600 bps")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial Port (default: COM19)")
    parser.add_argument("--debug-port", default="COM21", help="Debug Telemetry Port (default: COM21)")
    parser.add_argument("--count", type=int, default=100, help="Number of benchmark trials (default: 100)")
    parser.add_argument("--interval", type=float, default=0.05, help="Inter-packet interval in seconds (default: 0.05)")
    parser.add_argument("--timeout", type=float, default=0.15, help="Read timeout in seconds (default: 0.15)")

    args = parser.parse_args()

    run_benchmark(
        port=args.port,
        debug_port=args.debug_port,
        count=args.count,
        interval=args.interval,
        timeout=args.timeout
    )


if __name__ == "__main__":
    main()
