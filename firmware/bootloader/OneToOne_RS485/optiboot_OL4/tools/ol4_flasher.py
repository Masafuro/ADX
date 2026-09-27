#!/usr/bin/env python3
"""
Optiboot_OL4 - LN-485 Master Broker Diagnostic & Flasher Tool
============================================================
Communicates with ADX Core-D as an LN-485 Master Broker.
Enforces deterministic schedule slots (25ms control, 60ms flash write),
atomic frame transmissions (Header + Payload in single flush),
and CRC16-CCITT verification.

Usage:
    python ol4_flasher.py --port COM19 --probe
    python ol4_flasher.py --port COM19 --info
    python ol4_flasher.py --port COM19 --read-page 0x0400
    python ol4_flasher.py --port COM19 --hex ../releases/test_ol4_app_0400.hex
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

# LN-485 Master Schedule Slot Durations (seconds)
# User Strategy: Expanding Master polling cycles guarantees deterministic stability.
SLOT_CONTROL  = 0.040  # 40ms (25Hz) for PING, GET_INFO, SET_ADDR, READ_PAGE
SLOT_WRITE    = 0.100  # 100ms (10Hz) for WRITE_PAGE (NVM erase/write: ~25ms + 75ms slack)
PAGE_INTERVAL = 0.030  # 30ms quiet bus settlement interval between full page cycles


def status_str(status: int) -> str:
    if status == STATUS_OK:
        return "OK"
    elif status == STATUS_ERR_CRC:
        return "CRC_ERROR"
    elif status == STATUS_ERR_ADDR:
        return "ADDR_ERROR"
    elif status == STATUS_ERR_FLASH:
        return "FLASH_ERROR"
    elif status == STATUS_ERR_UNKNOWN:
        return "TIMEOUT"
    else:
        return f"0x{status:02X}"


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


class LN485MasterBroker:
    def __init__(self, port_name: str, baud_rate: int = 115200, timeout: float = 0.3):
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
        time.sleep(0.05)
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        print(f"[INIT] Connected successfully to {self.port_name}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("[INFO] Serial port closed.")

    def execute_slot(self, pid: int, payload: bytes = b"", slot_duration: float = SLOT_CONTROL, timeout: float = 0.25) -> Tuple[bool, int, bytes]:
        """
        Executes an atomic LN-485 transaction within a strict Master Schedule Slot:
          1. Sends Break (18 Tbit LOW via 57600bps 0x00).
          2. Sends [0x55, PID] + payload atomically in a single write/flush.
          3. Receives Slave response [Status, Len, Payload, CRC16].
          4. Strictly waits for the remainder of slot_duration (Slack Time conservation).
        """
        t_start = time.perf_counter()
        self.ser.reset_input_buffer()

        # 1. Hardware Break via Baud-Rate Trick
        trick_baud = self.baud_rate // 2  # 57600 bps
        self.ser.baudrate = trick_baud
        self.ser.write(b'\x00')
        self.ser.flush()

        # 2. Atomic Frame: Sync + PID + Payload in ONE write
        self.ser.baudrate = self.baud_rate
        frame_bytes = bytes([0x55, pid]) + payload
        self.ser.write(frame_bytes)
        self.ser.flush()

        # 3. Receive Slave Response
        self.ser.timeout = timeout
        hdr = self.ser.read(2)
        if len(hdr) < 2:
            ok, status, data = False, STATUS_ERR_UNKNOWN, b""
        else:
            status = hdr[0]
            length = hdr[1]
            rest = self.ser.read(length + 2)
            if len(rest) < (length + 2):
                ok, data = False, b""
            else:
                data = rest[:length]
                rx_crc = (rest[length] << 8) | rest[length + 1]
                if length > 0:
                    calc_crc = crc16_ccitt(data)
                    if calc_crc != rx_crc:
                        ok, status = False, STATUS_ERR_CRC
                    else:
                        ok = (status == STATUS_OK)
                else:
                    ok = (status == STATUS_OK)

        # 4. Enforce Slot Duration (Slack Time Conservation)
        t_elapsed = time.perf_counter() - t_start
        if t_elapsed < slot_duration:
            time.sleep(slot_duration - t_elapsed)

        return ok, status, data

    def ping(self) -> bool:
        """Sends CMD_PING and expects STATUS_OK."""
        t0 = time.time()
        ok, status, _ = self.execute_slot(PID_PING, b"", slot_duration=SLOT_CONTROL)
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
            ok, status, _ = self.execute_slot(PID_PING, b"", slot_duration=SLOT_CONTROL, timeout=0.06)
            if ok:
                elapsed = time.time() - start
                print(f"[STAGE 1: PASS] Power-on detected in {elapsed:.2f}s (probe #{probes})!")
                # Inter-stage settlement delay (ensure slave finishes TXCIF and WFB re-arm)
                time.sleep(0.080)
                return True
            time.sleep(0.02)
        print("[STAGE 1: FAIL] Timeout waiting for Core-D.")
        return False

    def get_info(self, max_retries: int = 3) -> Optional[Tuple[str, str]]:
        """Queries Device Signature and Bootloader Version with visible retry telemetry."""
        status = STATUS_ERR_UNKNOWN
        for attempt in range(1, max_retries + 1):
            ok, status, payload = self.execute_slot(PID_GET_INFO, b"", slot_duration=SLOT_CONTROL)
            if ok and len(payload) >= 5:
                sig = f"0x{payload[0]:02X} 0x{payload[1]:02X} 0x{payload[2]:02X}"
                ver = f"{payload[3]}.{payload[4]}"
                if attempt > 1:
                    print(f"  [DEVICE INFO] Signature: {sig} | Optiboot_OL4 Version: {ver} (recovered on retry #{attempt})")
                else:
                    print(f"  [DEVICE INFO] Signature: {sig} | Optiboot_OL4 Version: {ver}")
                return sig, ver
            else:
                if attempt < max_retries:
                    print(f"  [INFO RETRY #{attempt}/{max_retries}] get_info failed ({status_str(status)}), retrying slot...")
                    time.sleep(0.03)

        print(f"  [ERROR] Failed to read device info after {max_retries} attempts ({status_str(status)})")
        return None

    def set_address(self, addr: int, max_retries: int = 2) -> bool:
        """Sets target Flash address with visible retry telemetry."""
        payload = bytes([2, addr & 0xFF, (addr >> 8) & 0xFF, 0x00, 0x00])
        status = STATUS_ERR_UNKNOWN
        for attempt in range(1, max_retries + 1):
            ok, status, _ = self.execute_slot(PID_SET_ADDR, payload, slot_duration=SLOT_CONTROL)
            if ok:
                if attempt > 1:
                    print(f"    [RETRY OK] set_address 0x{addr:04X} succeeded on attempt #{attempt}")
                return True
            else:
                if attempt < max_retries:
                    print(f"    [ADDR RETRY #{attempt}/{max_retries}] set_address 0x{addr:04X} failed ({status_str(status)}), retrying...")
                    time.sleep(0.02)

        print(f"  [ERROR] Failed to set address 0x{addr:04X} ({status_str(status)})")
        return False

    def write_page(self, addr: int, data: bytes, max_retries: int = 2) -> bool:
        """Writes a 64-byte Flash page with CRC16 and visible retry telemetry."""
        if not self.set_address(addr):
            return False

        crc = crc16_ccitt(data)
        packet = bytes([PAGE_SIZE]) + data + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        status = STATUS_ERR_UNKNOWN

        for attempt in range(1, max_retries + 1):
            # Flash erase/write takes ~25ms, allocate 100ms slot duration
            ok, status, _ = self.execute_slot(PID_WRITE_PAGE, packet, slot_duration=SLOT_WRITE, timeout=0.5)
            if ok:
                if attempt > 1:
                    print(f"    [RETRY OK] write_page 0x{addr:04X} succeeded on attempt #{attempt}")
                return True
            else:
                if attempt < max_retries:
                    print(f"    [WRITE RETRY #{attempt}/{max_retries}] write_page 0x{addr:04X} failed ({status_str(status)}), retrying slot...")
                    # Re-send set_address before re-writing page
                    self.set_address(addr)
                    time.sleep(0.03)

        print(f"  [ERROR] Write page failed at 0x{addr:04X} ({status_str(status)})")
        return False

    def read_page(self, addr: int, max_retries: int = 2) -> Optional[bytes]:
        """Reads a 64-byte Flash page with visible retry telemetry."""
        if not self.set_address(addr):
            return None

        status = STATUS_ERR_UNKNOWN
        for attempt in range(1, max_retries + 1):
            ok, status, payload = self.execute_slot(PID_READ_PAGE, b"", slot_duration=SLOT_CONTROL)
            if ok and len(payload) == PAGE_SIZE:
                if attempt > 1:
                    print(f"    [RETRY OK] read_page 0x{addr:04X} succeeded on attempt #{attempt}")
                return payload
            else:
                if attempt < max_retries:
                    print(f"    [READ RETRY #{attempt}/{max_retries}] read_page 0x{addr:04X} failed ({status_str(status)}), retrying slot...")
                    self.set_address(addr)
                    time.sleep(0.02)

        print(f"  [ERROR] Read page failed at 0x{addr:04X} ({status_str(status)})")
        return None

    def reboot(self):
        """Sends REBOOT command to launch application."""
        print("[REBOOT] Requesting Core-D reboot into application...")
        self.execute_slot(PID_REBOOT, b"", slot_duration=SLOT_CONTROL, timeout=0.05)


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
    parser = argparse.ArgumentParser(description="Optiboot_OL4 (LN-485 Master Broker) Host Flasher")
    parser.add_argument("--port", default="COM19", help="Serial port (e.g. COM19)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default 115200)")
    parser.add_argument("--probe", action="store_true", help="Send PING probe")
    parser.add_argument("--info", action="store_true", help="Read device signature and version")
    parser.add_argument("--read-page", default=None, help="Read 64B page at hex address (e.g. 0x0400)")
    parser.add_argument("--hex", default=None, help="Hex file to write & verify")
    parser.add_argument("--reboot", action="store_true", help="Reboot to application")
    args = parser.parse_args()

    broker = LN485MasterBroker(args.port, args.baud)
    try:
        broker.connect()

        if not broker.poll_power_on():
            return

        if args.probe:
            broker.ping()

        if args.info or args.hex:
            info = broker.get_info()
            if not info and args.hex:
                print("[ABORT] Cannot proceed with flashing because device info query failed.")
                return

        if args.read_page:
            target_addr = int(args.read_page, 16)
            print(f"\n[READ PAGE] Address 0x{target_addr:04X}...")
            data = broker.read_page(target_addr)
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
            t_total_start = time.time()
            for p_idx in range(total_pages):
                curr_addr = start_p + p_idx * PAGE_SIZE
                page_bytes = bytes([hex_data.get(curr_addr + i, 0xFF) for i in range(PAGE_SIZE)])

                print(f"  Flashing Page {p_idx+1}/{total_pages} @ 0x{curr_addr:04X}...", end="", flush=True)
                t_w0 = time.time()
                if not broker.write_page(curr_addr, page_bytes):
                    print(" [WRITE FAIL]")
                    all_ok = False
                    break

                readback = broker.read_page(curr_addr)
                t_page = (time.time() - t_w0) * 1000.0
                if readback == page_bytes:
                    print(f" [VERIFY PASS] ({t_page:.1f}ms)")
                else:
                    print(" [VERIFY MISMATCH]")
                    all_ok = False
                    break

                # Inter-page settlement delay (ensures slave is completely settled in WFB)
                time.sleep(PAGE_INTERVAL)

            if all_ok:
                t_total = time.time() - t_total_start
                print(f"\n[SUCCESS] All {total_pages} pages written and verified with CRC16 successfully in {t_total:.2f}s!")
                broker.reboot()

        if args.reboot and not args.hex:
            broker.reboot()

    except KeyboardInterrupt:
        print("\n[ABORT] Interrupted by user.")
    finally:
        broker.close()


if __name__ == '__main__':
    main()
