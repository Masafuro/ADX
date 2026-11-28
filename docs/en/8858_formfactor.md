<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# 8858 Form Factor Specification

[ **English** | [日本語](../ja/8858_formfactor.md) ]

- **Specification Status**: **Official Alpha (`v0.1-alpha`)**
- **Revision Date**: 2026-11-28
- **Reference Implementation**: [ADX Reference CARD v1 Specification](../../hardware/CARD/Reference/concept.md) (`v5.0` / `C9039` spec finalized, CAD update pending) / Baseline [`Gerber_PCB1_2026-11-23.zip`](../../hardware/CARD/Reference/data/Gerber_PCB1_2026-11-23.zip) (`v0` mechanical & marker verified)
- **Design Rationale**: [8858 Form Factor Study (JA)](../../hardware/CARD/Reference/8858ff_plan.md)

---

## Overview

The **8858 Form Factor** is the official alpha standard physical profile for the ADX modular hardware ecosystem. It unifies microcontrollers (ATtiny / ESP32 / RP2040), compact Linux compute modules (Raspberry Pi CM0 / Pi Zero, and CM4 / CM5 via rear-side vertical rack mounting), and industrial expansion cards (isolated power, isolated RS-485/CAN, motor drivers, 24V I/O, audio) under a single mechanical footprint.

Evolving from the legacy `8854` (`88.0 × 54.0 mm`) and `8748` (`87.0 × 48.0 mm`) profiles, the **`88.0 × 58.0 mm`** footprint (aspect ratio **`1.52 : 1`**, matching bridge-size playing cards and Raspberry Pi Model B) hits the sweet spot across four requirements:

