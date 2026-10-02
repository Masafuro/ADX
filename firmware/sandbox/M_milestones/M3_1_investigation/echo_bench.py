#!/usr/bin/env python3
"""
M3-1 Pure Physical RS-485 Echo Bench (32-byte Mirror Verification)
SPDX-License-Identifier: MIT

Usage:
  python echo_bench.py --port COM22 [--baud 115200]
"""

import sys
import time
import argparse
from typing import Optional, Tuple

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] 'pyserial' is not installed.")
    sys.exit(1)


def run_echo_pattern(ser: serial.Serial, tx_data: bytes, pattern_name: str) -> bool:
    assert len(tx_data) == 32
    print("-" * 65)
    print(f" [PATTERN] {pattern_name}")
    print(f"  TX HEX (32B): {tx_data.hex()}")

    # Clear input buffer
    ser.reset_input_buffer()
    t_start = time.perf_counter()

    ser.write(tx_data)
    ser.flush()

    rx_data = b""
    deadline = t_start + 0.15  # 150ms timeout
    while len(rx_data) < 32 and time.perf_counter() < deadline:
        chunk = ser.read(32 - len(rx_data))
        if chunk:
            rx_data += chunk

    rtt_ms = (time.perf_counter() - t_start) * 1000.0

    print(f"  RX HEX ({len(rx_data)}B): {rx_data.hex()} (RTT={rtt_ms:.2f}ms)")

    if len(rx_data) == 0:
        print("  [FAIL] TIMEOUT (0 bytes received from target!)")
        return False

    if rx_data == tx_data:
        print("  [PASS] ★ 100% BIT-FOR-BIT PERFECT MIRROR ECHO! ★")
        return True

    # Byte-by-byte comparison
    print("  [DIFF ANALYSIS]")
    mismatches = 0
    diff_str = []
    for i in range(max(len(tx_data), len(rx_data))):
        tb = f"{tx_data[i]:02X}" if i < len(tx_data) else "--"
        rb = f"{rx_data[i]:02X}" if i < len(rx_data) else "--"
        if tb == rb:
            diff_str.append(f"{i:02d}:{tb}=={rb}")
        else:
            diff_str.append(f"{i:02d}:{tb}!={rb}")
            mismatches += 1

    print("   " + " ".join(diff_str[:16]))
    print("   " + " ".join(diff_str[16:32]))

    # Shift check
    if len(rx_data) == 32:
        if rx_data[1:] == tx_data[:31]:
            print(f"  [CRITICAL INSIGHT] RX is RIGHT-SHIFTED by 1 byte! First byte is 0x{rx_data[0]:02X}")
        elif rx_data[:31] == tx_data[1:]:
            print("  [CRITICAL INSIGHT] RX is LEFT-SHIFTED by 1 byte! First byte was skipped!")

    return False


def main():
    parser = argparse.ArgumentParser(description="M3-1 Pure RS-485 Physical Echo Bench")
    parser.add_argument("--port", "-p", type=str, help="RS-485 Serial Port (e.g. COM22)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    args = parser.parse_args()

    if not args.port:
        ports = serial.tools.list_ports.comports()
        print("[DETECTED PORTS]")
        for p in ports:
            print(f"  - {p.device}: {p.description}")
        print("\nPlease specify serial port using --port <PORT_NAME>")
        sys.exit(0)

    print("=" * 65)
    print("     M3-1 PURE RS-485 PHYSICAL MIRROR ECHO TEST SUITE")
    print(f"     Port: {args.port} @ {args.baud} bps (8N1)")
    print("=" * 65)

    try:
        ser = serial.Serial(
            port=args.port,
            baudrate=args.baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.01,
            write_timeout=0.2
        )
    except Exception as e:
        print(f"[ERROR] Could not open port {args.port}: {e}")
        sys.exit(1)

    try:
        time.sleep(0.05)

        # Pattern 1: Sequential Counter (0x00..0x1F) - NO 0x55 or 0xAD inside
        p1 = bytes(range(32))
        run_echo_pattern(ser, p1, "Sequential 0x00..0x1F (No 0x55 inside)")

        time.sleep(0.05)

        # Pattern 2: Standard MR32 Ping Header
        p2 = bytes.fromhex("55ad01001001000000000000000000000000000000000000000000000000e5aa")
        run_echo_pattern(ser, p2, "Standard MR32 Ping Frame (Starts with 0x55 0xAD)")

        time.sleep(0.05)

        # Pattern 3: Inverted Checkerboard (0xAA 0x55 alternating)
        p3 = bytes([0xAA, 0x55] * 16)
        run_echo_pattern(ser, p3, "Alternating 0xAA 0x55 (Square Wave Stress)")

        time.sleep(0.05)

        # Pattern 4: Constant ASCII text
        p4 = b"ADX CORE-D RS485 PURE ECHO 2026!"
        run_echo_pattern(ser, p4, "ASCII Text String (32 Bytes)")

        print("=" * 65)

    finally:
        ser.close()
        print("[INFO] Port closed.")


if __name__ == "__main__":
    main()
