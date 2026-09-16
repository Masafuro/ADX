<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ATtiny1616 Peripheral Functions & PORTMUX Integrated Reference

[ **English** | [日本語 (../ja/adx_attiny1616_mcu_matrix.md)](../ja/adx_attiny1616_mcu_matrix.md) ]

## 1. Overview
This document serves as an integrated technical reference defining the full MCU pin multiplexing and **PORTMUX (Port Multiplexer) register configuration rules** for the ADX microcomputer board based on the Microchip **ATtiny1616-MNR (QFN-20)**.

For physical pinout definitions and ribbon cable shielding structure, refer to the [ADX Pinout Specification (v2)](ADX_pinout.md). Connector pin assignments are referenced as **"IDC Pin 1–20"** (schematic reference designator: `CN2`).

---

## 2. PORTMUX Design Rules on ADX Board

The ATtiny1616 provides highly flexible I/O multiplexing via PORTMUX. However, on the ADX board, 7 MCU pins are hard-wired to dedicated onboard circuits (RS-485 transceiver, BMC boot control / indicator LED, and UPDI).
Firmware developers must adhere to the following **mandatory and restricted configuration rules**:

### ① PORTMUX Configuration Guidelines

| Register | Bit Field | Function | ADX Rule | Value | Technical Rationale & Behavior |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **PORTMUX.CTRLB** | **SPI0** (bit 2) | SPI0 Communication Pins | **Mandatory Alternate** | `1` | Default pins (PA1..PA4) are occupied by the onboard RS-485 transceiver. Must route SPI to IDC connector pins **PC0–PC3 (IDC Pins 16–19)**. |
| **PORTMUX.CTRLB** | **TWI0** (bit 4) | I2C (TWI0) Communication Pins | **Mandatory Default** | `0` | Default routes I2C to IDC connector pins **PB0/PB1 (IDC Pins 13, 14)**. Alternate pins (PA1/PA2) conflict directly with RS-485 and are strictly prohibited. |
| **PORTMUX.CTRLB** | **USART0** (bit 0) | USART0 Communication Pins | **Application Dependent** | `0` / `1` | `0`: Connects to onboard **RS-485 transceiver (PA1/PA2)**.<br>`1`: Connects to IDC external **UART (PB2/PB3: IDC Pins 11, 12)**. |
| **PORTMUX.CTRLC** | **TCA00–05** | TCA0 PWM Waveform Output | **Flexible** | See Table | Selectable per pin in Normal or Split mode. |
| **PORTMUX.CTRLD** | **TCB0** (bit 0) | TCB0 PWM Output | **Flexible** | `0` / `1` | `0`: Routes to **IDC Pin 7 (PA5)**<br>`1`: Routes to **IDC Pin 19 (PC0)** |
| **PORTMUX.CTRLD** | **TCB1** (bit 1) | TCB1 PWM Output | **Default** | `0` | Default routes to **IDC Pin 9 (PA3)**. |
| **PORTMUX.CTRLA** | **LUT0/1** (bit 4,5) | CCL Output Pins | **Application Dependent** | `0` / `1` | Select alternative pin locations to avoid contention with dedicated circuits. |
| **PORTMUX.CTRLA** | **EVOUT0–2** | Event System Output | **Flexible** | `0` / `1` | Exports internal event system clocks/signals externally (EVOUT1: PB2 / Pin 11, EVOUT2: PC2 / Pin 17). |

---

## 3. ATtiny1616-MNR Complete 18-Pin Multiplexing Matrix

Complete multiplexed pin capability matrix based on Microchip Datasheet Table 5-1 (PORT Function Multiplexing):

