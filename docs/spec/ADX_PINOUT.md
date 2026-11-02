<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus Specification

**Standard Identifier**: ADX-BUS-SPEC-v4.0  
**Category**: Core Electrical & Interconnect Specification (Single Source of Truth)  
**Status**: Standard (Release)  
**Maintained by**: ADX Hardware Standards Committee  

---

## 1. Overview & Scope

The **ADX 20-Pin Bus** is the universal physical and electrical interconnect standard for the ADX modular hardware ecosystem. It establishes a unified, vibration-resistant backplane interface connecting **Core Boards** (main MCUs and host controllers) and **Card Modules** (expansion addons, sensors, actuators, and communication cards).

This document serves as the authoritative **Single Source of Truth (SSOT)** for physical pin assignments, electrical limits, signal definitions, and mandatory design rules for the 20-pin IDC interface.

---

## 2. Physical & Mechanical Interface

- **Connector Family**: $2 \times 10$-pin (20-pin), 2.54 mm (0.100") pitch shrouded box header or ejector/latch header (MIL-DTL-83503 compliant).
- **Mating Cable**: Standard 20-conductor flat ribbon cable (AWG 28 stranded).
- **Pin Numbering Convention**: Standard zigzag (interleaved) ribbon cable numbering:
  - Row 1 (Odd pins): 1, 3, 5, 7, 9, 11, 13, 15, 17, 19
  - Row 2 (Even pins): 2, 4, 6, 8, 10, 12, 14, 16, 18, 20

```text
                  【 20-Pin Shrouded Box Header (Top View) 】

                         ┌───────[ Center Key ]───────┐
              GND   [ 1]  │  ●   ●  │  [ 2]  OWD / PROG
              AIN   [ 3]  │  ●   ●  │  [ 4]  GND
           EXTCLK   [ 5]  │  ●   ●  │  [ 6]  GND
          UART_TX   [ 7]  │  ●   ●  │  [ 8]  UART_RX
          I2C_SCL   [ 9]  │  ●   ●  │  [10]  I2C_SDA
              GND   [11]  │  ●   ●  │  [12]  SPI_CS
         SPI_MOSI   [13]  │  ●   ●  │  [14]  SPI_MISO
          SPI_SCK   [15]  │  ●   ●  │  [16]  GND
             VBUS   [17]  │  ●   ●  │  [18]  VBUS
             VBUS   [19]  │  ●   ●  │  [20]  VBUS
                         └────────────────────────────┘
```

---

## 3. Universal Pin Assignment Table

| Pin | Net Name | Signal Type | Electrical Standard | Primary Function / Role | Shield & Topology Role |
| :---: | :--- | :--- | :--- | :--- | :--- |
| **1** | **GND** | Power / Shield | 0V Return | Cable Outer Edge Shield & Signal Return | Wire #1 (Edge ESD / Mechanical Guard) |
| **2** | **OWD** | Bi-dir (1-Wire)| 3.3V LVCMOS (Weak PU) | One-Wire Debug & Production Flash (UPDI / SDI) | Wire #2 (Quasi-static Test Line) |
| **3** | **AIN** | Analog In | 0V – 3.3V Analog | High-Precision Analog Input (ADC) / Opt. DAC | Wire #3 (Coplanar Shielded by Pins 1 & 4) |
| **4** | **GND** | Power / Shield | 0V Return | Analog / Clock Isolation Shield | Wire #4 (EXTCLK Coplanar Guard ①) |
| **5** | **EXTCLK** | Digital / GPIO | 3.3V LVCMOS | External Clock Input / High-Speed Timer / PWM | Wire #5 (Clock Waveguide Trace) |
| **6** | **GND** | Power / Shield | 0V Return | Clock / Communications Isolation Shield | Wire #6 (EXTCLK Coplanar Guard ②) |
| **7** | **UART_TX** | Digital Out | 3.3V LVCMOS | Asynchronous Serial Transmit (TXD) | Wire #7 (UART Balanced Pair) |
| **8** | **UART_RX** | Digital In | 3.3V LVCMOS | Asynchronous Serial Receive (RXD) | Wire #8 (UART Balanced Pair) |
| **9** | **I2C_SCL** | Open-Drain | 3.3V LVCMOS (Pull-up req.)| I2C Bus Serial Clock | Wire #9 (I2C Bus Pair) |
| **10** | **I2C_SDA** | Open-Drain | 3.3V LVCMOS (Pull-up req.)| I2C Bus Serial Data | Wire #10 (I2C Bus Pair) |
| **11** | **GND** | Power / Shield | 0V Return | Mid-bus Shield (I2C / SPI Group Boundary) | Wire #11 (Open-Drain to Push-Pull Barrier) |
| **12** | **SPI_CS** | Digital Out | 3.3V LVCMOS | SPI Chip Select (Active LOW) | Wire #12 (SPI Bus Group) |
| **13** | **SPI_MOSI**| Digital Out | 3.3V LVCMOS | SPI Controller Out / Peripheral In | Wire #13 (SPI Bus Group) |
| **14** | **SPI_MISO**| Digital In | 3.3V LVCMOS | SPI Controller In / Peripheral Out | Wire #14 (SPI Bus Group) |
| **15** | **SPI_SCK** | Digital Out | 3.3V LVCMOS | SPI Serial Clock | Wire #15 (SPI Bus Group) |
| **16** | **GND** | Power / Shield | 0V Return | VBUS Short-Circuit Barrier Wall & SPI Return | Wire #16 (Protective Boundary Wall) |
| **17** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Gravity Fail-Safe Cluster) | Wire #17 ($2 \times 2$ Solid Power Plane) |
| **18** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Gravity Fail-Safe Cluster) | Wire #18 ($2 \times 2$ Solid Power Plane) |
| **19** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Gravity Fail-Safe Cluster) | Wire #19 ($2 \times 2$ Solid Power Plane) |
| **20** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Gravity Fail-Safe Cluster) | Wire #20 ($2 \times 2$ Solid Power Plane) |

