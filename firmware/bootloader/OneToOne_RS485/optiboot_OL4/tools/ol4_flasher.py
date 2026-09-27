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
PID_PING        = 0x80  # ID 0x00: Ping / Keep-Alive
PID_GET_INFO    = 0xC1  # ID 0x01: Device Info & Signature
PID_SET_ADDR    = 0x42  # ID 0x02: Load 16-bit Flash Address
PID_WRITE_CHUNK = 0x03  # ID 0x03: Write 8-byte Flash Chunk
PID_COMMIT_PAGE = 0xC4  # ID 0x04: Erase & Write Page Buffer to Flash
PID_READ_CHUNK  = 0x85  # ID 0x05: Read 8-byte Flash Chunk
PID_REBOOT      = 0x06  # ID 0x06: Reboot to Application (0x0400)

# Status Codes
STATUS_OK          = 0x00
STATUS_ERR_CRC     = 0x01
STATUS_ERR_ADDR    = 0x02
STATUS_ERR_FLASH   = 0x03
STATUS_ERR_UNKNOWN = 0xFF

PAGE_SIZE       = 64
CHUNK_SIZE      = 8
CHUNKS_PER_PAGE = 8

# LN-485 Master Schedule Slot Durations (seconds)
# User Strategy: Expanding Master polling cycles guarantees deterministic stability.
SLOT_CONTROL  = 0.040  # 40ms (25Hz) for PING, GET_INFO, SET_ADDR
SLOT_CHUNK    = 0.035  # 35ms (~28Hz) for WRITE_CHUNK, READ_CHUNK (huge slack for 1.5ms transmission)
SLOT_COMMIT   = 0.080  # 80ms (12.5Hz) for COMMIT_PAGE (NVM erase/write: ~25ms + 55ms slack)
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

    def execute_slot(self, pid: int, payload: bytes = b"", slot_duration: float = SLOT_CONTROL, timeout: float = 0.25) -> Tuple[bool, int, bytes, float, int]:
        """
        Executes an atomic LN-485 transaction within a strict Master Schedule Slot:
          1. Sends Break (18 Tbit LOW via 57600bps 0x00).
          2. Sends [0x55, PID] + payload atomically in a single write/flush.
          3. Receives Slave response [Status, Len, Payload, CRC16].
          4. Strictly waits for the remainder of slot_duration (Slack Time conservation).
        Returns: (ok, status, data, rtt_ms, raw_rx_len)
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
        t_tx_done = time.perf_counter()

        # 3. Receive Slave Response
        self.ser.timeout = timeout
        raw_rx = bytearray()
        hdr = self.ser.read(2)
        raw_rx.extend(hdr)
        t_rx_hdr = time.perf_counter()
        rtt_ms = (t_rx_hdr - t_tx_done) * 1000.0

        if len(hdr) < 2:
            ok, status, data = False, STATUS_ERR_UNKNOWN, b""
        else:
            status = hdr[0]
            length = hdr[1]
            rest = self.ser.read(length + 2)
            raw_rx.extend(rest)
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

        return ok, status, data, rtt_ms, len(raw_rx)

    def ping(self) -> bool:
        """Sends CMD_PING and expects STATUS_OK."""
        ok, status, _, rtt_ms, _ = self.execute_slot(PID_PING, b"", slot_duration=SLOT_CONTROL)
        if ok:
            print(f"  [PING PASS] Core-D responded with STATUS_OK (RTT={rtt_ms:.1f}ms)")
            return True
        else:
            print(f"  [PING FAIL] status=0x{status:02X} RTT={rtt_ms:.1f}ms")
            return False

    def poll_power_on(self, max_wait_sec: float = 30.0) -> bool:
        """Waits for Core-D power on by sending periodic PING headers."""
        print(f"\n[Step 1-2] Waiting for Core-D power on/reset (up to {max_wait_sec}s)...")
        print(">>> POWER ON OR RESET CORE-D NOW <<<")
        start = time.time()
        probes = 0
        while time.time() - start < max_wait_sec:
            probes += 1
            ok, status, _, rtt_ms, _ = self.execute_slot(PID_PING, b"", slot_duration=SLOT_CONTROL, timeout=0.06)
            if ok:
                elapsed = time.time() - start
                print(f"[Step 1-2: PASS] Power-on detected in {elapsed:.2f}s (probe #{probes}, RTT={rtt_ms:.1f}ms)!")
                # Inter-stage settlement delay (ensure slave finishes TXCIF and WFB re-arm)
                time.sleep(0.150)
                return True
            time.sleep(0.02)
        print("[Step 1-2: FAIL] Timeout waiting for Core-D.")
        return False

    def get_info(self, max_retries: int = 3) -> Optional[Tuple[str, str]]:
        """Queries Device Signature and Bootloader Version with visible retry telemetry."""
        status = STATUS_ERR_UNKNOWN
        print("[Step 2-1] Querying device info (GET_INFO)...")
        for attempt in range(1, max_retries + 1):
            ok, status, payload, rtt_ms, rx_len = self.execute_slot(PID_GET_INFO, b"", slot_duration=SLOT_CONTROL)
            if ok and len(payload) >= 5:
                sig = f"0x{payload[0]:02X} 0x{payload[1]:02X} 0x{payload[2]:02X}"
                ver = f"{payload[3]}.{payload[4]}"
                retry_tag = f" (recovered on retry #{attempt})" if attempt > 1 else ""
                print(f"  [Step 2-1: PASS] Signature: {sig} | Optiboot_OL4 Version: {ver} (RTT={rtt_ms:.1f}ms){retry_tag}")
                return sig, ver
            else:
                if attempt < max_retries:
                    print(f"  [Step 2-1: RETRY #{attempt}/{max_retries}] get_info failed ({status_str(status)}, rx={rx_len}B), retrying slot...")
                    time.sleep(0.03)

        print(f"  [Step 2-1: FAIL] Failed to read device info after {max_retries} attempts ({status_str(status)}, rx={rx_len}B)")
        return None

    def set_address(self, addr: int, step_id: str = "", max_retries: int = 2) -> bool:
        """Sets target Flash address with numbered telemetry."""
        payload = bytes([2, addr & 0xFF, (addr >> 8) & 0xFF, 0x00, 0x00])
        status = STATUS_ERR_UNKNOWN
        prefix = f"  [{step_id}] " if step_id else "  "
        rx_len = 0
        for attempt in range(1, max_retries + 1):
            ok, status, _, rtt_ms, rx_len = self.execute_slot(PID_SET_ADDR, payload, slot_duration=SLOT_CONTROL)
            if ok:
                retry_tag = f" (recovered on retry #{attempt})" if attempt > 1 else ""
                print(f"{prefix}set_address 0x{addr:04X}... PASS (RTT={rtt_ms:.1f}ms){retry_tag}")
                return True
            else:
                if attempt < max_retries:
                    print(f"{prefix}set_address 0x{addr:04X}... RETRY #{attempt}/{max_retries} ({status_str(status)}, rx={rx_len}B), retrying...")
                    time.sleep(0.02)

        print(f"{prefix}set_address 0x{addr:04X}... FAIL ({status_str(status)}, rx={rx_len}B)")
        return False

    def write_chunk(self, offset: int, chunk: bytes, step_id: str = "", max_retries: int = 2) -> bool:
        """Writes an 8-byte chunk directly to the Flash page buffer with numbered telemetry."""
        assert len(chunk) == CHUNK_SIZE
        payload = bytes([offset]) + chunk
        crc = crc16_ccitt(payload)
        packet = payload + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        prefix = f"  [{step_id}] " if step_id else "  "
        rx_len = 0

        status = STATUS_ERR_UNKNOWN
        for attempt in range(1, max_retries + 1):
            ok, status, _, rtt_ms, rx_len = self.execute_slot(PID_WRITE_CHUNK, packet, slot_duration=SLOT_CHUNK, timeout=0.15)
            if ok:
                retry_tag = f" (recovered on retry #{attempt})" if attempt > 1 else ""
                print(f"{prefix}write_chunk @ offset={offset:02d} (8B)... PASS (RTT={rtt_ms:.1f}ms, ACK=OK){retry_tag}")
                return True
            else:
                if attempt < max_retries:
                    print(f"{prefix}write_chunk @ offset={offset:02d} (8B)... RETRY #{attempt}/{max_retries} ({status_str(status)}, rx={rx_len}B), retrying...")
                    time.sleep(0.01)

        print(f"{prefix}write_chunk @ offset={offset:02d} (8B)... FAIL ({status_str(status)}, rx={rx_len}B)")
        return False

    def commit_page(self, step_id: str = "", max_retries: int = 2) -> bool:
        """Executes Flash page erase and write (NVMCTRL) with numbered telemetry."""
        status = STATUS_ERR_UNKNOWN
        prefix = f"  [{step_id}] " if step_id else "  "
        rx_len = 0
        for attempt in range(1, max_retries + 1):
            ok, status, _, rtt_ms, rx_len = self.execute_slot(PID_COMMIT_PAGE, b"", slot_duration=SLOT_COMMIT, timeout=0.25)
            if ok:
                retry_tag = f" (recovered on retry #{attempt})" if attempt > 1 else ""
                print(f"{prefix}commit_page (Flash Erase/Write)... PASS (RTT={rtt_ms:.1f}ms, ACK=OK){retry_tag}")
                return True
            else:
                if attempt < max_retries:
                    print(f"{prefix}commit_page... RETRY #{attempt}/{max_retries} ({status_str(status)}, rx={rx_len}B), retrying...")
                    time.sleep(0.02)

        print(f"{prefix}commit_page... FAIL ({status_str(status)}, rx={rx_len}B)")
        return False

    def read_chunk(self, offset: int, step_id: str = "", max_retries: int = 2) -> Optional[bytes]:
        """Reads an 8-byte chunk from Flash with numbered telemetry."""
        packet = bytes([offset, 0x00, 0x00])  # offset + dummy CRC
        status = STATUS_ERR_UNKNOWN
        prefix = f"  [{step_id}] " if step_id else "  "
        rx_len = 0
        for attempt in range(1, max_retries + 1):
            ok, status, payload, rtt_ms, rx_len = self.execute_slot(PID_READ_CHUNK, packet, slot_duration=SLOT_CHUNK, timeout=0.15)
            if ok and len(payload) == CHUNK_SIZE:
                retry_tag = f" (recovered on retry #{attempt})" if attempt > 1 else ""
                print(f"{prefix}read_chunk @ offset={offset:02d} (8B)... PASS (RTT={rtt_ms:.1f}ms, CRC=OK){retry_tag}")
                return payload
            else:
                if attempt < max_retries:
                    print(f"{prefix}read_chunk @ offset={offset:02d} (8B)... RETRY #{attempt}/{max_retries} ({status_str(status)}, rx={rx_len}B), retrying...")
                    time.sleep(0.01)

        print(f"{prefix}read_chunk @ offset={offset:02d} (8B)... FAIL ({status_str(status)}, rx={rx_len}B)")
        return None

    def write_page(self, page_num: int, total_pages: int, addr: int, data: bytes) -> bool:
        """Writes a 64-byte Flash page in 8x 8-byte chunks followed by a page commit."""
        assert len(data) == PAGE_SIZE
        if not self.set_address(addr, step_id=f"Step 4-{page_num}-ADDR"):
            return False

        for chunk_idx in range(CHUNKS_PER_PAGE):
            offset = chunk_idx * CHUNK_SIZE
            chunk_data = data[offset:offset + CHUNK_SIZE]
            if not self.write_chunk(offset, chunk_data, step_id=f"Step 4-{page_num}-W{chunk_idx}"):
                return False

        if not self.commit_page(step_id=f"Step 4-{page_num}-COMMIT"):
            return False

        return True

    def read_page(self, page_num: int, total_pages: int, addr: int) -> Optional[bytes]:
        """Reads a 64-byte Flash page in 8x 8-byte chunks."""
        if not self.set_address(addr, step_id=f"Step 4-{page_num}-VADDR"):
            return None

        result = bytearray()
        for chunk_idx in range(CHUNKS_PER_PAGE):
            offset = chunk_idx * CHUNK_SIZE
            chunk = self.read_chunk(offset, step_id=f"Step 4-{page_num}-R{chunk_idx}")
            if chunk is None:
                return None
            result.extend(chunk)

        return bytes(result)

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
        print(f"[Step 1-1] Opening serial port {args.port} at {args.baud} bps...")
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
            data = broker.read_page(1, 1, target_addr)
            if data:
                print(f"Read 64 bytes: {hex_dump(data[:16])} ...")

        if args.hex:
            hex_data = parse_intel_hex(args.hex)
            print(f"\n[Step 3-1] Loading HEX file {args.hex}...")

            addrs = list(hex_data.keys())
            min_addr = min(addrs)
            max_addr = max(addrs)

            # Optiboot_OL4 application starts at 0x0400
            start_p = max(0x0400, (min_addr // PAGE_SIZE) * PAGE_SIZE)
            end_p = ((max_addr + PAGE_SIZE) // PAGE_SIZE) * PAGE_SIZE
            total_pages = (end_p - start_p) // PAGE_SIZE

            print(f"[Step 3-1: PASS] Loaded {len(hex_data)} bytes. Plan: Flashing {total_pages} pages (0x{start_p:04X} ~ 0x{end_p:04X})...")
            print("\n[Step 4] Starting Flashing & Verification Sequence:")
            all_ok = True
            t_total_start = time.time()
            for p_idx in range(total_pages):
                curr_addr = start_p + p_idx * PAGE_SIZE
                page_bytes = bytes([hex_data.get(curr_addr + i, 0xFF) for i in range(PAGE_SIZE)])

                print(f"\n--- Page {p_idx+1}/{total_pages} @ 0x{curr_addr:04X} ---")
                t_w0 = time.time()
                if not broker.write_page(p_idx + 1, total_pages, curr_addr, page_bytes):
                    print(f"[PAGE {p_idx+1} WRITE FAIL] Failed to write page at 0x{curr_addr:04X}")
                    all_ok = False
                    break

                time.sleep(0.020)  # Brief quiet bus settlement before verify read
                readback = broker.read_page(p_idx + 1, total_pages, curr_addr)
                t_page = (time.time() - t_w0) * 1000.0
                if readback == page_bytes:
                    print(f"  [Step 4-{p_idx+1}: PASS] Page {p_idx+1}/{total_pages} @ 0x{curr_addr:04X} verified in {t_page:.1f}ms")
                else:
                    print(f"  [Step 4-{p_idx+1}: FAIL] Verify mismatch at page {p_idx+1} (0x{curr_addr:04X})")
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
