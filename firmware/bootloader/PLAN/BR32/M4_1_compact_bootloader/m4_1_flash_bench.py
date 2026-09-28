#!/usr/bin/env python3
"""
============================================================================
Milestone 4-1 (M4-1): Compact Bootloader (<1024 Bytes) Single Flash Page Write
Target MCU: Microchip ATtiny1616-MNR on ADX Core-D

Hard Constraint: Total bootloader size <= 1024 Bytes (BOOTEND = 0x04)
                 Target Page 16 (0x0400) is 100% clean application space!

Verification Steps:
  1. Discovery: Identify slave and obtain 10-byte SIGROW UID.
  2. Baseline Read: Read Page 16 before write via CMD_READ_CHUNK.
  3. 4-Chunk Write & Commit: Send 4 chunks (16B x 4 = 64B) to Page 16 with
     COMMIT_PAGE flag on Chunk 3. Verify immediate ACK and async flash write.
  4. Physical Flash Verify: Read back Page 16 via CMD_READ_CHUNK from
     physical Flash and verify 64/64 bytes 100% Bit-for-Bit match.
  5. Security Protection Test: Attempt write to protected Page 0 (0x0000)
     and verify slave returns STATUS_ERR_PROTECT (0x03).
============================================================================
"""

import argparse
import sys
import time
from typing import Optional, Dict, Any, Tuple
import serial

# BR32 Frame Commands
CMD_IDENTIFY    = 0x01
CMD_WRITE_CHUNK = 0x10
CMD_READ_CHUNK  = 0x20
CMD_BOOT_APP    = 0x30

# BR32 Status Codes
STATUS_OK          = 0x00
STATUS_ERR_CRC     = 0x01
STATUS_ERR_TIMEOUT = 0x02
STATUS_ERR_PROTECT = 0x03

STATUS_NAMES = {
    0x00: "STATUS_OK",
    0x01: "STATUS_ERR_CRC",
    0x02: "STATUS_ERR_TIMEOUT",
    0x03: "STATUS_ERR_PROTECT",
}

# Target Flash Configuration
APP_START_PAGE     = 16      # 0x0400 (First 1KB protected)
TARGET_PAGE_IDX    = 16      # 0x0400
PROTECTED_PAGE_IDX = 0       # 0x0000 (Bootloader area)
FLASH_PAGE_SIZE    = 64
FLASH_CHUNK_SIZE   = 16

# Chunk Flags
FLAG_COMMIT_PAGE   = 0x04

def crc16_ccitt(data: bytes) -> int:
    """CRC-16-CCITT (XMODEM: Poly 0x1021, Init 0xFFFF)."""
    crc = 0xFFFF
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

