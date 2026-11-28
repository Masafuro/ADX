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
| 🚀 **Build with the Flagship Core** | 3-Mode (`-40°C~+85°C` target) USB & Field Core (`ATtiny1616` + `CH342K`) | [CORE-U (rev6) Specification](../../hardware/CORE-U/rev6/spec.md) |
| 🔌 **Build custom addon CARDs** | Universal 20-pin bus pinout (`v5.0`) & RS-485 / power layout rules | [ADX 20-Pin Bus (SSOT: v5.0)](../spec/ADX_PINOUT.md) |
| 📖 **Learn pinout engineering rationale**| Panel-aligned topology, dual-GND shielding & `UART_DE` details | [ADX Pinout Architecture Guide](ADX_pinout.md) |
| 📦 **Design or 3D-print cases** | Dimensions, M3 hole spacing & rail margins | [8858 Form Factor Specification (Alpha)](8858_formfactor.md) |

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
         ・8858 Form Factor (88.0 mm × 58.0 mm profile / Official Alpha, straight V-cut)
         ・Corner M3 Mounting Holes (76.0 mm × 46.0 mm pitch) with Φ10.0 mm Clearances
         ・Unidirectional I/O Edge (66.0 mm span on bottom edge)
         ・ADX Pinout v5.0 (P01–P11 Abstraction, Dedicated UART_DE, Bottom VBUS, 3.3V)
       ════════════════════════════════════════════════════════════════════
                                       │
            ┌──────────────────────────┴──────────────────────────┐
            ▼                                                     ▼
   【 Flagship Core Board 】                             【 Modular CARDs 】
     ・ADX CORE-U (rev6 Flagship / ATtiny1616 / 3-Mode)               ADX CARD Ecosystem
       - Full Embedded Mode: -40°C~+85°C target + Eject Header       ・Isolated RS-485 / LIN CARDs
       - Isolated Onboard Dual-CDC USB (UPDI + UART, 0mW when idle)  ・Isolated 24V / 48V Power CARDs
       - AI Observability (V/I/W Telemetry + 3-Mode Self-ID)         ・Reference / Proto CARDs & Enclosures
```

### 2.1 8858 Form Factor (Official Alpha) & Mechanical Mounting
- **Dimensions**: Standardized `88.0 mm × 58.0 mm` (`3465 mil × 2283 mil`, crisp `1.52 : 1` proportion) with near-square `186 × 184 mm` 2×3 straight V-cut panelization.
- **Clearance for Real Screws**: Four-corner M3 mounting holes (`76.0 mm × 46.0 mm` pitch, `6.0 mm` edge margin) with generous **Φ10.0 mm washer clearances**, ensuring screws and lock washers never bite into traces.
- **Enclosure Grab & Optical Inspection**: Dedicated `3.0 mm` card guide keep-outs, underside **SMD-only zones** (`36.0 × 8.0 mm × 2`), and chiral **optical alignment markers** (`76.0 × 30.0 mm` pitch) support PLC vertical slide-in racks, 3D-printed snap-fit tabs, and automated inspection.
- *Reference*: [8858 Form Factor Specification (Official Alpha)](8858_formfactor.md) *(Legacy: [8854](8854_formfactor.md) / [8748](8748_formfactor.md))*

### 2.2 Unidirectional Interface Edge
External field connections (power and communications) are strictly aligned along **one single edge of the PCB**.  
By eliminating multi-directional wire routing, enclosure cutouts and front-panel machining are simplified to a single opening.

### 2.3 Universal 20-Pin Bus (ADX-BUS-SPEC-v5.0)
All Core boards and Card modules interconnect via a standardized 2×10-pin shrouded/latched IDC connector:
- **MCU-Independent Logical Pin IDs (`P01`–`P11`)**:  
  Power rails (`GND`, `VBUS`) are unnumbered, and all 11 signal contacts follow a pure, unrestricted sequential ID series **`P01`–`P11`** (mapped to collision-safe `ADX_P01`–`ADX_P11` macros and aliases such as `ADX_AIN`, `ADX_DE`, `ADX_TX`, `ADX_CS` in `ADX_<BOARD>.h`).
- **Panel-Aligned Power Bus (`VBUS`: Pins 17–20)**:  
  Directly matches industrial wiring practices (weak signals on top, heavy power on bottom), routing straight into front terminals with zero trace crossings. Continuous $2 \times 2$ grid rated for **2.0A–3.0A continuous** bidirectional power.
- **Protective Boundary Wall (`Pin 16: GND`)**:  
  Direct short barrier directly above `VBUS`, safely grounding crimp misalignments.
- **Triple Odd-Row Ground Guard (`Pins 1, 3, 5: GND`)**:  
  `P01` (Pin 2: `AIN`) is shielded between Pins 1 and 3 (`GND`), and `P02` (Pin 4: `EXTCLK`) is shielded between Pins 3 and 5 (`GND`).
- **3-Wire Field UART & RS-485 Direction Control (`P03`–`P05` / Pins 6–8)**:  
  Dedicated **`P03` (`UART_DE` / `XDIR`)** on Pin 6 (with mandatory Card-side pull-down) enables collision-free RS-485/LIN stacking alongside I2C (`P06`–`P07`) and SPI (`P08`–`P11`).
- **Core-Side Programming Immunity**:  
  Programming/debug lines (`UPDI` / `SWD`) are omitted from the 20-pin ribbon cable (eliminating HV-noise/accidental-reset vectors) and handled internally by each Core board.
- **Native 3.3V Logic**: All 11 signal pins (`P01`–`P11`) operate at **3.3V LVCMOS**.
- *Reference (SSOT)*: [ADX 20-Pin Bus Specification (v5.0)](../spec/ADX_PINOUT.md) | [Architecture Guide](ADX_pinout.md)

---

## 3. Core Board Portfolio

1. **[ADX CORE-U (rev6)](../../hardware/CORE-U/rev6/spec.md) (ADX Flagship Core)**:
   - **MCU**: Microchip ATtiny1616-MNR (8-bit AVR 3.3V 10MHz, Flash 16KB, SRAM 2KB, `-40°C ~ +105°C` silicon)
   - **Highlights**: 3-Mode operation (`Full embedded` `-40°C ~ +85°C` design target with latched THT IDC Eject Header & `CH342K` 0mW isolation), Dual Type-C (`USB-POW` 5V/2A + `USB-COM` isolated Dual CDC UPDI/UART), `VBUS` V/I/W telemetry, `PB5` 3-Mode self-ID & deep-sleep wake-up, and `PB4` hardware PWM status LED. Isolated RS-485/LIN and 24V/48V power inputs are delegated to modular expansion CARDs.
2. **Value-Added / Legacy Derivative Boards (`CORE-A5` etc. — Archive)**:
   - Integrated single-board variants (such as legacy [CORE-A5](../../hardware/archive/CORE-A5/spec_study.md)) are classified as specialized value-added derivatives rather than the primary Core line.
