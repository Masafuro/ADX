# ADX Pinout Specification

[ **English** | [日本語 (../ja/ADX_pinout.md)](../ja/ADX_pinout.md) ]

## Overview
The ADX Pinout is a unified 20-pin (2×10-pin, 2.54 mm pitch) expansion connector standard designed for both master and slave boards.
To minimize signal crosstalk during ribbon cable and stack interconnects, the pinout incorporates dedicated GND shielding for sensitive clock/data signals and uses static control lines to buffer analog and high-speed communication signals.

---

## Connector Specification
* **Type:** 2×10-pin, 2.54 mm pitch shrouded pin header / female socket (CN2)
* **Power Supply:** 5V (VDD), Ground (GND_5V)

---

## Pin Assignment

| Pin | Net Name | GPIO | Analog / Special | PWM | Communication / Control | Notes |
|:---:|:---|:---:|:---|:---:|:---|:---|
| **1** | *N.C.* | - | - | - | - | Unconnected |
| **2** | `VDD` | - | - | - | - | 5V Power Supply |
| **3** | `VDD` | - | - | - | - | 5V Power Supply |
| **4** | *N.C.* | - | - | - | - | Unconnected |
| **5** | `PA3/EXTCLK` | PA3 | AIN3 / PTC | - | **EXTCLK** (External Clock) | Note 1 |
| **6** | `GND_5V` | - | - | - | - | GND (EXTCLK Shield) |
| **7** | `PA6/DAC0` | PA6 | AIN6 / PTC / **DAC0 Out** | TCD0 WOC | - | AC Input |
| **8** | `PA5/AIN5` | PA5 | AIN5 / PTC | TCA0 WO5 etc. | - | **VREFA**, AC0 Out |
| **9** | `PA7/RE` | PA7 | - | - | UART `\RE` Control | Crosstalk buffer for analog lines (Note 2) |
| **10** | `PA4/DE` | PA4 | - | - | UART `XDIR` (DE) | Crosstalk buffer for I2C lines (Note 3) |
| **11** | `PB0/SCL` | PB0 | - | - | **I2C SCL** | - |
| **12** | `PB1/SDA` | PB1 | - | - | **I2C SDA** | - |
| **13** | `GND_5V` | - | - | - | - | GND (RxD Shield) |
| **14** | `PA2/R` | PA2 | - | - | **UART RxD** (Receive) | - |
| **15** | `PA1/D` | PA1 | - | - | **UART TxD** (Transmit) | - |
| **16** | `PC3/SS` | PC3 | - | - | **SPI SS** | - |
| **17** | `PC2/MOSI` | PC2 | - | - | **SPI MOSI** | - |
| **18** | `PC1/MISO` | PC1 | - | - | **SPI MISO** | - |
| **19** | `PC0/SCK` | PC0 | - | - | **SPI SCK** | - |
| **20** | `GND_5V` | - | - | - | - | GND (SCK Shield) |

---

## Technical Notes
* **Note 1 (PA3 / EXTCLK / AIN3):** Serves as the external clock input (`EXTCLK`) when configured as a master board. In slave configurations where an external clock is unnecessary, this pin functions as a standard analog input (`AIN3`).
* **Note 2 (PA7 / RE):** Receiver enable control line. Since its logic state is static during continuous transmission/reception, it acts as an isolation buffer to protect adjacent analog input (`AIN5`) from high-speed digital switching noise.
* **Note 3 (PA4 / DE):** Driver enable control line. Its stable state during bus idle/transmission minimizes digital coupling into the adjacent I2C clock line (`SCL`).
