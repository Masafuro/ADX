# ADX (Advanced Devices eXtended)

[ **English (docs/en)** | [日本語 (docs/ja)](docs/ja/README.md) ]

ADX (Advanced Devices eXtended) is an open-source hardware standard and modular control platform designed to provide industrial-grade reliability, noise immunity, and electromechanical precision with the simplicity of open-source embedded development.

---

## 1. Core Specifications (Overview)

### 1.1 8748 Form Factor
An electromechanical co-design standard ensuring seamless dimensional alignment between PCB CAD (mil-based) and mechanical enclosure fabrication (mm-based), reducing conversion errors to <15 µm.

* **Dimensions:** `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **Mounting Pitch:** `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`), M3 screws (`Φ3.3 mm`)
* **Full Specification:** [English (docs/en/8748_formfactor.md)](docs/en/8748_formfactor.md) | [日本語 (docs/ja/8748_formfactor.md)](docs/ja/8748_formfactor.md)

### 1.2 ADX Pinout
A 20-pin unified expansion interface (2×10-pin, 2.54 mm pitch) featuring GND shielding and static control lines for noise isolation across SPI, I2C, UART, and analog signals.

* **Connector:** 2×10-pin 2.54 mm pitch header (CN2)
* **Full Specification:** [English (docs/en/ADX_pinout.md)](docs/en/ADX_pinout.md) | [日本語 (docs/ja/ADX_pinout.md)](docs/ja/ADX_pinout.md)

---

## 2. Hardware Lineup

| Board / Module | Description | Specifications & Manufacturing Data |
| :--- | :--- | :--- |
| **ADX Core-D** | LN-485 (LIN-based RS-485) and custom bootloader development board with USB-C, CH342K SerialUPDI flashing, and 12 MHz crystal oscillator. | [Proposal](dev/ADX_Core-D/proposal.md) / [Data](dev/ADX_Core-D/data/) |
| **ADX Prototyping CARD** | 8748 Form Factor expansion add-on card with lower 20-pin ADX bus breakout and upper 20-pin custom prototyping grid. | [Proposal](dev/CARD/Prototyping/proposal.md) / [Data](dev/CARD/Prototyping/data/) |

---

## 3. Documentation (多言語ドキュメント)

* **English Documentation:** [docs/en/README.md](docs/en/README.md)
  * [8748 Form Factor Specification](docs/en/8748_formfactor.md)
  * [ADX Pinout Specification](docs/en/ADX_pinout.md)
* **日本語ドキュメント (Japanese):** [docs/ja/README.md](docs/ja/README.md)
  * [8748 フォームファクタ仕様書](docs/ja/8748_formfactor.md)
  * [ADX ピンアサイン仕様書](docs/ja/ADX_pinout.md)

---

## 4. Repository Structure

```text
ADX/
├── README.md               # Global portal & quick reference (this file)
├── docs/                   # Full multilingual documentation
│   ├── en/                 # English documentation
│   └── ja/                 # Japanese documentation
├── dev/                    # Hardware design & production files (Gerber, BOM, etc.)
│   ├── ADX_Core-D/         # LN-485 bootloader development board
│   └── CARD/Prototyping/   # Prototyping add-on card
├── memo/                   # Development notes & reports
│   └── REPORT/             # Proposals & migration plans
└── Project_Snapshot.md     # Project progress & roadmap
```

---

## 5. Links & Resources

* **Project Snapshot:** [Project_Snapshot.md](Project_Snapshot.md)
* **ADX Platform Official:** [https://adxplatform.com/](https://adxplatform.com/)
* **Developer Blog:** [https://dev-blog.adxplatform.com/](https://dev-blog.adxplatform.com/)