---

## 4. Electrical Requirements & Design Rules

### 4.1 Gravity Fail-Safe Power Architecture (`VBUS`: Pins 17–20)
- **Bottom-Heavy Physical Placement**:  
  Pins 17, 18, 19, and 20 are positioned at the lowest physical row of the connector. When modules are slotted vertically (e.g., in DIN-rail enclosures or vertical chassis), high-current and external power wiring remains at the bottom of the assembly. If an external supply wire disconnects due to mechanical vibration or screw-terminal loosening, gravity pulls the conductor downward toward the chassis floor or wiring duct, physically preventing unintended contact with delicate communication lines (RS-485, I2C, SPI) or analog inputs located higher on the assembly.
- **Operating Voltage Window**: Nominal $5.0\,\text{V} \pm 10\%$ ($4.5\,\text{V} \sim 5.0\,\text{V}$ operating range).
- **Current Carrying Capacity**: Rated for **$2.0\,\text{A} \sim 3.0\,\text{A}$ continuous** across Pins 17–20 combined.
- **$2 \times 2$ Grid Copper Plane**: Pins 17, 18, 19, and 20 form a symmetrical $2 \times 2$ square grid. PCB layouts MUST bond all four pins together into a continuous solid copper pour on outer and inner layers to minimize IR drop and parasitic inductance.
- **Mandatory Sourcing Protection (MUST)**:
  Any module injecting power into `VBUS` (Power Cards, Core Board external supplies, USB inputs) MUST incorporate:
  1. **Reverse-Current Blocking**: Schottky barrier diodes, ideal diode controllers, or P-channel MOSFET switches to prevent reverse back-feeding into external sources.
  2. **Short-Circuit / Over-Current Limitation**: Self-recovering PTC devices, eFuses, or current-limited load switches capable of isolating faults within microsecond thresholds.

### 4.2 Protective Boundary Wall (`Pin 16: GND`)
- **Fault Isolation Barrier**: Pin 16 is dedicated as a solid `GND` return directly flanking the high-power `VBUS` cluster (Pins 17–20).
- **Crimp Misalignment Protection**: In flat ribbon cabling, should a manufacturing crimp misalignment or conductor fray occur along the power boundary, `VBUS` shorts directly to `GND` (Pin 16). This safely triggers upstream over-current protection rather than exposing low-voltage logic lines (Pins 1–15) to $5\,\text{V}$ bus power.

