<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English** | [日本語 (docs/ja/README.md)](docs/ja/README.md) ]

**ADX (Advanced Devices eXtended)** is an open-source modular hardware standard and control platform engineered around the core principle of **"Enclosure-Friendly"** hardware design.

While traditional prototyping boards excel on the workbench, deploying them into real-world enclosures, harsh industrial cabinets, or outdoor environments introduces friction—screw holes colliding with traces, multi-directional wiring protrusions, and rigid pin headers that cannot absorb mechanical tolerances.

ADX solves this through the **8748 Form Factor** (`87.0 mm × 48.0 mm`) and a flexible **20-pin IDC Eject Header bus**. Built upon a single, indestructible, industrial-grade core (**ADX CORE-A: $28**) and an open, dongle-free field protocol (**MR32**), ADX provides an uncompromised bridge from desktop prototyping to professional social infrastructure.

---

## 1. Key Architectural Features

* **Enclosure-Friendly Mechanical Alignment (8748 Form Factor)**:
  `87.0 mm × 48.0 mm` outline with standardized M3 mounting holes (`77.0 mm × 38.0 mm` pitch) featuring generous $5.0\text{ mm}$ edge margins to accommodate standard screw washers without damaging PCB copper.
* **Floating Safe-Zone Architecture (Zero Self-Heating)**:
  High-voltage DC-DC converters are decoupled from the core board and delegated to modular Power CARDs. The core board remains a pure, cool 5V floating zone, achieving **full Industrial Grade (-40°C to +105°C) durability** even in sealed IP67 waterproof boxes.
* **Galvanically Isolated Differential Bus (RS-485)**:
  Onboard Mornsun `TDA51S485HC` transceiver with integrated isolated power DC-DC (2500VDC isolation) directly coupled to a screwless, front-accessible spring clamp terminal block (`DB142R-5.08`).
* **PC-Protected Dual-Feed Power Topology**:
  USB Type-C 6P input protected by an ultra-low-loss ideal diode (TI `LM66100`), ensuring **zero back-feed into connected host PCs/laptops** when external field power is active.
* **Latch-Locking 20-Pin Eject Header**:
  Heavy-duty MIL-DTL-83503 compliant ejector header ensuring zero vibration disconnects and 100% reverse/misalignment prevention, providing bidirectional 5V/2A power delivery.
* **Ultra-Low Power BMC Supervisor**:
  Secondary MCU (ATtiny412) operating in the nanoampere sleep tier, enabling touchless remote Over-The-Wire (OTW) firmware flashing via mobile PWA.
* **MR32 Protocol (Dongle-Free Architecture)**:
  Deterministic 32-byte fixed-length packet protocol enabling instant, 3-second OTW firmware flashing and inter-node networking using **any generic $3–$5 USB-RS485 dongle** directly from smartphone browsers (Android PWA) and PCs without proprietary hardware lock-in.

---

## 2. Hardware Lineup & Ecosystem

```
┌────────────────────────────────────────────────────────────────────────┐
│                   【 ADX CORE-A 】($28 / Universal Core)               │
│  - 5V-Dedicated / -40°C to +105°C Industrial Grade / Zero Heat         │
│  - Fully Isolated RS-485 (2500VDC) + Screwless Spring Terminal Block    │
│  - USB Type-C 6P (Ideal Diode Reverse-Current Protection)              │
│  - 20P Latch Eject Header (5V/2A Bidirectional Power Bus)              │
│  - Board Management Controller (BMC) for <10µA Sleep & Remote Flashing │
│  - Native MR32 Protocol Stack (Standard UART 8N1 + CRC-16)             │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
        ┌───────────────────────────┴───────────────────────────┐
        ▼ 20-Pin IDC Expansion Bus                              ▼ Front RS-485 Terminal
┌───────────────────────────────────┐               ┌───────────────────────────────────┐
│     【 CARD Series (Modular) 】   │               │ 【 Generic USB-RS485 Dongles 】   │
│                                   │               │ (Dongle-Free Architecture)        │
│ ① AAA 2S Power CARD (Battery)     │               │ - Off-the-shelf $3–$10 converters │
│    5V Boost / BMC-Gated Sleep     │               │   (CH340G, CP2102, FTDI, PL2303)  │
│ ② DC 12V–48V Power CARD (Telecom) │               │ - Direct Phone (Android PWA) & PC │
│    Isolated DCDC / -48V & 24V IN  │               │ - 100% Electrically Protected by  │
│ ③ Prototyping CARD                │               │   CORE-A Onboard 2500V Isolation  │
│    20P Pass-Through + Breadboard  │               │ - 3-second MR32 OTW Flashing      │
│ (Future: 24V I/O, Relay CARDs)    │               │                                   │
└───────────────────────────────────┘               └───────────────────────────────────┘
```