class M41FlashBenchmark:
    def __init__(self, port: str, baudrate: int = 19200):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.slave_uid: Optional[bytes] = None

    def connect(self):
        print(f"[INIT] Opening RS-485 Serial Port {self.port} @ {self.baudrate} bps...")
        self.ser = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.25
        )
        print(f"[INIT] Connected successfully to {self.port}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    @staticmethod
    def build_frame(cmd: int, seq: int, target_uid: bytes, page_idx: int, chunk_flags: int, payload: bytes) -> bytes:
        if len(target_uid) != 10:
            raise ValueError("target_uid must be exactly 10 bytes")
        if len(payload) > 16:
            raise ValueError("payload cannot exceed 16 bytes")

        padded_payload = payload.ljust(16, b'\x00')
        header = bytes([cmd, seq]) + target_uid + bytes([page_idx, chunk_flags]) + padded_payload
        crc = crc16_ccitt(header)
        frame = header + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        return frame

    @staticmethod
    def parse_response(raw: bytes) -> Optional[Dict[str, Any]]:
        if len(raw) != 32:
            return None

        calc_crc = crc16_ccitt(raw[:30])
        rx_crc = (raw[30] << 8) | raw[31]
        crc_ok = (calc_crc == rx_crc)

        uid_bytes = raw[4:14]
        uid_hex = " ".join(f"{b:02X}" for b in uid_bytes)
        status_val = raw[0]

        return {
            "status": status_val,
            "status_name": STATUS_NAMES.get(status_val, f"STATUS_UNKNOWN(0x{status_val:02X})"),
            "seq": raw[1],
            "rx_count": raw[2],
            "dev_state": raw[3],
            "uid_bytes": uid_bytes,
            "uid_hex": uid_hex,
            "data": raw[14:30],
            "crc_calc": calc_crc,
            "crc_rx": rx_crc,
            "crc_ok": crc_ok
        }

    def send_br32_frame(self, frame: bytes) -> Tuple[bool, float, bytes, Optional[Dict[str, Any]]]:
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()

        # Step 1: Drop to half-baud and send 0x00 (LIN Break)
        self.ser.baudrate = self.baudrate // 2
        self.ser.write(b'\x00')
        self.ser.flush()

        # Step 2: Restore normal baudrate
        self.ser.baudrate = self.baudrate

        # Step 3: Send LIN Sync byte (0x55) followed by the 32-byte BR32 frame
        self.ser.write(bytes([0x55]) + frame)
        self.ser.flush()

        # Step 4: Wait for 32-byte response from slave
        rx = self.ser.read(32)
        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        parsed = self.parse_response(rx)
        success = (len(rx) == 32 and parsed is not None and parsed["crc_ok"])
        return success, rtt_ms, rx, parsed

    def discover_slave(self) -> bool:
        print("\n[DISCOVERY] Probing bus with All-Zero UID to discover slave...")
        all_zeros = b'\x00' * 10
        frame = self.build_frame(CMD_IDENTIFY, 1, all_zeros, 0, 0, b'')

        success, rtt, rx, parsed = self.send_br32_frame(frame)
        if success and parsed and parsed["status"] == STATUS_OK:
            self.slave_uid = parsed["uid_bytes"]
            print(f"[DISCOVERY] Found Slave SIGROW UID: 0x{parsed['uid_hex']} (RTT: {rtt:.2f} ms)")
            return True
        else:
            print(f"[DISCOVERY] FAILED to identify slave. (RX len: {len(rx)})")
            return False

    def read_physical_page(self, page_idx: int) -> Optional[bytes]:
        """Reads 4 chunks (64 bytes) from physical Flash memory."""
        full_page = bytearray()
        for chunk in range(4):
            frame = self.build_frame(CMD_READ_CHUNK, chunk + 1, self.slave_uid, page_idx, chunk, b'')
            success, rtt, rx, parsed = self.send_br32_frame(frame)
            if not success or not parsed or parsed["status"] != STATUS_OK:
                print(f"  [Read Chunk {chunk}] FAILED! Status: {parsed['status_name'] if parsed else 'No Resp'}")
                return None
            full_page.extend(parsed["data"])
            time.sleep(0.05)
        return bytes(full_page)

    def run_benchmark(self):
        print("=" * 76)
        print("    MILESTONE 4-1: BR32 COMPACT BOOTLOADER (<1024B) BENCHMARK")
        print("=" * 76)
        print(f" Target Port       : {self.port} @ {self.baudrate} bps")
        print(f" Target Flash Page : Page {TARGET_PAGE_IDX} (0x{TARGET_PAGE_IDX*FLASH_PAGE_SIZE:04X})")
        print(f" Protected Area    : Pages 0..{APP_START_PAGE-1} (0x0000..0x{APP_START_PAGE*FLASH_PAGE_SIZE-1:04X})")
        print(f" Bootloader Constraint: Size <= 1024B (Strictly Pages 0..15)")
        print("=" * 76)

        if not self.discover_slave():
            return

        time.sleep(0.2)

        # --------------------------------------------------------------------
        # STEP 1: Baseline Read of Page 16 (Before Writing)
        # --------------------------------------------------------------------
        print("\n" + "=" * 76)
        print("       STEP 1: BASELINE READ OF PAGE 16 (BEFORE WRITE)")
        print("=" * 76)
        baseline_data = self.read_physical_page(TARGET_PAGE_IDX)
        if baseline_data:
            print(f" [Current Flash 64B] (Hex): {baseline_data.hex(' ')}")
            ascii_repr = "".join(chr(b) if 32 <= b <= 126 else "." for b in baseline_data)
            print(f" [Current Flash 64B] (ASCII): '{ascii_repr}'")
        else:
            print(" [WARNING] Could not read baseline Flash data.")

        time.sleep(0.2)

        # --------------------------------------------------------------------
        # STEP 2: Write 4 Chunks to Page 16 with Async Flash Commit
        # --------------------------------------------------------------------
        print("\n" + "=" * 76)
        print("       STEP 2: WRITING 4 CHUNKS & COMMITTING TO PHYSICAL FLASH")
        print("=" * 76)

        test_chunks = [
            b"ADX FLASH M4 OK!",  # Chunk 0
            b"PAGE 16 @ 0x0400",  # Chunk 1
            b"ATtiny1616 NVM  ",  # Chunk 2
            b"BR32 PROTO 2026!"   # Chunk 3 (Triggers COMMIT_PAGE)
        ]
        expected_page_data = b"".join(test_chunks)

        write_success = True
        for chunk_idx, chunk_data in enumerate(test_chunks):
            flags = chunk_idx | (FLAG_COMMIT_PAGE if chunk_idx == 3 else 0x00)
            frame = self.build_frame(CMD_WRITE_CHUNK, chunk_idx + 10, self.slave_uid, TARGET_PAGE_IDX, flags, chunk_data)

            success, rtt, rx, parsed = self.send_br32_frame(frame)
            if success and parsed and parsed["status"] == STATUS_OK:
                echo_match = (parsed["data"] == chunk_data)
                echo_str = "MATCH" if echo_match else "MISMATCH"
                print(f"  [Chunk {chunk_idx}] TX: '{chunk_data.decode('ascii')}' -> ACK [STATUS_OK] (Echo: {echo_str}, RTT: {rtt:.2f} ms)")
                if not echo_match:
                    write_success = False
            else:
                print(f"  [Chunk {chunk_idx}] FAILED! Status: {parsed['status_name'] if parsed else 'No Resp'}")
                write_success = False

            # 200 ms Metronome bus clock
            time.sleep(0.2)

        if not write_success:
            print("\n[FAIL] Step 2 Write failed.")
            return

        # --------------------------------------------------------------------
        # STEP 3: Verify Physical Flash Data (Bit-for-Bit Verification)
        # --------------------------------------------------------------------
        print("\n" + "=" * 76)
        print("       STEP 3: READING PHYSICAL FLASH & VERIFYING BIT-FOR-BIT")
        print("=" * 76)
        read_back_data = self.read_physical_page(TARGET_PAGE_IDX)

        if not read_back_data:
            print("[FAIL] Step 3 Read back failed.")
            return

        print(f" [Expected Data] : {expected_page_data.hex(' ')}")
        print(f" [Physical Flash]: {read_back_data.hex(' ')}")

        match_count = sum(1 for a, b in zip(expected_page_data, read_back_data) if a == b)
        print(f" [Byte Match]    : {match_count} / {FLASH_PAGE_SIZE} Bytes ({match_count/FLASH_PAGE_SIZE*100:.1f}%)")

        is_perfect = (match_count == FLASH_PAGE_SIZE)
        if is_perfect:
            print(" [VERIFY RESULT] : >>> 100% BIT-FOR-BIT PERFECT MATCH! <<<")
        else:
            print(" [VERIFY RESULT] : >>> MISMATCH DETECTED! <<<")

        time.sleep(0.2)

        # --------------------------------------------------------------------
        # STEP 4: Security Protection Test (Attempt Write to Protected Page 0)
        # --------------------------------------------------------------------
        print("\n" + "=" * 76)
        print("       STEP 4: SECURITY TEST - WRITE TO PROTECTED PAGE 0 (0x0000)")
        print("=" * 76)
        dummy_chunk = b"POISON BOOTLOAD!"
        protect_frame = self.build_frame(CMD_WRITE_CHUNK, 99, self.slave_uid, PROTECTED_PAGE_IDX, 0x00, dummy_chunk)
        p_success, p_rtt, p_rx, p_parsed = self.send_br32_frame(protect_frame)

        protection_ok = False
        if p_success and p_parsed:
            print(f"  Attempted write to Page 0 -> Slave returned: {p_parsed['status_name']} (0x{p_parsed['status']:02X})")
            if p_parsed["status"] == STATUS_ERR_PROTECT:
                print("  [SECURITY PASS] Slave correctly rejected write to protected bootloader area!")
                protection_ok = True
            else:
                print("  [SECURITY FAIL] Slave did not return STATUS_ERR_PROTECT!")
        else:
            print("  [SECURITY FAIL] No valid response received.")

        # --------------------------------------------------------------------
        # SUMMARY SCORECARD
        # --------------------------------------------------------------------
        print("\n" + "=" * 76)
        print("               MILESTONE 4-1 FINAL SCORECARD & RATING")
        print("=" * 76)
        print(f" 1. Discovery & SIGROW UID Check   : PASS (UID: 0x{self.slave_uid.hex().upper()})")
        print(f" 2. 4-Chunk 64B Write & Echo       : {'PASS' if write_success else 'FAIL'}")
        print(f" 3. Physical Flash 64B Verification: {'PASS (100% Match)' if is_perfect else 'FAIL'}")
        print(f" 4. Bootloader Area Protection     : {'PASS (STATUS_ERR_PROTECT)' if protection_ok else 'FAIL'}")
        print("-" * 76)

        if write_success and is_perfect and protection_ok:
            print(" >>> FINAL GRADE: GRADE A+ (MILESTONE 4-1 100% ACHIEVED) <<<")
        else:
            print(" >>> FINAL GRADE: FAIL (Please inspect logs) <<<")
        print("=" * 76)

def main():
    parser = argparse.ArgumentParser(description="Milestone 4-1: BR32 Compact Bootloader (<1024B) Benchmark")
    parser.add_argument("--port", default="COM19", help="RS-485 Serial Port (Default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 Baudrate (Default: 19200)")
    args = parser.parse_args()

    bench = M41FlashBenchmark(port=args.port, baudrate=args.baud)
    try:
        bench.connect()
        bench.run_benchmark()
    except KeyboardInterrupt:
        print("\n[ABORT] User interrupted benchmark.")
    finally:
        bench.close()

if __name__ == "__main__":
    main()
