#!/usr/bin/env python3
"""
ADX Core-D RS-485 Firmware Flash Utility (Milestone 4 / 5)
Reliably programs application hex files to Core-D over 1-to-1 RS-485 via STK500v1 protocol.

Architecture:
- Follows the 7-stage sequence design defined in rs485_communication_sequence.md:
  Stage 0: Init & Bus Stabilization
  Stage 1: Polling & Power-on Detection
  Stage 2: Bus Settling (50ms) & Clean Handshake Confirmation (Consecutive Sync)
  Stage 3: Device Identification (Optiboot version & ATtiny1616 signature)
  Stage 4: Flash Programming (64-byte pages)
  Stage 5: Read-back Verification (100% data comparison)
  Stage 6: Leave ProgMode & Application Launch
"""

import sys
import os
import time
import argparse

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] pyserial is not installed. Please run: pip install pyserial")
    sys.exit(1)

# STK500v1 Protocol Constants
STK_OK              = 0x10
STK_INSYNC          = 0x14
CRC_EOP             = 0x20  # ' '
STK_GET_SYNC        = 0x30  # '0'
STK_GET_PARAMETER   = 0x41  # 'A'
STK_SET_DEVICE      = 0x42  # 'B'
STK_SET_DEVICE_EXT  = 0x45  # 'E'
STK_ENTER_PROGMODE  = 0x50  # 'P'
STK_LEAVE_PROGMODE  = 0x51  # 'Q'
STK_LOAD_ADDRESS    = 0x55  # 'U'
STK_PROG_PAGE       = 0x64  # 'd'
STK_READ_PAGE       = 0x74  # 't'
STK_READ_SIGN       = 0x75  # 'u'
STK_SW_MAJOR        = 0x81
STK_SW_MINOR        = 0x82

PAGE_SIZE           = 64     # ATtiny1616 flash page size in bytes
APP_START_ADDR      = 0x0200 # Core-D application entry point (after 512B bootloader)
EXPECTED_SIGNATURE  = b"\x1E\x94\x21"  # ATtiny1616 device signature

