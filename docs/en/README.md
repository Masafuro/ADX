<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English** | [日本語 (../ja/README.md)](../ja/README.md) ]

<p align="center">
  <img src="../images/hero_no_photo.svg" alt="ADX: The Working Arduino" width="100%">
</p>

### *The Working Arduino*
> **A tougher, enclosure-native board engineered to run your Arduino sketches reliably in the real world.**

**ADX** is an open-hardware modular ecosystem designed for projects that need to step off the workbench and run continuously inside real enclosures—neatly mounted, cleanly wired, and shielded with field-ready protections.

---

## Quick Navigation by Goal

Find what you need based on what you want to achieve:

| What do you want to do? | Key Documentation | Primary Links |
| :--- | :--- | :--- |
| 🚀 **Build with USB & UF2** | Next-generation USB Core board with CH32X035 | [CORE-CX5 Concept](../../hardware/CORE-CX5/concept.md) |
| 🛡 **Deploy tough RS-485 nodes** | High-temp (-40℃ ~ +105℃) isolated Core board | [CORE-A5 Specification](../../hardware/CORE-A5/spec_study.md) |
| 🔌 **Build custom addon CARDs** | Universal 20-pin bus pinout & layout rules | [ADX 20-Pin Bus (SSOT)](../spec/ADX_PINOUT.md) (v4.1) |
| 📖 **Learn pinout engineering rationale**| Panel-aligned topology & coplanar shielding details | [ADX Pinout Architecture Guide](ADX_pinout.md) |
| 📦 **Design or 3D-print cases** | Dimensions, M3 hole spacing & rail margins | [8748 Form Factor Specification](8748_formfactor.md) |

---

## 1. Overview: From Workbench to Workplace

Prototyping on a workbench with a breadboard is an incredible, creative way to bring ideas to life.

However, when it is time to deploy those sketches permanently—inside a factory control cabinet, greenhouse watering rig, outdoor lighting fixture, or custom architectural enclosure—makers and engineers face identical physical friction:

1. **Awkward Mounting**: Mounting holes are too close to delicate traces, making tightening M3 screws feel risky.
2. **Hard to Enclose**: Connectors and cables protrude in all four directions, forcing complex multi-sided case cutouts.
3. **Loose Wiring**: Standard jumper wires loosen under minor vibration, ambient temperature swings, or accidental tugs.
4. **Maintenance Friction**: Dragging a heavy laptop up a ladder or into a crawl space just to tweak a few lines of code is exhausting.

**ADX solves these hurdles with a grounded philosophy: "The Working Arduino."**  
Not a high-cost, proprietary industrial PLC, but an accessible, open-hardware companion that makes taking Arduino sketches into real-world installations natural, safe, and robust.

---

## 2. The Foundation: Open Hardware Standards

The ADX ecosystem is built upon two open, modular physical standards:

```text
       ════════════════════════════════════════════════════════════════════
                         【 ADX ECOSYSTEM ARCHITECTURE 】
         ・8748 Form Factor (87.0 mm × 48.0 mm standardized profile)
         ・Corner M3 Mounting Holes with Φ5.0 mm Washer Clearances
         ・Unidirectional I/F Edge (All field wiring on a single face)
         ・ADX Pinout v4.1 (Panel-Aligned Power, Edge Guard, Native 3.3V)
       ════════════════════════════════════════════════════════════════════
                                       │
            ┌──────────────────────────┴──────────────────────────┐
            ▼                                                     ▼
   【 Main Core Boards 】                                【 Modular CARDs 】
     ・ADX CORE-CX5 (USB Flagship / CH32X035 / UF2)        ADX CARD Ecosystem
     ・ADX CORE-A5  (Tough Isolated RS-485 / -40℃~+105℃)   ・Prototyping CARD
     ・Dual Type-C or Isolated Push-in Terminals           ・Power Supply CARDs
     ・Automated 20P IDC Fixture Inspection & Flashing     ・3D-Printable Enclosures
```

### 2.1 8748 Form Factor & Mechanical Mounting
- **Dimensions**: Standardized `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`).
- **Clearance for Real Screws**: Four-corner M3 mounting holes (`77.0 mm × 38.0 mm` pitch) with generous **Φ5.0 mm washer clearances**, ensuring screws and lock washers never bite into traces.
- **Enclosure Grab Margins**: Dedicated keep-out zones along both long edges support **slot-in rail mounting** and **3D-printed snap-fit tabs**.
- *Reference*: [8748 Form Factor Specification](8748_formfactor.md)

### 2.2 Unidirectional Interface Edge
External field connections (power and communications) are strictly aligned along **one single edge of the PCB**.  
By eliminating multi-directional wire routing, enclosure cutouts and front-panel machining are simplified to a single opening.

### 2.3 Universal 20-Pin Bus (ADX-BUS-SPEC-v4.1)
All Core boards and Card modules interconnect via a standardized 2×10-pin shrouded IDC connector:
- **Panel-Aligned Power Bus (`VBUS`: Pins 17–20)**:  
  Directly matches industrial wiring practices (weak signals on top, heavy power on bottom), routing straight into front terminals with zero trace crossings. Secondarily guards against loose wiring contact. Continuous $2 \times 2$ grid rated for **2.0A–3.0A continuous**.
- **Protective Boundary Wall (`Pin 16: GND`)**:  
  Direct short barrier directly above VBUS, safely grounding crimp misalignments.
- **Outer Edge Guard & Coplanar Analog (`Pins 1–4`)**:  
  Pin 1 (GND) protects cable perimeter; Pin 3 (AIN) is shielded between Pins 1 and 4 from high-speed digital switching.
- **System Programming & Debug (`Pins 2 & 5`)**:  
  Unified 1-wire (UPDI) and 2-wire (SDI) automated flashing via the 20-pin IDC fixture with Card-side DNP isolation rules.
- **Native 3.3V Logic**: All digital pins (UART, I2C, SPI) operate at **3.3V LVCMOS**.
- *Reference (SSOT)*: [ADX 20-Pin Bus Specification](../spec/ADX_PINOUT.md) | [Architecture Guide](ADX_pinout.md)

---

## 3. Core Board Portfolio

Choose the right Core board based on your operating environment and connectivity needs:

1. **[ADX CORE-CX5](../../hardware/CORE-CX5/concept.md) (Standard USB Core)**:
   - **MCU**: WCH CH32X035 (32-bit RISC-V 48MHz, Flash 62KB, SRAM 20KB)
   - **Highlights**: Internal USB PHY, Dual Type-C ports (Communication 16P + Power 6P), PC reverse protection, cross-platform UF2 drag-and-drop sketch updating.
2. **[ADX CORE-A5](../../hardware/CORE-A5/spec_study.md) (High-Reliability Edge Slave)**:
   - **MCU**: Microchip ATtiny1616-MNR (8-bit 10MHz, negligible self-heating)
   - **Highlights**: Industrial -40℃ to +105℃ rated components, 2500V galvanic isolated RS-485, push-in spring clamp terminals, CCL autonomous break-reset.
