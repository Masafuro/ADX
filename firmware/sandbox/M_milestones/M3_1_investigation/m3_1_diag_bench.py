#!/usr/bin/env python3
"""
Milestone 3-1 (M3-1): Advanced Post-Write Hardware State & Diagnostic Console Suite
SPDX-License-Identifier: MIT

Usage:
  # 1. Non-destructive inspection of current firmware (NO writes, inspect Read/CRC raw responses):
  python m3_1_diag_bench.py --port COM22 --mode inspect --page 20

  # 2. Test post-write liveness (perform 1 write, then Ping immediately to test CPU survival):
  python m3_1_diag_bench.py --port COM22 --mode survival --page 20

  # 3. Read hardware telemetry registers (RSTCTRL.RSTFR, NVMCTRL.STATUS, Clock):
  python m3_1_diag_bench.py --port COM22 --mode telemetry

  # 4. Full automated investigation suite:
  python m3_1_diag_bench.py --port COM22 --mode full --page 20
"""

import sys
import time
import struct
import argparse
from typing import Optional, Tuple, List, Dict, Any, Union

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] 'pyserial' is not installed. Please run: pip install pyserial")
    sys.exit(1)

# Protocol Constants
MR32_SYNC_BYTE       = 0x55
MR32_MAGIC_BYTE      = 0xAD
MR32_FRAME_LEN       = 32
MR32_HEADER_LEN      = 6
MR32_PAYLOAD_LEN     = 24
MR32_CRC_LEN         = 2

# Standard Commands
CMD_BOOT_PING        = 0x10
CMD_BOOT_WRITE_CHUNK = 0x11
CMD_BOOT_READ_CHUNK  = 0x12
CMD_BOOT_CRC_CHECK   = 0x13

# Milestone 3-1 Diagnostic Commands
CMD_DIAG_SYS_STATUS  = 0x20
CMD_DIAG_SOFT_RESET  = 0x22

# Status Codes
STATUS_OK            = 0x00
STATUS_ERR_CRC       = 0x01
STATUS_ERR_TIMEOUT   = 0x02
STATUS_ERR_PARAM     = 0x03
STATUS_PAGE_DONE     = 0x10

STATUS_NAMES = {
    0x00: "STATUS_OK",
    0x01: "STATUS_ERR_CRC",
    0x02: "STATUS_ERR_TIMEOUT",
    0x03: "STATUS_ERR_PARAM (Protected)",
    0x10: "STATUS_PAGE_DONE (NVM Committed)",
}


def calculate_crc16_ccitt(data: Union[bytes, bytearray], init_val: int = 0xFFFF) -> int:
    """Calculate CRC-16-CCITT (Polynomial: 0x1021, Initial: 0xFFFF)."""
    crc = init_val
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def build_mr32_frame(dst_id: int, src_id: int, cmd: int, seq_num: int, payload: bytes = b"") -> bytes:
    """Build a 32-byte fixed-length MR32 frame."""
    if len(payload) < MR32_PAYLOAD_LEN:
        padded_payload = payload + b"\x00" * (MR32_PAYLOAD_LEN - len(payload))
    else:
        padded_payload = payload[:MR32_PAYLOAD_LEN]

    header = struct.pack("BBBBBB", MR32_SYNC_BYTE, MR32_MAGIC_BYTE, dst_id, src_id, cmd, seq_num)
    body = header[2:] + padded_payload  # 28 bytes for CRC
    crc = calculate_crc16_ccitt(body)
    return header + padded_payload + struct.pack("<H", crc)


