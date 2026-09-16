<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended) - English Documentation

[ **English** | [日本語 (../ja/README.md)](../ja/README.md) ]

**ADX (Advanced Devices eXtended)** is an open-source hardware standard and modular control platform focused on electromechanical enclosure fit and field installation.

While conventional prototyping boards offer great accessibility on the benchtop, mounting them into real-world enclosures often presents mechanical challenges—such as tight screw clearances, multi-directional cabling, and rigid stacking constraints. ADX addresses these integration challenges through the **8748 Form Factor** and a flexible IDC-based expansion bus, providing reliable M3 screw mounting, a unified wiring interface edge, and tolerance-friendly packaging compatible with 3D-printed enclosures, electrical boxes, and waterproof casings, all while maintaining familiar open-source embedded workflows.

---

## 1. Key Design Features

* **Secure M3 Fastening with Washer Clearance**:
  Mounting holes ($\Phi 3.3\text{ mm}$) are positioned with $5.0\text{ mm}$ edge margins, ensuring ample clearance for standard washers and reliable torque application without damaging PCB traces.
* **Unified Interface Edge**:
  Terminal blocks and external connections are aligned along designated board edges, simplifying enclosure cutouts and sealing.
* **Flexible Coupling via IDC Ribbon Cable**:
  Expansion boards connect via a 20-pin IDC ribbon cable (ADX Pinout) rather than rigid pin headers. This absorbs mechanical tolerances, allows flexible internal placement (stacked, planar, or angled), and supports daisy-chaining.
* **Enclosure Compatibility**:
  Designed to mount cleanly into off-the-shelf waterproof casings, industrial electrical utility boxes, desktop 3D-printed enclosures, and dedicated slot-in guide rails.

---

## 2. Hardware Lineup

The ADX ecosystem consists of **Core Boards** (main processing units) and **CARDs** (stackable/chainable add-on modules), sharing the same 8748 Form Factor and 20-pin expansion bus.

### 2.1 Core Boards (MCU)

| Board | Description | Status & Resources |
| :--- | :--- | :--- |
| **ADX CORE-U** | General-purpose standard board providing familiar Arduino-compatible accessibility with native enclosure integration. | Under Development |
| **ADX Core-D** | Differential communication board equipped with LN-485 (LIN-based RS-485) transceiver for high-noise and long-distance field buses. | [Proposal](../../hardware/ADX_Core-D/proposal.md) / [Data](../../hardware/ADX_Core-D/data/) |

### 2.2 CARDs (Expansion Modules)

| Board | Description | Status & Resources |
| :--- | :--- | :--- |
| **ADX Prototyping CARD** | 8748 Form Factor expansion add-on card with lower 20-pin ADX bus breakout and upper 20-pin prototyping grid. | [Proposal](../../hardware/CARD/Prototyping/proposal.md) / [Data](../../hardware/CARD/Prototyping/data/) |

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
│   ├── ADX_Core-D/         # LN-485 bootloader development board
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

