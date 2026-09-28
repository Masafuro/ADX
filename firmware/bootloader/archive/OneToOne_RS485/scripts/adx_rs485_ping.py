#!/usr/bin/env python3
"""
ADX Core-D RS-485 Bootloader Ping Utility (Milestone 3)
Verifies physical and logical communication between host PC and Core-D over RS-485.

Protocol: STK500v1 (Optiboot)
Baud rate: 115,200 bps (8N1)
"""

import sys
import time
import argparse

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] pyserial is not installed. Please run: pip install pyserial")
    sys.exit(1)

# STK500v1 Protocol Constants
STK_GET_SYNC      = b'\x30\x20'      # '0 '
STK_INSYNC        = 0x14             # 20
STK_OK            = 0x10             # 16
STK_GET_PARAMETER = 0x41             # 'A'
STK_SW_MAJOR      = 0x81
STK_SW_MINOR      = 0x82

def find_default_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = p.description.lower()
        if "ch340" in desc or "ch341" in desc or "ch342" in desc or "ftdi" in desc or "usb-serial" in desc or "prolific" in desc or "cp210" in desc:
            return p.device
    if ports:
        return ports[0].device
    return "COM19"

def ping_loop(port_name, baudrate=115200, timeout_sec=30):
    print("=" * 65)
    print("  ADX Core-D 1-to-1 RS-485 Bootloader Ping Utility (M3)")
    print("=" * 65)
    print(f"Target Port : {port_name}")
    print(f"Baud Rate   : {baudrate} bps (8N1)")
    print(f"Timeout     : {timeout_sec} seconds")
    print("-" * 65)

    try:
        ser = serial.Serial(
            port=port_name,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.08  # 80ms read timeout
        )
    except Exception as e:
        print(f"[ERROR] Failed to open port '{port_name}': {e}")
        print("Please check that the port name is correct and not used by another program.")
        sys.exit(1)

    print(f"[INFO] Port {port_name} opened successfully.")
    print("[INFO] Waiting for Core-D to power on / reset...")
    print(">>> Power on or reset the Core-D NOW (Red LED PB2 will start blinking) <<<\n")

    start_time = time.time()
    last_print = 0
    dots = 0

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    while time.time() - start_time < timeout_sec:
        elapsed = int(time.time() - start_time)
        if elapsed != last_print:
            last_print = elapsed
            dots = (dots + 1) % 4
            sys.stdout.write(f"\r[POLLING] Sending STK_GET_SYNC probe... ({elapsed}s / {timeout_sec}s) {'.' * dots:3s}")
            sys.stdout.flush()

        # Send STK_GET_SYNC
        t_sent = time.time()
        ser.write(STK_GET_SYNC)
        ser.flush()

        # Read response (look for 0x14 0x10)
        resp = ser.read(8)
        if STK_INSYNC in resp:
            idx = resp.index(STK_INSYNC)
            # Check if followed by STK_OK
            if len(resp) > idx + 1 and resp[idx + 1] == STK_OK:
                rtt_ms = (time.time() - t_sent) * 1000.0
                print("\n\n" + "=" * 65)
                print("  [PASS] ADX Bootloader is ALIVE on RS-485!")
                print("=" * 65)
                print(f"Response Received : 0x14 (STK_INSYNC) 0x10 (STK_OK)")
                print(f"Round-Trip Time   : {rtt_ms:.1f} ms")

                # Try reading version parameters
                try:
                    # Minor version
                    ser.write(bytes([STK_GET_PARAMETER, STK_SW_MINOR, 0x20]))
                    ser.flush()
                    v_resp = ser.read(4)
                    minor = v_resp[1] if len(v_resp) >= 3 and v_resp[0] == STK_INSYNC else None

                    # Major version
                    ser.write(bytes([STK_GET_PARAMETER, STK_SW_MAJOR, 0x20]))
                    ser.flush()
                    v_resp = ser.read(4)
                    major = v_resp[1] if len(v_resp) >= 3 and v_resp[0] == STK_INSYNC else None

                    if major is not None and minor is not None:
                        print(f"Bootloader Ver    : Optiboot {major}.{minor}")
                except Exception:
                    pass

                print("=" * 65)
                print("[RESULT] Milestone 3 RS-485 Communication Test: PASS\n")
                ser.close()
                return True

        time.sleep(0.04)  # 40ms interval

    print(f"\n\n[TIMEOUT] No response from Core-D after {timeout_sec} seconds.")
    print("Troubleshooting Checklist:")
    print("  1. [CRITICAL] Did Red LED (PB2) turn off IMMEDIATELY upon power-on?")
    print("     --> A and B wires are almost certainly reversed! Swap A and B on the terminal block.")
    print("  2. Did Red LED blink normally for ~5-8s but no response was received?")
    print("     --> Check GND connection, COM port selection, or driver status.")
    print("  3. Check Device Manager to ensure the USB-RS485 adapter is COM19.")
    ser.close()
    return False

def main():
    parser = argparse.ArgumentParser(description="ADX Core-D RS-485 Bootloader Ping Utility")
    parser.add_argument("port", nargs="?", default=None, help="COM port of the USB-RS485 adapter (e.g. COM19)")
    parser.add_argument("-b", "--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("-t", "--timeout", type=int, default=30, help="Timeout in seconds (default: 30)")
    args = parser.parse_args()

    port = args.port or find_default_port()
    success = ping_loop(port, args.baud, args.timeout)
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