def parse_mr32_frame_detailed(frame: bytes) -> Tuple[bool, Optional[dict], str]:
    """Detailed parsing of a 32-byte frame with verbose error reporting."""
    if len(frame) != MR32_FRAME_LEN:
        return False, None, f"Frame length mismatch: expected 32, got {len(frame)} bytes"

    if frame[0] != MR32_SYNC_BYTE:
        return False, None, f"SYNC error: 0x{frame[0]:02X} (expected 0x55)"

    if frame[1] != MR32_MAGIC_BYTE:
        return False, None, f"MAGIC error: 0x{frame[1]:02X} (expected 0xAD)"

    expected_crc = calculate_crc16_ccitt(frame[2:30])
    received_crc = struct.unpack("<H", frame[30:32])[0]
    crc_ok = (expected_crc == received_crc)

    parsed = {
        "sync": frame[0],
        "magic": frame[1],
        "dst_id": frame[2],
        "src_id": frame[3],
        "cmd": frame[4],
        "seq_num": frame[5],
        "payload": frame[6:30],
        "crc_expected": expected_crc,
        "crc_received": received_crc,
        "crc_ok": crc_ok,
        "raw_hex": frame.hex()
    }

    if not crc_ok:
        return False, parsed, f"CRC mismatch: Calc=0x{expected_crc:04X} != Recv=0x{received_crc:04X}"

    return True, parsed, "OK"


