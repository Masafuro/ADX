#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT

WU-4: BR32 Virtual SRAM Page Painter & Chunk Transfer Benchmark
Host-side tool for testing 4-Chunk (16B x 4 = 64B) Virtual SRAM buffering
and readback verification on ADX Core-D (100% Flash-Safe).

Features:
  1. 64-byte text/binary message slicing into 4x 16-byte chunks.
  2. Sequential CMD_WRITE_CHUNK (0x10) transfer with real-time verification.
  3. Sequential CMD_READ_CHUNK (0x20) retrieval.
  4. 100% Bit-by-bit Verification (64/64 Bytes).
  5. Formatted ASCII & HEX Hexdump display.
  6. High-load random binary stress test (--stress N).
"""

import sys
import os
import time
import argparse
import threading
from typing import Optional, Tuple, List, Dict, Any

try:
    import serial
except ImportError:
    print("[ERROR] pyserial is required. Please install via: pip install pyserial")
    sys.exit(1)


# Status definitions
STATUS_NAMES = {
    0x00: "STATUS_OK",
    0x01: "STATUS_ERR_CRC",
    0x02: "STATUS_ERR_TIMEOUT",
}


def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    """Calculate CRC-16-CCITT / XMODEM (Polynomial 0x1021, Initial 0xFFFF, MSB-first)."""
    crc = init
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


class DebugMonitor:
    """Asynchronous listener for the Core-D Soft-UART debug stream (COM21 @ 9600 bps)."""

    def __init__(self, port: str = "COM21", baudrate: int = 9600):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.running = False
        self.thread: Optional[threading.Thread] = None
        self.silent = False

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
            self.running = True
            self.thread = threading.Thread(target=self._listen_loop, daemon=True)
            self.thread.start()
            print(f"[DEBUG-MON] Listening on {self.port} @ {self.baudrate} bps (Soft-UART PB4)")
        except Exception as e:
            print(f"[DEBUG-MON] Note: Could not open debug port {self.port}: {e}")

    def _listen_loop(self):
        while self.running and self.ser and self.ser.is_open:
            try:
                line = self.ser.readline().decode('ascii', errors='replace')
                if line and not self.silent:
                    sys.stdout.write(f"  |-> [{self.port}] {line}")
                    sys.stdout.flush()
            except Exception:
                break

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()


class BR32SRAMPainter:
    """Master controller for BR32 Virtual SRAM Chunk Operations."""

    def __init__(self, port: str = "COM19", baudrate: int = 19200):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.slave_uid: Optional[bytes] = None

    def open(self):
        print(f"[INIT] Opening RS-485 Serial Port {self.port} @ {self.baudrate} bps...")
        self.ser = serial.Serial(self.port, baudrate=self.baudrate, timeout=0.15)
        time.sleep(0.05)
        print(f"[INIT] Connected successfully to {self.port}.")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    @staticmethod
    def build_frame(cmd: int, seq: int, uid: bytes,
                    page_idx: int = 0x00, chunk_flags: int = 0x00,
                    data_16: bytes = b'\x00' * 16) -> bytes:
        """
        Construct a standard 32-byte BR32 master request frame with CRC-16.
        Format (32B):
          [0]    CMD (1B)
          [1]    SEQ (1B)
          [2..11] TARGET_SIGROW (10B)
          [12]   PAGE_IDX (1B)
          [13]   CHUNK_FLAGS (1B)
          [14..29] DATA (16B)
          [30..31] CRC16 (2B)
        """
        if len(uid) != 10:
            uid = uid.ljust(10, b'\x00')[:10]

        data_16 = data_16.ljust(16, b'\x00')[:16]

        header = bytes([
            cmd & 0xFF,
            seq & 0xFF,
        ]) + uid + bytes([
            page_idx & 0xFF,
            chunk_flags & 0xFF,
        ]) + data_16

        crc = crc16_ccitt(header)
        frame = header + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
        return frame

    @staticmethod
    def parse_response(raw: bytes) -> Optional[Dict[str, Any]]:
        """
        Parse and validate a standard 32-byte BR32 slave response frame.
        Format (32B):
          [0]    STATUS (0x00 = STATUS_OK)
          [1]    ECHO_SEQ
          [2]    RX_COUNT (32)
          [3]    DEV_STATE
          [4..13] MY_SIGROW (10B)
          [14..29] DATA (16B payload)
          [30..31] CRC16
        """
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
        """Sends a 32B frame via Method 1 (Half-Baud Break + 0x55 Sync prefix)."""
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
        success = (parsed is not None and parsed["crc_ok"] and parsed["status"] == 0x00)

        return success, rtt_ms, rx, parsed

    def discover_slave_uid(self) -> Optional[bytes]:
        """Discover slave UID using standard 32B IDENTIFY frame."""
        print("\n[DISCOVERY] Probing bus with All-Zero UID to discover slave...")
        frame = self.build_frame(cmd=0x01, seq=0x01, uid=b'\x00' * 10)
        ok, rtt, rx, parsed = self.send_br32_frame(frame)

        if ok and parsed:
            self.slave_uid = parsed["uid_bytes"]
            print(f"[DISCOVERY] Found Slave SIGROW UID: 0x{parsed['uid_hex']} (RTT: {rtt:.2f} ms)")
            return self.slave_uid
        else:
            print("[DISCOVERY] FAIL: No response from slave. Check wiring and firmware.")
            return None

    def write_chunk(self, chunk_idx: int, data_16: bytes, seq: int) -> Tuple[bool, float]:
        """Write a 16-byte chunk to slave's virtual SRAM (0..3)."""
        frame = self.build_frame(
            cmd=0x10,  # CMD_WRITE_CHUNK
            seq=seq,
            uid=self.slave_uid,
            page_idx=0x00,
            chunk_flags=(chunk_idx & 0x03),
            data_16=data_16
        )
        ok, rtt, rx, parsed = self.send_br32_frame(frame)
        return (ok and parsed is not None and parsed["data"] == data_16), rtt

    def read_chunk(self, chunk_idx: int, seq: int) -> Tuple[bool, float, bytes]:
        """Read back a 16-byte chunk from slave's virtual SRAM (0..3)."""
        frame = self.build_frame(
            cmd=0x20,  # CMD_READ_CHUNK
            seq=seq,
            uid=self.slave_uid,
            page_idx=0x00,
            chunk_flags=(chunk_idx & 0x03),
            data_16=b'\x00' * 16
        )
        ok, rtt, rx, parsed = self.send_br32_frame(frame)
        read_data = parsed["data"] if (ok and parsed) else b''
        return ok, rtt, read_data

    def paint_and_verify(self, message_64: bytes, interval_ms: float = 200.0) -> bool:
        """
        Execute full 4-chunk Write -> Readback -> Verify sequence.
        """
        if len(message_64) != 64:
            message_64 = message_64.ljust(64, b' ')[:64]

        chunks_tx = [message_64[i * 16:(i + 1) * 16] for i in range(4)]
        chunks_rx = []

        print("\n" + "=" * 76)
        print("          STEP 1: WRITING 4 CHUNKS (16B x 4 = 64B) TO VIRTUAL SRAM")
        print("=" * 76)

        write_rtts = []
        for idx in range(4):
            data = chunks_tx[idx]
            ok, rtt = self.write_chunk(idx, data, seq=(0x10 + idx))
            write_rtts.append(rtt)
            text_repr = data.decode('ascii', errors='replace')
            status_str = f"[PASS] (RTT: {rtt:.2f} ms)" if ok else "[FAIL]"
            print(f"  [Chunk {idx}] TX: '{text_repr}' ({data.hex(' ')}) -> {status_str}")
            if not ok:
                print(f"  [ERROR] Write failed on chunk {idx}!")
                return False
            time.sleep(interval_ms / 1000.0)

        print("\n" + "=" * 76)
        print("          STEP 2: READING BACK 4 CHUNKS (16B x 4 = 64B) FROM SRAM")
        print("=" * 76)

        read_rtts = []
        for idx in range(4):
            ok, rtt, data = self.read_chunk(idx, seq=(0x20 + idx))
            read_rtts.append(rtt)
            chunks_rx.append(data)
            text_repr = data.decode('ascii', errors='replace')
            match_str = "MATCH" if data == chunks_tx[idx] else "MISMATCH"
            status_str = f"[PASS] ({match_str}, RTT: {rtt:.2f} ms)" if ok else "[FAIL]"
            print(f"  [Chunk {idx}] RX: '{text_repr}' ({data.hex(' ')}) -> {status_str}")
            if not ok or data != chunks_tx[idx]:
                print(f"  [ERROR] Readback mismatch on chunk {idx}!")
                return False
            time.sleep(interval_ms / 1000.0)

        # Full 64B Reassembly
        assembled_rx = b"".join(chunks_rx)

        print("\n" + "=" * 76)
        print("                  STEP 3: 64-BYTE VIRTUAL SRAM REASSEMBLY")
        print("=" * 76)
        print("  [ORIGINAL TX (64B)]:")
        print(f"  ASCII: '{message_64.decode('ascii', errors='replace')}'")
        print(f"  HEX  : {message_64.hex(' ')}")
        print("  --------------------------------------------------------------------------")
        print("  [READBACK RX (64B)]:")
        print(f"  ASCII: '{assembled_rx.decode('ascii', errors='replace')}'")
        print(f"  HEX  : {assembled_rx.hex(' ')}")
        print("  --------------------------------------------------------------------------")

        if assembled_rx == message_64:
            avg_w = sum(write_rtts) / len(write_rtts)
            avg_r = sum(read_rtts) / len(read_rtts)
            print("  >>> VERIFY RESULT: ★ 100% BIT-FOR-BIT PERFECT MATCH (64/64 BYTES) ★")
            print(f"      Average Write RTT: {avg_w:.2f} ms | Average Read RTT: {avg_r:.2f} ms")
            print("=" * 76 + "\n")
            return True
        else:
            print("  >>> VERIFY RESULT: [FAIL] Data corruption detected!")
            print("=" * 76 + "\n")
            return False

    def run_stress_test(self, cycles: int = 20, interval_ms: float = 200.0):
        """Run continuous stress test with randomized 64B buffers."""
        print("\n" + "=" * 76)
        print(f"       WU-4: VIRTUAL SRAM STRESS TEST ({cycles} CYCLES x 64 BYTES)")
        print("=" * 76)

        pass_count = 0
        rtts = []

        for i in range(cycles):
            rand_64 = os.urandom(64)
            # Write 4 chunks
            w_ok = True
            for c in range(4):
                ok, rtt = self.write_chunk(c, rand_64[c * 16:(c + 1) * 16], seq=((i * 8 + c) & 0xFF))
                rtts.append(rtt)
                if not ok:
                    w_ok = False
                    break
                time.sleep(interval_ms / 1000.0)

            if not w_ok:
                print(f"  [Cycle {i + 1:3d}/{cycles:3d}] WRITE FAIL!")
                break

            # Read 4 chunks
            rx_parts = []
            r_ok = True
            for c in range(4):
                ok, rtt, d = self.read_chunk(c, seq=((i * 8 + 4 + c) & 0xFF))
                rtts.append(rtt)
                if not ok:
                    r_ok = False
                    break
                rx_parts.append(d)
                time.sleep(interval_ms / 1000.0)

            if r_ok and b"".join(rx_parts) == rand_64:
                pass_count += 1
                sys.stdout.write(f"\r  Progress: [{i + 1:3d}/{cycles:3d}] | Verify PASS: {pass_count}/{i + 1} (100.0%)")
                sys.stdout.flush()
            else:
                print(f"\n  [Cycle {i + 1:3d}/{cycles:3d}] VERIFY FAIL!")
                break

        print("\n" + "=" * 76)
        if pass_count == cycles:
            avg_rtt = sum(rtts) / len(rtts)
            print(f"  FINAL VERDICT: ★ GRADE A+ (STRESS PASS: {cycles * 64:,} BYTES PERFECT VERIFIED) ★")
            print(f"                 Average Transaction RTT: {avg_rtt:.2f} ms")
        else:
            print(f"  FINAL VERDICT: [FAIL] Failed during cycle {pass_count + 1}")
        print("=" * 76 + "\n")


