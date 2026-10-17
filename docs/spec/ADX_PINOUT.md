<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus Specification

**Standard Identifier**: ADX-BUS-SPEC-v3.0  
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
  - Row 2 (Even pins): 2, 4, 6, 8, 10, 12, 14, 16, 18, 20

```text
                  【 20-Pin Shrouded Header (Top View) 】

                         ┌───────[ Center Key ]───────┐
            VBUS   [ 1]  │  ●   ●  │  [ 2]  VBUS
            VBUS   [ 3]  │  ●   ●  │  [ 4]  VBUS
             GND   [ 5]  │  ●   ●  │  [ 6]  SYS_RST / PROG
             AIN   [ 7]  │  ●   ●  │  [ 8]  GND
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
| **1** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (Pins 1–4) |
| **2** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (Pins 1–4) |
| **3** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (Pins 1–4) |
| **4** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) | Power Zone (Pins 1–4) |
| **5** | **GND** | Power / Shield | 0V Return | VBUS Return & Zone Shield Wall | Primary Power Return |
| **6** | **SYS_RST** | Digital I/O | 3.3V LVCMOS (Active LOW) | System Reset / MCU Programming | Quiet Zone (between Pin 5 & Pin 7) |
| **7** | **AIN** | Analog In | 3.3V Analog / LVCMOS | Analog Input (ADC) / Voltage Reference | Analog Zone |
| **8** | **GND** | Power / Shield | 0V Return | Coplanar Shield (Clock guard ①) | GND Shield |
| **9** | **EXTCLK** | Digital / GPIO | 3.3V LVCMOS | External Clock Input / PWM / GPIO | High-Speed Clock Line |
| **10** | **GND** | Power / Shield | 0V Return | Isolation Shield (Clock guard ②) | GND Shield |
| **11** | **UART_TX**| Digital Out | 3.3V LVCMOS | Serial Transmit (TXD) | UART Signal Pair |
| **12** | **UART_RX**| Digital In | 3.3V LVCMOS | Serial Receive (RXD) | UART Signal Pair |
| **13** | **I2C_SCL**| Open-Drain | 3.3V LVCMOS (Pull-up req.)| I2C Bus Clock | I2C Signal Pair |
| **14** | **I2C_SDA**| Open-Drain | 3.3V LVCMOS (Pull-up req.)| I2C Bus Data | I2C Signal Pair |
| **15** | **GND** | Power / Shield | 0V Return | Mid-bus Shield (I2C/SPI Boundary) | GND Shield |
| **16** | **SPI_CS** | Digital Out | 3.3V LVCMOS | SPI Chip Select (Active Low) | SPI Bus Group |
| **17** | **SPI_MOSI**| Digital Out | 3.3V LVCMOS | SPI Controller Out / Peripheral In | SPI Bus Group |
| **18** | **SPI_MISO**| Digital In | 3.3V LVCMOS | SPI Controller In / Peripheral Out | SPI Bus Group |
| **19** | **SPI_SCK** | Digital Out | 3.3V LVCMOS | SPI Serial Clock | SPI Bus Group |
| **20** | **GND** | Power / Shield | 0V Return | Bus Termination Shield | GND Shield |

---

## 4. Electrical Requirements & Design Rules

### 4.1 Power Architecture (`VBUS` & `GND`)
- **Voltage Range**: Nominal $5.0\,\text{V} \pm 10\%$ ($4.5\,\text{V} \sim 5.0\,\text{V}$ operating window).
- **Current Capacity**: Rated up to **$2.0\,\text{A}$ continuous** across Pins 1–4 combined.
- **Mandatory Sourcing Protection (MUST)**:
  - Any module that injects power into `VBUS` (Power CARDs, Core board external supplies, USB inputs) **MUST** implement:
    1. **Reverse-Current Blocking**: Prevents back-feeding power into sources or unpowered circuits.
    2. **Short-Circuit / Over-Current Protection**: Must autonomously limit or cut off output current upon a short-circuit condition (e.g., via high-side load switch, eFuse, or DC-DC hiccup mode).
- **Primary Ground Return (Pin 5)**:
  - Pin 5 serves as the primary ground return for the VBUS power cluster. Placing Pin 5 adjacent to Pin 4 minimizes high-current loop area and parasitic inductance, suppressing EMI.
  - In flat ribbon cabling, wire 5 (GND) acts as an electrostatic and short-circuit barrier between the VBUS cluster (wires 1–4) and the logic/signal zone (wires 6–20).
- **Physical Layout Rule**: Pins 1 to 4 form a $2 \times 2$ grid. PCB artwork MUST join these four pins into a continuous solid copper plane.

### 4.2 System Reset & Programming Pin (`Pin 6: SYS_RST / PROG`)
- **Signal Definition**: Active-LOW open-drain / bidirectional line with an onboard pull-up resistor on the Core board ($10\,\text{k}\Omega \sim 47\,\text{k}\Omega$ to 3.3V).
- **Card-Side Rule**: Expansion Cards MUST leave Pin 6 disconnected (N.C.) unless specifically implementing an open-drain hardware reset trigger (e.g., watchdog supervisor). Cards MUST NOT drive Pin 6 HIGH with a push-pull driver.
- **Controller-Specific Mapping**:
  - **ATtiny1616 (CORE-A5)**: Mapped to `PA0` (UPDI / RESET). Enables 1-wire programming, debugging, and factory flash programming through Pin 6 without requiring auxiliary connectors or test pads.
  - **RP2040 (CORE-R)**: Mapped to `RUN` (Hardware Reset).
  - **Other MCUs**: Mapped to native active-LOW reset line (`NRST` / `RESET#`).

### 4.3 Logic Levels & Point of Load Regulation
- **Standard**: All digital I/O pins (Pins 6, 7, 9, 11–14, 16–19) operate strictly at **3.3V LVCMOS native levels**:
  - $V_{IL} \le 0.8\,\text{V}$
  - $V_{IH} \ge 2.0\,\text{V}$
  - $V_{OL} \le 0.4\,\text{V}$
  - $V_{OH} \ge 2.4\,\text{V}$
- **Point of Load (PoL) Rule**: 3.3V logic power is NOT distributed over the 20-pin bus. Addon Cards requiring 3.3V power MUST receive `VBUS` (Pins 1–4) and regulate it locally using an onboard LDO or DC-DC converter. This prevents ground loops, line drops, and ensures that Card-side power faults do not brown out the Core MCU.

### 4.4 Shielding & Signal Integrity Topology
- **Pin 5 Power/Logic Barrier**: Isolates the high-current `VBUS` cluster (wires 1–4) from `SYS_RST` (wire 6) and analog inputs (wire 7). If an IDC crimping short occurs at the power boundary, VBUS shorts directly to GND (Pin 5), safely triggering sourcing over-current protection without exposing MCU logic to 5V.
- **Pin 6 Quiet Placement**: Located between Pin 5 (GND) and Pin 7 (AIN: static analog input), isolating the sensitive reset/UPDI line from high-speed digital switching.
- **EXTCLK Ground Sandwich**: Pin 9 (`EXTCLK`) is flanked by Pin 8 (`GND`) and Pin 10 (`GND`), forming a coplanar waveguide structure to suppress clock EMI.
- **Digital Bus Segregation & SPI Shielding**: Pin 10 (`GND`) isolates clock lines from the digital communications cluster (UART/I2C/SPI). Pin 15 (`GND`) isolates the I2C open-drain bus from the SPI push-pull bus. Pin 15 and Pin 20 provide coplanar returns flanking the SPI bus group.

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
