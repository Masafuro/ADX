#!/usr/bin/env python3
"""
WU-5-1 Diagnostic Probe Comparison Tool (Python Reference)
Target MCU: ATtiny1616 with wu5_diag.hex on ADX Core-D

Tests two break generation methods against wu5_diag firmware:
  Test A: Method 1 (Half-Baud Trick) - Gold standard from WU-0/M4-1
  Test B: Method 2 (send_break API)  - WebSerial/Win32 equivalent

Usage:
  python wu5_diag_probe.py --port COM19
"""

import sys
import time
import argparse
import serial

def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

def build_frame(cmd: int, seq: int) -> bytes:
    # 32-Byte frame: cmd(1) + seq(1) + 28 bytes zeros + CRC(2)
    payload = bytes([cmd, seq]) + (b'\x00' * 28)
    crc = crc16_ccitt(payload)
    return payload + bytes([(crc >> 8) & 0xFF, crc & 0xFF])

def run_test(ser: serial.Serial, method_name: str, use_half_baud: bool, count: int = 5):
    print(f"\n{'='*72}")
    print(f"  RUNNING TEST: {method_name}")
    print(f"{'='*72}")

    for i in range(1, count + 1):
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        time.sleep(0.05)

        t_start = time.perf_counter()
        frame = build_frame(cmd=0x01, seq=i)

        if use_half_baud:
            # Method 1: Half-Baud Trick (WU-0 Gold Standard)
            ser.baudrate = 9600
            ser.write(b'\x00')
            ser.flush()
            ser.baudrate = 19200
        else:
            # Method 2: send_break (Win32 SetCommBreak/ClearCommBreak)
            ser.send_break(duration=0.002) # ~2ms break
            time.sleep(0.0005)              # 0.5ms delimiter

        # Send Sync (0x55) + 32-Byte Master Frame
        ser.write(b'\x55' + frame)
        ser.flush()

        # Read 32-Byte Slave Response
        rx = ser.read(32)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        if len(rx) == 32:
            calc_crc = crc16_ccitt(rx[:30])
            rx_crc = (rx[30] << 8) | rx[31]
            crc_ok = (calc_crc == rx_crc)
            status = rx[0]
            rx_count = rx[1]
            b0 = rx[2]
            b1 = rx[3]
            uid = " ".join(f"{b:02X}" for b in rx[4:14])
            raw_dump = " ".join(f"{b:02X}" for b in rx[14:30])

            status_str = "DIAG_OK (0xAA)" if status == 0xAA else f"STATUS_0x{status:02X}"
            crc_str = "CRC_OK" if crc_ok else "CRC_ERR"

            print(f"[#{i}] RTT:{rtt_ms:5.1f}ms | {status_str} | SlaveCaptured:{rx_count:2d}B | "
                  f"Byte[0]:0x{b0:02X} Byte[1]:0x{b1:02X} | {crc_str}")
            print(f"     ↳ Slave Buffer Dump[0..15]: {raw_dump}")
        else:
            print(f"[#{i}] TIMEOUT / Incomplete: Received {len(rx)}/32 bytes (RTT: {rtt_ms:.1f}ms)")

        time.sleep(0.15) # 150ms interval

def main():
    parser = argparse.ArgumentParser(description="WU-5-1 Diagnostic Probe Comparison Tool")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial Port (default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="Baudrate (default: 19200)")
    args = parser.parse_args()

    print("============================================================================")
    print("       WU-5-1 DIAGNOSTIC PROBE COMPARISON: HALF-BAUD vs SEND_BREAK")
    print("============================================================================")
    print(f" Port: {args.port} @ {args.baud} bps")
    print(f" Target Firmware: wu5_diag.hex on ADX Core-D")
    print("============================================================================")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.25)
    except Exception as e:
        print(f"[ERROR] Failed to open {args.port}: {e}")
        sys.exit(1)

    try:
        # Test A: Half-Baud Trick (Expected: Byte[0]=0x01, Byte[1]=seq, Captured=32B)
        run_test(ser, "Method 1: Half-Baud Trick (WU-0 Standard)", use_half_baud=True, count=3)

        # Test B: send_break (Expected: Mirroring WebSerial behavior)
        run_test(ser, "Method 2: send_break (WebSerial equivalent)", use_half_baud=False, count=3)

    finally:
        ser.close()
        print("\n[DONE] Serial port closed.")

if __name__ == "__main__":
    main()