### 2.1 Core Board
* **ADX CORE-A ($28.00 USD)**:
  The absolute standard MCU board of the ADX ecosystem. Driven by a Microchip ATtiny1616-MNR and ATtiny412-SSNR, featuring fully isolated RS-485, AEC-Q102 automotive LEDs (-40°C to +110°C), and rugged front terminals.

### 2.2 Modular Expansion CARDs
* **AAA 2S Battery CARD**: Ultra-low quiescent boost converter feeding 5V into the VDD bus, with BMC-linked complete shutdown.
* **DC 12V–48V Industrial Power CARD**: Galvanically isolated wide-input DC-DC feeding clean 5V, supporting factory 24V and telecom -48V infrastructure.
* **Prototyping CARD**: Universal breadboard area with 20-pin pass-through.

---

## 3. Core Specifications & Guidelines

### 3.1 8748 Form Factor
Mechanical dimensioning co-designed for seamless enclosure integration and 3D printing.
* **Dimensions:** `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **Mounting Holes:** `77.0 mm × 38.0 mm` pitch, M3 screws (`Φ3.3 mm`), C3 chamfer
* **Full Specification:** [English (docs/en/8748_formfactor.md)](docs/en/8748_formfactor.md) | [日本語 (docs/ja/8748_formfactor.md)](docs/ja/8748_formfactor.md)

### 3.2 ADX Pinout (v2)
A unified 20-pin ribbon bus featuring 5V/2A power delivery, short-prevention isolation, EXTCLK shielding, and dynamic PORTMUX peripheral routing.
* **Connector:** 2×10-pin 2.54 mm pitch latch-locking Eject Header (IDC Pins 1–20)
* **Full Specification:** [English (docs/en/ADX_pinout.md)](docs/en/ADX_pinout.md) | [日本語 (docs/ja/ADX_pinout.md)](docs/ja/ADX_pinout.md)

### 3.3 Hardware Design Guidelines (RFC 2119: MUST / SHOULD / MAY)
Standards governing official and third-party extension CARD design to guarantee zero-accident ecosystem compatibility.
* **【MUST】Mandatory**: Power CARDs must implement reverse-current blocking diodes on VDD output; Pin 5 N.C. isolation barrier must remain unconnected.
* **【SHOULD】Recommended (Official Grade)**: Latch-locking Eject Header, galvanically isolated DCDCs for industrial voltages, 8748 dimensions, -40°C to +105°C temperature ratings.
* **【MAY】Optional**: Non-isolated buck converters for hobby use, custom board outlines, shroud box headers, unpopulated battery monitor lines.
* **Full Specification:** [日本語 (docs/ja/adx_formfactor_guidelines.md)](docs/ja/adx_formfactor_guidelines.md)

---

## 4. Software & Cloud Ecosystem (PWA + Cloud API)

Eliminating the maintenance burden of native mobile apps (16KB page size crashes, OS scoped-storage limitations, app store gatekeepers):

```
┌────────────────────────────────────────────────────────┐
│  ① Cloud Compile API (arduino-cli / MegaCoreX)         │
│     - Receives code ➔ Compiles in 1 sec ➔ Returns .hex │
└───────────────────────────┬────────────────────────────┘
                            │ .hex (Compact Binary)
                            ▼
