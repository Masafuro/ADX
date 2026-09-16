<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout Specification (v2)

[ **English** | [日本語 (../ja/ADX_pinout.md)](../ja/ADX_pinout.md) ]

## 1. Overview
This document defines the 2×10-pin IDC connector pinout (schematic reference designator: `CN2`) and the onboard peripheral (RS-485, BMC boot control, onboard LED, UPDI) and PORTMUX routing specifications (v2) for the ADX microcomputer board powered by the Microchip ATtiny1616-MNR. Connector pin numbers are standardized as **"IDC Pin 1–20"**.

---

## 2. ATtiny1616 Pin Allocation & PORTMUX Switching

### ① Dedicated Onboard Pins (Non-IDC breakout: 7 pins)
Out of the 18 MCU GPIOs, the following 7 pins are exclusively dedicated to onboard peripheral circuits:

- **RS-485 Transceiver Circuit (4 pins)**:
  - `PA1`: USART0 Default TxD (DI)
  - `PA2`: USART0 Default RxD (RO)
  - `PA4`: USART0 Default XDIR (DE - Driver Enable)
  - `PA7`: GPIO Control (RE - Receiver Enable)
- **Onboard Management, Indicator & Debug Circuit (3 pins)**:
  - `PA0`: UPDI / RESET (Onboard THT breakout, programming and hardware debugging)
  - `PB4`: Onboard LED (Red, Active HIGH)
  - `PB5`: BOOT_REQ (Bootloader request input from onboard BMC, pull-down & series protection resistor equipped)

### ② Hardware UART Routing via PORTMUX
The USART0 module can dynamically switch its pin routing between onboard and external interfaces via the `PORTMUX.CTRLB` register:

- **`PORTMUX.CTRLB.USART0 = 0` (Default Position)**:
  - Routes USART0 communication to the onboard **RS-485 transceiver (PA1 / PA2)**.
  - Enables hardware auto-direction control (`XDIR` / `PA4`) and LIN break detection / auto-baud (`LINAUTO`).
- **`PORTMUX.CTRLB.USART0 = 1` (Alternate Position)**:
  - Routes USART0 to the **IDC connector (PB2 / PB3: IDC Pins 11, 12)** for direct external UART communication.
  - Mutually exclusive with onboard RS-485, preventing bus collisions and signal contention.

---

## 3. IDC 2x10 Connector Pinout Specification (v2)

### Key Electrical & Structural Benefits
1. **Enhanced Power Delivery (Pins 1–3)**: Triple parallel VDD lines provide robust **5V / 2A** power supply capacity to ribbon-cable-connected daughterboards.
2. **Short-Prevention Isolation (Pin 4 N.C.)**: An unconnected buffer pin between the VDD zone (Pin 3) and GND (Pin 5) physically prevents catastrophic shorts during ribbon cable IDC crimping or alignment errors.
3. **Co-planar Clock Shielding (Pin 6)**: The high-speed clock line `EXTCLK` (Pin 6) is sandwiched between dedicated ground lines (Pins 5 and 7), completely suppressing EMI and radiation.
4. **Isolated Analog Domain (Pins 8, 9 & Pin 10 GND)**: Precision analog signals (`DAC0`, `AIN5`) are bracketed between Pin 7 (GND) and the newly added Pin 10 (GND), isolating them from digital communication switching transients.
5. **Unified Bus Grouping (Pins 11–20)**: Preserves adjacent signal pairs for UART (PB2/PB3), I2C (PB0/PB1), and SPI (PC0–PC3) separated by intermediate shields.

| IDC Pin | Net Name | MCU Pin | Primary Function / Role | Ribbon Cable Structure & Shielding |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **VDD** | - | **Power Supply (5V/2A Delivery)** | #1 (Power Zone) |
| **2** | **VDD** | - | **Power Supply (5V/2A Delivery)** | #2 (Power Zone) |
| **3** | **VDD** | - | **Enhanced Power (5V/2A Delivery)** | #3 (Power Zone) |
| **4** | *N.C.* | - | **Isolation (Power/GND Short-Prevention Buffer)** | #4 (Buffer Zone) |
| **5** | **GND_5V** | - | **GND (EXTCLK Guard ①)** | #5 (GND Shield) |
| **6** | **PA3/EXTCLK** | PA3 | **EXTCLK** / AIN3 | #6 (EXTCLK Signal Line) |
| **7** | **GND_5V** | - | **GND (EXTCLK Guard ②)** | #7 (GND Shield) |
| **8** | PA6/DAC0 | PA6 | **DAC0** Output / AIN6 | #8 (Analog Zone) |
| **9** | PA5/AIN5 | PA5 | **VREFA** / AIN5 | #9 (Analog Zone) |
| **10** | **GND_5V** | - | **GND (Analog/Digital Isolation Guard - New)** | #10 (GND Shield) |
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

> **Note (Pin Header / Cable Mapping):**  
> ※ IDC connector pin numbers follow standard MIL-spec IDC ribbon cable zigzag numbering (where conductor #1 to #20 maps 1:1 to pin numbers 1 to 20). Take care when assigning CAD schematic symbols and footprints not to confuse this with dual-in-line (DIP) sequential numbering.

---

## 4. IDC Major Peripheral Quick Reference

Quick summary of key peripheral capabilities available directly on the IDC expansion connector:

| IDC Pin | MCU Pin | Primary Comm Bus | Available PWM Timers | Analog / Special Features |
| :---: | :---: | :--- | :--- | :--- |
| **Pin 6** | PA3 | - | TCB1 / TCA0 WO3 | **EXTCLK** (External Clock) / ADC0 AIN3 |
| **Pin 8** | PA6 | - | - | **DAC0** Output / ADC0 AIN6 / ADC1 AIN2 |
| **Pin 9** | PA5 | - | **TCD0 WOB** (High-Freq) / TCB0 / TCA0 WO5 | **VREFA** (Ext Voltage Ref) / ADC0 AIN5 / ADC1 AIN1 |
| **Pin 11** | PB2 | **UART TxD** (Alt) | TCA0 WO2 | EVOUT1 (Event System Output) |
| **Pin 12** | PB3 | **UART RxD** (Alt) | TCA0 WO0 (Alt: Split) | TOSC1 |
| **Pin 13** | PB0 | **I2C SCL** | TCA0 WO0 | ADC0 AIN11 |
| **Pin 14** | PB1 | **I2C SDA** | TCA0 WO1 | ADC0 AIN10 |
| **Pin 16** | PC3 | **SPI SS** | TCA0 WO3 (Alt: Split) | ADC1 AIN9 |
| **Pin 17** | PC2 | **SPI MOSI** | - | EVOUT2 / ADC1 AIN8 |
| **Pin 18** | PC1 | **SPI MISO** | **TCD0 WOD** (High-Freq) | ADC1 AIN7 |
| **Pin 19** | PC0 | **SPI SCK** | **TCD0 WOC** (High-Freq) / TCB0 (Alt) | ADC1 AIN6 |

---

## 5. Related Documents
For the full MCU peripheral multiplexing matrix (CCL, PTC touch channels, analog comparators) and comprehensive PORTMUX register configuration guidelines:

- **MCU Peripheral & PORTMUX Matrix Specification:** [`adx_attiny1616_mcu_matrix.md`](adx_attiny1616_mcu_matrix.md)
