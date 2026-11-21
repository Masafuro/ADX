#!/usr/bin/env python3
"""
ADX 8854 Optical Board ID DataMatrix (ECC200 10x10) Generator
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT

Generates EasyEDA Pro / KiCad ready SVG and transparent PNG assets for the
ADX Official Optical Profile (M4 slot: 10x10 ECC200 DataMatrix).

Key features:
- Zero mandatory external dependencies (pure Python ISO/IEC 16022 ECC200 10x10
  encoder + Reed-Solomon GF(256) + built-in round-trip decoder self-check).
- Outputs clean, seam-free merged orthogonal SVG paths at exact physical mm.
- Outputs crisp, non-antialiased transparent PNG (black = silkscreen ON,
  transparent = bare soldermask OFF; requires Pillow).
- Default mode ("silk") has 0 mm quiet zone in the file itself so EDA import
  matches the exact 4.0 x 4.0 mm body without accidental border scaling, while
  producing Light-on-Dark on dark soldermask and Dark-on-Light on white PCB.
- Optional "negative-box" mode (solid silk quiet zone + mask cutouts), optional
  bleed compensation, and optional PCB color preview image.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import List, Sequence, Tuple

try:
    from PIL import Image, ImageDraw
    HAS_PIL = True
except ImportError:
    HAS_PIL = False


# ---------------------------------------------------------------------------
# ISO/IEC 16022 ECC200 10x10 (3 data codewords + 5 RS ECC codewords)
# ---------------------------------------------------------------------------

# GF(256) with primitive polynomial x^8 + x^5 + x^3 + x^2 + 1 (0x12D = 301)
_GF_EXP = [0] * 512
_GF_LOG = [0] * 256
_x = 1
for _i in range(255):
    _GF_EXP[_i] = _x
    _GF_LOG[_x] = _i
    _x <<= 1
    if _x & 0x100:
        _x ^= 0x12D
for _i in range(255, 512):
    _GF_EXP[_i] = _GF_EXP[_i - 255]

# RS generator polynomial for 5 ECC codewords: prod_{i=1..5}(x - alpha^i)
_RS_GEN_5 = (1, 62, 111, 15, 48, 228)


def _gf_mul(a: int, b: int) -> int:
    if a == 0 or b == 0:
        return 0
    return _GF_EXP[_GF_LOG[a] + _GF_LOG[b]]


def encode_ascii_codewords_10x10(payload: str) -> List[int]:
    """Encode payload into exactly 3 ECC200 data codewords (10x10 symbol)."""
    cws: List[int] = []
    i = 0
    n = len(payload)
    while i < n:
        ch = payload[i]
        if i + 1 < n and ch.isdigit() and payload[i + 1].isdigit():
            cws.append(130 + int(payload[i : i + 2]))
            i += 2
        else:
            code = ord(ch)
            if code > 127:
                raise ValueError(f"Non-ASCII character {ch!r} is not supported in 10x10 mode.")
            cws.append(code + 1)
            i += 1

    if len(cws) > 3:
        raise ValueError(
            f"Payload {payload!r} requires {len(cws)} codewords; "
            "10x10 ECC200 holds at most 3 codewords (up to 6 digits or 3 ASCII chars)."
        )

    # ISO/IEC 16022 253-state padding if fewer than 3 codewords
    if len(cws) < 3:
        cws.append(129)
    while len(cws) < 3:
        pos = len(cws) + 1  # 1-based codeword index
        pseudo = ((149 * pos) % 253) + 1
        pad = 129 + pseudo
        if pad > 254:
            pad -= 254
        cws.append(pad)

    return cws


def compute_rs_ecc5(data_cws: Sequence[int]) -> List[int]:
    """Compute 5 Reed-Solomon ECC codewords for 10x10 ECC200."""
    rem = [0, 0, 0, 0, 0]
    for d in data_cws:
        factor = d ^ rem[0]
        rem = rem[1:] + [0]
        for j in range(5):
            rem[j] ^= _gf_mul(_RS_GEN_5[j + 1], factor)
    return rem


def verify_rs_syndrome5(codewords8: Sequence[int]) -> bool:
    """Verify all 5 Reed-Solomon syndromes evaluate to zero."""
    if len(codewords8) != 8:
        return False
    for root_pow in range(1, 6):
        alpha = _GF_EXP[root_pow]
        acc = 0
        for cw in codewords8:
            acc = _gf_mul(acc, alpha) ^ cw
        if acc != 0:
            return False
    return True


def _utah_bit_positions_8x8() -> List[List[Tuple[int, int]]]:
    """Return ordered list of 8 codeword bit-coordinate lists for an 8x8 data region."""
    nrow, ncol = 8, 8
    visited = [[False] * ncol for _ in range(nrow)]
    placements: List[List[Tuple[int, int]]] = []

    def wrap(r: int, c: int) -> Tuple[int, int]:
        if r < 0:
            r += nrow
            c += 4 - ((nrow + 4) % 8)
        if c < 0:
            c += ncol
            r += 4 - ((ncol + 4) % 8)
        visited[r][c] = True
        return (r, c)

    def utah(r: int, c: int) -> List[Tuple[int, int]]:
        return [
            wrap(r - 2, c - 2),
            wrap(r - 2, c - 1),
            wrap(r - 1, c - 2),
            wrap(r - 1, c - 1),
            wrap(r - 1, c),
            wrap(r, c - 2),
            wrap(r, c - 1),
            wrap(r, c),
        ]

    def corner1() -> List[Tuple[int, int]]:
        pts = [(nrow - 1, 0), (nrow - 1, 1), (nrow - 1, 2), (0, ncol - 2), (0, ncol - 1), (1, ncol - 1), (2, ncol - 1), (3, ncol - 1)]
        return [wrap(r, c) for r, c in pts]

    def corner2() -> List[Tuple[int, int]]:
        pts = [(nrow - 3, 0), (nrow - 2, 0), (nrow - 1, 0), (0, ncol - 4), (0, ncol - 3), (0, ncol - 2), (0, ncol - 1), (1, ncol - 1)]
        return [wrap(r, c) for r, c in pts]

    def corner3() -> List[Tuple[int, int]]:
        pts = [(nrow - 3, 0), (nrow - 2, 0), (nrow - 1, 0), (0, ncol - 2), (0, ncol - 1), (1, ncol - 1), (2, ncol - 1), (3, ncol - 1)]
        return [wrap(r, c) for r, c in pts]

    def corner4() -> List[Tuple[int, int]]:
        pts = [(nrow - 1, 0), (nrow - 1, ncol - 1), (0, ncol - 3), (0, ncol - 2), (0, ncol - 1), (1, ncol - 3), (1, ncol - 2), (1, ncol - 1)]
        return [wrap(r, c) for r, c in pts]

    row, col = 4, 0
    while True:
        if row == nrow and col == 0:
            placements.append(corner1())
        elif row == nrow - 2 and col == 0 and (ncol % 4):
            placements.append(corner2())
        elif row == nrow - 2 and col == 0 and (ncol % 8 == 4):
            placements.append(corner3())
        elif row == nrow + 4 and col == 2 and (ncol % 8 == 0):
            placements.append(corner4())

        while True:
            if row < nrow and col >= 0 and not visited[row][col]:
                placements.append(utah(row, col))
            row -= 2
            col += 2
            if not (row >= 0 and col < ncol):
                break
        row += 1
        col += 3

        while True:
            if row >= 0 and col < ncol and not visited[row][col]:
                placements.append(utah(row, col))
            row += 2
            col -= 2
            if not (row < nrow and col >= 0):
                break
        row += 3
        col += 1

        if not (row < nrow or col < ncol):
            break

    if len(placements) != 8 or not all(all(r) for r in visited):
        raise RuntimeError("Internal Utah placement check failed.")
    return placements


_UTAH_8X8 = _utah_bit_positions_8x8()


def encode_datamatrix_10x10(payload: str) -> Tuple[List[List[int]], List[int]]:
    """Return (10x10 binary matrix [1=module ON, 0=OFF], 8 codewords)."""
    data_cws = encode_ascii_codewords_10x10(payload)
    ecc_cws = compute_rs_ecc5(data_cws)
    all_cws = data_cws + ecc_cws

    inner = [[0] * 8 for _ in range(8)]
    for cw, coords in zip(all_cws, _UTAH_8X8):
        for bit_idx, (r, c) in enumerate(coords):
            inner[r][c] = (cw >> (7 - bit_idx)) & 1

    # Assemble 10x10 with L-shaped solid finder (left & bottom) and clock tracks (top & right)
    grid = [[0] * 10 for _ in range(10)]
    for c in range(10):
        grid[0][c] = 1 if (c % 2 == 0) else 0  # top timing track: 1,0,1,0,...
        grid[9][c] = 1                          # bottom solid finder
    for r in range(10):
        grid[r][0] = 1                          # left solid finder
        grid[r][9] = 1 if (r % 2 == 1) else 0   # right timing track: 0,1,0,1,...
    for r in range(8):
        for c in range(8):
            grid[r + 1][c + 1] = inner[r][c]

    return grid, all_cws


def decode_datamatrix_10x10(grid: Sequence[Sequence[int]]) -> Tuple[str, List[int], bool]:
    """Decode a 10x10 matrix and verify finder pattern + RS syndromes."""
    if len(grid) != 10 or any(len(row) != 10 for row in grid):
        raise ValueError("Expected a 10x10 binary matrix.")

    # Verify finder & clock tracks
    for c in range(10):
        if grid[0][c] != (1 if c % 2 == 0 else 0) or grid[9][c] != 1:
            raise ValueError("Invalid horizontal finder/clock pattern.")
    for r in range(10):
        if grid[r][0] != 1 or grid[r][9] != (1 if r % 2 == 1 else 0):
            raise ValueError("Invalid vertical finder/clock pattern.")

    cws: List[int] = []
    for coords in _UTAH_8X8:
        v = 0
        for r, c in coords:
            v = (v << 1) | int(grid[r + 1][c + 1])
        cws.append(v)

    rs_ok = verify_rs_syndrome5(cws)

    out: List[str] = []
    for cw in cws[:3]:
        if cw == 129:
            break
        elif 130 <= cw <= 229:
            out.append(f"{cw - 130:02d}")
        elif 1 <= cw <= 128:
            out.append(chr(cw - 1))
        else:
            raise ValueError(f"Unsupported codeword {cw} in ASCII mode.")

    return "".join(out), cws, rs_ok


# ---------------------------------------------------------------------------
# Merged Orthogonal Polygon Contour Tracing (Seam-Free SVG + Bleed Comp)
# ---------------------------------------------------------------------------

def build_ink_grid(
    dm_grid: Sequence[Sequence[int]],
    mode: str,
    quiet_cells: int,
) -> List[List[bool]]:
    """
    Build the boolean ink mask (True = silkscreen ink ON).
    - mode == 'silk': modules (1) are ink ON, quiet zone is ink OFF.
    - mode == 'negative-box': quiet zone and blank cells (0) are ink ON,
      modules (1) are bare soldermask cutouts (ink OFF).
    """
    size = 10 + 2 * quiet_cells
    bg_ink = (mode == "negative-box")
    ink = [[bg_ink] * size for _ in range(size)]
    for r in range(10):
        for c in range(10):
            is_mod = bool(dm_grid[r][c])
            ink[r + quiet_cells][c + quiet_cells] = (not is_mod) if mode == "negative-box" else is_mod
    return ink


def trace_orthogonal_loops(ink: Sequence[Sequence[bool]]) -> List[List[Tuple[int, int]]]:
    """
    Trace all boundary loops of a 2D boolean grid into simple, non-self-intersecting
    orthogonal polygons (outer loops CW, hole loops CCW in SVG +x right / +y down).
    """
    h = len(ink)
    w = len(ink[0]) if h else 0

    outgoing: dict[Tuple[int, int], List[Tuple[int, int]]] = {}

    def add_edge(p0: Tuple[int, int], p1: Tuple[int, int]) -> None:
        outgoing.setdefault(p0, []).append(p1)

    for r in range(h):
        for c in range(w):
            if not ink[r][c]:
                continue
            if r == 0 or not ink[r - 1][c]:
                add_edge((c, r), (c + 1, r))          # top (right)
            if c == w - 1 or not ink[r][c + 1]:
                add_edge((c + 1, r), (c + 1, r + 1))  # right (down)
            if r == h - 1 or not ink[r + 1][c]:
                add_edge((c + 1, r + 1), (c, r + 1))  # bottom (left)
            if c == 0 or not ink[r][c - 1]:
                add_edge((c, r + 1), (c, r))          # left (up)

    loops: List[List[Tuple[int, int]]] = []

    while outgoing:
        start = next(iter(outgoing))
        nxt = outgoing[start].pop()
        if not outgoing[start]:
            del outgoing[start]

        pts = [start, nxt]
        prev = start
        curr = nxt

        while curr != start:
            cands = outgoing[curr]
            if len(cands) == 1:
                chosen = cands.pop()
            else:
                # Resolve checkerboard touching vertex: prefer 90-deg RIGHT turn
                # in SVG coords: (dx, dy) -> (-dy, dx) keeps interior on right.
                dx = curr[0] - prev[0]
                dy = curr[1] - prev[1]
                pref_dirs = [(-dy, dx), (dx, dy), (dy, -dx)]
                chosen = None
                for pdx, pdy in pref_dirs:
                    target = (curr[0] + pdx, curr[1] + pdy)
                    if target in cands:
                        cands.remove(target)
                        chosen = target
                        break
                if chosen is None:
                    chosen = cands.pop()

            if not cands:
                del outgoing[curr]

            prev = curr
            curr = chosen
            pts.append(curr)

        # Remove duplicate closing point and collapse collinear vertices
        pts.pop()
        simplified: List[Tuple[int, int]] = []
        m = len(pts)
        for idx in range(m):
            p_prev = pts[(idx - 1) % m]
            p_cur = pts[idx]
            p_next = pts[(idx + 1) % m]
            if (p_cur[0] - p_prev[0]) * (p_next[1] - p_cur[1]) != (p_cur[1] - p_prev[1]) * (p_next[0] - p_cur[0]):
                simplified.append(p_cur)
        loops.append(simplified)

    return loops


def offset_orthogonal_loop_mm(
    loop: Sequence[Tuple[int, int]],
    cell_mm: float,
    inset_mm: float,
) -> List[Tuple[float, float]]:
    """Scale integer grid loop to mm and inset inward along boundary normals by inset_mm."""
    n = len(loop)
    out: List[Tuple[float, float]] = []
    for i in range(n):
        p_prev = loop[(i - 1) % n]
        p_cur = loop[i]
        p_next = loop[(i + 1) % n]

        # Unit direction vectors of incoming and outgoing edges
        dx1 = 0 if p_cur[0] == p_prev[0] else (1 if p_cur[0] > p_prev[0] else -1)
        dy1 = 0 if p_cur[1] == p_prev[1] else (1 if p_cur[1] > p_prev[1] else -1)
        dx2 = 0 if p_next[0] == p_cur[0] else (1 if p_next[0] > p_cur[0] else -1)
        dy2 = 0 if p_next[1] == p_cur[1] else (1 if p_next[1] > p_cur[1] else -1)

        # Inward normals (interior is to the RIGHT of each directed edge in SVG coords)
        nx1, ny1 = -dy1, dx1
        nx2, ny2 = -dy2, dx2

        x = p_cur[0] * cell_mm + (nx1 + nx2) * inset_mm
        y = p_cur[1] * cell_mm + (ny1 + ny2) * inset_mm
        out.append((x, y))
    return out


def render_svg(
    ink: Sequence[Sequence[bool]],
    cell_mm: float,
    bleed_comp_mm: float,
    payload: str,
    mode: str,
) -> str:
    """Render seam-free merged vector SVG at exact physical mm."""
    n_cells = len(ink)
    size_mm = n_cells * cell_mm
    loops = trace_orthogonal_loops(ink)

    def fmt(v: float) -> str:
        s = f"{v:.4f}".rstrip("0").rstrip(".")
        return "0" if s in ("", "-0") else s

    path_parts: List[str] = []
    for loop in loops:
        pts = offset_orthogonal_loop_mm(loop, cell_mm, bleed_comp_mm)
        seg = [f"M{fmt(pts[0][0])},{fmt(pts[0][1])}"]
        for x, y in pts[1:]:
            seg.append(f"L{fmt(x)},{fmt(y)}")
        seg.append("Z")
        path_parts.append("".join(seg))

    d_attr = " ".join(path_parts)
    size_str = fmt(size_mm)
    return (
        f'<?xml version="1.0" encoding="UTF-8"?>\n'
        f'<!-- ADX 8854 DataMatrix ECC200 10x10 | payload="{payload}" | '
        f'mode={mode} | cell={fmt(cell_mm)}mm | bleed_comp={fmt(bleed_comp_mm)}mm | '
        f'size={size_str}x{size_str}mm -->\n'
        f'<svg xmlns="http://www.w3.org/2000/svg" '
        f'width="{size_str}mm" height="{size_str}mm" '
        f'viewBox="0 0 {size_str} {size_str}">\n'
        f'  <path fill="#000000" fill-rule="evenodd" d="{d_attr}"/>\n'
        f'</svg>\n'
    )


# ---------------------------------------------------------------------------
# Raster PNG Output (Requires Pillow)
# ---------------------------------------------------------------------------

def render_silk_png(
    ink: Sequence[Sequence[bool]],
    px_per_cell: int,
    bleed_comp_px: int,
    opaque_white_bg: bool = False,
) -> "Image.Image":
    """Render crisp, non-antialiased silkscreen PNG (black = silk ON)."""
    if not HAS_PIL:
        raise RuntimeError("Pillow (PIL) is required for PNG output. Install with: pip install pillow")

    n_cells = len(ink)
    size_px = n_cells * px_per_cell
    bg = (255, 255, 255, 255) if opaque_white_bg else (255, 255, 255, 0)
    fg = (0, 0, 0, 255)

    img = Image.new("RGBA", (size_px, size_px), bg)
    draw = ImageDraw.Draw(img)

    for r in range(n_cells):
        for c in range(n_cells):
            if not ink[r][c]:
                continue
            x0 = c * px_per_cell
            y0 = r * px_per_cell
            x1 = x0 + px_per_cell - 1
            y1 = y0 + px_per_cell - 1
            if bleed_comp_px > 0:
                if r == 0 or not ink[r - 1][c]:
                    y0 += bleed_comp_px
                if r == n_cells - 1 or not ink[r + 1][c]:
                    y1 -= bleed_comp_px
                if c == 0 or not ink[r][c - 1]:
                    x0 += bleed_comp_px
                if c == n_cells - 1 or not ink[r][c + 1]:
                    x1 -= bleed_comp_px
            if x1 >= x0 and y1 >= y0:
                draw.rectangle([x0, y0, x1, y1], fill=fg)

    return img


def render_preview_png(
    ink: Sequence[Sequence[bool]],
    cell_mm: float,
    reserved_mm: float = 6.0,
    px_per_mm: int = 50,
) -> "Image.Image":
    """Render 3-panel PCB preview (Green / Black / White soldermask) inside 6x6 mm slot."""
    if not HAS_PIL:
        raise RuntimeError("Pillow (PIL) is required for preview PNG output.")

    panels = [
        ("Green PCB", (23, 92, 53, 255), (248, 250, 252, 255)),
        ("Black PCB", (28, 31, 36, 255), (248, 250, 252, 255)),
        ("White PCB", (241, 245, 242, 255), (17, 24, 39, 255)),
    ]
    n_cells = len(ink)
    body_mm = n_cells * cell_mm
    box_mm = max(reserved_mm, body_mm + 0.4)
    box_px = int(round(box_mm * px_per_mm))
    pad = 24
    header_h = 28

    total_w = len(panels) * box_px + (len(panels) + 1) * pad
    total_h = box_px + header_h + pad * 2
    canvas = Image.new("RGBA", (total_w, total_h), (30, 34, 40, 255))
    draw = ImageDraw.Draw(canvas)

    offset_mm = (box_mm - body_mm) / 2.0

    for idx, (label, mask_rgba, silk_rgba) in enumerate(panels):
        ox = pad + idx * (box_px + pad)
        oy = pad + header_h
        draw.text((ox, pad + 4), label, fill=(226, 232, 240, 255))
        draw.rectangle([ox, oy, ox + box_px - 1, oy + box_px - 1], fill=mask_rgba, outline=(234, 179, 8, 255))

        for r in range(n_cells):
            for c in range(n_cells):
                if not ink[r][c]:
                    continue
                x0 = ox + int(round((offset_mm + c * cell_mm) * px_per_mm))
                y0 = oy + int(round((offset_mm + r * cell_mm) * px_per_mm))
                x1 = ox + int(round((offset_mm + (c + 1) * cell_mm) * px_per_mm)) - 1
                y1 = oy + int(round((offset_mm + (r + 1) * cell_mm) * px_per_mm)) - 1
                draw.rectangle([x0, y0, x1, y1], fill=silk_rgba)

    return canvas


# ---------------------------------------------------------------------------
# CLI Entry Point
# ---------------------------------------------------------------------------

def format_ascii_matrix(grid: Sequence[Sequence[int]]) -> str:
    return "\n".join("".join("██" if cell else "  " for cell in row) for row in grid)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Generate ADX 8854 10x10 ECC200 DataMatrix silkscreen assets (SVG / PNG).",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "board_id",
        help="6-digit ADX Board ID (e.g. 000000, 100010). Use --raw for arbitrary <=3 codeword ASCII.",
    )
    parser.add_argument(
        "-o",
        "--out-dir",
        type=Path,
        default=Path("."),
        help="Output directory for generated files.",
    )
    parser.add_argument(
        "--prefix",
        default="adx_dm",
        help="Filename prefix.",
    )
    parser.add_argument(
        "--mode",
        choices=("silk", "negative-box"),
        default="silk",
        help=(
            "'silk' (ADX standard): modules=ink ON, quiet zone=bare soldermask; "
            "'negative-box' (experimental): solid silk quiet zone + mask cutouts."
        ),
    )
    parser.add_argument(
        "--cell-mm",
        type=float,
        default=0.40,
        help="Module pitch in mm (0.40 -> 4.0mm body; 0.45 -> 4.5mm body).",
    )
    parser.add_argument(
        "--quiet-cells",
        type=int,
        default=None,
        help="Quiet zone border in integer cells included in file (default: 0 for silk, 2 for negative-box).",
    )
    parser.add_argument(
        "--bleed-comp-mm",
        type=float,
        default=0.0,
        help="Optional silkscreen ink-spread inset (mm) applied only to exposed outer edges of ink regions.",
    )
    parser.add_argument(
        "--px-per-cell",
        type=int,
        default=40,
        help="PNG resolution in pixels per module (40 -> 400x400 px for 10x10).",
    )
    parser.add_argument(
        "--white-bg",
        action="store_true",
        help="Use opaque white background in PNG instead of transparent background.",
    )
    parser.add_argument(
        "--no-png",
        action="store_true",
        help="Skip PNG generation (SVG only; requires no Pillow).",
    )
    parser.add_argument(
        "--preview",
        action="store_true",
        help="Also output a Green/Black/White PCB visual preview PNG.",
    )
    parser.add_argument(
        "--raw",
        action="store_true",
        help="Allow arbitrary ASCII payload fitting in 3 codewords instead of strictly 6 digits.",
    )

    args = parser.parse_args(argv)

    payload: str = args.board_id
    if not args.raw and not (len(payload) == 6 and payload.isdigit()):
        parser.error(
            f"board_id must be a 6-digit number (e.g. 000000, 100010). "
            f"Got {payload!r}. Pass --raw to encode arbitrary short ASCII."
        )

    if args.cell_mm <= 0:
        parser.error("--cell-mm must be positive.")
    if args.bleed_comp_mm < 0 or args.bleed_comp_mm >= args.cell_mm * 0.45:
        parser.error(f"--bleed-comp-mm must be in [0, {args.cell_mm * 0.45:.3f}).")

    quiet_cells = args.quiet_cells
    if quiet_cells is None:
        quiet_cells = 0 if args.mode == "silk" else 2
    if quiet_cells < 0:
        parser.error("--quiet-cells must be >= 0.")

    # Encode + round-trip self-verify
    dm_grid, codewords = encode_datamatrix_10x10(payload)
    decoded, decoded_cws, rs_ok = decode_datamatrix_10x10(dm_grid)
    if decoded != payload or decoded_cws != codewords or not rs_ok:
        raise RuntimeError(
            f"Self-verification failed: expected {payload!r}, decoded {decoded!r}, rs_ok={rs_ok}"
        )

    ink_grid = build_ink_grid(dm_grid, args.mode, quiet_cells)
    total_mm = len(ink_grid) * args.cell_mm

    args.out_dir.mkdir(parents=True, exist_ok=True)
    mode_tag = "" if args.mode == "silk" else f"_{args.mode}"
    bleed_tag = "" if args.bleed_comp_mm == 0 else f"_bc{args.bleed_comp_mm:.2f}"
    stem = f"{args.prefix}_{payload}_{total_mm:.1f}mm{mode_tag}{bleed_tag}"

    # 1. Write SVG
    svg_path = args.out_dir / f"{stem}.svg"
    svg_text = render_svg(ink_grid, args.cell_mm, args.bleed_comp_mm, payload, args.mode)
    svg_path.write_text(svg_text, encoding="utf-8")

    written = [svg_path]

    # 2. Write PNG (unless --no-png)
    if not args.no_png:
        if not HAS_PIL:
            print("Warning: Pillow not installed; skipping PNG output (SVG written).", file=sys.stderr)
        else:
            bleed_px = int(round(args.bleed_comp_mm / args.cell_mm * args.px_per_cell))
            png_img = render_silk_png(ink_grid, args.px_per_cell, bleed_px, opaque_white_bg=args.white_bg)
            png_path = args.out_dir / f"{stem}.png"
            png_img.save(png_path, format="PNG")
            written.append(png_path)

            if args.preview:
                prev_img = render_preview_png(ink_grid, args.cell_mm)
                prev_path = args.out_dir / f"{stem}_preview.png"
                prev_img.save(prev_path, format="PNG")
                written.append(prev_path)

    # Summary output
    print(f"ADX DataMatrix ECC200 10x10 — SELF-VERIFY OK")
    print(f"  Payload      : {payload!r}")
    print(f"  Codewords    : data={codewords[:3]}  ecc={codewords[3:]}  (RS syndrome=0)")
    print(f"  Mode         : {args.mode} (quiet_cells={quiet_cells}, bleed_comp={args.bleed_comp_mm:.2f} mm)")
    print(f"  Physical size: {total_mm:.2f} x {total_mm:.2f} mm  (cell={args.cell_mm:.2f} mm)")
    print()
    print(format_ascii_matrix(dm_grid))
    print()
    print("Generated files:")
    for p in written:
        print(f"  - {p}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
