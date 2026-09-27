#!/usr/bin/env python3
"""
ADX Core-D RS-485 Optiboot Protocol Debug & Diagnostic Tool
===========================================================
This tool performs step-by-step, low-level STK500v1 protocol transactions
with the ADX Core-D over a USB-RS485 adapter, printing full raw TX/RX hex dumps,
precise timestamps, and round-trip times for every single packet.

Usage:
    python debug_flasher.py --port COM19 --baud 115200 --hex ../releases/test_rs485_serial.hex
    python debug_flasher.py --port COM19 --read-only --start 0x0200 --pages 11
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

# STK500 Constants
STK_OK             = 0x10
STK_FAILED         = 0x11
STK_UNKNOWN        = 0x12
STK_NODEVICE       = 0x13
STK_INSYNC         = 0x14
STK_NOSYNC         = 0x15
CRC_EOP            = 0x20
STK_GET_SYNC       = 0x30
STK_GET_PARAMETER  = 0x41
STK_SET_DEVICE     = 0x42
STK_SET_DEVICE_EXT = 0x45
STK_ENTER_PROGMODE = 0x50
STK_LEAVE_PROGMODE = 0x51
STK_LOAD_ADDRESS   = 0x55
STK_UNIVERSAL      = 0x56
STK_PROG_PAGE      = 0x64
STK_READ_PAGE      = 0x74
STK_READ_SIGN      = 0x75

PAGE_SIZE = 64


def hex_dump(data: bytes) -> str:
    return " ".join(f"{b:02X}" for b in data)


class OptibootDebugger:
    def __init__(self, port_name: str, baud_rate: int = 115200, timeout: float = 1.0):
        self.port_name = port_name
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.ser: Optional[serial.Serial] = None

    def connect(self):
        print(f"[INIT] Opening serial port {self.port_name} at {self.baud_rate} bps...")
        self.ser = serial.Serial(
            port=self.port_name,
            baudrate=self.baud_rate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=self.timeout
        )
        time.sleep(0.1)
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        print(f"[INIT] Connected successfully to {self.port_name}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("[INFO] Serial port closed.")

    def send_cmd(self, cmd_bytes: bytes, expect_payload_len: int = 0, timeout: float = 1.0) -> Tuple[bool, bytes, bytes]:
        """
        Sends [cmd_bytes, CRC_EOP], then reads STK_INSYNC + optional payload + STK_OK.
        Returns: (success, payload, raw_rx_data)
        """
        self.ser.timeout = timeout
        self.ser.reset_input_buffer()

        packet = cmd_bytes + bytes([CRC_EOP])
        t_start = time.time()
        self.ser.write(packet)
        self.ser.flush()

        # Expected total bytes to read: 1 (INSYNC) + expect_payload_len + 1 (OK)
        expected_len = 1 + expect_payload_len + 1
        rx = self.ser.read(expected_len)
        rtt_ms = (time.time() - t_start) * 1000.0

        tx_hex = hex_dump(packet)
        rx_hex = hex_dump(rx) if rx else "<TIMEOUT NO DATA>"

        if len(rx) < expected_len:
            print(f"  [TX] {tx_hex}")
            print(f"  [RX TIMEOUT/SHORT] len={len(rx)}/{expected_len} RTT={rtt_ms:.1f}ms: {rx_hex}")
            return False, b"", rx

        if rx[0] != STK_INSYNC:
            print(f"  [TX] {tx_hex}")
            print(f"  [RX SYNC FAIL] expected 0x14, got 0x{rx[0]:02X} RTT={rtt_ms:.1f}ms: {rx_hex}")
            return False, b"", rx

        if rx[-1] != STK_OK:
            print(f"  [TX] {tx_hex}")
            print(f"  [RX OK FAIL] expected 0x10, got 0x{rx[-1]:02X} RTT={rtt_ms:.1f}ms: {rx_hex}")
            return False, b"", rx

        payload = rx[1:-1]
        print(f"  [TX] {tx_hex:<24} -> [RX] {rx_hex:<28} (RTT={rtt_ms:4.1f}ms) PASS")
        return True, payload, rx

    def poll_power_on(self, max_wait_sec: float = 30.0) -> bool:
        print(f"\n[STAGE 1] Waiting for Core-D power on/reset (up to {max_wait_sec}s)...")
        print(">>> POWER ON OR RESET CORE-D NOW <<<")
        start = time.time()
        poll_count = 0
        while time.time() - start < max_wait_sec:
            poll_count += 1
            self.ser.reset_input_buffer()
            self.ser.write(bytes([STK_GET_SYNC, CRC_EOP]))
            self.ser.flush()

            self.ser.timeout = 0.08
            rx = self.ser.read(2)
            if len(rx) == 2 and rx[0] == STK_INSYNC and rx[1] == STK_OK:
                elapsed = time.time() - start
                print(f"[STAGE 1: PASS] Power-on detected in {elapsed:.2f}s (probe #{poll_count})!")
                return True
            time.sleep(0.04)
        print("[STAGE 1: FAIL] Timeout waiting for power-on response.")
        return False

    def enter_progmode(self) -> bool:
        print("\n[STAGE 2] Checking Sync & Entering Programming Mode...")
        # 2 handshake checks
        for i in range(2):
            ok, _, _ = self.send_cmd(bytes([STK_GET_SYNC]), 0, timeout=0.2)
            if not ok:
                print(f"[STAGE 2: FAIL] Sync probe {i+1} failed.")
                return False

        # Read signature
        ok, sig, _ = self.send_cmd(bytes([STK_READ_SIGN]), 3, timeout=0.3)
        if ok:
            sig_str = " ".join(f"0x{b:02X}" for b in sig)
            print(f"  Device Signature: {sig_str}")

        # Enter ProgMode
        ok, _, _ = self.send_cmd(bytes([STK_ENTER_PROGMODE]), 0, timeout=0.3)
        if ok:
            print("[STAGE 2: PASS] Successfully entered programming mode!")
            return True
        else:
            print("[STAGE 2: FAIL] Failed to enter programming mode.")
            return False

    def probe_health(self):
        print("\n  [HEALTH CHECK] Probing if Core-D is still alive in bootloader...")
        for i in range(3):
            time.sleep(0.05)
            ok, _, raw = self.send_cmd(bytes([STK_GET_SYNC]), 0, timeout=0.3)
            if ok:
                print(f"  [HEALTH: ALIVE] Core-D responded to sync probe #{i+1}! It is NOT dead, but sync was lost.")
                return True
        print("  [HEALTH: DEAD] Core-D did NOT respond to sync probes. It likely reset or crashed.")
        return False

    def test_page_write_and_read(self, page_addr: int, data: bytes, delay: float = 0.02) -> bool:
        """
        Tests programming a single 64-byte page and then immediately reading it back.
        """
        print(f"\n--- Testing Page 0x{page_addr:04X} ({len(data)} bytes) ---")
        addr_low = page_addr & 0xFF
        addr_high = (page_addr >> 8) & 0xFF

        # 1. Load Address for Write
        time.sleep(delay)
        ok, _, _ = self.send_cmd(bytes([STK_LOAD_ADDRESS, addr_low, addr_high]), 0, timeout=0.3)
        if not ok:
            print(f"  [ERROR] Load address failed for write at 0x{page_addr:04X}")
            self.probe_health()
            return False

        # 2. Program Page
        time.sleep(delay)
        prog_packet = bytes([STK_PROG_PAGE, 0x00, PAGE_SIZE, ord('F')]) + data
        ok, _, _ = self.send_cmd(prog_packet, 0, timeout=1.0)
        if not ok:
            print(f"  [ERROR] Write page failed at 0x{page_addr:04X}")
            self.probe_health()
            return False

        # 3. Load Address for Read
        time.sleep(delay)
        ok, _, _ = self.send_cmd(bytes([STK_LOAD_ADDRESS, addr_low, addr_high]), 0, timeout=0.3)
        if not ok:
            print(f"  [ERROR] Load address failed for read at 0x{page_addr:04X}")
            self.probe_health()
            return False

        # 4. Read Page
        time.sleep(delay)
        read_cmd = bytes([STK_READ_PAGE, 0x00, PAGE_SIZE, ord('F')])
        ok, read_buf, _ = self.send_cmd(read_cmd, PAGE_SIZE, timeout=1.0)
        if not ok:
            print(f"  [ERROR] Read page failed at 0x{page_addr:04X}")
            self.probe_health()
            return False

        # 5. Verify
        if read_buf == data:
            print(f"  [VERIFY PASS] Page 0x{page_addr:04X} matches perfectly!")
            return True
        else:
            print(f"  [VERIFY MISMATCH] Page 0x{page_addr:04X}:")
            for i in range(min(len(data), len(read_buf))):
                if data[i] != read_buf[i]:
                    print(f"    Offset +{i:02d} (0x{page_addr+i:04X}): Expected 0x{data[i]:02X}, Read 0x{read_buf[i]:02X}")
            return False

    def test_page_read_only(self, page_addr: int, delay: float = 0.02) -> Optional[bytes]:
        """
        Reads a 64-byte page without writing.
        """
        addr_low = page_addr & 0xFF
        addr_high = (page_addr >> 8) & 0xFF

        time.sleep(delay)
        ok, _, _ = self.send_cmd(bytes([STK_LOAD_ADDRESS, addr_low, addr_high]), 0, timeout=0.3)
        if not ok:
            print(f"  [ERROR] Load address failed for read at 0x{page_addr:04X}")
            self.probe_health()
            return None

        time.sleep(delay)
        read_cmd = bytes([STK_READ_PAGE, 0x00, PAGE_SIZE, ord('F')])
        ok, read_buf, raw = self.send_cmd(read_cmd, PAGE_SIZE, timeout=1.0)
        if not ok:
            print(f"  [ERROR] Read page failed at 0x{page_addr:04X}")
            self.probe_health()
            return None
        return read_buf

    def leave_progmode(self):
        print("\n[STAGE: EXIT] Leaving programming mode...")
        self.send_cmd(bytes([STK_LEAVE_PROGMODE]), 0, timeout=0.3)


def parse_intel_hex(hex_path: str) -> Dict[int, int]:
    data = {}
    with open(hex_path, 'r') as f:
        for line_no, line in enumerate(f, 1):
            line = line.strip()
            if not line.startswith(':'):
                continue
            byte_count = int(line[1:3], 16)
            addr = int(line[3:7], 16)
            rec_type = int(line[7:9], 16)
            if rec_type == 0x00:
                for i in range(byte_count):
                    data[addr + i] = int(line[9 + i*2: 11 + i*2], 16)
            elif rec_type == 0x01:
                break
    return data


def main():
    parser = argparse.ArgumentParser(description="ADX Core-D RS-485 Optiboot Protocol Debugger")
    parser.add_argument("--port", default="COM19", help="Serial port (e.g. COM19 or /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default 115200)")
    parser.add_argument("--hex", help="Path to HEX file to test")
    parser.add_argument("--read-only", action="store_true", help="Run read-only verification without writing")
    parser.add_argument("--start", default="0x0200", help="Start address (hex, default 0x0200)")
    parser.add_argument("--pages", type=int, default=11, help="Number of 64B pages to test (default 11)")
    parser.add_argument("--single-page", default=None, help="Test only a single page (hex, e.g. 0x0440)")
    parser.add_argument("--delay", type=float, default=0.02, help="Quiet delay between packets in seconds (default 0.02 = 20ms)")
    args = parser.parse_args()

    dbg = OptibootDebugger(args.port, args.baud)
    try:
        dbg.connect()
        if not dbg.poll_power_on():
            return

        if not dbg.enter_progmode():
            return

        start_addr = int(args.start, 16)

        if args.single_page is not None:
            target_addr = int(args.single_page, 16)
            print(f"\n=== TARGET TEST: SINGLE PAGE 0x{target_addr:04X} ===")
            read_data = dbg.test_page_read_only(target_addr, delay=args.delay)
            if read_data:
                print(f"Read {len(read_data)} bytes: {hex_dump(read_data[:16])} ...")
            dbg.leave_progmode()
            return

        if args.read_only:
            print(f"\n=== MODE: READ-ONLY AUDIT ({args.pages} pages from 0x{start_addr:04X}, delay={args.delay*1000:.0f}ms) ===")
            for p_idx in range(args.pages):
                curr_addr = start_addr + p_idx * PAGE_SIZE
                print(f"Auditing page {p_idx+1}/{args.pages} @ 0x{curr_addr:04X}...")
                read_buf = dbg.test_page_read_only(curr_addr, delay=args.delay)
                if read_buf is None:
                    print(f"!!! HALTED AT PAGE 0x{curr_addr:04X} !!!")
                    break
            dbg.leave_progmode()
            return

        if args.hex:
            hex_path = args.hex
            if not os.path.exists(hex_path):
                # Try relative to script directory
                script_dir = os.path.dirname(os.path.abspath(__file__))
                candidate = os.path.join(script_dir, "..", "releases", os.path.basename(hex_path))
                if os.path.exists(candidate):
                    hex_path = candidate
                else:
                    candidate = os.path.join(script_dir, hex_path)
                    if os.path.exists(candidate):
                        hex_path = candidate

            hex_data = parse_intel_hex(hex_path)
            print(f"[HEX] Loaded {len(hex_data)} bytes from {hex_path}")

            addrs = list(hex_data.keys())
            min_addr = min(addrs)
            max_addr = max(addrs)
            start_p = (min_addr // PAGE_SIZE) * PAGE_SIZE
            end_p = ((max_addr + PAGE_SIZE) // PAGE_SIZE) * PAGE_SIZE
            total_pages = (end_p - start_p) // PAGE_SIZE
            print(f"[PLAN] Total pages: {total_pages} (0x{start_p:04X} ~ 0x{end_p:04X}, delay={args.delay*1000:.0f}ms)")

            print("\n=== MODE: FULL WRITE & VERIFY PER PAGE ===")
            for p_idx in range(total_pages):
                curr_addr = start_p + p_idx * PAGE_SIZE
                page_bytes = bytes([hex_data.get(curr_addr + i, 0xFF) for i in range(PAGE_SIZE)])
                success = dbg.test_page_write_and_read(curr_addr, page_bytes, delay=args.delay)
                if not success:
                    print(f"!!! HALTED AT PAGE 0x{curr_addr:04X} !!!")
                    break

            dbg.leave_progmode()
            print("\n[FINISH] Test session ended.")

    except KeyboardInterrupt:
        print("\n[ABORT] User interrupted.")
    finally:
        dbg.close()


if __name__ == '__main__':
    main()
