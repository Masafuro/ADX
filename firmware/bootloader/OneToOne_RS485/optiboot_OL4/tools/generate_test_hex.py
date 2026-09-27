#!/usr/bin/env python3
"""
Generate test Intel HEX file for Optiboot_OL4 (starts at 0x0400).
Generates 11 pages (704 bytes) with deterministic test patterns and opcodes.
"""

def generate_intel_hex(start_addr: int, total_pages: int, page_size: int = 64) -> str:
    lines = []
    for p in range(total_pages):
        page_addr = start_addr + p * page_size
        # Create 64 bytes for this page:
        # First 4 bytes: Page marker [0xAA, page_idx, page_addr_H, page_addr_L]
        # Rest: rolling counter with distinctive pattern
        data = bytearray(page_size)
        data[0] = 0xAA
        data[1] = p & 0xFF
        data[2] = (page_addr >> 8) & 0xFF
        data[3] = page_addr & 0xFF
        for i in range(4, page_size):
            data[i] = ((p * 17) + (i * 3)) & 0xFF

        # Intel HEX format: :10AAAA00DDDD...CC (16 bytes per line)
        for chunk in range(0, page_size, 16):
            addr = page_addr + chunk
            chunk_data = data[chunk:chunk+16]
            byte_count = len(chunk_data)
            rec_type = 0x00

            # Checksum: 0 - (sum of all fields) & 0xFF
            cs = byte_count + ((addr >> 8) & 0xFF) + (addr & 0xFF) + rec_type + sum(chunk_data)
            cs = (-cs) & 0xFF

            hex_line = f":{byte_count:02X}{addr:04X}{rec_type:02X}{''.join(f'{b:02X}' for b in chunk_data)}{cs:02X}"
            lines.append(hex_line)

    lines.append(":00000001FF") # End of File record
    return "\n".join(lines) + "\n"

if __name__ == '__main__':
    out_path = "../releases/test_ol4_app_0400.hex"
    content = generate_intel_hex(0x0400, 11)
    with open(out_path, "w") as f:
        f.write(content)
    print(f"Generated 11 pages (704 bytes) starting at 0x0400 -> {out_path}")
