<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended) - English Documentation

[ **English** | [日本語 (../ja/README.md)](../ja/README.md) ]

**ADX (Advanced Devices eXtended)** is an open-source hardware standard and modular control ecosystem designed around the principle of **"Enclosure-Friendly"** engineering.

While conventional prototyping boards excel on the workbench, transitioning them into actual enclosures, installations, or field deployments often brings physical frustration—mounting screws colliding with traces, lack of washer clearance, multi-directional cable protrusions, and rigid shield stacking that cannot absorb real-world mechanical tolerances. 

ADX bridges the gap between desktop prototyping and physical deployment through the **8748 Form Factor** (`87.0 mm × 48.0 mm`) and a flexible 20-pin IDC ribbon bus. From robust, daisy-chainable field networking reminiscent of professional audio equipment to 5V high-power battery-integrated enclosed devices, ADX explores what is possible when hardware is built to fit cleanly inside 3D-printed enclosures, off-the-shelf waterproof cases, and electrical utility boxes.

---

## 1. Key Design Features

* **Enclosure-Friendly Mechanical Alignment (8748 Form Factor)**:
  `87.0 mm × 48.0 mm` outline with standardized M3 mounting holes (`77.0 mm × 38.0 mm` pitch) featuring generous $5.0\text{ mm}$ edge margins to accommodate standard screw washers without biting into PCB traces.
* **Unified Interface Edge**:
  Terminal blocks, external ports, and connectors are aligned along designated board edges, making panel cutouts and environmental sealing clean and straightforward.
* **Tolerance-Absorbing IDC Ribbon Coupling**:
  Expansion boards (CARDs) couple via a 20-pin IDC ribbon cable rather than rigid stacking pin headers. This absorbs enclosure manufacturing tolerances, allows flexible internal placement (stacked, coplanar, or folded), and eliminates pin-bending stress.
* **Safe Bidirectional 5V Power Trunk (Double Check-Valve Architecture)**:
  The Core board can either receive power from Type-C (protected by an LM66100 ideal diode) to supply 5V / 2A+ to the IDC expansion bus, or accept 5V reverse power from a Power CARD via the IDC expansion bus. Bidirectional safety is guaranteed by "Double Check Valves"—the CORE-A PC back-feed prevention diode and the mandatory Power CARD reverse-current protection circuit (MUST requirement), physically eliminating cross-feed hazards and battery damage.

---

## 2. Hardware Lineup
The ADX platform is anchored by the universal **ADX CORE-A** MCU board, the **RPR4 Smart Probe** diagnostics tool, and flexible modular **CARDs**:

### 2.1 Core Board (MCU)

| Board | Role & Power Architecture | Communication & Features | Target Applications & Status |
| :--- | :--- | :--- | :--- |
| **ADX CORE-A**<br>*(Absolute / Universal)*<br>**Target Price: $28** | **5V-Dedicated / Zero Self-Heating**<br>USB Type-C 6P (Ideal Diode LM66100 PC back-feed protection). Bidirectional 5V delivery via 20P Eject Header. | **Fully Isolated RS-485 (2500VDC)**<br>Mornsun TDA51S485HC. Direct front spring clamp terminal. Automotive AEC-Q102 LEDs (-40°C~+110°C). Secondary BMC supervisor. | Desktop, high-temp vehicle cabins, sealed IP67 cases, industrial control panels, -48V telecom infrastructure.<br>*(rev1 Netlist & Cost Locked, Pre-production)* |
| *ADX Core-D*<br>*(R&D Testbed)* | *DC 5V USB / Terminal* | *LN-485 protocol and bootloader verification board. Validated across Phases 1–5.* | *Internal R&D testbed (Not for production).* |

### 2.2 Smart Diagnostics & Programmer