1. **Sub-12 μm metric/mil CAD alignment** between PCB layout (mil) and mechanical CAD (mm).
2. **Near-square 200 mm-class PCBA panelization** (`2 × 3 = 6 pcs/panel`, **`186.0 × 184.0 mm`** with 4-side 5 mm rails, preserving cognitive batch ergonomics under Miller's Law).
3. **Generous usable area for MCU, PRO isolated cards, CM0, Pi Zero, and rear-mounted CM4/CM5**, combined with a standardized **silkscreen optical alignment & Board ID marker specification**.
4. **Realistic enclosure fit including rear 20-pin IDC ribbon cable clearance** for PLC-style vertical slide-in racks, 3D-printed enclosures (~100 mm height × 70 mm depth class), and off-the-shelf `W=70 mm` enclosures.

---

## 1. Key Advancements from the 8854 Standard

| Item | Legacy 8854 | **8858 (Official Alpha)** | Rationale & Benefits |
| :--- | :--- | :--- | :--- |
| **Board Outline** | `88.0 × 54.0 mm` | **`88.0 × 58.0 mm`** | Preserves the entire X-axis (`88 mm` width / `66 mm` I/O Edge) while extending Y by `+4.0 mm`. Keeps a crisp `1.52 : 1` card proportion. |
| **Mounting Pitch** | `76.0 × 42.0 mm` | **`76.0 × 46.0 mm`** | Maintains `6.0 mm` corner margins and `Φ10.0 mm` washer keep-outs; Y-pitch mil error improves to **`-0.6 μm`**. |
| **20P IDC Center** | `(44.0, 47.0) mm` | **`(44.0, 51.0) mm`** | Preserves `7.0 mm` top-edge offset; leaves **`46.5 mm`** below standard vertical box header body. |
| **Bottom SMD-Only Zone** | `8.0 × 32.0 mm × 2` | **`8.0 × 36.0 mm × 2`** | Expands clear vertical span between corner M3 keep-outs to **`36.0 mm`**, giving Pi Zero (`30 mm`) and optical markers generous clearance. |
| **Panelization (2×3)** | `186 × 162 mm` (2-side)<br>`186 × 172 mm` (4-side) | **`186 × 184 mm` (4-side 5 mm rails)** | Near-1:1 square (`1.01 : 1`) maximizes reflow rigidity and stencil printing stability while maintaining 6 pcs/panel. |
| **Optical Markers** | Draft proposal | **Integrated `M1–M4` (`76.0 × 30.0 mm`)** | Aligns marker X coordinates (`6.0 / 82.0 mm`) with M3 mounting holes; includes chiral Top/Bottom auto-detection. |
| **Modules & Enclosures** | Tight front I/O for CM0 | **CM0 front USB-C / CM4 vertical-rack rear mount / W=70 case + IDC clearance** | Fits PLC vertical racks (`~100 × 70 mm`) and off-the-shelf W=70 cases (e.g., Takachi `MB7-5-10`) with ~6 mm rear IDC cable escape. |

---

## 2. Mechanical Specification

Dimensional definitions for mechanical designers (3D CAD, PLC vertical racks, sheet metal, CNC, 3D printing):

* **Board Outline:** `88.0 mm × 58.0 mm` (Area `51.0 cm²`, aspect ratio `1.52 : 1`)
* **Corner Treatment:** `C0` (Sharp right angles, no chamfer; continuous straight V-scoring ready)
* **Origin:** Lower-left corner `(0.0, 0.0) mm`
* **Board Center:** `(44.0, 29.0) mm`
* **Mounting Hole Pitch:** `76.0 mm × 46.0 mm`
* **Mounting Hole Edge Margin:** `6.0 mm` from all four corner edges
* **Clearance Hole Diameter:** `Φ3.30 mm` (NPTH, standard M3 clearance)
* **Mounting Keep-out (Washer Clearance):** Radius `5.0 mm` (`Φ10.0 mm`, leaving `1.0 mm` solid PCB margin to edges)
* **I/O Edge Span:** `66.0 mm` (centered on bottom edge, `X = 11.0 mm ~ 77.0 mm`)
* **Card Guide Keep-out:** `3.0 mm` strip along left and right short edges
* **Bottom SMD-Only Zone:** Left `X = 3.0 ~ 11.0 mm` and right `X = 77.0 ~ 85.0 mm`, `Y = 11.0 ~ 47.0 mm` (`8.0 mm × 36.0 mm × 2`; no THT lead protrusions permitted)
  * Excluding the four optional `6.0 × 6.0 mm` corner Marker Zones leaves **`24.0 × 8.0 mm × 2`** of central SMD-only area.
* **Top Usable Area (below 20P IDC):** `66.0 mm × 44.0 mm` (excluding left/right 11 mm mounting zones and top 14.0 mm IDC band; account separately for bottom I/O connector depth)
* **Outline Trace Clearance:** `20 mil` (approx. `0.508 mm`) along top and bottom long edges

> **Mechanical Design Guidelines (PLC Vertical Rack & Commercial Enclosures):**
> * Model mounting bosses at `76.0 mm × 46.0 mm`.
> * In PLC-style vertical slide-in racks, orienting the `88 mm` edge vertically (~100 mm rack height) and the `58 mm` edge as depth (~70 mm depth including 6–8 mm rear IDC ribbon clearance) enables natural chimney-effect cooling (suitable for CM0, rear-mounted CM4/CM5 heatsinks, and isolated power cards) while standardizing front `66 mm` I/O panels.
> * For commercial off-the-shelf cases, target the **W=70 mm class**—such as Takachi `MB7-5-10` (internal `65.2 × 97.4 × H48.0 mm`, allowing ~6.2 mm rear IDC cable escape and 2-board stacking), `YM10-3-7` (single board), or `SW-125` (single board + battery/power bay). (`SW-100` has an effective inner width of `57.8 mm` and is not recommended for 8858.)

---

## 3. Electrical Specification

Dimensional definitions for PCB designers (KiCad, Altium, EasyEDA Pro, etc.):

* **Board Outline (Edge.Cuts):** `3465 mil × 2283 mil` (`88.0 mm × 58.0 mm`)
* **Origin:** Lower-left corner `(0, 0) mil` / `(0.0, 0.0) mm`
* **Mounting Hole Centers:**
  * Lower-left: `[236, 236] mil` (`(6.0, 6.0) mm`)
  * Upper-left: `[236, 2047] mil` (`(6.0, 52.0) mm`)
  * Lower-right: `[3228, 236] mil` (`(82.0, 6.0) mm`)
  * Upper-right: `[3228, 2047] mil` (`(82.0, 52.0) mm`)
* **Mounting Hole Pitch:** `2992 mil × 1811 mil` (`76.0 mm × 46.0 mm`)
* **Drill Diameters:**
  * Mounting Holes (NPTH): `130 mil` (`3.30 mm`)
  * 20-Pin Bus (PTH): `43 mil` (`1.10 mm`)
* **20-Pin IDC Header Center:** **`(44.0, 51.0) mm`** / `[1732, 2008] mil` (centered on long edge, `7.0 mm` from top edge; compatible with both SMD and THT vertical box headers; standard shroud lower edge `Y ≈ 46.5 mm`)
* **Long-Edge Copper Clearance:** `≥ 20 mil` (`0.508 mm`) from top and bottom edges
* **Short-Edge Card Guide Copper Clearance:** `≥ 3.0 mm` from left and right edges (`3.1 mm` recommended)
* **Recommended Grid:** `100 mil` or `50 mil`

---

## 4. 20-Pin IDC Expansion Header Interface

The 8858 Form Factor integrates the standardized 2×10-pin 2.54 mm vertical box header in full compliance with the [ADX 20-Pin Bus Specification (SSOT)](../spec/ADX_PINOUT.md).

### 4.1 Header Pinout (Top View / `ADX-BUS-SPEC-v5.0`)
```text
[Upper Row (Even Pins / AIN side: Pins 2, 4, ..., 20)]
P01/AIN  | P02/EXTCLK | P03/DE   | P05/RX   | P07/SDA  | P08/CS   | P10/MISO | GND      | VBUS | VBUS

[Lower Row (Odd Pins / Pin 1 & GND side: Pins 1, 3, ..., 19)]
GND      | GND        | GND      | P04/TX   | P06/SCL  | GND      | P09/MOSI | P11/SCK  | VBUS | VBUS
```

### 4.2 Power Requirements (VBUS)
* **Bus Operating Voltage Tolerance (All Boards [MUST]):** **`4.5 V ~ 5.5 V DC`** (nominal `5.0 V ±10%`, covering both USB Type-C `vSafe5V` up to `5.50 V` and 2 A cable/connector IR drops; every Core and CARD must operate safely across this full range)
* **Power Sourcing Recommended Accuracy [SHOULD]:** `4.75 V ~ 5.25 V DC` (`5.0 V ±5%`, strictly within `4.5 V ~ 5.5 V`; upstream OVLO must not nuisance-trip at steady-state `5.5 V`)
* **Mandatory Protection (Sourcing Modules [MUST]):**
  * Overcurrent Protection (approx. `2 A`)
  * Short-Circuit Protection
  * Reverse Current Blocking (Schottky diode, ideal diode, or eFuse)

---

## 5. Optical Alignment & Board ID Marker Specification (Optional Marker Zone)

To enable low-cost, open-source automated post-PCBA inspection (vision, LED checks, bed-of-nails probing) across OSP finishes, any soldermask color, and commodity USB cameras, 8858 defines a two-layer silkscreen optical marker specification.

### 5.1 Layer 1: Spatial Reservation — Optional Marker Zone (`[MAY]`)

Four `6.0 × 6.0 mm` zones are reserved at the upper and lower corners of the left/right `SMD-Only Zones`, **sharing the exact X coordinates (`6.0 / 82.0 mm`) of the M3 mounting holes**:

| Area ID | Center `(X, Y)` | Mil Coordinates | Reserved Box (`6.0 × 6.0 mm`) | Official Profile Assignment |
| :---: | :---: | :---: | :---: | :--- |
| **`M1` (Top-Left)** | `(6.0, 44.0) mm` | `[236, 1732] mil` | `X = 3.0~9.0, Y = 41.0~47.0` | Bullseye (`◎`) |
| **`M2` (Top-Right)** | `(82.0, 44.0) mm` | `[3228, 1732] mil` | `X = 79.0~85.0, Y = 41.0~47.0` | Bullseye (`◎`) |
| **`M3` (Bottom-Left)** | `(6.0, 14.0) mm` | `[236, 551] mil` | `X = 3.0~9.0, Y = 11.0~17.0` | Bullseye (`◎`) |
| **`M4` (Bottom-Right)** | `(82.0, 14.0) mm` | `[3228, 551] mil` | `X = 79.0~85.0, Y = 11.0~17.0` | DataMatrix ECC200 `10×10` (inverted silk) |

* **Symmetric Integer Grid:** Centered at `±(38.0, 15.0) mm` from board center `(44.0, 29.0) mm` (pitch **`76.0 × 30.0 mm`**). Aligning with the M3 hole X pitch (`76.0 mm`) provides **`3.0 mm`** clearance from the central 66 mm component area while retaining **`1.00 mm`** (2.5 cells) to the card guide (`3.0 / 85.0 mm`) and M3 keep-outs (`R = 5.0 mm`) and **`~0.90 mm`** (2.24 cells) of uniform copper pour quiet zone.
* **Optionality:** DIY and dense cards may omit markers (`[MAY]`). Underlying copper/routing is unrestricted (uniform GND pour without vias is recommended under Official Profile markers).

### 5.2 Layer 2: Official Optical Profile (`[SHOULD]`)

1. **Alignment Bullseyes (`M1`–`M3`, `Φ4.0 mm` body + `1.0 mm` margin):**
   * Center filled dot: **`Φ1.20 mm`** (`R = 0.60 mm`)
   * Dark resist gap ring: width **`0.90 mm`** (`Φ1.20 ~ 3.00 mm`)
   * Outer white ring: OD **`Φ4.00 mm`** / ID **`Φ3.00 mm`** (CAD centerline `Φ3.50 mm`, stroke width `0.50 mm`; white:dark area ratio ≈ `53% : 47%`). Native CAD primitives recommended.
2. **Board ID DataMatrix (`M4`, `4.0 × 4.0 mm` body + `1.0 mm` quiet zone):**
   * Symbol: **DataMatrix ECC200 / `10 × 10` cells** (numeric-compressed 6-digit Board ID), cell size **`0.40 × 0.40 mm`**.
   * Polarity & Orientation: Light-on-dark inverted silkscreen on dark resist (standard dark-on-light on white resist); solid L-finder along **Left & Bottom** edges.
   * Generator: [`software/tools/gen_adx_datamatrix.py`](../../software/tools/gen_adx_datamatrix.py)
3. **Bottom-Side Placement & Chiral Side Auto-Detection:**
   * Place Bottom markers on `BottomSilkscreen` at the **exact same CAD coordinates (`M1`–`M3` bullseyes, `M4` DataMatrix)** without manual mirroring.
   * When physically flipped and viewed by a camera, the DataMatrix appears at **Bottom-Left**, and the scalene right triangle (`76.0 × 30.0 mm`) flips handedness from **Clockwise (CW) on Top** to **Counter-Clockwise (CCW) on Bottom**—uniquely resolving side, 0°–360° rotation, and affine transform from the 3 bullseyes alone prior to barcode decoding.
   * Placing Bottom DataMatrix at `M3` or `M2` is **prohibited**, as it becomes congruent with Top orientation and breaks chiral side detection.

---

## 6. Unit Precision & Theoretical Tolerances

Theoretical rounding error between metric and mil definitions remains within **12 microns**, fully absorbed by PCB fabrication tolerances (±0.1–0.2 mm) and M3 clearance holes (~0.3 mm).

| Measurement | Metric Nominal | Mil Equivalent | Converted Metric | Theoretical Error | Remarks |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **X-axis Outline** | 88.0 mm | 3465 mil | 88.011 mm | `+0.011 mm` (+11 μm) | Negligible in fabrication |
| **Y-axis Outline** | 58.0 mm | 2283 mil | 57.988 mm | `-0.012 mm` (-12 μm) | Slight negative tolerance (fit-friendly) |
| **X-axis Pitch (M3 / Marker)** | 76.0 mm | 2992 mil | 75.997 mm | `-0.003 mm` (-3 μm) | Absorbed in M3 clearance |
| **Y-axis Pitch (M3)** | 46.0 mm | 1811 mil | 45.999 mm | `-0.001 mm` (-0.6 μm) | Near-exact match |
| **Hole Edge Margin** | 6.0 mm | 236 mil | 5.994 mm | `-0.006 mm` (-6 μm) | Measured from edges |
| **IDC Center Y** | 51.0 mm | 2008 mil | 51.003 mm | `+0.003 mm` (+3 μm) | Compatible with SMD & THT headers |
| **Y-axis Pitch (Marker)** | 30.0 mm | 1181 mil | 29.997 mm | `-0.003 mm` (-3 μm) | Chiral triangle short leg |

---

## 7. Manufacturing & DFM Panelization Guidelines

Standard panelization configuration for production orders (e.g., JLCPCB):

* **Standard Panel Configuration (200 mm Safe Envelope / Recommended):**
  * **Panel Matrix:** 2 columns × 3 rows (**6 pcs per panel**, within Miller's Law Magical Number 7)
  * **Edge Rails:** `5.0 mm` on all 4 sides (or `5.0 mm` on left/right 2 sides)
  * **Panel Overall Dimensions:** **`186.0 mm × 184.0 mm`** (aspect ratio `1.01 : 1` near-square; `186.0 × 174.0 mm` with 2-side rails)
  * **Separation Method:** Continuous straight V-Scoring (V-cut)
  * **Fiducials:** 2 to 4 panel rail fiducials (`Φ1.0 mm` pad, `Φ2.0 mm` mask opening)
  * **Tooling Holes:** `Φ1.152 mm` or `Φ2.0 mm` locating holes at panel corners
* **High-Volume Extension (250 mm Envelope / 1.6 mm FR4):**
  * **Panel Matrix:** 2 columns × 4 rows (**8 pcs per panel**)
  * **Panel Overall Dimensions:** `186.0 mm × 242.0 mm` (4-side 5 mm rails) / `186.0 mm × 232.0 mm` (2-side rails)
