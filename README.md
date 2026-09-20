<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English (docs/en)** | [日本語 (docs/ja)](docs/ja/README.md) ]

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

| Board | Grade | Description | Status & Resources |
| :--- | :--- | :--- | :--- |
| **ADX CORE-I** | Production (Isolated) | Dual-isolated (power & communication) flagship controller for safe, unconstrained field bus integration. | In Design |
| **ADX CORE-S-P** | Professional (Non-isolated) | Ultra-low-cost ($5–7/pc) reference board for skilled developers and internal machinery. Self-fabrication (DIY) model. | [Details & Review](hardware/CORE-S-P/README.md) |
| **ADX Core-D** | Development (Non-isolated) | Differential communication board equipped with LN-485 transceiver for bootloader and firmware development. | [Proposal](hardware/Core-D/proposal.md) / [Data](hardware/Core-D/data/) |
| **ADX CORE-U** | Production | General-purpose standard board providing familiar Arduino-compatible accessibility with native enclosure integration. | Under Development |

### 2.2 CARDs (Expansion Modules)

| Board | Description | Status & Resources |
| :--- | :--- | :--- |
| **ADX Prototyping CARD** | 8748 Form Factor expansion add-on card with lower 20-pin ADX bus breakout and upper 20-pin prototyping grid. | [Proposal](hardware/CARD/Prototyping/proposal.md) / [Data](hardware/CARD/Prototyping/data/) |

---

## 3. Core Specifications (Overview)

### 3.1 8748 Form Factor
An electromechanical co-design standard ensuring dimensional alignment between PCB CAD (mil-based) and mechanical enclosure fabrication (mm-based).

* **Dimensions:** `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **Mounting Pitch:** `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`), M3 screws (`Φ3.3 mm`)
* **Corner Treatment:** C3 chamfer (`120 mil`), 5.0 mm margin from edges
* **Full Specification:** [English (docs/en/8748_formfactor.md)](docs/en/8748_formfactor.md) | [日本語 (docs/ja/8748_formfactor.md)](docs/ja/8748_formfactor.md)

### 3.2 ADX Pinout (v2)
A unified 20-pin expansion interface (2×10-pin, 2.54 mm pitch IDC, IDC Pins 1–20) optimized for ribbon cable coupling. Key v2 enhancements include 5V/2A power delivery (3× VDD pins), short-prevention isolation, co-planar GND shielding for EXTCLK and analog domains, and conflict-free UART switching via PORTMUX.

* **Connector:** 2×10-pin 2.54 mm pitch IDC ribbon connector (IDC Pins 1–20)
* **Full Specification:** [English (docs/en/ADX_pinout.md)](docs/en/ADX_pinout.md) | [日本語 (docs/ja/ADX_pinout.md)](docs/ja/ADX_pinout.md)
* **MCU Peripheral & PORTMUX Matrix:** [English (docs/en/adx_attiny1616_mcu_matrix.md)](docs/en/adx_attiny1616_mcu_matrix.md) | [日本語 (docs/ja/adx_attiny1616_mcu_matrix.md)](docs/ja/adx_attiny1616_mcu_matrix.md)

---

## 4. Documentation (多言語ドキュメント)

* **English Documentation:** [docs/en/README.md](docs/en/README.md)
  * [8748 Form Factor Specification](docs/en/8748_formfactor.md)
  * [ADX Pinout Specification (v2)](docs/en/ADX_pinout.md)
  * [ATtiny1616 Peripheral & PORTMUX Reference](docs/en/adx_attiny1616_mcu_matrix.md)
* **日本語ドキュメント (Japanese):** [docs/ja/README.md](docs/ja/README.md)
  * [8748 フォームファクタ仕様書](docs/ja/8748_formfactor.md)
  * [ADX ピンアサイン仕様書 (v2)](docs/ja/ADX_pinout.md)
  * [ATtiny1616 ペリフェラル＆PORTMUX仕様書](docs/ja/adx_attiny1616_mcu_matrix.md)

---

## 5. Repository Structure

```text
ADX/
├── README.md               # Global portal & overview (this file)
├── LICENSE.md              # Multi-licensing policy & trademark notices
├── LICENSES/               # REUSE-compliant license texts (CC-BY-4.0, CERN-OHL-P-2.0, MIT)
├── docs/                   # Full multilingual documentation (CC BY 4.0)
│   ├── en/                 # English documentation
│   └── ja/                 # Japanese documentation
├── hardware/               # Hardware design & production files (CERN-OHL-P-v2)
│   ├── ADX_Core-D/         # LN-485 bootloader development board
│   └── CARD/Prototyping/   # Prototyping add-on card
├── firmware/               # Drivers, BSP, and sample sketches (MIT)
├── logo/                   # Brand assets & logos
├── memo/                   # Development notes & planning
│   ├── what_is_adx.md      # Core vision & motivation
│   ├── PLAN/               # Roadmap & revision plans
│   └── REPORT/             # Technical proposals & evaluation reports
└── Project_Snapshot.md     # Project progress & roadmap
```

---

## 6. Links & Resources

* **Project Snapshot:** [Project_Snapshot.md](Project_Snapshot.md)
* **ADX Platform Official:** [https://adxplatform.com/](https://adxplatform.com/)
* **Developer Blog:** [https://dev-blog.adxplatform.com/](https://dev-blog.adxplatform.com/)

---

## 7. License

ADX Project is released under a tri-license structure tailored for documentation, open hardware, and embedded software:

* **Documentation & Specifications** (`docs/`, `memo/`):  
  [Creative Commons Attribution 4.0 International (CC BY 4.0)](LICENSES/CC-BY-4.0.txt)  
  *SPDX-License-Identifier: `CC-BY-4.0`*

* **Hardware Design & Production Files** (`hardware/`):  
  [CERN Open Hardware Licence Version 2 - Permissive (CERN-OHL-P-v2)](LICENSES/CERN-OHL-P-2.0.txt)  
  *SPDX-License-Identifier: `CERN-OHL-P-2.0`*

* **Firmware, Drivers & Software** (`firmware/`):  
  [MIT License](LICENSES/MIT.txt)  
  *SPDX-License-Identifier: `MIT`*

See [LICENSE.md](LICENSE.md) for full licensing policy and trademark terms.