┌────────────────────────────────────────────────────────┐
│  ② Web PWA (Zero-Install, Runs in Any Browser)         │
│     - Real-time parameter tuning via EEPROM (<0.1s)    │
│     - WebSerial / WebUSB transfer (MR32 Protocol)      │
└───────────────────────────┬────────────────────────────┘
                            │ USB (Generic RS-485 Dongle)
                            ▼
┌────────────────────────────────────────────────────────┐
│  ③ Generic USB-RS485 Dongle ➔ ADX CORE-A               │
│     - 3-second deterministic OTW flashing via MR32     │
│     - 2,500V onboard galvanic isolation for safety     │
└────────────────────────────────────────────────────────┘
```

---

## 5. Documentation (多言語ドキュメント)

* **English Documentation:** [docs/en/README.md](docs/en/README.md)
  * [8748 Form Factor Specification](docs/en/8748_formfactor.md)
  * [ADX Pinout Specification (v2)](docs/en/ADX_pinout.md)
  * [ATtiny1616 Peripheral & PORTMUX Reference](docs/en/adx_attiny1616_mcu_matrix.md)
* **日本語ドキュメント (Japanese):** [docs/ja/README.md](docs/ja/README.md)
  * [8748 フォームファクタ仕様書](docs/ja/8748_formfactor.md)
  * [ADX ピンアサイン仕様書 (v2)](docs/ja/ADX_pinout.md)
  * [ADX 開発ガイドライン (MUST / SHOULD / MAY)](docs/ja/adx_formfactor_guidelines.md)
  * [ATtiny1616 ペリフェラル＆PORTMUX仕様書](docs/ja/adx_attiny1616_mcu_matrix.md)

---

## 6. Repository Structure

```text
ADX/
├── README.md               # Global portal & overview (this file)
├── LICENSE.md              # Multi-licensing policy & trademark notices
├── LICENSES/               # REUSE-compliant license texts (CC-BY-4.0, CERN-OHL-P-2.0, MIT)
├── Project_Snapshot.md     # Project status, history, and roadmap
├── docs/                   # Full multilingual documentation (CC BY 4.0)
│   ├── en/                 # English documentation
│   └── ja/                 # Japanese documentation
├── hardware/               # Hardware design & production files (CERN-OHL-P-v2)
│   ├── CORE-A/             # Flagship universal tough MCU board (-40°C~+105°C, isolated RS-485)
│   ├── CARD/               # Modular add-on boards (Power, Prototyping)
│   ├── Formfactor/         # Mechanical 8748 templates and CAD models
│   └── archive/            # Retired / archived hardware designs (RPR4, Core-S, Core-D, Core-O)
├── firmware/               # Drivers, BSP, bootloaders, and samples (MIT)
├── logo/                   # Brand assets & logos
└── memo/                   # Engineering notes & strategic concept reports
```

---

## 7. License

The ADX project is released under a tri-license architecture tailored for open collaboration:

* **Documentation & Specifications** (`docs/`, `memo/`): [Creative Commons Attribution 4.0 International](LICENSES/CC-BY-4.0.txt) (`CC-BY-4.0`)
* **Hardware Designs & Production Files** (`hardware/`): [CERN Open Hardware Licence Version 2 - Strongly Reciprocal](LICENSES/CERN-OHL-P-2.0.txt) (`CERN-OHL-P-2.0`)
* **Firmware, Software & Web Assets** (`firmware/`, software tools): [MIT License](LICENSES/MIT.txt) (`MIT`)

Refer to [LICENSE.md](LICENSE.md) for full licensing terms and trademark policies.
