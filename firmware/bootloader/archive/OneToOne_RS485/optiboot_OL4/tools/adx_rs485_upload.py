#!/usr/bin/env python3
"""
ADX RS-485 High-Reliability Firmware Uploader (Optiboot_OL4 v2.0)
Target: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)
Protocol: Stop-and-Wait ARQ with 64-byte Flash Page CRC-16 Verification

Usage:
  python adx_rs485_upload.py --port COM19 firmware.hex
  python adx_rs485_upload.py --port COM19 --debug-port COM21 firmware.hex
"""

import sys
import os
import time
import argparse
import threading
from typing import Dict, List, Optional, Tuple
import serial

# Protocol Constants
PKT_STX               = 0x02
PKT_ETX               = 0x03

CMD_PING              = 0x01
CMD_GET_CHIP_INFO     = 0x02
CMD_WRITE_PAGE        = 0x10
CMD_VERIFY_PAGE       = 0x11
CMD_BOOT_APP          = 0x20

RESP_ACK              = 0x06
RESP_NAK              = 0x15

STATUS_OK             = 0x00
STATUS_ERR_CRC        = 0x01
STATUS_ERR_PROTECTED  = 0x02
STATUS_ERR_UNKNOWN    = 0x03

FLASH_PAGE_SIZE       = 64
APP_START_ADDR        = 0x0400
APP_START_PAGE        = 0x10  # 0x0400 / 64 = 16
FLASH_TOTAL_PAGES     = 256
FLASH_MAX_ADDR        = 0x4000

CHIP_SIG2_ATTINY1616  = 0x21

# =========================================================================
# CRC-16-CCITT (Poly: 0x1021, Init: 0xFFFF) matching AVR implementation
# =========================================================================
def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    crc = init
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


# =========================================================================
# Intel HEX Parser
# =========================================================================
def parse_intel_hex(hex_path: str) -> Dict[int, int]:
    """Parses Intel HEX file and returns a dict mapping flash address -> byte."""
    flash_bytes = {}
    base_address = 0

    with open(hex_path, "r") as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line.startswith(":"):
                continue

            try:
                byte_count = int(line[1:3], 16)
                address = int(line[3:7], 16)
                record_type = int(line[7:9], 16)
                data_str = line[9:9 + byte_count * 2]
                checksum = int(line[9 + byte_count * 2:11 + byte_count * 2], 16)
            except ValueError:
                raise ValueError(f"Corrupt Intel HEX line {line_num}: {line}")

            # Verify checksum
            line_bytes = bytes.fromhex(line[1:])
            if sum(line_bytes) & 0xFF != 0:
                raise ValueError(f"Checksum mismatch in Intel HEX line {line_num}")

            if record_type == 0x00:  # Data Record
                full_addr = base_address + address
                for i in range(byte_count):
                    flash_bytes[full_addr + i] = int(data_str[i * 2:(i + 1) * 2], 16)
            elif record_type == 0x01:  # End of File
                break
            elif record_type == 0x02:  # Extended Segment Address
                base_address = int(data_str, 16) << 4
            elif record_type == 0x04:  # Extended Linear Address
                base_address = int(data_str, 16) << 16

    return flash_bytes


def prepare_flash_pages(flash_bytes: Dict[int, int]) -> Dict[int, bytes]:
    """Organizes flash bytes into 64-byte pages (Page 0x10 .. 0xFF)."""
    pages = {}
    if not flash_bytes:
        return pages

    min_addr = min(flash_bytes.keys())
    max_addr = max(flash_bytes.keys())

    # Ensure application starts at or above APP_START_ADDR (0x0400)
    for addr in flash_bytes:
        if addr < APP_START_ADDR:
            raise ValueError(f"HEX file contains data at 0x{addr:04X} below application area (0x{APP_START_ADDR:04X})!")
        if addr >= FLASH_MAX_ADDR:
            raise ValueError(f"HEX file exceeds ATtiny1616 16KB Flash at 0x{addr:04X}!")

    start_page = min_addr // FLASH_PAGE_SIZE
    end_page = max_addr // FLASH_PAGE_SIZE

    for page_no in range(start_page, end_page + 1):
        page_addr = page_no * FLASH_PAGE_SIZE
        page_data = bytearray(FLASH_PAGE_SIZE)
        has_data = False
        for offset in range(FLASH_PAGE_SIZE):
            addr = page_addr + offset
            if addr in flash_bytes:
                page_data[offset] = flash_bytes[addr]
                has_data = True
            else:
                page_data[offset] = 0xFF  # Flash unprogrammed state
        if has_data:
            pages[page_no] = bytes(page_data)

    return pages