| Tool | Role & Architecture | Key Features | Target Price & Status |
| :--- | :--- | :--- | :--- |
| **RPR4 Smart Probe** | **Fully Isolated RS-485 Pocket Analyzer**<br>Raspberry Pi RP2040 + Isolated RS-485 (TDA51S485HC). | **RP2040 PIO Real-time Analysis**<br>Auto-Baud detection, Modbus RTU / DMX / LN-485 decoding, dedicated Web PWA sniffer, 3-sec wireless OTW flashing. | **$25.00**<br>*(rev1 Netlist Locked, In Development)* |

### 2.3 CARDs (Modular Add-ons)

| Board | Description | Status & Resources |
| :--- | :--- | :--- |
| **AAA 2S Power CARD** | 2× AAA battery boost converter feeding clean 5V, featuring BMC-gated nanoampere sleep shutdown. | Conceptual Planning |
| **DC 12V–48V Power CARD** | Wide-input isolated DC-DC supporting factory 24V, automotive, and telecom -48V with centralized surge protection. | Conceptual Planning |
| **ADX Prototyping CARD** | 8748 Form Factor companion board with 20P Eject Header pass-through and universal prototyping grid. | [Proposal](../../hardware/CARD/Prototyping/proposal.md) / [Data](../../hardware/CARD/Prototyping/data/) |

---

## 3. Specifications

### 3.1 8748 Form Factor (Board Dimensions)
An electromechanical co-design standard optimizing dimensional alignment between mil-based PCB layout and metric enclosure fabrication, minimizing rounding errors (<15 µm).
See [8748_formfactor.md](8748_formfactor.md) for full details.

