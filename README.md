<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English** | [日本語 (docs/ja/README.md)](docs/ja/README.md) ]

<p align="center">
  <img src="docs/images/hero_no_photo.svg" alt="ADX CORE-A5: The Working Arduino" width="100%">
</p>

### *The Working Arduino*
> **A tougher, enclosure-native board engineered to run your Arduino sketches reliably in the real world.**

**ADX** is an open-hardware modular ecosystem designed for projects that need to step off the workbench and run continuously inside real enclosures—neatly mounted, cleanly wired, and shielded with field-ready protections.

---

## Quick Navigation by Goal

Find what you need based on what you want to achieve:

| What do you want to do? | Key Documentation | Primary Links |
| :--- | :--- | :--- |
| 🚀 **Run sketches on ADX** | Arduino IDE & megaTinyCore setup guide | [Arduino Setup Guide](#4-arduino-ide-compatibility) |
| 🛠 **Inspect hardware & schematics** | Complete schematics, netlist & BOM for CORE-A5 | [CORE-A5 Specification](hardware/CORE-A5/spec_study.md) |
| 🔌 **Build custom addon CARDs** | Universal 20-pin bus pinout & layout rules | [ADX 20-Pin Bus (SSOT)](docs/spec/ADX_PINOUT.md) |
| 📦 **Design or 3D-print cases** | Dimensions, M3 hole spacing & rail margins | [8748 Form Factor Specification](docs/en/8748_formfactor.md) |

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
         ・20-Pin Universal Bus (VBUS, GND, Native 3.3V LVCMOS Signals)
       ════════════════════════════════════════════════════════════════════
                                       │
            ┌──────────────────────────┴──────────────────────────┐
            ▼                                                     ▼
   【 Main Core Board 】                                 【 Modular CARDs 】
     ADX CORE-A5                                           ADX CARD Ecosystem
     ・Field-Tough Edge Slave (-40℃ ~ +105℃)               ・Prototyping CARD
     ・Push-in Spring Terminals (5VIN & RS-485)            ・Power Supply CARDs
     ・2500V Galvanic Isolation & CCL OTW                  ・3D-Printable Enclosures
```

### 2.1 8748 Form Factor & Mechanical Mounting
- **Dimensions**: Standardized `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`).
- **Clearance for Real Screws**: Four-corner M3 mounting holes (`77.0 mm × 38.0 mm` pitch) with generous **Φ5.0 mm washer clearances**, ensuring screws and lock washers never bite into traces.
- **Enclosure Grab Margins**: Dedicated keep-out zones along both long edges support **slot-in rail mounting** and **3D-printed snap-fit tabs**.
- *Reference*: [8748 Form Factor Specification](docs/en/8748_formfactor.md)

### 2.2 Unidirectional Interface Edge
External field connections (power and communications) are strictly aligned along **one single edge of the PCB**.  
By eliminating multi-directional wire routing, enclosure cutouts and front-panel machining are simplified to a single opening.

### 2.3 Universal 20-Pin Bus (VBUS & Native 3.3V GPIO)
All Core boards and Card modules interconnect via a standardized 2×10-pin shrouded IDC connector:
- **`VBUS` Power Highway**: 4 dedicated pins (Pins 1–4) delivering up to **2.0A continuous** bidirectional power ($4.5\text{V} \sim 5.0\text{V}$).
- **Short-Circuit Barrier**: Pin 5 is intentionally `N.C.` (No Connect) to physically prevent power-to-signal shorts during cable crimping.
- **Native 3.3V Logic**: All digital pins (UART, I2C, SPI, GPIO) operate at **3.3V LVCMOS**, eliminating level-shifter clutter on addon cards.
- **Flexible Stacking**: Supports direct board-to-board mezzanine stacking as well as off-the-shelf **flat IDC ribbon cables** for side-by-side placement.
- *Reference (SSOT)*: [ADX 20-Pin Bus Specification](docs/spec/ADX_PINOUT.md) | [Architecture Guide](docs/en/ADX_pinout.md)

---

## 3. Flagship Core Board: ADX CORE-A5

**ADX CORE-A5** is the production-grade edge slave controller designed for long-term continuous operation in extreme environments.

```text
┌────────────────────────────────────────────────────────────────────────┐
│             【 ADX CORE-A5 】(High-Reliability Edge Slave)              │
│  - MCU: Microchip ATtiny1616-MNR (3.3V @ 10MHz / Zero Self-Heating)    │
│  - Temperature: -40℃ to +105℃ (Full Industrial / Automotive Grade)     │
│  - Field Power: Front 5VIN Spring Terminal Block (SY6280 Protection)   │
│  - Isolated Comm: 2500V Galvanic Isolated RS-485 (TDA51S485HC, 500kbps)│
│  - OTW Bootloader: Hardware CCL Break auto-detection & reboot          │
│  - Form Factor: Native 8748 profile with 20-pin bus header             │
└────────────────────────────────────────────────────────────────────────┘
```

- **Screwless Push-in Terminals**: Front-facing 5.08mm spring clamp terminal blocks (DORABO DB142R) resist severe industrial vibration without loosening.
- **Standalone 5VIN Power**: Operates standalone with local 5V field power (with reverse-current blocking, short-circuit, and overcurrent protection), or draws power from a stacked Power Card via `VBUS`.
- **CCL-Driven Over-The-Wire (OTW) Updates**: Eliminates physical buttons and secondary management chips. An internal Configurable Custom Logic (CCL) block monitors the RS-485 line for a standard Break signal from off-the-shelf USB-RS485 dongles to trigger bootloader reboot safely.
- *Specifications*: [CORE-A5 Specification Study](hardware/CORE-A5/spec_study.md) | [Hardware Concept](hardware/CORE-A5/concept.md)

---

## 4. Arduino IDE Compatibility

You do not need to learn a proprietary PLC language or install heavy toolchains.

- **Familiar Code**: Develop sketches using standard Arduino C/C++ syntax and common libraries via **`megaTinyCore`**.
- **Instant Boot & Zero Delay**: Boots user sketches in **0 ms** on power-on (no artificial bootloader delay).
- **MCU Hardware Mapping**: See [ATtiny1616 Hardware Mapping](docs/spec/mcu_mapping/attiny1616.md) for timer allocations, PWM channels, and PORTMUX details.

---

## 5. The Modular CARD Ecosystem

ADX boards expand seamlessly through stackable, modular CARDs sharing the identical 8748 footprint:

- **Prototyping CARD**: A universal 2.54mm breadboard prototyping area with breakout access to the 20-pin bus. ([Details](hardware/CARD/Prototyping/proposal.md))
- **Power CARDs**: Isolated DC 12V/24V step-down cards and battery pack cards for off-grid operations.
- **3D-Printable Enclosures**: Downloadable STL/STEP CAD models for DIN-rail mounts, desktop enclosures, and snap-fit cases.

---

## 6. Open Hardware Standards & Repository Sitemap

All designs, dimensions, firmware, and documentation are openly published:

- **Core Standards (SSOT)**:
  - **ADX 20-Pin Bus Specification**: [docs/spec/ADX_PINOUT.md](docs/spec/ADX_PINOUT.md)
  - **8748 Form Factor Specification**: [docs/en/8748_formfactor.md](docs/en/8748_formfactor.md) | [日本語 (docs/ja/8748_formfactor.md)](docs/ja/8748_formfactor.md)
- **Design & Architecture Guides**:
  - **Bus Architecture & Shielding Guide**: [docs/en/ADX_pinout.md](docs/en/ADX_pinout.md) | [日本語 (docs/ja/ADX_pinout.md)](docs/ja/ADX_pinout.md)
  - **ATtiny1616 Hardware Mapping**: [docs/spec/mcu_mapping/attiny1616.md](docs/spec/mcu_mapping/attiny1616.md)
- **Active Hardware Projects**:
  - **CORE-A5 Controller**: [hardware/CORE-A5/spec_study.md](hardware/CORE-A5/spec_study.md) | [concept.md](hardware/CORE-A5/concept.md)
  - **CARD Modules**: [hardware/CARD/Prototyping/proposal.md](hardware/CARD/Prototyping/proposal.md)
- **Hardware Portfolio Overview**: [hardware/README.md](hardware/README.md)