def main():
    parser = argparse.ArgumentParser(
        description="WU-4: BR32 Virtual SRAM Page Painter & Chunk Transfer Benchmark",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Paint default message into virtual SRAM and readback:
  python wu4_sram_bench.py --port COM19 --debug-port COM21 --paint

  # Paint custom 64-character text:
  python wu4_sram_bench.py --port COM19 --debug-port COM21 --paint --text "Hello World! ADX Core-D BR32 4-Chunk 64-Byte SRAM Painting is Fun!"

  # Run 20-cycle random binary stress test:
  python wu4_sram_bench.py --port COM19 --debug-port COM21 --stress 20
        """
    )
    parser.add_argument("--port", type=str, default="COM19", help="RS-485 serial port (default: COM19)")
    parser.add_argument("--baud", type=int, default=19200, help="RS-485 baudrate (default: 19200)")
    parser.add_argument("--debug-port", type=str, default="COM21", help="Core-D Soft-UART debug port (default: COM21)")
    parser.add_argument("--paint", action="store_true", help="Run 4-chunk paint and readback verify sequence")
    parser.add_argument("--text", type=str, default="ADX Core-D BR32 Virtual SRAM OK! 4-Chunk 64B Paint 2026-09-28 PASS",
                        help="64-byte text message to paint")
    parser.add_argument("--stress", type=int, default=0, help="Run N cycles of random 64B binary stress test")
    parser.add_argument("--interval", type=float, default=200.0, help="Packet interval in ms (default: 200.0)")

    args = parser.parse_args()

    dbg_mon = None
    if args.debug_port:
        dbg_mon = DebugMonitor(port=args.debug_port, baudrate=9600)
        dbg_mon.start()

    painter = BR32SRAMPainter(port=args.port, baudrate=args.baud)

    try:
        painter.open()
        uid = painter.discover_slave_uid()
        if not uid:
            print("[ABORT] Cannot proceed without discovering slave UID.")
            return

        time.sleep(args.interval / 1000.0)

        if args.stress > 0:
            painter.run_stress_test(cycles=args.stress, interval_ms=args.interval)
        else:
            # Default action: paint and verify
            msg_bytes = args.text.encode('ascii', errors='replace')
            painter.paint_and_verify(message_64=msg_bytes, interval_ms=args.interval)

    except KeyboardInterrupt:
        print("\n[INFO] Benchmark interrupted by user.")
    except Exception as e:
        print(f"\n[ERROR] Test execution failed: {e}")
        import traceback
        traceback.print_exc()
    finally:
        painter.close()
        if dbg_mon:
            dbg_mon.stop()


if __name__ == "__main__":
    main()