class M31DiagnosticBench:
    def __init__(self, port_name: str, baudrate: int = 115200):
        self.port_name = port_name
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None

    def connect(self):
        print(f"[INIT] Opening RS-485 port {self.port_name} @ {self.baudrate} bps (8N1)...")
        self.ser = serial.Serial(
            port=self.port_name,
            baudrate=self.baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.01,
            write_timeout=0.2
        )
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        time.sleep(0.05)
        print("[INIT] Serial port opened successfully.\n")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("[INFO] Port closed.")

    def send_and_receive_verbose(self, frame: bytes, timeout: float = 0.1, label: str = "") -> Tuple[Optional[bytes], float, Optional[dict], str]:
        """Send frame, capture exact raw response and return detailed diagnostic parse."""
        self.ser.reset_input_buffer()
        t_start = time.perf_counter()
        self.ser.write(frame)
        self.ser.flush()

        resp = b""
        deadline = t_start + timeout
        while len(resp) < MR32_FRAME_LEN and time.perf_counter() < deadline:
            chunk = self.ser.read(MR32_FRAME_LEN - len(resp))
            if chunk:
                resp += chunk

        rtt_ms = (time.perf_counter() - t_start) * 1000.0

        if len(resp) == 0:
            return None, rtt_ms, None, "TIMEOUT (0 bytes received from target)"

        valid, parsed, msg = parse_mr32_frame_detailed(resp)
        return resp, rtt_ms, parsed, msg

    # =========================================================================
    # Diagnostic Scenario 1: Non-destructive Raw Response Inspection
    # =========================================================================
    def scenario_inspect_raw(self, page_no: int = 20, node: int = 0x01) -> bool:
        print("=" * 70)
        print(f" [DIAGNOSTIC 1] Non-Destructive Raw Packet Inspection (Page {page_no})")
        print("   -> NO writes performed. Reading directly to analyze MCU responses.")
        print("=" * 70)

        # 1. Ping
        print("\n--- Step 1: Probe Ping (CMD 0x10) ---")
        tx_ping = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=1)
        resp, rtt, parsed, msg = self.send_and_receive_verbose(tx_ping, label="Ping")
        print(f" TX HEX: {tx_ping.hex()}")
        if resp:
            print(f" RX HEX: {resp.hex()} (len={len(resp)}, RTT={rtt:.2f}ms)")
            print(f" Parse : {'[VALID]' if parsed and parsed['crc_ok'] else '[INVALID]'} -> {msg}")
            if parsed and parsed.get("crc_ok"):
                p = parsed["payload"]
                mcu_id = (p[1] << 8) | p[2]
                print(f" Target: MCU 0x{mcu_id:04X} | Flash: {p[3]}KB | Page: {p[4]}B")
        else:
            print(f" RX    : {msg} (RTT={rtt:.2f}ms)")

        # 2. Read Chunk 0..3 Raw
        print(f"\n--- Step 2: Readback Chunks 0..3 (CMD 0x12) from Page {page_no} ---")
        read_success = True
        for c in range(4):
            payload = struct.pack("<HB", page_no, c) + b"\x00" * 21
            tx_read = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_READ_CHUNK, seq_num=c, payload=payload)
            resp, rtt, parsed, msg = self.send_and_receive_verbose(tx_read, label=f"ReadChunk{c}")

            print(f" [Chunk {c}]")
            print(f"   TX HEX: {tx_read.hex()}")
            if resp:
                print(f"   RX HEX: {resp.hex()} (len={len(resp)}, RTT={rtt:.2f}ms)")
                print(f"   Status: {'PASS' if (parsed and parsed['crc_ok']) else 'FAIL'} -> {msg}")
                if parsed:
                    p = parsed["payload"]
                    st = p[0]
                    st_str = STATUS_NAMES.get(st, f"0x{st:02X}")
                    ret_page = p[1] | (p[2] << 8)
                    ret_chunk = p[3]
                    data_hex = p[4:20].hex()
                    flash_crc = p[20] | (p[21] << 8) if len(p) >= 22 else 0
                    print(f"   Decoded: Status={st_str}, Page={ret_page}, Chunk={ret_chunk}")
                    print(f"   Data 16B: {data_hex} | ASCII: '{p[4:20].decode('utf-8', errors='replace')}'")
                if not (parsed and parsed["crc_ok"]):
                    read_success = False
            else:
                print(f"   RX    : {msg} (RTT={rtt:.2f}ms)")
                read_success = False

        # 3. CRC Check Command Raw
        print(f"\n--- Step 3: Hardware Flash CRC Check Command (CMD 0x13) ---")
        payload = struct.pack("<H", page_no) + b"\x00" * 22
        tx_crc = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_CRC_CHECK, seq_num=1, payload=payload)
        resp, rtt, parsed, msg = self.send_and_receive_verbose(tx_crc, label="CRCCheck")
        print(f" TX HEX: {tx_crc.hex()}")
        if resp:
            print(f" RX HEX: {resp.hex()} (len={len(resp)}, RTT={rtt:.2f}ms)")
            print(f" Parse : {'[VALID]' if parsed and parsed['crc_ok'] else '[INVALID]'} -> {msg}")
            if parsed:
                p = parsed["payload"]
                crc_val = p[1] | (p[2] << 8)
                pg_val = p[3] | (p[4] << 8)
                print(f" Decoded: Target Page={pg_val}, Reported Flash CRC16=0x{crc_val:04X}")
        else:
            print(f" RX    : {msg} (RTT={rtt:.2f}ms)")

        return read_success

    # =========================================================================
    # Diagnostic Scenario 2: Post-Flash-Write Liveness & Survival Probe
    # =========================================================================
    def scenario_post_write_survival(self, page_no: int = 20, node: int = 0x01) -> bool:
        print("=" * 70)
        print(f" [DIAGNOSTIC 2] Post-Flash-Write Liveness & CPU Survival Test (Page {page_no})")
        print("   -> Commits a physical Flash page, then IMMEDIATELY probes MCU liveness.")
        print("=" * 70)

        test_data = ("ADX Milestone 3-1: Real-time Post-Write CPU Liveness Test! " + str(time.time()))[:64].encode("utf-8").ljust(64, b" ")
        expected_crc = calculate_crc16_ccitt(test_data)
        print(f" Test Payload CRC16: 0x{expected_crc:04X}")

        # Write 4 Chunks
        print("\n [Step 1] Committing 4 chunks to physical Flash...")
        commit_rtt = 0.0
        for c in range(4):
            c_data = test_data[c * 16 : (c + 1) * 16]
            payload = struct.pack("<HB", page_no, c) + c_data + b"\x00" * 5
            tx_frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_WRITE_CHUNK, seq_num=c, payload=payload)
            resp, rtt, parsed, msg = self.send_and_receive_verbose(tx_frame, timeout=0.1)

            if not resp or not (parsed and parsed["crc_ok"]):
                print(f"  [FAIL] Chunk {c} write failed: {msg}")
                return False

            p = parsed["payload"]
            st = p[0]
            st_str = "PAGE_DONE (Hardware Flash Written!)" if st == STATUS_PAGE_DONE else "OK"
            print(f"  Chunk {c}: RTT={rtt:.2f}ms | status={st_str} | mask=0x{p[4]:02X}")
            if c == 3:
                commit_rtt = rtt
                hw_crc = p[5] | (p[6] << 8)
                print(f"  -> Hardware Readback Flash CRC16: 0x{hw_crc:04X} (Expected: 0x{expected_crc:04X})")

        print(f"\n [Step 2] FLASH WRITE COMPLETED in Chunk 3 (RTT: {commit_rtt:.2f}ms)!")
        print("          Now testing IMMEDIATE MCU Liveness with zero-sleep Ping probe...")

        # Immediate Ping 1 (0ms pause)
        tx_ping = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_BOOT_PING, seq_num=10)
        resp1, rtt1, parsed1, msg1 = self.send_and_receive_verbose(tx_ping, timeout=0.05)
        p1_ok = (resp1 is not None) and (parsed1 is not None) and parsed1["crc_ok"]
        print(f"  Probe 1 (Immediate +0ms)  : {'[PASS - CPU ALIVE!]' if p1_ok else '[FAIL - NO REPLY]'} RTT={rtt1:.2f}ms -> {msg1}")

        # Ping 2 (+20ms pause)
        time.sleep(0.02)
        resp2, rtt2, parsed2, msg2 = self.send_and_receive_verbose(tx_ping, timeout=0.05)
        p2_ok = (resp2 is not None) and (parsed2 is not None) and parsed2["crc_ok"]
        print(f"  Probe 2 (After +20ms)     : {'[PASS - CPU ALIVE!]' if p2_ok else '[FAIL - NO REPLY]'} RTT={rtt2:.2f}ms -> {msg2}")

        # Ping 3 (+100ms pause)
        time.sleep(0.10)
        resp3, rtt3, parsed3, msg3 = self.send_and_receive_verbose(tx_ping, timeout=0.05)
        p3_ok = (resp3 is not None) and (parsed3 is not None) and parsed3["crc_ok"]
        print(f"  Probe 3 (After +100ms)    : {'[PASS - CPU ALIVE!]' if p3_ok else '[FAIL - NO REPLY]'} RTT={rtt3:.2f}ms -> {msg3}")

        overall_survival = p1_ok and p2_ok and p3_ok
        print("-" * 70)
        if overall_survival:
            print(" >>> ★ MCU SURVIVED POST-FLASH COMMIT WITH ZERO STALL / ZERO HANG! ★ <<<")
        else:
            print(" >>> [ALERT] MCU exhibited unresponsive behavior following Flash commit! <<<")
        return overall_survival

    # =========================================================================
    # Diagnostic Scenario 3: Hardware Telemetry Register Inspection
    # =========================================================================
    def scenario_system_telemetry(self, node: int = 0x01) -> bool:
        print("=" * 70)
        print(" [DIAGNOSTIC 3] Hardware Telemetry & Register Inspection (CMD 0x20)")
        print("   -> Captures RSTCTRL.RSTFR, NVMCTRL.STATUS, CLKCTRL, and Flash Counters.")
        print("=" * 70)

        tx_frame = build_mr32_frame(dst_id=node, src_id=0x00, cmd=CMD_DIAG_SYS_STATUS, seq_num=1)
        resp, rtt, parsed, msg = self.send_and_receive_verbose(tx_frame, timeout=0.1)

        print(f" TX HEX: {tx_frame.hex()}")
        if not resp:
            print(f" RX    : {msg} (Target firmware might be running standard M3 without CMD 0x20)")
            return False

        print(f" RX HEX: {resp.hex()} (len={len(resp)}, RTT={rtt:.2f}ms)")
        if not (parsed and parsed["crc_ok"]):
            print(f" [FAIL] Parse error: {msg}")
            return False

        p = parsed["payload"]
        st = p[0]
        if st != STATUS_OK:
            print(f" [WARN] MCU returned error status: 0x{st:02X}")
            return False

        boot_rstfr       = p[1]
        nvm_current      = p[2]
        nvm_last         = p[3]
        write_count      = p[4] | (p[5] << 8)
        last_page        = p[6] | (p[7] << 8)
        last_flash_crc   = p[8] | (p[9] << 8)
        clk_status       = p[10]
        sreg             = p[11]

        # Decode Reset Reason (RSTCTRL.RSTFR)
        reset_causes = []
        if boot_rstfr & 0x01: reset_causes.append("Power-On Reset (POR)")
        if boot_rstfr & 0x02: reset_causes.append("Brown-Out Detection (BOD)")
        if boot_rstfr & 0x04: reset_causes.append("External Reset Pin (EXTRF)")
        if boot_rstfr & 0x08: reset_causes.append("Watchdog Timer (WDT)")
        if boot_rstfr & 0x20: reset_causes.append("Software Reset (SWRF)")
        if boot_rstfr & 0x40: reset_causes.append("UPDI Reset (UPDIRF)")
        reset_str = ", ".join(reset_causes) if reset_causes else "None / Undefined"

        print("\n [TELEMETRY REGISTER DECODE]")
        print(f"  - Boot RSTCTRL.RSTFR : 0x{boot_rstfr:02X} [{reset_str}]")
        print(f"  - NVMCTRL.STATUS Curr: 0x{nvm_current:02X} (FBUSY={bool(nvm_current & 0x01)}, EEBUSY={bool(nvm_current & 0x02)})")
        print(f"  - Post-Write NVM Stat: 0x{nvm_last:02X}")
        print(f"  - Flash Write Count  : {write_count} pages committed")
        print(f"  - Last Written Page  : Page {last_page} (Address 0x{last_page*64:04X})")
        print(f"  - Last Page Flash CRC: 0x{last_flash_crc:04X}")
        print(f"  - Main Clock Status  : 0x{clk_status:02X} (Oscillator Stable={bool(clk_status & 0x10)})")
        print(f"  - CPU SREG Status    : 0x{sreg:02X} (Global Interrupts={bool(sreg & 0x80)})")

        return True