### 4.3 One-Wire Debug & Production Flash Interface (`Pin 2: OWD`)
- **Signal Definition**: 3.3V LVCMOS bidirectional 1-wire serial debug and programming interface with an onboard weak pull-up ($10\,\text{k}\Omega \sim 47\,\text{k}\Omega$ to 3.3V) on the Core board.
- **Primary Function**:
  1. Automated functional testing, unique ID (UID) readout, and factory firmware / bootloader flashing via the 20-pin IDC Eject Header test bench fixture during manufacturing.
  2. Direct hardware debugger connection (WCH-LinkE, SerialUPDI, or 1-Wire SWD probe).
- **Card-Side Mandatory Rules (MUST)**:
  - Addon expansion Cards **MUST leave Pin 2 unconnected (N.C.)** under normal operation.
  - Dedicated factory test fixtures or debugging adapters are the sole modules permitted to drive Pin 2.
  - **Direct Grounding Prohibited**: Cards MUST NOT connect push-buttons or hardware pull-down switches directly between Pin 2 and GND.
- **System Reset Architecture**:
  System-level resets are governed via primary `VBUS` power cycling, internal watchdog timers (WDT), or verified protocol commands over communication buses (USB / RS-485).

### 4.4 Coplanar Shielded Analog Channel (`Pin 3: AIN`)
- **Coplanar Ground Guard**: Pin 3 (`AIN`) is flanked by Pin 1 (`GND`) on the outer cable perimeter and Pin 4 (`GND`) on the inner boundary.
- **Noise Immunity**: This topology forms a coplanar waveguide shield that isolates high-impedance, delicate analog measurements from adjacent high-speed switching noise (such as `EXTCLK` on Pin 5) and external electrostatic interference.
- **Signal Range**: $0\,\text{V} \sim 3.3\,\text{V}$ single-ended analog input for ADC acquisition. Where host MCUs support pin multiplexing (e.g., ATtiny1616), Pin 3 may optionally function as a true analog DAC output.

### 4.5 Digital Signal Integrity & Logic Levels
- **Clock Isolation Sandwich**: Pin 5 (`EXTCLK`) is guarded by Pin 4 (`GND`) and Pin 6 (`GND`), suppressing clock harmonic radiation (EMI).
- **Bus Group Segregation**: Pin 11 (`GND`) separates open-drain, pull-up dependent lines (I2C) from high-speed push-pull lines (SPI).
- **Native 3.3V LVCMOS Levels**: All digital I/O lines (Pins 2, 5, 7–10, 12–15) operate strictly at **3.3V LVCMOS**:
  - $V_{IL} \le 0.8\,\text{V}$
  - $V_{IH} \ge 2.0\,\text{V}$
  - $V_{OL} \le 0.4\,\text{V}$
  - $V_{OH} \ge 2.4\,\text{V}$
- **Point of Load (PoL) Regulation Rule**: 3.3V power is NOT distributed across the 20-pin bus. Addon Cards requiring 3.3V logic supply MUST receive `VBUS` (Pins 17–20) and regulate down locally using dedicated on-card LDO or DC-DC stages.

---

## 5. Implementation Mapping per Controller Architecture

For detailed pin multiplexing (PORTMUX), timer mapping, and register-level configurations for each supported controller, refer to the dedicated specification references:

- **Microchip ATtiny1616 (CORE-A5)**: [`mcu_mapping/attiny1616.md`](mcu_mapping/attiny1616.md)
- **WCH CH32X035 (CORE-CX3)**: [`mcu_mapping/ch32x035.md`](mcu_mapping/ch32x035.md)
- **Raspberry Pi RP2040 (CORE-R)**: [`mcu_mapping/rp2040.md`](mcu_mapping/rp2040.md)

---

## 6. Document Architecture & Loose Coupling Note

To maintain absolute specification integrity, this document (`docs/spec/ADX_PINOUT.md`) is maintained strictly in **English as the Single Source of Truth (SSOT)**.

Explanatory architecture guides in Japanese ([`docs/ja/ADX_pinout.md`](../ja/ADX_pinout.md)) and English ([`docs/en/ADX_pinout.md`](../en/ADX_pinout.md)) provide background design rationale, historical context, and application guidelines without duplicating authoritative tables, ensuring all future specification revisions are maintained in exactly one place.
