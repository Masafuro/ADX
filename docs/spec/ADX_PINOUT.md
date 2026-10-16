<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus Specification

**Standard Identifier**: ADX-BUS-SPEC-v2.1  
**Category**: Core Electrical & Interconnect Specification  
**Status**: Standard (Release)  
**Maintained by**: ADX Hardware Standards Committee  

---

## 1. Overview & Scope

The **ADX 20-Pin Bus** is the standard electrical and physical interconnect for the ADX modular ecosystem. It establishes a unified backplane interface between **Core Boards** (main MCUs/controllers) and **Card Modules** (expansion addons, sensors, actuators, and communication cards).

This document serves as the **Single Source of Truth (SSOT)** for the physical pinout, logical signal definitions, and electrical limits of the 20-pin IDC interface.

---

## 2. Physical & Mechanical Interface

- **Connector Family**: 2×10-pin (20-pin), 2.54 mm (0.100") pitch shrouded box header or ejector/latch header (MIL-DTL-83503 compliant).
- **Mating Cable**: Standard 20-conductor flat ribbon cable (AWG 28 stranded).
- **Pin Numbering Convention**: Standard zigzag (interleaved) ribbon cable numbering:
  - Row 1 (Odd pins): 1, 3, 5, 7, 9, 11, 13, 15, 17, 19
  - Row 2: (Even pins): 2, 4, 6, 8, 10, 12, 14, 16, 18, 20

```text
                  【 20-Pin Shrouded Header (Top View) 】

                        ┌───────[ Center Key ]───────┐
           VBUS   [ 1]  │  ●   ●  │  [ 2]  VBUS
           VBUS   [ 3]  │  ●   ●  │  [ 4]  VBUS
           N.C.   [ 5]  │  ●   ●  │  [ 6]  AOUT / DAC
           AIN    [ 7]  │  ●   ●  │  [ 8]  GND
         EXTCLK   [ 9]  │  ●   ●  │  [10]  GND
        UART_TX   [11]  │  ●   ●  │  [12]  UART_RX
        I2C_SCL   [13]  │  ●   ●  │  [14]  I2C_SDA
            GND   [15]  │  ●   ●  │  [16]  SPI_CS
       SPI_MOSI   [17]  │  ●   ●  │  [18]  SPI_MISO
        SPI_SCK   [19]  │  ●   ●  │  [20]  GND
                        └────────────────────────────┘
```

---

## 3. Universal Pin Assignment Table

| Pin | Net Name | Signal Type | Electrical Standard | Primary Function / Role | Shield & Structure |
| :---: | :--- | :--- | :--- | :--- | :--- |
| **1** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (2×2 block) |
| **2** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (2×2 block) |
| **3** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (2×2 block) |
| **4** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (2×2 block) |
| **5** | **N.C.** | Passive | - | Isolation Barrier (Short prevention) | Buffer Zone |
| **6** | **AOUT** | Analog / GPIO | 3.3V Analog Out / LVCMOS | DAC Output / Analog Channel | Analog Zone |
| **7** | **AIN** | Analog / GPIO | 3.3V Analog In / LVCMOS | Analog Input / Voltage Reference | Analog Zone |
| **8** | **GND** | Power / Shield | 0V Return | Coplanar Shield (Clock guard ①) | GND Shield |
| **9** | **EXTCLK** | Digital / GPIO | 3.3V LVCMOS | External Clock Input / GPIO | High-Speed Clock Line |
| **10** | **GND** | Power / Shield | 0V Return | Isolation Shield (Clock guard ②) | GND Shield |
| **11** | **UART_TX**| Digital Out | 3.3V LVCMOS | Serial Transmit (TXD) | UART Signal Pair |
| **12** | **UART_RX**| Digital In | 3.3V LVCMOS | Serial Receive (RXD) | UART Signal Pair |
| **13** | **I2C_SCL**| Open-Drain | 3.3V LVCMOS (Pull-up req.)| I2C Bus Clock | I2C Signal Pair |
| **14** | **I2C_SDA**| Open-Drain | 3.3V LVCMOS (Pull-up req.)| I2C Bus Data | I2C Signal Pair |
| **15** | **GND** | Power / Shield | 0V Return | Mid-bus Shield | GND Shield |
| **16** | **SPI_CS** | Digital Out | 3.3V LVCMOS | SPI Chip Select (Active Low) | SPI Bus Group |
| **17** | **SPI_MOSI**| Digital Out | 3.3V LVCMOS | SPI Controller Out / Peripheral In | SPI Bus Group |
| **18** | **SPI_MISO**| Digital In | 3.3V LVCMOS | SPI Controller In / Peripheral Out | SPI Bus Group |
| **19** | **SPI_SCK** | Digital Out | 3.3V LVCMOS | SPI Serial Clock | SPI Bus Group |
| **20** | **GND** | Power / Shield | 0V Return | Bus Termination Shield | GND Shield |

---

## 4. Electrical Requirements & Design Rules

### 4.1 Power Architecture (`VBUS` & `GND`)
- **Voltage Range**: Nominal $5.0\,\text{V} \pm 10\%$ ($4.5\,\text{V} \sim 5.0\,\text{V}$ operating window).
  - *Tolerance note*: Designed to tolerate diode forward drops from external 5V inputs, power switches (e.g., SY6280), or USB supplies.
- **Current Capacity**: Rated up to **$2.0\,\text{A}$ continuous** across Pins 1–4 combined.
- **Bidirectional Operation**:
  - A Core board may supply power to Cards via `VBUS`.
  - A dedicated Power Card (e.g., DC 12V/24V step-down or battery pack) may inject power into `VBUS` to energize the entire stack.
- **Physical Layout Rule**: Pins 1 to 4 form a $2 \times 2$ grid. PCB artwork MUST join these four pins into a continuous solid copper plane for low impedance and heat dissipation.

### 4.2 Logic Levels (`3.3V LVCMOS Native`)
- **Standard**: All digital I/O pins (Pins 6, 7, 9, 11–14, 16–19) operate strictly at **3.3V LVCMOS native levels**:
  - $V_{IL} \le 0.8\,\text{V}$
  - $V_{IH} \ge 2.0\,\text{V}$
  - $V_{OL} \le 0.4\,\text{V}$
  - $V_{OH} \ge 2.4\,\text{V}$
- **Card Compatibility**: Addon Cards MUST be designed for 3.3V logic. No onboard level shifters are required on Card modules.

### 4.3 Shielding & Isolation Topology
- **Pin 5 Isolation Barrier**: Intentionally left unconnected (No Connect). It physically isolates the high-current `VBUS` cluster (Pins 1–4) from sensitive analog/GPIO signals (Pin 6+), preventing destructive shorts during ribbon cable IDC crimping faults.
- **EXTCLK Ground Sandwich**: Pin 9 (`EXTCLK`) is flanked by Pin 8 (`GND`) and Pin 10 (`GND`). This coplanar waveguide structure suppresses high-frequency radiated EMI.
- **Digital Bus Segregation**: Pin 10 (`GND`) isolates clock lines from the digital communications cluster (UART/I2C/SPI). Pin 15 (`GND`) and Pin 20 (`GND`) provide intermediate and termination returns.

---

## 5. Cross-References & Related Documents

- **Implementation Mapping per Controller**:
  - [ATtiny1616 Hardware Mapping & PORTMUX Reference](mcu_mapping/attiny1616.md) (used by CORE-A5)
  - [RP2040 Hardware Mapping Reference](mcu_mapping/rp2040.md) (used by CORE-R)
- **Design Guides & Rationale**:
  - [ADX Pinout 設計・解説ガイド (日本語)](../ja/ADX_pinout.md)
  - [ADX Pinout Architecture Guide (English)](../en/ADX_pinout.md)
- **Form Factor Standards**:
  - [ADX 8748 Form Factor Specification](../ja/8748_formfactor.md)
