#!/usr/bin/env python3
"""
ADX Core-D RS-485 Interactive Debug Probe Tool
Sends single STK_GET_SYNC packet upon Enter key, dumps all raw RX bytes in HEX/ASCII.
Used for Milestone 3 packet boundary and physical loopback investigation.
"""

import sys
import time
import argparse

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is not installed. Please run: pip install pyserial")
    sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="ADX Core-D RS-485 Debug Probe Tool")
    parser.add_argument("port", nargs="?", default="COM19", help="COM port of USB-RS485 adapter (default: COM19)")
    parser.add_argument("-b", "--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    args = parser.parse_args()

    print("=" * 65)
    print("  ADX Core-D RS-485 Interactive Debug Probe Tool")
    print("=" * 65)
    print(f"Target Port : {args.port}")
    print(f"Baud Rate   : {args.baud} bps")
    print("-" * 65)

    try:
        ser = serial.Serial(
            port=args.port,
            baudrate=args.baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.05
        )
    except Exception as e:
        print(f"[ERROR] Failed to open port '{args.port}': {e}")
        sys.exit(1)

    print(f"[INFO] Port {args.port} opened successfully.")
    print("\nInstructions:")
    print("  1. Power on Core-D and confirm Red LED (PB2) is blinking.")
    print("  2. Press [ENTER] to send a single STK_GET_SYNC (0x30 0x20).")
    print("  3. Observe whether LED stops and if any bytes are received.")
    print("  4. Type 'q' and press [ENTER] to quit.\n")

    while True:
        try:
            cmd = input("Press [ENTER] to send probe (or 'q' to quit) > ")
        except (KeyboardInterrupt, EOFError):
            break

        if cmd.strip().lower() == 'q':
            break

        ser.reset_input_buffer()
        ser.reset_output_buffer()

        probe = b'\x30\x20'
        print(f"\n[TX] Sending: {probe.hex(' ')} ('0 ') ...")
        t0 = time.time()
        ser.write(probe)
        ser.flush()

        # Listen for 500ms
        raw_rx = bytearray()
        while time.time() - t0 < 0.5:
            chunk = ser.read(64)
            if chunk:
                raw_rx.extend(chunk)

        if raw_rx:
            hex_str = ' '.join(f"{b:02X}" for b in raw_rx)
            ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in raw_rx)
            print(f"[RX] Received {len(raw_rx)} bytes: {hex_str} (ASCII: '{ascii_str}')")
            if 0x14 in raw_rx and 0x10 in raw_rx:
                print("  --> [SUCCESS] 0x14 (STK_INSYNC) and 0x10 (STK_OK) detected!")
            else:
                print("  --> [INFO] Data received, but not the expected 0x14 0x10.")
        else:
            print("[RX] No response received within 500ms.")

        print("-" * 65)

    ser.close()
    print("\nExited.")

if __name__ == '__main__':
    main()
