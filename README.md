<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended)

[ **English (docs/en)** | [日本語 (docs/ja)](docs/ja/README.md) ]

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
* **Single-Directional Protected Power Flow**:
  Strict architectural power routing: core power is fed through designated, protected input gateways, while the 20-pin IDC expansion bus serves strictly as an outbound power delivery bus (5V / 2A+), preventing hazardous cross-feed and back-power collisions.

---

## 2. Hardware Lineup

The ADX platform is anchored by two complementary Core MCU boards sharing the 8748 Form Factor, accompanied by flexible expansion CARDs:

### 2.1 Core Boards (MCU)

| Board | Role & Power Architecture | Communication & Features | Target Applications & Status |
| :--- | :--- | :--- | :--- |
| **ADX CORE-S**<br>*(Standard / Field-Ready)* | **DC 12V / 24V Wide Input**<br>Onboard step-down DC-DC (TPS5430), switched 5V power gating via dedicated BMC supervisor (ATtiny412). | **Daisy-Chainable LN-485 (RS-485)**<br>Hardware LIN-assisted differential bus for multi-node networks, robust against field noise and long runs. | Architectural lighting, stage & interactive art installations, outdoor exhibits, multi-drop field sensing.<br>*(In Review / Pre-production)* |
| **ADX CORE-U**<br>*(Universal / Utility)* | **DC 5V High-Power (2A+)**<br>DCDC omitted to reinvest in rock-solid protection: **eFuse + 12V Overvoltage Protection (OVP)**. Single DC jack with physical interlock. | **USB Type-C (UPDI / Dual UART)**<br>WCH CH342K for seamless flashing and debugging. Native 5V ATtiny1616 logic. | Enclosure-integrated standalone devices, robotics & servo control, battery-powered portable IoT via CARDs.<br>*(rev0 Conceptualized)* |
| *ADX Core-D*<br>*(R&D Testbed)* | *DC 5V USB / Terminal* | *LN-485 protocol and bootloader verification board. Validated across Phases 1–5.* | *Internal R&D testbed (Not for production).* |

### 2.2 CARDs (Expansion Modules)

| Board | Description | Status & Resources |
| :--- | :--- | :--- |
| **ADX Prototyping CARD** | 8748 Form Factor companion board with lower 20-pin ADX bus breakout and upper 20-pin prototyping grid. | [Proposal](hardware/CARD/Prototyping/proposal.md) / [Data](hardware/CARD/Prototyping/data/) |
| **ADX Battery / Power CARD** | External battery regulation (LiPo / Dry Cell) feeding 5V into CORE-U for fully enclosed battery-powered systems. | Conceptual Planning |

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
│   ├── CORE-S/             # Standard field-ready MCU board (12V/24V, LN-485)
│   ├── CORE-U/             # Universal 5V/2A eFuse protected board
│   ├── Core-D/             # LN-485 R&D verification testbed
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