| Pin Name | QFN | SOIC | ADX Connection | ADC0 | ADC1 | PWM / Timers | Comm (USART/SPI/I2C) | Analog / Special | Other (CCL/PTC/EV) |
| :--- | :---: | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **PA0** | 19 | 16 | **Onboard (UPDI/RESET)** | AIN0 | - | - | - | RESET / UPDI | LUT0-IN0 |
| **PA1** | 20 | 17 | **Onboard (RS-485 TxD)** | AIN1 | - | - | USART0 TxD (Def) / SPI MOSI(Def) / *TWI SDA(Alt)* | - | LUT0-IN1 |
| **PA2** | 1 | 18 | **Onboard (RS-485 RxD)** | AIN2 | - | - | USART0 RxD (Def) / SPI MISO(Def) / *TWI SCL(Alt)* | - | EVOUT0 / LUT0-IN2 |
| **PA3** | 2 | 19 | **IDC Pin 9** | AIN3 | - | TCA0 WO3(Def) / **TCB1 WO** | USART0 XCK (Def) / SPI SCK (Def) | **EXTCLK** | - |
| **PA4** | 5 | 2 | **Onboard (RS-485 DE)** | AIN4 | AIN0 | TCA0 WO4(Def) / **TCD0 WOA** | USART0 XDIR (Def) / SPI SS (Def) | - | X0/Y0 / LUT0-OUT(Def) |
| **PA5** | 6 | 3 | **IDC Pin 7** | AIN5 | AIN1 | TCA0 WO5(Def) / **TCB0 WO(Def)** / **TCD0 WOB** | - | **VREFA** / AC0 OUT / AC2 AINN0 | X1/Y1 |
| **PA6** | 7 | 4 | **IDC Pin 6** | AIN6 | AIN2 | - | - | **DAC0 OUT** / AC0-2 AIN | X2/Y2 |
| **PA7** | 8 | 5 | **Onboard (RS-485 RE)** | AIN7 | AIN3 | - | - | AC0-2 AIN | X3/Y3 / LUT1-OUT(Def) |
| **PB0** | 14 | 11 | **IDC Pin 13** | AIN11 | - | **TCA0 WO0(Def)** | **TWI0 SCL (Def)** / USART0 XDIR(Alt) | AC0/1 AIN | X5/Y5 |
| **PB1** | 13 | 10 | **IDC Pin 14** | AIN10 | - | **TCA0 WO1(Def)** | **TWI0 SDA (Def)** / USART0 XCK(Alt) | AC0 AIN | X4/Y4 |
| **PB2** | 12 | 9 | **IDC Pin 11** | - | - | **TCA0 WO2(Def)** | **USART0 TxD (Alt)** | AC1 OUT / TOSC2 | EVOUT1 |
| **PB3** | 11 | 8 | **IDC Pin 12** | - | - | **TCA0 WO0(Alt)** | **USART0 RxD (Alt)** | AC0 OUT / TOSC1 | - |
| **PB4** | 10 | 7 | **Onboard (Red LED)** | AIN9 | - | TCA0 WO1(Alt) | - | AC1/2 AIN | X13/Y13 / LUT0-OUT(Alt) |
| **PB5** | 9 | 6 | **Onboard (BOOT_REQ)** | AIN8 | - | TCA0 WO2(Alt) | - | CLKOUT / AC1/2 AIN | X12/Y12 |
| **PC0** | 15 | 12 | **IDC Pin 19** | - | AIN6 | **TCB0 WO(Alt)** / **TCD0 WOC** | **SPI0 SCK (Alt)** | - | X6/Y6 |
| **PC1** | 16 | 13 | **IDC Pin 18** | - | AIN7 | **TCD0 WOD** | **SPI0 MISO (Alt)** | - | X7/Y7 / LUT1-OUT(Alt) |
| **PC2** | 17 | 14 | **IDC Pin 17** | - | AIN8 | - | **SPI0 MOSI (Alt)** | - | EVOUT2 / X8/Y8 |
| **PC3** | 18 | 15 | **IDC Pin 16** | - | AIN9 | **TCA0 WO3(Alt)** | **SPI0 SS (Alt)** | - | X9/Y9 / LUT1-IN0 |

*(Def) denotes Default pin location; (Alt) denotes Alternate position selected via PORTMUX.*

---

## 4. IDC Connector Quick Reference by Peripheral

### ① PWM / Timer Output Capabilities

ATtiny1616 integrates 3 independent timer peripherals (TCA0, TCBn, TCD0), providing rich PWM outputs across the IDC connector:

| IDC Pin | MCU Pin | Compatible Timer Channels | PORTMUX Setting | Features & Notes |
| :---: | :---: | :--- | :--- | :--- |
| **Pin 7** | PA5 | **TCB0 WO (Def)**<br>**TCD0 WOB**<br>TCA0 WO5 (Def) | `PORTMUX.CTRLD.TCB0 = 0`<br>Enable TCD0 control register<br>`PORTMUX.CTRLC.TCA05 = 0` | 16-bit PWM (TCB0) / **High-Frequency 12-bit PWM (TCD0)** / 16-bit PWM (TCA0) |
| **Pin 9** | PA3 | **TCB1 WO**<br>TCA0 WO3 (Def) | `PORTMUX.CTRLD.TCB1 = 0`<br>`PORTMUX.CTRLC.TCA03 = 0` | 16-bit PWM (TCB1) / Shared with EXTCLK |
| **Pin 11** | PB2 | **TCA0 WO2 (Def)** | `PORTMUX.CTRLC.TCA02 = 0` | 16-bit PWM (TCA0 Normal) / 8-bit PWM (Split) / Shared with UART TxD |
| **Pin 12** | PB3 | **TCA0 WO0 (Alt)** | `PORTMUX.CTRLC.TCA00 = 1` | 8-bit PWM (TCA0 Split mode) / Shared with UART RxD |
| **Pin 13** | PB0 | **TCA0 WO0 (Def)** | `PORTMUX.CTRLC.TCA00 = 0` | 16-bit PWM (TCA0 Normal) / 8-bit PWM (Split) / Shared with I2C SCL |
| **Pin 14** | PB1 | **TCA0 WO1 (Def)** | `PORTMUX.CTRLC.TCA01 = 0` | 16-bit PWM (TCA0 Normal) / 8-bit PWM (Split) / Shared with I2C SDA |
| **Pin 16** | PC3 | **TCA0 WO3 (Alt)** | `PORTMUX.CTRLC.TCA03 = 1` | 8-bit PWM (TCA0 Split mode) / Shared with SPI SS |
| **Pin 18** | PC1 | **TCD0 WOD** | Enable TCD0 control register | **High-Frequency 12-bit PWM (TCD0)** / Shared with SPI MISO |
| **Pin 19** | PC0 | **TCD0 WOC**<br>**TCB0 WO (Alt)** | Enable TCD0 control register<br>`PORTMUX.CTRLD.TCB0 = 1` | **High-Frequency 12-bit PWM (TCD0)** / 16-bit PWM (TCB0) / Shared with SPI SCK |

