#!/usr/bin/env python3
"""
Optiboot_OL4 - Host Diagnostic & Flasher Tool
=============================================
Communicates with ADX Core-D via LN-485 (LIN-based RS-485) protocol.
Supports dynamic Break generation via Baud-Rate Trick, CRC16 verification,
and full hex flash programming.

Usage:
    python ol4_flasher.py --port COM19 --probe
    python ol4_flasher.py --port COM19 --info
    python ol4_flasher.py --port COM19 --read-page 0x0400
    python ol4_flasher.py --port COM19 --hex ../releases/test_rs485_serial.hex
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

# Protocol PIDs
PID_PING        = 0x80  # ID 0x00
PID_GET_INFO    = 0xC1  # ID 0x01
PID_SET_ADDR    = 0x42  # ID 0x02
PID_WRITE_PAGE  = 0x03  # ID 0x03
PID_READ_PAGE   = 0xC4  # ID 0x04
PID_REBOOT      = 0x85  # ID 0x05

# Status Codes
STATUS_OK          = 0x00
STATUS_ERR_CRC     = 0x01
STATUS_ERR_ADDR    = 0x02
STATUS_ERR_FLASH   = 0x03
STATUS_ERR_UNKNOWN = 0xFF

PAGE_SIZE = 64


def crc16_ccitt(data: bytes, initial: int = 0xFFFF) -> int:
    crc = initial
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def hex_dump(data: bytes) -> str:
    return " ".join(f"{b:02X}" for b in data)


class OL4Flasher:
    def __init__(self, port_name: str, baud_rate: int = 115200, timeout: float = 0.5):
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

    def send_header(self, pid: int):
        """Sends Break (18 Tbit LOW via Baud Trick) + Sync(0x55) + PID."""
        trick_baud = self.baud_rate // 2 # 57600 bps for 115200 bps
        self.ser.baudrate = trick_baud
        self.ser.write(b'\x00')
        self.ser.flush()

        self.ser.baudrate = self.baud_rate
        self.ser.write(bytes([0x55, pid]))
        self.ser.flush()

    def receive_response(self, timeout: float = 0.5) -> Tuple[bool, int, bytes]:
        """
        Receives Slave response: [Status: 1B] + [Length: 1B] + [Payload: N Bytes] + [CRC16: 2B]
        Returns: (success, status_code, payload)
        """
        self.ser.timeout = timeout
        # 1. Read Status + Length (2 bytes)
        hdr = self.ser.read(2)
        if len(hdr) < 2:
            return False, STATUS_ERR_UNKNOWN, b""

        status = hdr[0]
        length = hdr[1]

        # 2. Read Payload + CRC16
        rest = self.ser.read(length + 2)
        if len(rest) < (length + 2):
            return False, status, b""

        payload = rest[:length]
        rx_crc = (rest[length] << 8) | rest[length + 1]

        # 3. Verify CRC16 if length > 0
        if length > 0:
            calc_crc = crc16_ccitt(payload)
            if calc_crc != rx_crc:
                print(f"  [CRC ERROR] Expected 0x{calc_crc:04X}, received 0x{rx_crc:04X}")
                return False, STATUS_ERR_CRC, payload

        return (status == STATUS_OK), status, payload

    def ping(self, timeout: float = 0.3) -> bool:
        """Sends CMD_PING and expects STATUS_OK."""
        self.ser.reset_input_buffer()
        t0 = time.time()
        self.send_header(PID_PING)
        ok, status, _ = self.receive_response(timeout=timeout)
        rtt_ms = (time.time() - t0) * 1000.0
        if ok:
            print(f"  [PING PASS] Core-D responded with STATUS_OK (RTT={rtt_ms:.1f}ms)")
            return True
        else:
            print(f"  [PING FAIL] status=0x{status:02X} RTT={rtt_ms:.1f}ms")
            return False

    def poll_power_on(self, max_wait_sec: float = 30.0) -> bool:
        """Waits for Core-D power on by sending periodic PING headers."""
        print(f"\n[STAGE 1] Waiting for Core-D power on/reset (up to {max_wait_sec}s)...")
        print(">>> POWER ON OR RESET CORE-D NOW <<<")
        start = time.time()
        probes = 0
        while time.time() - start < max_wait_sec:
            probes += 1
            self.ser.reset_input_buffer()
            self.send_header(PID_PING)
            ok, status, _ = self.receive_response(timeout=0.08)
            if ok:
                elapsed = time.time() - start
                print(f"[STAGE 1: PASS] Power-on detected in {elapsed:.2f}s (probe #{probes})!")
                return True
            time.sleep(0.04)
        print("[STAGE 1: FAIL] Timeout waiting for Core-D.")
        return False

    def get_info(self) -> Optional[Tuple[str, str]]:
        """Queries Device Signature and Bootloader Version."""
        self.ser.reset_input_buffer()
        self.send_header(PID_GET_INFO)
        ok, status, payload = self.receive_response(timeout=0.3)
        if ok and len(payload) >= 5:
            sig = f"0x{payload[0]:02X} 0x{payload[1]:02X} 0x{payload[2]:02X}"
            ver = f"{payload[3]}.{payload[4]}"
            print(f"  [DEVICE INFO] Signature: {sig} | Optiboot_OL4 Version: {ver}")
            return sig, ver
        return None

    def set_address(self, addr: int) -> bool:
        """Sets target Flash address."""
        self.ser.reset_input_buffer()
        self.send_header(PID_SET_ADDR)
        payload = bytes([2, addr & 0xFF, (addr >> 8) & 0xFF, 0x00, 0x00])
        self.ser.write(payload)
        self.ser.flush()
        ok, status, _ = self.receive_response(timeout=0.2)
        return ok

    def write_page(self, addr: int, data: bytes) -> bool:
        """Writes a 64-byte Flash page with CRC16."""
        if not self.set_address(addr):
            print(f"  [ERROR] Failed to set address 0x{addr:04X}")
            return False

        crc = crc16_ccitt(data)
        packet = bytes([PAGE_SIZE]) + data + bytes([(crc >> 8) & 0xFF, crc & 0xFF])

        self.ser.reset_input_buffer()
        self.send_header(PID_WRITE_PAGE)
        self.ser.write(packet)
        self.ser.flush()

        # Writing flash takes ~25ms
        ok, status, _ = self.receive_response(timeout=0.2)
        return ok

    def read_page(self, addr: int) -> Optional[bytes]:
        """Reads a 64-byte Flash page."""
        if not self.set_address(addr):
            print(f"  [ERROR] Failed to set address 0x{addr:04X}")
            return None

        self.ser.reset_input_buffer()
        self.send_header(PID_READ_PAGE)
        ok, status, payload = self.receive_response(timeout=0.3)
        if ok and len(payload) == PAGE_SIZE:
            return payload
        return None

    def reboot(self):
        """Sends REBOOT command to launch application."""
        print("[REBOOT] Requesting Core-D reboot into application...")
        self.ser.reset_input_buffer()
        self.send_header(PID_REBOOT)
        self.receive_response(timeout=0.1)


def parse_intel_hex(hex_path: str) -> Dict[int, int]:
    data = {}
    with open(hex_path, 'r') as f:
        for line in f:
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
    parser = argparse.ArgumentParser(description="Optiboot_OL4 (LN-485) Host Flasher")
    parser.add_argument("--port", default="COM19", help="Serial port (e.g. COM19)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default 115200)")
    parser.add_argument("--probe", action="store_true", help="Send PING probe")
    parser.add_argument("--info", action="store_true", help="Read device signature and version")
    parser.add_argument("--read-page", default=None, help="Read 64B page at hex address (e.g. 0x0400)")
    parser.add_argument("--hex", default=None, help="Hex file to write & verify")
    parser.add_argument("--reboot", action="store_true", help="Reboot to application")
    args = parser.parse_args()

    flasher = OL4Flasher(args.port, args.baud)
    try:
        flasher.connect()

        if not flasher.poll_power_on():
            return

        if args.probe:
            flasher.ping()

        if args.info or args.hex:
            flasher.get_info()

        if args.read_page:
            target_addr = int(args.read_page, 16)
            print(f"\n[READ PAGE] Address 0x{target_addr:04X}...")
            data = flasher.read_page(target_addr)
            if data:
                print(f"Read 64 bytes: {hex_dump(data[:16])} ...")

        if args.hex:
            hex_data = parse_intel_hex(args.hex)
            print(f"[HEX] Loaded {len(hex_data)} bytes from {args.hex}")

            addrs = list(hex_data.keys())
            min_addr = min(addrs)
            max_addr = max(addrs)

            # Optiboot_OL4 application starts at 0x0400
            start_p = max(0x0400, (min_addr // PAGE_SIZE) * PAGE_SIZE)
            end_p = ((max_addr + PAGE_SIZE) // PAGE_SIZE) * PAGE_SIZE
            total_pages = (end_p - start_p) // PAGE_SIZE

            print(f"[PLAN] Flashing {total_pages} pages (0x{start_p:04X} ~ 0x{end_p:04X})...")
            all_ok = True
            for p_idx in range(total_pages):
                curr_addr = start_p + p_idx * PAGE_SIZE
                page_bytes = bytes([hex_data.get(curr_addr + i, 0xFF) for i in range(PAGE_SIZE)])

                print(f"  Flashing Page {p_idx+1}/{total_pages} @ 0x{curr_addr:04X}...", end="", flush=True)
                t_w0 = time.time()
                if not flasher.write_page(curr_addr, page_bytes):
                    print(" [WRITE FAIL]")
                    all_ok = False
                    break

                readback = flasher.read_page(curr_addr)
                t_page = (time.time() - t_w0) * 1000.0
                if readback == page_bytes:
                    print(f" [VERIFY PASS] ({t_page:.1f}ms)")
                else:
                    print(" [VERIFY MISMATCH]")
                    all_ok = False
                    break

            if all_ok:
                print("\n[SUCCESS] All pages written and verified with CRC16 successfully!")
                flasher.reboot()

        if args.reboot and not args.hex:
            flasher.reboot()

    except KeyboardInterrupt:
        print("\n[ABORT] Interrupted by user.")
    finally:
        flasher.close()


if __name__ == '__main__':
    main()
