<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# 8748 Form Factor Specification

[ **English** | [日本語 (../ja/8748_formfactor.md)](../ja/8748_formfactor.md) ]

## Overview
The "8748 Form Factor" is a compact board standard designed to optimize dimensional alignment between PCB CAD (mil-based) and mechanical enclosure CAD (mm-based).
By minimizing unit conversion rounding errors and accounting for manufacturing tolerances, it ensures reliable mechanical fit and clearance across enclosures, 3D prints, and mounting hardware.

---

## 1. Mechanical Specification
Absolute dimensions for mechanical designers (3D CAD, CNC machining, sheet metal fabrication, 3D printing). Dimensions are defined in clean metric values.

* **Board Outline:** `87.0 mm × 48.0 mm`
* **Mounting Hole Pitch:** `77.0 mm × 38.0 mm`
* **Hole Margin:** `5.0 mm` from board edges
* **Clearance Hole Diameter:** `Φ3.3 mm` (compatible with standard M3 screws)
* **Corner Treatment:** C3 Chamfer
* **Interface Usable Length:** approx. `70.0 mm` (usable span for 5.08 mm pitch terminal blocks)

> **Mechanical Design Guidelines:**
> Model mounting standoffs at `77.0 mm × 38.0 mm`. Nominal dimensions ensure sufficient clearance against typical 3D-printing tolerances and PCB manufacturing variations.

---

## 2. Electrical Specification
Absolute dimensions for PCB designers (KiCad, Altium, Eagle, EasyEDA Pro, etc.). Defined on standard mil coordinates for clean grid snapping.

* **Board Outline (Edge.Cuts):** `3425 mil × 1890 mil`
* **Hole Margin:** `197 mil` from board edges
* **Mounting Hole Pitch:** `3031 mil × 1496 mil`
* **Drill Diameter:** `130 mil` (approx. 3.30 mm)
* **Corner Chamfer:** C3 (`120 mil`)
* **Recommended Component Grid:** `100 mil` or `50 mil`

> **PCB Design Guidelines:**
> Set the CAD grid to `1 mil` or `5 mil` to place the board outline and mounting holes using the exact mil coordinates above. After defining the board geometry, switch to a `100 mil` or `50 mil` grid for component placement and routing.

---

## 3. Unit Conversion & Precision Tolerances
The 8748 standard maintains a theoretical rounding error of **less than 15 microns** between mil and metric dimensions. This is negligible compared to typical PCB fabrication tolerances (±0.1–0.2 mm) and M3 hole clearance (approx. 0.3 mm), ensuring seamless mechanical assembly.

| Dimension | Metric Nominal | Mil Coordinate Value | Theoretical Error | Remarks |
| :--- | :--- | :--- | :--- | :--- |
| **X-axis Outline** | 87.0 mm | 3425 mil (86.995 mm) | `-0.005 mm` (-5 μm) | Slight negative tolerance (improves fit) |
| **Y-axis Outline** | 48.0 mm | 1890 mil (48.006 mm) | `+0.006 mm` (+6 μm) | Negligible error |
| **X-axis Pitch** | 77.0 mm | 3031 mil (76.987 mm) | `-0.013 mm` (-13 μm) | Fully absorbed within M3 clearance hole |
| **Y-axis Pitch** | 38.0 mm | 1496 mil (37.998 mm) | `-0.002 mm` (-2 μm) | Near-exact match |
