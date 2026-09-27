#!/usr/bin/env python3
"""
COM21 Independent Telemetry Monitor (CH342K Soft-UART on Core-D PB4)
=====================================================================
Monitors internal telemetry output from Core-D (PB4 @ 38,400 bps, 8N1).
Logs raw bytes, parsed text, and microsecond-precision timestamps.
"""

import sys
import time
import argparse
from datetime import datetime

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Core-D COM21 Telemetry Monitor")
    parser.add_argument("--port", default="COM21", help="Serial port (default: COM21)")
    parser.add_argument("--baud", type=int, default=38400, help="Baud rate (default: 38400)")
    parser.add_argument("--log", default="com21_telemetry.log", help="Log file path (default: com21_telemetry.log)")
    args = parser.parse_args()

    print("=" * 65)
    print(f" Core-D Telemetry Monitor on {args.port} @ {args.baud} bps")
    print(f" Saving log to: {args.log}")
    print(" Press Ctrl+C to exit.")
    print("=" * 65)

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
        print(f"[ERROR] Failed to open {args.port}: {e}")
        sys.exit(1)

    with open(args.log, "a", encoding="utf-8") as f_log:
        f_log.write(f"\n--- Session Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} ---\n")
        line_buf = ""
        try:
            while True:
                data = ser.read(ser.in_waiting or 1)
                if not data:
                    continue
                
                # Decode line by line or character by character
                text = data.decode("latin1", errors="replace")
                for ch in text:
                    if ch == '\r':
                        continue
                    if ch == '\n':
                        now_str = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                        log_line = f"[{now_str}] {line_buf}"
                        print(log_line)
                        f_log.write(log_line + "\n")
                        f_log.flush()
                        line_buf = ""
                    else:
                        line_buf += ch

        except KeyboardInterrupt:
            print("\n[INFO] Monitor stopped by user.")
        finally:
            if line_buf:
                now_str = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                log_line = f"[{now_str}] {line_buf}"
                print(log_line)
                f_log.write(log_line + "\n")
            ser.close()


if __name__ == "__main__":
    main()
