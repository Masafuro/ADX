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

### 3.2 ADX Pinout (Unified 20-Pin Interface)
A unified 20-pin expansion connector for master and slave modules, featuring dedicated GND shielding for clock/communication lines and static control lines as noise isolation buffers.
See [ADX_pinout.md](ADX_pinout.md) for full details.

**Connector Specification:** 2×10-pin, 2.54 mm pitch shrouded header (CN2)

| Pin | Net Name | GPIO | Analog / Special | Comm / Control | Notes |
|:---:|:---|:---:|:---|:---|:---|
| **1** | *N.C.* | - | - | - | Unconnected |
| **2** | `VDD` | - | - | - | 5V Power Supply |
| **3** | `VDD` | - | - | - | 5V Power Supply |
| **4** | *N.C.* | - | - | - | Unconnected |
| **5** | `PA3/EXTCLK` | PA3 | AIN3 / PTC | **EXTCLK (External Clock)** | Note 1 |
| **6** | `GND_5V` | - | - | - | GND (EXTCLK Shield) |
| **7** | `PA6/DAC0` | PA6 | AIN6 / PTC / **DAC0 Out** | - | TCD0 WOC / AC Input |
| **8** | `PA5/AIN5` | PA5 | AIN5 / PTC / **VREFA** | - | TCA0 WO5 / AC0 Out |
| **9** | `PA7/RE` | PA7 | - | UART `\RE` Control | Analog line isolation buffer (Note 2) |
| **10** | `PA4/DE` | PA4 | - | UART `XDIR (DE)` | I2C line isolation buffer (Note 3) |
| **11** | `PB0/SCL` | PB0 | - | **I2C SCL** | - |
| **12** | `PB1/SDA` | PB1 | - | **I2C SDA** | - |
| **13** | `GND_5V` | - | - | - | GND (RxD Shield) |
| **14** | `PA2/R` | PA2 | - | **UART RxD** (Receive) | - |
| **15** | `PA1/D` | PA1 | - | **UART TxD** (Transmit) | - |
| **16** | `PC3/SS` | PC3 | - | **SPI SS** | - |
| **17** | `PC2/MOSI` | PC2 | - | **SPI MOSI** | - |
| **18** | `PC1/MISO` | PC1 | - | **SPI MISO** | - |
| **19** | `PC0/SCK` | PC0 | - | **SPI SCK** | - |
| **20** | `GND_5V` | - | - | - | GND (SCK Shield) |

*Note 1: Used as external clock input on master modules; usable as AIN3 analog input on slave modules.*  
*Note 2: Static logic level during active bus states buffers adjacent AIN5 analog input against crosstalk.*  
*Note 3: Driver enable line buffers adjacent I2C SCL clock line against switching transients.*

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