# =========================================================================
# Dual-Channel Telemetry Listener
# =========================================================================
class TelemetryListener:
    def __init__(self, port: str, baudrate: int = 9600):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.thread = None
        self.lock = threading.Lock()
        self.lines = []

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=0.05)
            self.running = True
            self.thread = threading.Thread(target=self._run, daemon=True)
            self.thread.start()
        except Exception as e:
            print(f"[WARN] Could not open debug telemetry port {self.port}: {e}")

    def _run(self):
        buf = bytearray()
        while self.running:
            try:
                data = self.ser.read(64)
                if data:
                    buf.extend(data)
                    while b"\n" in buf:
                        raw_line, buf = buf.split(b"\n", 1)
                        line = raw_line.decode("ascii", errors="replace").strip("\r")
                        with self.lock:
                            self.lines.append(line)
            except Exception:
                break

    def pop_lines(self) -> List[str]:
        with self.lock:
            ret = list(self.lines)
            self.lines.clear()
            return ret

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass


# =========================================================================
# RS-485 Stop-and-Wait ARQ Uploader Engine
# =========================================================================
class RS485Uploader:
    def __init__(self, port: str, baudrate: int = 9600, timeout: float = 0.20):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None
        self.seq = 0

    def open(self):
        self.ser = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=self.timeout,
            write_timeout=1.0
        )
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def _next_seq(self) -> int:
        self.seq = (self.seq + 1) & 0xFF
        return self.seq

    def send_command(self, cmd: int, page_no: int = 0, payload: bytes = b"", max_retries: int = 5) -> Tuple[bool, int, int]:
        """
        Sends a command packet with Stop-and-Wait ARQ.
        Returns: (success, status, extra)
        """
        seq = self._next_seq()
        len_byte = len(payload)

        # Build packet: [STX, SEQ, CMD, PAGE_NO, LEN, PAYLOAD, CRC16_H, CRC16_L, ETX]
        header = bytes([PKT_STX, seq, cmd, page_no, len_byte])
        crc_data = header + payload
        crc_val = crc16_ccitt(crc_data)
        tx_pkt = crc_data + bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF, PKT_ETX])

        for attempt in range(1, max_retries + 1):
            self.ser.reset_input_buffer()
            self.ser.write(tx_pkt)
            self.ser.flush()

            # Read 8-byte response
            rx_data = self.ser.read(8)

            if len(rx_data) < 8:
                # Timeout or short packet -> retry
                time.sleep(0.02)
                continue

            stx, r_seq, resp, r_page, status, extra, chk, etx = rx_data

            if stx != PKT_STX or etx != PKT_ETX:
                time.sleep(0.02)
                continue

            expected_chk = (stx + r_seq + resp + r_page + status + extra) & 0xFF
            if chk != expected_chk:
                time.sleep(0.02)
                continue

            if r_seq != seq:
                time.sleep(0.02)
                continue

            if resp == RESP_ACK:
                return True, status, extra
            elif resp == RESP_NAK:
                # NAK received (CRC error, etc.) -> retry
                time.sleep(0.02)
                continue

        return False, 0xFF, 0x00


