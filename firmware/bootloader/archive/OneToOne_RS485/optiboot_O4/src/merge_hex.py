#!/usr/bin/env python3
"""
Merge Optiboot hex and blank application hex into a single Intel HEX file.
"""
import sys

def merge(boot_hex, app_hex, out_hex):
    with open(boot_hex, 'r') as f:
        boot_lines = [l.strip() for l in f if not l.startswith(':00000001')]

    with open(app_hex, 'r') as f:
        app_lines = [l.strip() for l in f if not l.startswith(':04000003') and not l.startswith(':00000001')]

    combined = boot_lines + app_lines + [':00000001FF']

    with open(out_hex, 'w') as f:
        f.write('\n'.join(combined) + '\n')

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: merge_hex.py <boot.hex> <app.hex> <output.hex>")
        sys.exit(1)
    merge(sys.argv[1], sys.argv[2], sys.argv[3])
