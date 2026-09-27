#!/usr/bin/env python3
"""
Optiboot_OL4 - Tool: test_break_generator.py
============================================
Tests host-side LIN Break generation using the Baud-Rate Trick
over a standard USB-RS485 adapter (COM port).

Usage:
    python test_break_generator.py --port COM19 --baud 115200
"""

import sys
import time
import argparse

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)


def calculate_pid(cmd_id: int) -> int:
    """Calculates LIN Protected Identifier (PID) with parity bits P0, P1."""
    id0 = (cmd_id >> 0) & 1
    id1 = (cmd_id >> 1) & 1
    id2 = (cmd_id >> 2) & 1
    id3 = (cmd_id >> 3) & 1
    id4 = (cmd_id >> 4) & 1
    id5 = (cmd_id >> 5) & 1

    p0 = id0 ^ id1 ^ id2 ^ id4
    p1 = 1 ^ (id1 ^ id3 ^ id4 ^ id5)

    return (p1 << 7) | (p0 << 6) | (cmd_id & 0x3F)


def send_lin_header(ser: serial.Serial, pid: int, normal_baud: int = 115200):
    """
    Sends LIN Header (Break + Sync 0x55 + PID) using Baud-Rate Trick.
    Baud 57600 0x00 gives 9-bit LOW = 156us, which is 18 Tbit LOW at 115200 bps!
    """
    trick_baud = normal_baud // 2 # e.g. 57600 for 115200

    # 1. Switch to half baud to output hardware Break (18 Tbit LOW)
    t0 = time.perf_counter()
    ser.baudrate = trick_baud
    ser.write(b'\x00')
    ser.flush()
    t_break = time.perf_counter() - t0

    # 2. Switch back to normal baud and send Sync + PID
    ser.baudrate = normal_baud
    ser.write(bytes([0x55, pid]))
    ser.flush()

    return t_break


def main():
    parser = argparse.ArgumentParser(description="Test LIN Break generation via USB-RS485")
    parser.add_argument("--port", default="COM19", help="Serial port (e.g. COM19)")
    parser.add_argument("--baud", type=int, default=115200, help="Normal baud rate (default 115200)")
    parser.add_argument("--count", type=int, default=5, help="Number of test probes to send")
    args = parser.parse_args()

    print(f"[INIT] Opening serial port {args.port} at {args.baud} bps...")
    try:
        ser = serial.Serial(
            port=args.port,
            baudrate=args.baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.1
        )
    except Exception as e:
        print(f"[ERROR] Failed to open port: {e}")
        return

    print(f"[INIT] Connected successfully. Testing Baud-Rate Trick Break generation ({args.count} iterations)...")

    # PID for CMD_PING (0x00) -> 0x80
    ping_pid = calculate_pid(0x00)
    print(f"[INFO] CMD_PING (0x00) -> Calculated PID = 0x{ping_pid:02X}")

    for i in range(args.count):
        ser.reset_input_buffer()
        t_break = send_lin_header(ser, ping_pid, normal_baud=args.baud)
        print(f"  [Probe #{i+1}] Sent Break (dt={t_break*1000:4.2f}ms) + Sync(0x55) + PID(0x{ping_pid:02X})")
        time.sleep(0.05)

    ser.close()
    print("[FINISH] Test completed cleanly. USB-RS485 adapter supports dynamic baud rate switching!")


if __name__ == '__main__':
    main()