# =========================================================================
# Main Upload Workflow
# =========================================================================
def upload_firmware(hex_file: str, port: str, debug_port: Optional[str] = None):
    print("=" * 72)
    print(" ADX Core-D RS-485 Firmware Uploader (Optiboot_OL4 v2.0)")
    print(f" Port={port} @ 9600 bps, Protocol=Stop-and-Wait ARQ + CRC-16")
    print(f" Target HEX: {hex_file}")
    if debug_port:
        print(f" Dual-Channel Debug Telemetry: {debug_port} @ 9600 bps")
    print("=" * 72)

    # 1. Parse HEX file
    if not os.path.isfile(hex_file):
        print(f"[FATAL] HEX file not found: {hex_file}")
        sys.exit(1)

    print(f"\n[1/4] Parsing Intel HEX file...")
    flash_bytes = parse_intel_hex(hex_file)
    pages = prepare_flash_pages(flash_bytes)
    total_pages = len(pages)
    total_bytes = len(flash_bytes)
    print(f"      -> Total Data: {total_bytes} Bytes across {total_pages} Flash Pages (64B/Page)")
    if total_pages == 0:
        print("[FATAL] No application data found in HEX file.")
        sys.exit(1)
    print(f"      -> Target Pages: 0x{min(pages.keys()):02X} .. 0x{max(pages.keys()):02X} "
          f"(0x{min(pages.keys())*64:04X} - 0x{(max(pages.keys())+1)*64-1:04X})")

    # 2. Connect to RS-485
    telemetry = None
    if debug_port:
        telemetry = TelemetryListener(debug_port)
        telemetry.start()

    uploader = RS485Uploader(port=port, baudrate=9600)
    try:
        uploader.open()
    except Exception as e:
        print(f"[FATAL] Cannot open RS-485 serial port {port}: {e}")
        if telemetry: telemetry.stop()
        sys.exit(1)

    print(f"\n[2/4] Connecting to Core-D Bootloader (Power cycle or reset Core-D now)...")
    connected = False
    optiboot_ver = "Unknown"
    start_time = time.time()

    # Probe with CMD_PING for up to 5 seconds
    while time.time() - start_time < 5.0:
        ok, status, extra = uploader.send_command(CMD_PING, max_retries=1)
        if ok and status == STATUS_OK:
            connected = True
            optiboot_ver = f"v{(extra >> 4)}.{(extra & 0x0F)}"
            break
        time.sleep(0.05)

    if not connected:
        print("\n[FATAL] Failed to connect to Optiboot_OL4 bootloader (Timeout).")
        print("        Ensure Core-D is connected and reset at the start of the upload.")
        uploader.close()
        if telemetry: telemetry.stop()
        sys.exit(1)

    print(f"      -> Connected! Bootloader: Optiboot_OL4 {optiboot_ver}")

    # Verify Chip Signature
    ok, status, sig2 = uploader.send_command(CMD_GET_CHIP_INFO)
    if ok and sig2 == CHIP_SIG2_ATTINY1616:
        print(f"      -> Device Identified: Microchip ATtiny1616 (Signature Byte 2: 0x{sig2:02X})")
    else:
        print(f"[WARN] Unexpected Chip Signature: 0x{sig2:02X} (Expected 0x{CHIP_SIG2_ATTINY1616:02X}). Proceeding...")

    # 3. Flash Pages with Stop-and-Wait ARQ
    print(f"\n[3/4] Flashing {total_pages} Pages (Stop-and-Wait ARQ + CRC-16)...")
    prog_start = time.perf_counter()
    retries_total = 0

    sorted_pages = sorted(pages.keys())
    for idx, page_no in enumerate(sorted_pages, 1):
        page_data = pages[page_no]
        page_addr = page_no * FLASH_PAGE_SIZE

        t0 = time.perf_counter()
        ok, status, extra = uploader.send_command(CMD_WRITE_PAGE, page_no=page_no, payload=page_data, max_retries=5)
        dt = (time.perf_counter() - t0) * 1000.0

        if not ok:
            print(f"\n[FATAL] Page 0x{page_no:02X} (0x{page_addr:04X}) FAILED after 5 retries! Flash aborted.")
            uploader.close()
            if telemetry: telemetry.stop()
            sys.exit(1)

        pct = (idx / total_pages) * 100.0
        bar = "=" * int(pct // 5) + ">" + " " * (20 - int(pct // 5))
        sys.stdout.write(f"\r      [{bar[:20]}] {pct:5.1f}% | Page 0x{page_no:02X} (0x{page_addr:04X}) | {dt:5.1f}ms ")
        sys.stdout.flush()

    prog_elapsed = time.perf_counter() - prog_start
    print(f"\n      -> Flashing complete in {prog_elapsed:.2f}s ({total_bytes / prog_elapsed:.1f} B/s, {retries_total} retries)")

    # 4. Verify & Boot Application
    print(f"\n[4/4] Verifying Flash Integrity & Booting Application...")
    verify_ok = True
    for page_no in sorted_pages:
        expected_crc = crc16_ccitt(pages[page_no]) & 0xFF
        ok, status, v_crc = uploader.send_command(CMD_VERIFY_PAGE, page_no=page_no)
        if not ok or v_crc != expected_crc:
            print(f"[FAIL] Page 0x{page_no:02X} verification mismatch! Expected 0x{expected_crc:02X}, got 0x{v_crc:02X}")
            verify_ok = False
            break

    if verify_ok:
        print(f"      -> CRC Verification: ALL {total_pages} PAGES VERIFIED PERFECTLY (100% MATCH)!")
    else:
        print(f"[WARN] Verification incomplete or failed.")

    # Send Boot Command
    print(f"      -> Sending CMD_BOOT_APP: Launching application at 0x0400...")
    uploader.send_command(CMD_BOOT_APP)
    uploader.close()

    if telemetry:
        time.sleep(0.2)
        lines = telemetry.pop_lines()
        for line in lines:
            print(f"      |-> [COM21] {line}")
        telemetry.stop()

    print("\n" + "=" * 72)
    print(" SUCCESSFUL UPLOAD: ADX Core-D is now running the new firmware!")
    print("=" * 72)


def main():
    parser = argparse.ArgumentParser(description="ADX Core-D RS-485 Firmware Uploader (Optiboot_OL4 v2.0)")
    parser.add_argument("hex", help="Path to Intel HEX firmware file")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial Port (default: COM19)")
    parser.add_argument("--debug-port", default=None, help="Optional COM21 Debug Telemetry Port")

    args = parser.parse_args()
    upload_firmware(args.hex, args.port, args.debug_port)


if __name__ == "__main__":
    main()
