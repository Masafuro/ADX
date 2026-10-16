<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English** | [日本語 (docs/ja/README.md)](docs/ja/README.md) ]

<p align="center">
  <img src="docs/images/hero_no_photo.svg" alt="ADX CORE-A: The Working Arduino" width="100%">
</p>

### *The Working Arduino*
> **A tougher, enclosure-friendly board for running your Arduino sketches in the real world.**

**ADX CORE-A** is an open-hardware microcontroller board designed for projects that need to step off the workbench and run reliably inside real-world spaces—neatly mounted, cleanly wired, and built with practical protections.

---

## 1. Overview: From Workbench to Workplace

Prototyping on a workbench with a breadboard is an incredible, creative way to bring ideas to life.

However, when it is time to move those sketches into a permanent spot—a workshop dust collector, a greenhouse watering rig, a storefront display, or a custom wall-mounted clock—makers often run into common physical challenges:

1. **Awkward Mounting**: Mounting holes are often too close to delicate traces, making tightening M3 screws feel risky.
2. **Hard to Enclose**: Wires and connectors stick out in multiple directions, requiring complicated multi-sided cutouts in enclosures.
3. **Loose Wiring**: Standard jumper wires can loosen under minor vibration or accidental tugs.
4. **Maintenance Friction**: Taking a heavy laptop to a crawl space or ladder just to change a few lines of code is inconvenient.

**ADX addresses these hurdles through a grounded philosophy: "The Working Arduino."**  
Not a complicated, high-cost industrial PLC, but an accessible, slightly tougher companion that makes putting your projects into real enclosures natural and trouble-free.

---

## 2. The Foundation: ADX Hardware Standard

The foundation of ADX is an open physical specification designed to ensure boards and modules mount cleanly and wire tidily.

```
       ════════════════════════════════════════════════════════════════════
                         【 ADX HARDWARE STANDARD 】
         ・8748 Form Factor (87.0 mm × 48.0 mm outline)
         ・Corner M3 Mounting Holes with Φ5.0 mm Washer Clearances
         ・Rail-Grab Margins for Slot-In & Snap-Fit Enclosures
         ・Unidirectional I/F Edge (All external wiring on one face)
         ・20-Pin Robust Bus (Direct stack & flat IDC ribbon cable)
       ════════════════════════════════════════════════════════════════════
                                       │
            ┌──────────────────────────┴──────────────────────────┐
            ▼                                                     ▼
   【 Main Core Board 】                                 【 Modular CARDs 】
     ADX CORE-A (Active R&D)                               ADX CARD Ecosystem
     ・Tougher, Enclosure-Native Core                      ・Prototyping CARD
     ・Spring Terminals & Protection                       ・AAA Battery CARD
     ・(See hardware/CORE-A/ for notes)                    ・3D-Print Mount STLs
```

### 2.1 8748 Form Factor & Mechanical Mounting
- **Dimensions**: `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`).
- **Clearance for Real Screws**: Standardized 4-corner M3 mounting holes (`77.0 mm × 38.0 mm` pitch) with generous **Φ5.0 mm washer clearances**, ensuring screw heads and washers never crush PCB traces.
- **Enclosure Grab Margins**: Dedicated keep-out zones along the board edges support **slot-in rail mounting** and **3D-printed snap-fit tabs**.

### 2.2 Unidirectional Interface Edge
External field connections are strictly aligned along a **single edge of the PCB**.  
By eliminating multi-directional wire protrusions, enclosure design is simplified—you only need to plan cutouts and openings on one face of your project box.

### 2.3 20-Pin Robust Bus & Flexible Placement
Expansion modules (CARDs) connect through a unified 20-pin expansion bus (VBUS, GND, 3.3V GPIO, ADC, DAC, I2C, SPI, UART).
- **Direct Stack**: CARDs can stack directly on top of or beneath the core board.
- **Flexible IDC Ribbon Cable**: The bus is standardized for off-the-shelf **flat IDC ribbon cables**. In shallow boxes, modules can sit side-by-side; in deep boxes, they can stack vertically—adapting to your enclosure shape rather than dictating it.

