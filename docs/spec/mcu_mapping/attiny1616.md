<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Hardware Mapping: Microchip ATtiny1616
(Target Boards: ADX CORE-A, ADX CORE-A5)

**Standard Reference**: [ADX 20-Pin Bus Specification (ADX_PINOUT.md)](../ADX_PINOUT.md)  
**Target MCU**: Microchip ATtiny1616-MNR (QFN-20 / SOIC-20)  
**Operating Condition**: 3.3V DC / 10 MHz internal high-frequency oscillator  

---

## 1. Overview

This document specifies how the universal signals of the **ADX 20-Pin Bus** are mapped to the physical pins, peripherals, and PORTMUX channels of the **Microchip ATtiny1616** microcontroller.

---

## 2. Bus to MCU Physical Interconnect Table

| IDC Pin | Bus Net Name | ATtiny1616 Pin | QFN Pin | Primary MCU Peripheral & Alternate Functions |
| :---: | :--- | :--- | :---: | :--- |
| **1** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) |
| **2** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) |
| **3** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) |
| **4** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) |
| **5** | **N.C.** | - | - | Isolation Barrier (No Connection) |
| **6** | **AOUT** | **PA6** | 7 | **DAC0 OUT** / ADC0 AIN6 / ADC1 AIN2 |
| **7** | **AIN** | **PA5** | 6 | **VREFA** / ADC0 AIN5 / **TCD0 WOB** / TCB0 WO |
| **8** | **GND** | - | - | Ground Return / Coplanar Shield |
| **9** | **EXTCLK** | **PA3** | 2 | **EXTCLK** / ADC0 AIN3 / TCB1 WO |
| **10**| **GND** | - | - | Ground Return / Inter-bus Isolation |
| **11**| **UART_TX** | **PB2** | 12 | **USART0 TxD (Alternate)** / TCA0 WO2 |
| **12**| **UART_RX** | **PB3** | 11 | **USART0 RxD (Alternate)** / TCA0 WO0 |
| **13**| **I2C_SCL** | **PB0** | 14 | **TWI0 SCL (Default)** / TCA0 WO0 |
| **14**| **I2C_SDA** | **PB1** | 13 | **TWI0 SDA (Default)** / TCA0 WO1 |
| **15**| **GND** | - | - | Ground Return / Mid-bus Shield |
| **16**| **SPI_CS** | **PC3** | 18 | **SPI0 SS (Alternate)** / TCA0 WO3 |
| **17**| **SPI_MOSI**| **PC2** | 17 | **SPI0 MOSI (Alternate)** / EVOUT2 |
| **18**| **SPI_MISO**| **PC1** | 16 | **SPI0 MISO (Alternate)** / TCD0 WOD |
| **19**| **SPI_SCK** | **PC0** | 15 | **SPI0 SCK (Alternate)** / TCD0 WOC / TCB0 WO |
| **20**| **GND** | - | - | Ground Return / Bus Termination Shield |

---

## 3. Dedicated Onboard Pins (Non-Exposed to IDC Bus)

The following 7 GPIO pins are reserved for dedicated onboard circuitry:

| MCU Pin | QFN Pin | Dedicated Board Function | Electrical Connection & Notes |
| :--- | :---: | :--- | :--- |
| **PA0** | 19 | **UPDI / RESET** | Dedicated through-hole programming pad. |
| **PA1** | 20 | **RS-485 TxD** | USART0 Default TxD (DI pin of RS-485 transceiver). |
| **PA2** | 1 | **RS-485 RxD** | USART0 Default RxD (RO pin) ➜ Hardwired to **CCL LUT0-IN2**. |
| **PA4** | 5 | **RS-485 DE** | Hardware Auto-direction (`XDIR`) / Driver Enable. |
| **PA7** | 8 | **RS-485 RE_** | Receiver Enable (Active Low, GPIO control). |
| **PB4** | 10 | **Red LED** | Active HIGH onboard status indicator. |
| **PB5** | 9 | **Blue LED** | Active HIGH onboard status/comm indicator. *(Formerly BOOT_REQ in rev1)* |

---

## 4. PORTMUX Hardware Configuration Rules

To route serial and timer peripherals to the universal 20-pin IDC bus without conflicting with the dedicated onboard RS-485 transceiver, firmware MUST apply the following register settings:

| Register | Bit / Field | Required Setting | Value | Rationale |
| :--- | :--- | :--- | :---: | :--- |
| **`PORTMUX.CTRLB`** | **`SPI0`** (bit 2) | **Alternate** | `1` | Default (PA1–PA4) conflicts with RS-485. Routes SPI to **PC0–PC3 (Pins 16–19)**. |
| **`PORTMUX.CTRLB`** | **`TWI0`** (bit 4) | **Default** | `0` | Default routes I2C to **PB0/PB1 (Pins 13, 14)**. Alternate conflicts with RS-485. |
| **`PORTMUX.CTRLB`** | **`USART0`** (bit 0) | **Dynamic** | `0` or `1` | `0`: Onboard RS-485 (`PA1/PA2`)<br>`1`: External IDC Bus (`PB2/PB3`, Pins 11, 12) |
| **`PORTMUX.CTRLD`** | **`TCB0`** (bit 0) | **User Select** | `0` or `1` | `0`: Route to Pin 7 (`PA5`)<br>`1`: Route to Pin 19 (`PC0`) |
| **`PORTMUX.CTRLD`** | **`TCB1`** (bit 1) | **Default** | `0` | Default routes TCB1 PWM output to Pin 9 (`PA3`). |

---

## 5. Clocking & Thermal Considerations

- **Supply Voltage**: Powered at **3.3V DC** via onboard LDO.
- **Operating Frequency**: Standard rated at **10 MHz** (OSC20M divided by 2).
  - Maintains strict compliance with Microchip Safe Operating Area down to -40°C and up to +105°C.
  - Keeps MCU self-heating virtually zero ($< 1\,\text{mW}$), maximizing thermal headroom in sealed enclosures.