def main():
    parser = argparse.ArgumentParser(description="M3-1 Advanced Post-Write Hardware State & Diagnostic Console Suite")
    parser.add_argument("--port", "-p", type=str, help="RS-485 Serial Port (e.g. COM22, /dev/ttyUSB0)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--node", "-n", type=int, default=1, help="Target Node ID (default: 1)")
    parser.add_argument("--page", type=int, default=20, help="Target Application Flash Page (default: 20)")
    parser.add_argument("--mode", "-m", type=str, default="inspect",
                        choices=["inspect", "survival", "telemetry", "full"],
                        help="Diagnostic Mode: 'inspect' (non-destructive raw dump), 'survival' (post-write liveness), 'telemetry' (registers), 'full'")
    args = parser.parse_args()

    if not args.port:
        ports = serial.tools.list_ports.comports()
        print("[DETECTED PORTS]")
        for p in ports:
            print(f"  - {p.device}: {p.description}")
        print("\nPlease specify a serial port using --port <PORT_NAME>")
        sys.exit(0)

    bench = M31DiagnosticBench(port_name=args.port, baudrate=args.baud)
    try:
        bench.connect()

        if args.mode == "inspect":
            bench.scenario_inspect_raw(page_no=args.page, node=args.node)

        elif args.mode == "survival":
            bench.scenario_post_write_survival(page_no=args.page, node=args.node)

        elif args.mode == "telemetry":
            bench.scenario_system_telemetry(node=args.node)

        elif args.mode == "full":
            bench.scenario_inspect_raw(page_no=args.page, node=args.node)
            print("\n\n")
            bench.scenario_post_write_survival(page_no=args.page, node=args.node)
            print("\n\n")
            bench.scenario_system_telemetry(node=args.node)

    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Benchmark cancelled by user.")
    finally:
        bench.close()


if __name__ == "__main__":
    main()
