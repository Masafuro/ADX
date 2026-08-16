<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended) - English Documentation

[ **English** | [日本語 (../ja/README.md)](../ja/README.md) ]

ADX (Advanced Devices eXtended) is an open-source hardware standard and modular control platform designed to bridge the gap between maker-friendly embedded prototyping and industrial-grade reliability, noise immunity, and electromechanical precision.

---

## 1. Specifications

### 1.1 8748 Form Factor (Board Dimensions)
An electromechanical co-design standard optimizing dimensional alignment between mil-based PCB layout and metric enclosure fabrication, minimizing rounding errors (<15 µm).
See [8748_formfactor.md](8748_formfactor.md) for full details.

* **Board Dimensions:** `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **Mounting Hole Pitch:** `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`)
* **Hole Margin / Diameter:** `5.0 mm` (`197 mil`) from edges / `Φ3.3 mm` (M3 screw / `130 mil`)
* **Corner Treatment:** C3 Chamfer (`120 mil`)

### 1.2 ADX Pinout (Unified 20-Pin Interface)
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

## 2. Hardware Lineup

### 2.1 ADX Core-D (LN-485 & Bootloader Development Board)
A core board engineered for developing LIN-based RS-485 (LN-485) communication and custom bootloader solutions.

* **MCU:** Microchip ATtiny1616-MNR (QFN-20, 5V, 20MHz)
* **LN-485 Communication:** SP485EEN-L/TR transceiver, 5.08 mm 3P pluggable terminal block, TVS diode surge protection (PSM712), selectable termination & DE/RE jumpers
* **USB / Programming:** USB Type-C + WCH CH342K (integrated SerialUPDI flashing & UART debug console)
* **Clock Source:** Onboard 12 MHz active crystal oscillator (jumper-selectable to EXTCLK)
* **Protection Circuits:** 1A PTC resettable fuse, ferrite beads, ESD protection diodes
* **Board Proposal:** [../../hardware/ADX_Core-D/proposal.md](../../hardware/ADX_Core-D/proposal.md)
* **Manufacturing Data:** [../../hardware/ADX_Core-D/data/](../../hardware/ADX_Core-D/data/)

### 2.2 ADX Prototyping CARD (Stackable Expansion Card)
A prototyping add-on card connecting directly to the ADX 20-pin header for custom circuit integration.

* **Interface:**
  - Lower Row (20-pin): Direct breakout of ADX GPIO and system bus
  - Upper Row (20-pin): Custom prototyping area breakout
* **Form Factor:** 8748 Form Factor compliant
* **Card Proposal:** [../../hardware/CARD/Prototyping/proposal.md](../../hardware/CARD/Prototyping/proposal.md)
* **Manufacturing Data:** [../../hardware/CARD/Prototyping/data/](../../hardware/CARD/Prototyping/data/)

---

## 3. Repository Structure

```text
ADX/
├── README.md               # Global portal (Summary & Language Switcher)
├── LICENSE.md              # Multi-licensing policy & trademark notices
├── LICENSES/               # REUSE-compliant license texts (CC-BY-4.0, CERN-OHL-P-2.0, MIT)
├── docs/                   # Multilingual documentation (CC BY 4.0)
│   ├── en/                 # English documentation
│   │   ├── README.md
│   │   ├── 8748_formfactor.md
│   │   └── ADX_pinout.md
│   └── ja/                 # Japanese documentation
│       ├── README.md
│       ├── 8748_formfactor.md
│       └── ADX_pinout.md
├── hardware/               # Hardware design & manufacturing files (CERN-OHL-P-v2)
│   ├── ADX_Core-D/         # LN-485 bootloader development board
│   └── CARD/Prototyping/   # Prototyping expansion card
├── firmware/               # Drivers, BSP, and sample sketches (MIT)
├── logo/                   # Brand assets & logos
├── memo/                   # Internal development notes & reports
│   └── REPORT/             # Proposals and planning documents
└── Project_Snapshot.md     # Project roadmap & progress tracker
```

---

## 4. Links & Resources

* **Project Snapshot:** [../../Project_Snapshot.md](../../Project_Snapshot.md)
* **ADX Platform Official:** [https://adxplatform.com/](https://adxplatform.com/)
* **Developer Blog:** [https://dev-blog.adxplatform.com/](https://dev-blog.adxplatform.com/)

---

## 5. License

ADX Project is released under a tri-license model:

* **Documentation & Specifications** (`docs/`, `memo/`): [CC BY 4.0](../../LICENSES/CC-BY-4.0.txt) (`CC-BY-4.0`)
* **Hardware Design & Production Files** (`hardware/`): [CERN-OHL-P-v2](../../LICENSES/CERN-OHL-P-2.0.txt) (`CERN-OHL-P-2.0`)
* **Firmware, Drivers & Software** (`firmware/`): [MIT License](../../LICENSES/MIT.txt) (`MIT`)

See [../../LICENSE.md](../../LICENSE.md) for full licensing policy and trademark terms.