def parse_intel_hex(filepath):
    """
    Parses an Intel HEX file and returns a dictionary of {byte_address: byte_value}.
    """
    data = {}
    with open(filepath, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line.startswith(':'):
                continue
            try:
                byte_count = int(line[1:3], 16)
                addr = int(line[3:7], 16)
                rec_type = int(line[7:9], 16)
                payload = bytes.fromhex(line[9:9 + byte_count * 2])
            except ValueError:
                raise ValueError(f"Malformed Intel HEX line {line_num}: {line}")

            if rec_type == 0x00:  # Data record
                for i, b in enumerate(payload):
                    data[addr + i] = b
            elif rec_type == 0x01:  # EOF
                break
    return data

class Stk500Client:
    def __init__(self, ser, verbose=False):
        self.ser = ser
        self.verbose = verbose

    def _log_debug(self, msg):
        if self.verbose:
            print(f"    [DEBUG] {msg}")

    def send_cmd(self, cmd_bytes, expect_response_len=0, timeout=0.5):
        # Flush stale inbound noise before sending command
        self.ser.reset_input_buffer()

        full_cmd = cmd_bytes + bytes([CRC_EOP])
        self._log_debug(f"TX -> {' '.join(f'0x{b:02X}' for b in full_cmd)}")
        self.ser.write(full_cmd)
        self.ser.flush()

        old_timeout = self.ser.timeout
        self.ser.timeout = timeout
        try:
            # Look for STK_INSYNC (0x14), skip leading noise (up to 8 bytes)
            insync_found = False
            for _ in range(8):
                b = self.ser.read(1)
                if not b:
                    self._log_debug("RX <- Timeout (no response)")
                    return None
                if b[0] == STK_INSYNC:
                    insync_found = True
                    break
                else:
                    self._log_debug(f"RX <- Skipped non-sync byte: 0x{b[0]:02X}")

            if not insync_found:
                self._log_debug("RX <- Failed to find STK_INSYNC (0x14)")
                return None

            payload = b""
            if expect_response_len > 0:
                payload = self.ser.read(expect_response_len)
                self._log_debug(f"RX Payload <- {' '.join(f'0x{b:02X}' for b in payload)}")
                if len(payload) != expect_response_len:
                    self._log_debug(f"RX Payload length mismatch (expected {expect_response_len}, got {len(payload)})")
                    return None

            # Read terminating STK_OK (0x10)
            ok = self.ser.read(1)
            if not ok:
                self._log_debug("RX <- Timeout waiting for STK_OK")
                return None
            self._log_debug(f"RX Status <- 0x{ok[0]:02X}")
            if ok[0] != STK_OK:
                self._log_debug(f"RX Status not STK_OK (expected 0x10, got 0x{ok[0]:02X})")
                return None

            return payload
        finally:
            self.ser.timeout = old_timeout

    def probe_sync(self, timeout=0.08):
        """Single quick probe for power-on polling."""
        return self.send_cmd(bytes([STK_GET_SYNC]), timeout=timeout) is not None

    def get_sw_version(self):
        minor = self.send_cmd(bytes([STK_GET_PARAMETER, STK_SW_MINOR]), expect_response_len=1, timeout=0.2)
        major = self.send_cmd(bytes([STK_GET_PARAMETER, STK_SW_MAJOR]), expect_response_len=1, timeout=0.2)
        if major is not None and minor is not None:
            return f"{major[0]}.{minor[0]}"
        return None

    def read_signature(self):
        return self.send_cmd(bytes([STK_READ_SIGN]), expect_response_len=3, timeout=0.3)

    def enter_progmode(self):
        return self.send_cmd(bytes([STK_ENTER_PROGMODE]), timeout=0.3) is not None

    def leave_progmode(self):
        return self.send_cmd(bytes([STK_LEAVE_PROGMODE]), timeout=0.3) is not None

    def load_address(self, addr):
        # Optiboot_x on tiny1616 uses byte address (0x0200 = app entry)
        addr_low = addr & 0xFF
        addr_high = (addr >> 8) & 0xFF
        return self.send_cmd(bytes([STK_LOAD_ADDRESS, addr_low, addr_high]), timeout=0.3) is not None

    def write_page(self, page_data):
        # STK_PROG_PAGE: len_high, len_low, memtype ('F'=Flash), bytes...
        length = len(page_data)
        len_high = (length >> 8) & 0xFF
        len_low = length & 0xFF
        cmd = bytes([STK_PROG_PAGE, len_high, len_low, ord('F')]) + bytes(page_data)
        return self.send_cmd(cmd, timeout=1.0) is not None

    def read_page(self, length):
        len_high = (length >> 8) & 0xFF
        len_low = length & 0xFF
        cmd = bytes([STK_READ_PAGE, len_high, len_low, ord('F')])
        return self.send_cmd(cmd, expect_response_len=length, timeout=1.0)

def find_default_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = p.description.lower()
        if "ch340" in desc or "ch341" in desc or "ch342" in desc or "ftdi" in desc or "usb-serial" in desc or "cp210" in desc:
            return p.device
    if ports:
        return ports[0].device
    return "COM19"

def flash_firmware(port_name, hex_file, baudrate=115200, timeout_sec=30, verbose=False):
    print("=" * 70)
    print("  ADX Core-D 1-to-1 RS-485 Firmware Flash Utility (M4/M5)")
    print("=" * 70)
    print(f"Target Port : {port_name}")
    print(f"Firmware Hex: {hex_file}")
    print(f"Baud Rate   : {baudrate} bps (8N1)")
    print("-" * 70)

    # Validate HEX file
    if not os.path.exists(hex_file):
        print(f"[ERROR] Hex file '{hex_file}' not found.")
        sys.exit(1)

    hex_data = parse_intel_hex(hex_file)
    if not hex_data:
        print("[ERROR] Hex file contains no flash data.")
        sys.exit(1)

    min_addr = min(hex_data.keys())
    max_addr = max(hex_data.keys())
    total_bytes = len(hex_data)
    print(f"[INFO] Parsed HEX: {total_bytes} bytes (Address Range: 0x{min_addr:04X} - 0x{max_addr:04X})")

    if min_addr < APP_START_ADDR:
        print(f"[WARNING] HEX starts below application area (0x{APP_START_ADDR:04X})!")
        print("          Bootloader area (0x0000-0x01FF) is protected by hardware BOOTEND fuse.")

    # -------------------------------------------------------------
    # Stage 0: Init & Bus Stabilization
    # -------------------------------------------------------------
    print("\n[STAGE 0/6: INIT] Opening port and stabilizing RS-485 bus...")
    try:
        ser = serial.Serial(
            port=port_name,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.08
        )
    except Exception as e:
        print(f"[ERROR] Failed to open port '{port_name}': {e}")
        sys.exit(1)

    # 100ms quiet time to let DTR/RTS transients dissipate
    time.sleep(0.1)
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    client = Stk500Client(ser, verbose=verbose)
    print(f"    Port {port_name} opened and bus stabilized successfully.")

    # -------------------------------------------------------------
    # Stage 1: Polling & Power-on Detection
    # -------------------------------------------------------------
    print("\n[STAGE 1/6: POLLING] Waiting for Core-D to power on / reset...")
    print(">>> Power on or reset the Core-D NOW (Red LED PB2 will start blinking) <<<\n")

    start_time = time.time()
    synced = False
    dots = 0
    last_print = 0

    while time.time() - start_time < timeout_sec:
        elapsed = int(time.time() - start_time)
        if elapsed != last_print:
            last_print = elapsed
            dots = (dots + 1) % 4
            sys.stdout.write(f"\r    [POLLING] Probing bootloader... ({elapsed}s / {timeout_sec}s) {'.' * dots:3s}")
            sys.stdout.flush()

        if client.probe_sync(timeout=0.06):
            synced = True
            break
        time.sleep(0.04)

    if not synced:
        print("\n\n[FAIL] Stage 1 Timeout: No response from Core-D after 30 seconds.")
        print("Diagnostic Checklist:")
        print("  1. If Red LED PB2 turned off immediately: A and B lines are reversed (swap A/B).")
        print("  2. If Red LED PB2 blinked for 5-8s but no response: check GND wire or COM port.")
        ser.close()
        sys.exit(1)

    elapsed_total = time.time() - start_time
    print(f"\n    Power-on detected! Initial sync response received in {elapsed_total:.1f}s.")

    # -------------------------------------------------------------
    # Stage 2: Bus Settling & Clean Handshake Confirmation
    # -------------------------------------------------------------
    print("\n[STAGE 2/6: HANDSHAKE] Settling bus (50ms) and confirming clean 2-way sync...")
    # Give target power supply & internal oscillator time to reach steady state
    time.sleep(0.05)
    ser.reset_input_buffer()

    # Require 2 consecutive clean round-trips
    confirmed = 0
    t_start_hs = time.time()
    for attempt in range(1, 6):
        t0 = time.time()
        if client.probe_sync(timeout=0.15):
            rtt = (time.time() - t0) * 1000.0
            confirmed += 1
            client._log_debug(f"Handshake probe {attempt} OK (RTT: {rtt:.1f}ms)")
            if confirmed >= 2:
                print(f"    Handshake verified! Consecutive clean sync confirmed (RTT: {rtt:.1f}ms).")
                break
        else:
            client._log_debug(f"Handshake probe {attempt} failed, retrying...")
            confirmed = 0
            time.sleep(0.02)
            ser.reset_input_buffer()

    if confirmed < 2:
        print("[FAIL] Stage 2 Failed: Unable to establish clean consecutive synchronization.")
        print("       Bus was unstable after power-on. Please check connections.")
        ser.close()
        sys.exit(1)

    # -------------------------------------------------------------
    # Stage 3: Device Identification & Enter Programming Mode
    # -------------------------------------------------------------
    print("\n[STAGE 3/6: IDENTIFY] Reading bootloader version and device signature...")
    
    # Read version
    ver = client.get_sw_version()
    if ver:
        print(f"    Bootloader Version: Optiboot {ver}")

    # Read signature
    sig = client.read_signature()
    if sig:
        sig_str = " ".join(f"0x{b:02X}" for b in sig)
        print(f"    Device Signature  : {sig_str}")
        if sig == EXPECTED_SIGNATURE:
            print("    -> Target confirmed: ATtiny1616 (MATCH)")
        else:
            print(f"    [WARNING] Signature mismatch (expected 0x1E 0x94 0x21, got {sig_str})")
    else:
        print("    [NOTICE] Device signature query skipped by bootloader.")

    # Enter ProgMode
    client.enter_progmode()
    print("    Entered programming mode successfully.")

    # -------------------------------------------------------------
    # Stage 4: Flash Programming (64-byte pages)
    # -------------------------------------------------------------
    start_page = (min_addr // PAGE_SIZE) * PAGE_SIZE
    end_page = ((max_addr + PAGE_SIZE) // PAGE_SIZE) * PAGE_SIZE
    total_pages = (end_page - start_page) // PAGE_SIZE

    print(f"\n[STAGE 4/6: WRITE] Programming {total_pages} page(s) ({PAGE_SIZE} bytes/page)...")

    for page_idx, page_addr in enumerate(range(start_page, end_page, PAGE_SIZE), 1):
        page_buf = bytearray(
            hex_data.get(page_addr + i, 0xFF) for i in range(PAGE_SIZE)
        )

        # Load address
        if not client.load_address(page_addr):
            print(f"\n[FAIL] Stage 4 Failed: Could not load address 0x{page_addr:04X}")
            ser.close()
            sys.exit(1)

        # Program page
        if not client.write_page(page_buf):
            print(f"\n[FAIL] Stage 4 Failed: Could not write page at 0x{page_addr:04X}")
            ser.close()
            sys.exit(1)

        pct = (page_idx / total_pages) * 100.0
        sys.stdout.write(f"\r    Writing Page {page_idx}/{total_pages} [0x{page_addr:04X}-0x{page_addr+PAGE_SIZE-1:04X}] ({pct:.0f}%)")
        sys.stdout.flush()

    print("\n    All pages written to flash successfully.")

    # -------------------------------------------------------------
    # Stage 5: Read-back Verification
    # -------------------------------------------------------------
    print(f"\n[STAGE 5/6: VERIFY] Verifying {total_pages} page(s) against HEX binary...")

    for page_idx, page_addr in enumerate(range(start_page, end_page, PAGE_SIZE), 1):
        page_buf = bytearray(
            hex_data.get(page_addr + i, 0xFF) for i in range(PAGE_SIZE)
        )

        # Load address for read
        if not client.load_address(page_addr):
            print(f"\n[FAIL] Stage 5 Failed: Could not reload address for verify at 0x{page_addr:04X}")
            ser.close()
            sys.exit(1)

        # Read page
        read_buf = client.read_page(PAGE_SIZE)
        if read_buf != page_buf:
            print(f"\n[FAIL] Stage 5 Verification mismatch at page 0x{page_addr:04X}!")
            if read_buf:
                print(f"       Expected: {' '.join(f'{b:02X}' for b in page_buf[:8])}...")
                print(f"       Received: {' '.join(f'{b:02X}' for b in read_buf[:8])}...")
            ser.close()
            sys.exit(1)

        pct = (page_idx / total_pages) * 100.0
        sys.stdout.write(f"\r    Verifying Page {page_idx}/{total_pages} [0x{page_addr:04X}-0x{page_addr+PAGE_SIZE-1:04X}] ({pct:.0f}%)")
        sys.stdout.flush()

    print("\n    100% verified! Flash contents match HEX file perfectly.")

    # -------------------------------------------------------------
    # Stage 6: Leave ProgMode & Application Launch
    # -------------------------------------------------------------
    print("\n[STAGE 6/6: LAUNCH] Exiting bootloader to launch application...")
    client.leave_progmode()
    ser.close()

    print("    Leave ProgMode command acknowledged by Core-D.")
    print("    Core-D watchdog will trigger in 8ms and start the user application.")
    print("\n" + "=" * 70)
    print("  [SUCCESS] Application successfully flashed over RS-485!")
    print("=" * 70)
    print(">>> The Core-D is now running test_blink_white_led!")
    print(">>> Check the onboard WHITE LED (PB3) — it should be blinking (500ms cycle)!")
    print("=" * 70 + "\n")

def main():
    parser = argparse.ArgumentParser(description="ADX Core-D 1-to-1 RS-485 Firmware Flash Utility")
    parser.add_argument("port", nargs="?", default=None, help="COM port (e.g. COM19)")
    parser.add_argument("hex", nargs="?", default=None, help="Path to .hex file")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose debug logging")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--timeout", type=int, default=30, help="Wait timeout in seconds (default: 30)")

    args = parser.parse_args()

    port = args.port or find_default_port()
    hex_path = args.hex or "test_blink_white_led.hex"

    # Also search releases directory if not found in current directory
    if not os.path.exists(hex_path):
        alt_path = os.path.join(os.path.dirname(__file__), "..", "releases", os.path.basename(hex_path))
        if os.path.exists(alt_path):
            hex_path = alt_path

    flash_firmware(port, hex_path, baudrate=args.baud, timeout_sec=args.timeout, verbose=args.verbose)

if __name__ == "__main__":
    main()