* **Board Dimensions:** `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **Mounting Hole Pitch:** `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`)
* **Hole Margin / Diameter:** `5.0 mm` (`197 mil`) from edges / `Φ3.3 mm` (M3 screw / `130 mil`)
* **Corner Treatment:** C3 Chamfer (`120 mil`)

### 3.2 ADX Pinout (Unified 20-Pin Interface / v2)
A unified 20-pin IDC expansion interface (IDC Pins 1–20) standard across master, slave, and daughterboard modules.
Key enhancements in v2 include 5V/2A power delivery, short-prevention isolation, co-planar GND shielding for EXTCLK, and full GND isolation for analog domains.
See [ADX_pinout.md](ADX_pinout.md) for full details.

**Connector Specification:** 2×10-pin, 2.54 mm pitch IDC ribbon connector (IDC Pins 1–20)

| IDC Pin | Net Name | MCU Pin | Primary Function / Role | Ribbon Cable Structure & Shielding |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **VDD** | - | **Power Supply (5V/2A Delivery)** | #1 (Power Zone: 2×2 Block) |
| **2** | **VDD** | - | **Power Supply (5V/2A Delivery)** | #2 (Power Zone: 2×2 Block) |
| **3** | **VDD** | - | **Power Supply (5V/2A Delivery)** | #3 (Power Zone: 2×2 Block) |
| **4** | **VDD** | - | **Enhanced Power (5V/2A Delivery)** | #4 (Power Zone: 2×2 Block) |
| **5** | *N.C.* | - | **Isolation (Power/Signal Short-Prevention Buffer)** | #5 (Buffer Zone) |
| **6** | PA6/DAC0 | PA6 | **DAC0** Output / AIN6 | #6 (Analog Zone) |
| **7** | PA5/AIN5 | PA5 | **VREFA** / AIN5 | #7 (Analog Zone) |
| **8** | **GND_5V** | - | **GND (EXTCLK Guard ①)** | #8 (GND Shield) |
| **9** | **PA3/EXTCLK** | PA3 | **EXTCLK** / AIN3 | #9 (EXTCLK Signal Line) |
| **10** | **GND_5V** | - | **GND (EXTCLK Guard ② / Digital Bus Isolation)** | #10 (GND Shield) |
| **11** | **PB2/TXD_EXT** | PB2 | **UART TxD (Alternate)** | #11 (UART Pair) |
| **12** | **PB3/RXD_EXT** | PB3 | **UART RxD (Alternate)** | #12 (UART Pair) |
| **13** | PB0/SCL | PB0 | **I2C SCL** | #13 (I2C Pair) |
| **14** | PB1/SDA | PB1 | **I2C SDA** | #14 (I2C Pair) |
| **15** | GND_5V | - | **GND (Inter-bus Shield)** | #15 |
| **16** | PC3/SS | PC3 | **SPI SS** | #16 (SPI Group) |
| **17** | PC2/MOSI | PC2 | **SPI MOSI** | #17 (SPI Group) |
| **18** | PC1/MISO | PC1 | **SPI MISO** | #18 (SPI Group) |
| **19** | PC0/SCK | PC0 | **SPI SCK** | #19 (SPI Group) |
| **20** | GND_5V | - | **GND (Termination Shield)** | #20 |

### 3.3 Hardware Naming Rules & Safety Classification
Defines product part numbers and the strict boundary between "Standard / Commercial Grade" and "Professional / DIY Grade (`-P`)".
See [hardware_naming_rules.md](hardware_naming_rules.md) for full details.

### 3.4 ADX Form Factor Guidelines (MUST / SHOULD / MAY)
Comprehensive design guidelines based on RFC 2119 for third parties and DIY developers designing compatible expansion cards.
See [adx_formfactor_guidelines.md](adx_formfactor_guidelines.md) for full details.

* **[MUST] Mandatory Requirements:** Reverse-current protection on Power CARDs, strict Pin 5 short-prevention isolation buffer.
* **[SHOULD] Official Standards:** Latched Eject Headers, industrial isolated DC-DC converters, 8748 Form Factor dimensions, -40°C~+105°C temperature ratings.
* **[MAY] Developer Freedom:** Non-isolated DC-DC converters, standard box headers, custom board shapes, application-specific sensors.

> * Note: For full MCU peripheral multiplexing (PWM, ADC) and PORTMUX register specifications, refer to [adx_attiny1616_mcu_matrix.md](adx_attiny1616_mcu_matrix.md).

---

## 4. Repository Structure

```text
ADX/
├── README.md               # Global portal & overview (Root)
├── LICENSE.md              # Multi-licensing policy & trademark notices
├── LICENSES/               # REUSE-compliant license texts (CC-BY-4.0, CERN-OHL-P-2.0, MIT)
├── docs/                   # Multilingual documentation (CC BY 4.0)
│   ├── en/                 # English documentation (this directory)
│   └── ja/                 # Japanese documentation
├── hardware/               # Hardware design & manufacturing files (CERN-OHL-P-v2)
│   ├── CORE-A/             # ADX CORE-A (Absolute / Fully Isolated RS-485 / 5V Dedicated)
│   ├── ADX_Core-D/         # LN-485 bootloader development board (R&D Testbed)
│   └── CARD/Prototyping/   # Prototyping expansion card
├── firmware/               # Drivers, BSP, and sample sketches (MIT)
├── logo/                   # Brand assets & logos
├── memo/                   # Development notes & planning
│   ├── what_is_adx.md      # Core vision & motivation
│   ├── PLAN/               # Roadmap & revision plans
│   └── REPORT/             # Technical proposals & evaluation reports
└── Project_Snapshot.md     # Project roadmap & progress tracker
```

---

## 5. Links & Resources

* **Project Snapshot:** [../../Project_Snapshot.md](../../Project_Snapshot.md)
* **ADX Platform Official:** [https://adxplatform.com/](https://adxplatform.com/)
* **Developer Blog:** [https://dev-blog.adxplatform.com/](https://dev-blog.adxplatform.com/)

---

## 6. License

ADX Project is released under a tri-license model:

* **Documentation & Specifications** (`docs/`, `memo/`): [CC BY 4.0](../../LICENSES/CC-BY-4.0.txt) (`CC-BY-4.0`)
* **Hardware Design & Production Files** (`hardware/`): [CERN-OHL-P-v2](../../LICENSES/CERN-OHL-P-2.0.txt) (`CERN-OHL-P-v2`)
* **Firmware, Drivers & Software** (`firmware/`): [MIT License](../../LICENSES/MIT.txt) (`MIT`)

See [../../LICENSE.md](../../LICENSE.md) for full licensing policy and trademark terms.