---

## 3. Flagship Hardware (In Active R&D)

ADX is actively developing dedicated hardware solutions to bring "The Working Arduino" to life.

```
┌────────────────────────────────────────────────────────────────────────┐
│             【 ADX CORE-A 】(Main Board — In Active R&D)                │
│  - Microchip ATtiny1616-MNR MCU (16KB Flash / 2KB SRAM)                │
│  - Push-in spring terminal blocks for vibration-resistant wiring        │
│  - Protective circuitry designed to cushion external electrical noise   │
│  - Native 8748 Form Factor with standardized corner M3 mounts          │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ Field connection
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│          【 RPR4 Smart Probe 】(Adapter Probe — In Active R&D)         │
│  - Experimental tool exploring browser WebSerial connectivity          │
│  - Researched to enable quick sketch updates directly in the field     │
│  - Compact form factor to avoid carrying heavy laptops to deployed rigs│
└────────────────────────────────────────────────────────────────────────┘
```

### 3.1 ADX CORE-A (Main Board)
CORE-A is the primary core board engineered for long-term installation.
- **Push-in Spring Terminals**: Screwless terminal blocks allow solid or ferruled wires to be inserted securely without loosening over time.
- **Protection by Design**: Circuitry designed to reduce the risk of accidental wiring damage, keeping the board steady when operating near switching devices or longer cable runs.
- *For detailed circuit schematics, protection topology, and current revision notes, please refer to [hardware/CORE-A/concept.md](hardware/CORE-A/concept.md).*

### 3.2 RPR4 Smart Probe (Adapter Probe)
RPR4 is an experimental adapter currently under research and prototyping.
- Built to explore convenient field servicing—such as quick parameter tweaks and sketch updates over browser-based WebSerial—aiming to free makers from balancing a laptop in awkward installation spaces.
- *For prototype development progress, please refer to [hardware/RPR4/](hardware/RPR4/).*

---

## 4. Arduino IDE Compatibility

You do not need to learn a new programming language or switch to unfamiliar development tools.

- **Familiar Code**: Programs can be written with the standard Arduino language and libraries using `megaTinyCore`.
- **Lightweight & Efficient**: Driven by an ATtiny1616, CORE-A keeps power consumption modest and startup instantaneous, making it well-suited for everyday sensing, timing, and device control tasks.

---

## 5. The Modular CARD Ecosystem

ADX boards expand seamlessly through modular CARDs sharing the same 8748 physical profile:

- **Prototyping CARD**: A universal 2.54 mm pitch breadboard area with pass-through 20-pin bus access, keeping custom circuits tidy and securely mountable.
- **AAA 2S Battery CARD**: Onboard boost circuitry providing 5V to the bus from two standard AAA batteries for standalone and portable projects.
- **Open 3D-Print STLs**: Downloadable STL/STEP files for customizable brackets, rail guides, and snap-fit project boxes.

---

## 6. Getting Started & Open Hardware Resources

All hardware designs, dimensions, and specifications are open source:

- **8748 Form Factor Specification**: [docs/en/8748_formfactor.md](docs/en/8748_formfactor.md) | [docs/ja/8748_formfactor.md](docs/ja/8748_formfactor.md)
- **ADX 20-Pin Bus Specification (SSOT)**: [docs/spec/ADX_PINOUT.md](docs/spec/ADX_PINOUT.md) | [Architecture Guide](docs/en/ADX_pinout.md)
- **CORE-A5 Hardware Notes**: [hardware/CORE-A5/concept.md](hardware/CORE-A5/concept.md)
- **RPR4 Development Notes**: [hardware/RPR4/](hardware/RPR4/)
- **Documentation Reorganization Plan**: [memo/PLAN/WORKING_ARDUINO_CORE-A_DOCUMENTATION_PLAN.md](memo/PLAN/WORKING_ARDUINO_CORE-A_DOCUMENTATION_PLAN.md)
