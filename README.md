<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English** | [日本語 (docs/ja/README.md)](docs/ja/README.md) ]

<p align="center">
  <img src="docs/images/hero_no_photo.svg" alt="ADX CORE-U Enclosure-Friendly Arduino Board" width="100%">
</p>

**ADX CORE-U** is an enclosure-friendly, Arduino-compatible board designed from the ground up to make putting your custom electronics projects into real boxes effortless, neat, and reliable.

---

## 1. Overview: From Workbench to Enclosure

Building prototypes on a workbench is fun, but putting them into a neat enclosure is where real projects truly come to life. 

Traditional prototyping boards often become awkward once placed inside an enclosure—mounting screws scratch circuit traces, wires protrude in all four directions, and supplying power to motors or servos without tripping a laptop's USB port becomes a delicate balancing act.

**ADX solves this through a dual-layered approach:**
1. **The ADX Hardware Standard**: An open mechanical and bus standard (8748 Form Factor) engineered specifically for effortless mounting in custom 3D-printed cases, off-the-shelf plastic project boxes, and electrical enclosures.
2. **ADX CORE-U**: An accessible flagship microcontroller board that lets you build, program, and iterate with the familiar **Arduino IDE and `megaTinyCore`** over **USB Type-C**.

---

## 2. The Foundation: ADX Hardware Standard

The ADX standard is an open physical specification designed to ensure that microcontrollers and expansion modules fit cleanly inside real-world enclosures.

```
       ══════════════════════════════════════════════════════════════════
                         【 ADX HARDWARE STANDARD 】
         ・8748 Form Factor (87.0 mm × 48.0 mm outline)
         ・Corner M3 Mounting Holes with Φ5.0 mm Washer Clearances
         ・Rail-Grab Margins for Slot-In & Snap-Fit Enclosures
         ・Unidirectional I/F Edge (All external ports on one side)
         ・20-Pin Expansion Bus (Direct stack & flat IDC ribbon cable)
       ══════════════════════════════════════════════════════════════════
                                       │
            ┌──────────────────────────┴──────────────────────────┐
            ▼                                                     ▼
   【 Flagship MCU Board 】                              【 Modular CARDs 】
     ADX CORE-U (The Enclosure Board)                      ADX CARD Ecosystem
     ・ATtiny1616 + USB Type-C                             ・Prototyping CARD
     ・Arduino IDE + megaTinyCore Native                   ・AAA Battery CARD
     ・Protected Power Architecture                        ・3D-Print Mount STLs
```

### 2.1 8748 Form Factor & Mechanical Tolerances
- **Dimensions**: `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`).
- **Mechanical Mounting**: Standardized 4-corner M3 mounting holes (`77.0 mm × 38.0 mm` pitch) with generous **Φ5.0 mm washer clearances**, ensuring standard screw heads never pinch or crush PCB copper traces.
- **Enclosure Grab Margins**: Dedicated keep-out zones along the board edges allow seamless **slot-in rail mounting** and **3D-printed snap-fit tabs** without component collision.

### 2.2 Unidirectional Interface Edge
All external I/O connectors (such as USB Type-C ports) are strictly aligned along a **single edge of the PCB**. 
This eliminates the multi-directional wire protrusions common on standard dev boards, allowing 3D-printed faceplates and drilled enclosure panels to require port cutouts on only one single side.

### 2.3 20-Pin Bus & Flexible Enclosure Topology
Expansion modules (CARDs) connect through a unified 20-pin expansion bus (5V VDD, GND, GPIO, ADC, DAC, I2C, SPI, UART).
- **Direct Rigid Stack**: CARDs can stack directly on top of or beneath the core board using standard 2.54 mm pin headers.
- **Flexible IDC Ribbon Cable**: The 20-pin bus is explicitly standardized for off-the-shelf **MIL flat IDC ribbon cables**. In slim enclosures, modules can sit side-by-side; in deep boxes, they can stack vertically; in offset enclosures, cables can route around obstacles with zero custom harness work.

---

## 3. The Flagship Board: ADX CORE-U

**ADX CORE-U** is the primary core board of the ADX ecosystem, built to offer a friction-free development workflow for makers, hobbyists, and educators.

```
┌────────────────────────────────────────────────────────────────────────┐
│                   【 ADX CORE-U 】(The Enclosure Board)                │
│  - Microchip ATtiny1616-MNR MCU (16KB Flash / 2KB SRAM / 20MHz)        │
│  - WCH CH342K USB-UART with Onboard SerialUPDI Auto-Flashing           │
│  - Native Arduino IDE + megaTinyCore Support (No bootloader required)   │
│  - USB Type-C Interface with Host-PC Protection                        │
│  - Standard ADX 20-Pin Expansion Bus Header                            │
└────────────────────────────────────────────────────────────────────────┘
```

### 3.1 Seamless Arduino IDE Experience
- **Instant Code & Flash**: Powered by a WCH `CH342K` USB bridge with hardware SerialUPDI support, you can flash sketches directly from the standard Arduino IDE using `megaTinyCore`.
- **Zero Bootloader Lag**: Unlike traditional AVRs, UPDI programs code directly into flash without occupying bootloader memory or introducing startup delays.
- **Targeted Scope & Honesty**: Driven by an ATtiny1616 (16KB Flash, 2KB SRAM), CORE-U is purpose-built for small-to-medium control tasks—reading sensors, driving servos, controlling Neopixels, and triggering relays. It keeps power consumption low and board footprint compact.

### 3.2 Power & Interface Design (Active R&D)
To ensure reliable operation when driving dynamic loads (such as servo motors and LED strips) without crashing your host computer's USB port, CORE-U incorporates dedicated power protection topology.
- *For detailed circuit schematics, ideal-diode power path architectures, and current hardware revision notes, please refer to [hardware/CORE-U/concept.md](hardware/CORE-U/concept.md).*

---

## 4. The Modular CARD Ecosystem

ADX boards expand through modular CARDs designed on the same 8748 physical profile:

- **Prototyping CARD**: A universal 2.54 mm pitch breadboard area with pass-through 20-pin bus access, making custom sensor and driver circuits tidy and stackable.
- **AAA 2S Battery CARD**: Onboard boost circuitry that supplies clean 5V to the bus from two standard AAA batteries for standalone and portable builds.
- **3D-Print Enclosure & Mount Files**: Open-source STL/STEP files for customizable brackets, rail guides, and snap-fit boxes.

---

## 5. Getting Started & Open Hardware Resources

All hardware designs, schematics, and mechanical dimensions are open source:

- **8748 Form Factor Specification**: [docs/en/8748_formfactor.md](docs/en/8748_formfactor.md) | [docs/ja/8748_formfactor.md](docs/ja/8748_formfactor.md)
- **ADX 20-Pin Bus Pinout**: [docs/en/ADX_pinout.md](docs/en/ADX_pinout.md) | [docs/ja/ADX_pinout.md](docs/ja/ADX_pinout.md)
- **CORE-U Hardware Notes**: [hardware/CORE-U/concept.md](hardware/CORE-U/concept.md)

---

## 6. Future Horizon: The Path to Industrial

*Note on ecosystem expansion:*  
The ADX 20-pin bus and 8748 form factor are architected with long-term modularity in mind. In the future, specialized heavy-duty core boards (such as galvanically isolated RS-485 variants designed for high-temperature and harsh field deployments) may join the ADX lineup, sharing the exact same expansion CARDs and mechanical mounting systems.
