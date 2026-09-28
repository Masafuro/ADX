#!/usr/bin/env python3
"""
Simple RS-485 Interactive Terminal / Monitor for testing Core-D User Applications
Connects to COM19 @ 9600 bps. Prints all incoming messages and allows typing messages.
"""

import sys
import time
import argparse
import threading
import serial

def main():
    parser = argparse.ArgumentParser(description="RS-485 Interactive Chat / Monitor")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial Port (default: COM19)")
    parser.add_argument("--baud", type=int, default=9600, help="Baud rate (default: 9600)")
    args = parser.parse_args()

    print("=" * 68)
    print(f" RS-485 Terminal / Monitor on {args.port} @ {args.baud} bps")
    print(" Listening for incoming messages from Core-D...")
    print(" Type any text and press ENTER to send. Press Ctrl+C to exit.")
    print("=" * 68)

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
        print(f"[FATAL] Cannot open serial port {args.port}: {e}")
        sys.exit(1)

    running = True

    # Background reader thread
    def read_loop():
        buf = bytearray()
        while running:
            try:
                data = ser.read(64)
                if data:
                    buf.extend(data)
                    while b"\n" in buf:
                        raw_line, buf = buf.split(b"\n", 1)
                        line = raw_line.decode("ascii", errors="replace").strip("\r")
                        now_str = time.strftime("%H:%M:%S")
                        print(f"\r[{now_str}] <RX> {line}")
                        sys.stdout.write("> ")
                        sys.stdout.flush()
                else:
                    time.sleep(0.01)
            except Exception:
                break

    reader_thread = threading.Thread(target=read_loop, daemon=True)
    reader_thread.start()

    # Foreground writer loop
    try:
        sys.stdout.write("> ")
        sys.stdout.flush()
        while True:
            line = sys.stdin.readline()
            if not line:
                break
            line_str = line.strip("\r\n")
            if line_str:
                # Send text over RS-485
                ser.write((line_str + "\r\n").encode("ascii"))
                ser.flush()
            sys.stdout.write("> ")
            sys.stdout.flush()
    except KeyboardInterrupt:
        print("\n[INFO] Exiting RS-485 Terminal.")
    finally:
        running = False
        ser.close()


if __name__ == "__main__":
    main()