> **About TCD0 (Timer Type D):**
> Operates at up to 48 MHz (via PLL or unprescaled asynchronous clock source). IDC connector breaks out **IDC Pin 7 (WOB)**, **IDC Pin 18 (WOD)**, and **IDC Pin 19 (WOC)**, ideal for high-resolution PWM, power electronic regulation, and half-bridge drive signals.

---

### ② Analog Functions (ADC0 / ADC1 / DAC0 / Comparators)

| IDC Pin | MCU Pin | Analog Capability | Description |
| :---: | :---: | :--- | :--- |
| **Pin 6** | PA6 | **DAC0 OUT**<br>ADC0 AIN6 / ADC1 AIN2<br>AC0/1/2 AIN | **8-bit DAC Output Pin** (Internal DAC true analog output)<br>Analog comparator inputs |
| **Pin 7** | PA5 | **VREFA** (External Voltage Ref)<br>ADC0 AIN5 / ADC1 AIN1<br>AC0 OUT | **External Reference Voltage Input (VREFA)**<br>Analog comparator AC0 output |
| **Pin 9** | PA3 | **ADC0 AIN3** | 10-bit general ADC input (when EXTCLK is unused) |
| **Pin 13** | PB0 | ADC0 AIN11 / AC0,1 AIN | 10-bit ADC input / Comparator input (Shared with I2C SCL) |
| **Pin 14** | PB1 | ADC0 AIN10 / AC0 AIN | 10-bit ADC input / Comparator input (Shared with I2C SDA) |
| **Pin 16** | PC3 | **ADC1 AIN9** | ADC1 10-bit ADC input (Shared with SPI SS) |
| **Pin 17** | PC2 | **ADC1 AIN8** | ADC1 10-bit ADC input (Shared with SPI MOSI) |
| **Pin 18** | PC1 | **ADC1 AIN7** | ADC1 10-bit ADC input (Shared with SPI MISO) |
| **Pin 19** | PC0 | **ADC1 AIN6** | ADC1 10-bit ADC input (Shared with SPI SCK) |

---

### ③ Event System (EVSYS) & Clock Outputs

| IDC Pin | MCU Pin | Special Feature | Configuration | Description |
| :---: | :---: | :--- | :--- | :--- |
| **Pin 9** | PA3 | **EXTCLK** | CLKCTRL | External oscillator clock input (System clock source) |
| **Pin 11** | PB2 | **EVOUT1** | `PORTMUX.CTRLA.EVOUT1 = 1` | Event system output (Timer events or frequency monitoring) |
| **Pin 17** | PC2 | **EVOUT2** | `PORTMUX.CTRLA.EVOUT2 = 1` | Event system output |
| All I/O | PA/PB/PC | **Pin Interrupts** | `PORTx.PINnCTRL` | Pin change interrupt on all GPIOs (PB2 and PC2 support fully asynchronous detection) |

---

## 5. Dedicated Onboard Pins (7 Pins) Protection

| Pin Name | Connection & Circuit | Hardware Role | Firmware Constraints |
| :--- | :--- | :--- | :--- |
| **PA0** | UPDI Programming / RESET | Hardware Debugger (Onboard THT breakout) | Do not reconfigure fuse to standard GPIO (prevents lockout). |
| **PA1** | SP485EEN DI (TxD) | RS-485 Transmit | Functional as USART0 TxD when `PORTMUX.CTRLB.USART0 = 0`. |
| **PA2** | SP485EEN RO (RxD) | RS-485 Receive | Functional as USART0 RxD when `PORTMUX.CTRLB.USART0 = 0`. |
| **PA4** | SP485EEN DE (Driver Enable) | Transmit Enable | Functional as hardware auto-direction control (`XDIR`). |
| **PA7** | SP485EEN /RE (Receiver Enable) | Receive Enable | Controlled via software GPIO (Active Low). |
| **PB4** | Onboard Red LED | Active HIGH Indicator | Software GPIO output. Usage is open (recommended as bootloader waiting indicator). |
| **PB5** | Onboard BMC BOOT_REQ | Boot Request Input (Pull-down & series resistor) | Sampled at startup. If HIGH, enters bootloader mode indefinitely; if LOW, starts user sketch immediately. |
