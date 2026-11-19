<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ATtiny1616 Peripheral Functions & PORTMUX Integrated Reference

[ **English** | [日本語](../ja/adx_attiny1616_mcu_matrix.md) ]

## 1. Overview
This document serves as an integrated technical reference defining the full MCU pin multiplexing and **PORTMUX (Port Multiplexer) register configuration rules** for ADX Core boards based on the Microchip **ATtiny1616-MNR (QFN-20)** (`CORE-U`, `CORE-A5` / `CORE-A24`, etc.).

For physical pinout definitions, Logical Pin IDs (`S00`, `P01`–`P10`), and ribbon cable shielding structure, refer to the master [ADX 20-Pin Bus Specification (SSOT: v4.2)](../spec/ADX_PINOUT.md) and [Architecture Guide](ADX_pinout.md). The authoritative English mapping reference is [docs/spec/mcu_mapping/attiny1616.md](../spec/mcu_mapping/attiny1616.md).

---

## 2. PORTMUX Design Rules on ADX Board

The ATtiny1616 provides highly flexible I/O multiplexing via PORTMUX. On ADX boards, 7 MCU pins are dedicated to onboard circuitry (CH342K dual UART, eFuse fault flag, and status LEDs on `CORE-U`; isolated RS-485 transceiver and LEDs on `CORE-A5`).  
Firmware developers must adhere to the following **mandatory and restricted configuration rules** (per Microchip Datasheet Table 5-1 & Section 15.3.2):

### ① PORTMUX Configuration Guidelines

| Register | Bit Field | Function | ADX Rule | Value | Arduino (`megaTinyCore`) | Technical Rationale & Behavior |
| :--- | :--- | :--- | :--- | :---: | :--- | :--- |
| **PORTMUX.CTRLB** | **SPI0** (bit 2) | SPI0 Communication Pins | **Mandatory Alternate** | `1` | `SPI.swap(1)` | Default pins (`PA1`–`PA4`) are occupied by onboard circuits. Must route SPI to **`P07`–`P10` (`PC3`–`PC0` / IDC Pins 12–15)**. |
| **PORTMUX.CTRLB** | **TWI0** (bit 4) | I2C (TWI0) Communication Pins | **Mandatory Default** | `0` | `Wire.swap(0)` (default) | Default routes I2C to **`P05`/`P06` (`PB0`/`PB1` / IDC Pins 9, 10)**. Alternate (`PA1`/`PA2`) conflicts with onboard circuits and is prohibited. |
| **PORTMUX.CTRLB** | **USART0** (bit 0) | USART0 Communication Pins | **Application Dependent** | `0` / `1` | `Serial.swap(0)` / `Serial.swap(1)` | `0` (Default): IDC external UART **`P03`/`P04` (`PB2`/`PB3` / Pins 7, 8)**.<br>`1` (Alternate): Onboard **CH342K / RS-485 (`PA1`/`PA2`)**. |
| **PORTMUX.CTRLC** | **TCA00–05** | TCA0 PWM Waveform Output | **Flexible** | See Table | — | Selectable per pin in Normal or Split mode. |
| **PORTMUX.CTRLD** | **TCB0** (bit 0) | TCB0 PWM Output | **Flexible** | `0` / `1` | — | `0`: `PA5` (Onboard)<br>`1`: **`P10` (`PC0` / IDC Pin 15)** |
| **PORTMUX.CTRLD** | **TCB1** (bit 1) | TCB1 PWM Output | **Default** | `0` | — | Default routes to **`P02` (`PA3` / IDC Pin 5)**. |
| **PORTMUX.CTRLA** | **LUT0/1** (bit 4,5) | CCL Output Pins | **Application Dependent** | `0` / `1` | — | Select pin locations avoiding contention with onboard circuits. |
| **PORTMUX.CTRLA** | **EVOUT0–2** | Event System Output | **Flexible** | `0` / `1` | — | Exports internal event signals to the bus (`EVOUT1`: `P03`/`PB2`, `EVOUT2`: `P08`/`PC2`). |

---

## 3. ATtiny1616-MNR Complete 18-Pin Multiplexing Matrix

Complete multiplexed pin capability matrix based on Microchip Datasheet Table 5-1 (PORT Function Multiplexing) aligned with `ADX-BUS-SPEC-v4.2`:

| Pin Name | QFN | SOIC | ADX Connection (`Logical ID` / IDC Pin) | ADC0 | ADC1 | PWM / Timers | Comm (USART/SPI/I2C) | Analog / Special | Other (CCL/PTC/EV) |
| :--- | :---: | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **PA0** | 19 | 16 | **`S00` / Pin 2 (`OWD / UPDI`)** | AIN0 | - | - | - | RESET / UPDI | LUT0-IN0 |
| **PA1** | 20 | 17 | **Onboard (CH342K / RS-485 TxD)** | AIN1 | - | - | USART0 TxD(Alt) / SPI MOSI(Def) / *TWI SDA(Alt)* | - | LUT0-IN1 |
| **PA2** | 1 | 18 | **Onboard (CH342K / RS-485 RxD)** | AIN2 | - | - | USART0 RxD(Alt) / SPI MISO(Def) / *TWI SCL(Alt)* | - | EVOUT0 / LUT0-IN2 |
| **PA3** | 2 | 19 | **`P02` / Pin 5 (`EXTCLK`)** | **AIN3** | - | TCA0 WO3(Def) / **TCB1 WO** | USART0 XCK(Alt) / SPI SCK(Def) | **EXTCLK** | - |
| **PA4** | 5 | 2 | **Onboard (RS-485 DE&/RE / LED pool)** | AIN4 | AIN0 | TCA0 WO4(Def) / **TCD0 WOA** | **USART0 XDIR(Alt)** / SPI SS(Def) | - | X0/Y0 / LUT0-OUT(Def) |
| **PA5** | 6 | 3 | **Onboard (LED pool / Reserve)** | AIN5 | AIN1 | TCA0 WO5(Def) / **TCB0 WO(Def)** / **TCD0 WOB** | - | **VREFA** / AC0 OUT / AC2 AINN0 | X1/Y1 |
| **PA6** | 7 | 4 | **`P01` / Pin 3 (`AIN / DAC`)** | **AIN6** | **AIN2** | - | - | **DAC0 OUT** / AC0–2 AIN | X2/Y2 |
| **PA7** | 8 | 5 | **Onboard (eFuse FLG / Reserve)** | AIN7 | AIN3 | - | - | AC0–2 AIN | X3/Y3 / LUT1-OUT(Def) |
| **PB0** | 14 | 11 | **`P05` / Pin 9 (`I2C_SCL`)** | **AIN11** | - | **TCA0 WO0(Def)** | **TWI0 SCL(Def)** / USART0 XDIR(Def) | AC0/1 AIN | X5/Y5 |
| **PB1** | 13 | 10 | **`P06` / Pin 10 (`I2C_SDA`)** | **AIN10** | - | **TCA0 WO1(Def)** | **TWI0 SDA(Def)** / USART0 XCK(Def) | AC0 AIN | X4/Y4 |
| **PB2** | 12 | 9 | **`P03` / Pin 7 (`UART_TX`)** | - | - | **TCA0 WO2(Def)** | **USART0 TxD(Def)** | AC1 OUT / TOSC2 | EVOUT1 |
| **PB3** | 11 | 8 | **`P04` / Pin 8 (`UART_RX`)** | - | - | **TCA0 WO0(Alt)** | **USART0 RxD(Def)** | AC0 OUT / TOSC1 | - |
| **PB4** | 10 | 7 | **Onboard (LED)** | AIN9 | - | TCA0 WO1(Alt) | - | AC1/2 AIN | X13/Y13 / LUT0-OUT(Alt) |
| **PB5** | 9 | 6 | **Onboard (LED)** | AIN8 | - | TCA0 WO2(Alt) | - | CLKOUT / AC1/2 AIN | X12/Y12 |
| **PC0** | 15 | 12 | **`P10` / Pin 15 (`SPI_SCK`)** | - | **AIN6** | **TCB0 WO(Alt)** / **TCD0 WOC** | **SPI0 SCK(Alt)** | - | X6/Y6 |
| **PC1** | 16 | 13 | **`P09` / Pin 14 (`SPI_MISO`)** | - | **AIN7** | **TCD0 WOD** | **SPI0 MISO(Alt)** | - | X7/Y7 / LUT1-OUT(Alt) |
| **PC2** | 17 | 14 | **`P08` / Pin 13 (`SPI_MOSI`)** | - | **AIN8** | - | **SPI0 MOSI(Alt)** | - | EVOUT2 / X8/Y8 |
| **PC3** | 18 | 15 | **`P07` / Pin 12 (`SPI_CS`)** | - | **AIN9** | **TCA0 WO3(Alt)** | **SPI0 SS(Alt)** | - | X9/Y9 / LUT1-IN0 |

*(Def) denotes Default pin location; (Alt) denotes Alternate position selected via PORTMUX.*

---

## 4. ADX Bus Quick Reference by Peripheral

Quick lookup by Logical Pin ID (`P01`–`P10` / code macros `ADX_P01`–`ADX_P10`) for ATtiny1616-specific bonus peripheral capabilities.

### ① PWM / Timer Output Capabilities

ATtiny1616 integrates 3 independent timer peripherals (TCA0, TCBn, TCD0), offering PWM outputs on 8 of the 10 bus pins (`P01`–`P10`):

| Logical ID | IDC Pin | Code Macro | MCU Pin | Compatible Timer Channels | PORTMUX Setting | Features & Notes |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P02`** | Pin 5 | `ADX_P02` | PA3 | **TCB1 WO**<br>TCA0 WO3 (Def) | `PORTMUX.CTRLD.TCB1 = 0`<br>`PORTMUX.CTRLC.TCA03 = 0` | 16-bit PWM (TCB1) / Shared with EXTCLK |
| **`P03`** | Pin 7 | `ADX_P03` | PB2 | **TCA0 WO2 (Def)** | `PORTMUX.CTRLC.TCA02 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / Shared with UART TxD |
| **`P04`** | Pin 8 | `ADX_P04` | PB3 | **TCA0 WO0 (Alt)** | `PORTMUX.CTRLC.TCA00 = 1` | 8-bit PWM (Split) / Shared with UART RxD |
| **`P05`** | Pin 9 | `ADX_P05` | PB0 | **TCA0 WO0 (Def)** | `PORTMUX.CTRLC.TCA00 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / Shared with I2C SCL |
| **`P06`** | Pin 10 | `ADX_P06` | PB1 | **TCA0 WO1 (Def)** | `PORTMUX.CTRLC.TCA01 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / Shared with I2C SDA |
| **`P07`** | Pin 12 | `ADX_P07` | PC3 | **TCA0 WO3 (Alt)** | `PORTMUX.CTRLC.TCA03 = 1` | 8-bit PWM (Split) / Shared with SPI CS |
| **`P09`** | Pin 14 | `ADX_P09` | PC1 | **TCD0 WOD** | Enable TCD0 control register | **High-Frequency 12-bit PWM (TCD0)** / Shared with SPI MISO |
| **`P10`** | Pin 15 | `ADX_P10` | PC0 | **TCD0 WOC**<br>**TCB0 WO (Alt)** | Enable TCD0 control register<br>`PORTMUX.CTRLD.TCB0 = 1` | **High-Frequency 12-bit PWM (TCD0)** / 16-bit PWM (TCB0) / Shared with SPI SCK |

> **About TCD0 (Timer Type D):**  
> Operates at up to 48 MHz (via PLL or unprescaled asynchronous clock source). The ADX bus breaks out **`P09` (Pin 14: WOD)** and **`P10` (Pin 15: WOC)**, ideal for high-resolution PWM, power regulation, and half-bridge drive signals.

---

### ② Analog Functions (ADC0 / ADC1 / DAC0 / Comparators)

While the bus guarantees **`P01` (Pin 3)** as the universal shielded analog channel across all Core boards, the ATtiny1616 silicon exposes **8 ADC-capable pins** on the bus (**4 on `ADC0`** directly usable with standard `analogRead()`):

| Logical ID | IDC Pin | Code Macro | MCU Pin | Analog Capability | Arduino (`megaTinyCore`) `analogRead()` | Description |
| :---: | :---: | :--- | :---: | :--- | :---: | :--- |
| **`P01`** | Pin 3 | `ADX_P01` | PA6 | **DAC0 OUT**<br>**ADC0 AIN6** / ADC1 AIN2<br>AC0–2 AIN | **Standard** | **Universal coplanar-shielded analog channel**.<br>**8-bit true DAC output** / 10-bit ADC input |
| **`P02`** | Pin 5 | `ADX_P02` | PA3 | **ADC0 AIN3** | **Standard** | Extra 10-bit ADC input (when EXTCLK is unused) |
| **`P05`** | Pin 9 | `ADX_P05` | PB0 | **ADC0 AIN11** / AC0,1 AIN | **Standard** | Extra 10-bit ADC input (when I2C SCL is unused) |
| **`P06`** | Pin 10 | `ADX_P06` | PB1 | **ADC0 AIN10** / AC0 AIN | **Standard** | Extra 10-bit ADC input (when I2C SDA is unused) |
| **`P07`** | Pin 12 | `ADX_P07` | PC3 | **ADC1 AIN9** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI CS) |
| **`P08`** | Pin 13 | `ADX_P08` | PC2 | **ADC1 AIN8** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI MOSI) |
| **`P09`** | Pin 14 | `ADX_P09` | PC1 | **ADC1 AIN7** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI MISO) |
| **`P10`** | Pin 15 | `ADX_P10` | PC0 | **ADC1 AIN6** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI SCK) |

> **\*Note on `ADC1` (`P07`–`P10` / `PC0`–`PC3`):**  
> Standard `megaTinyCore` `analogRead()` only operates on `ADC0`. Reading `P07`–`P10` as analog inputs requires direct `ADC1` register configuration (`ADC1.MUXPOS`, `ADC1.COMMAND`, `ADC1.RES`, etc.).

---

### ③ Event System (EVSYS) & Interrupts / Clock Outputs

| Logical ID | IDC Pin | Code Macro | MCU Pin | Special Feature | Configuration | Description |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P02`** | Pin 5 | `ADX_P02` | PA3 | **EXTCLK** | CLKCTRL | External oscillator clock input (System clock source) |
| **`P03`** | Pin 7 | `ADX_P03` | PB2 | **EVOUT1** | `PORTMUX.CTRLA.EVOUT1 = 1` | Event system output (Timer events or frequency monitoring) |
| **`P08`** | Pin 13 | `ADX_P08` | PC2 | **EVOUT2** | `PORTMUX.CTRLA.EVOUT2 = 1` | Event system output |
| `P01`–`P10` | All I/O | — | PA/PB/PC | **Pin Interrupts** | `PORTx.PINnCTRL` | Pin change interrupt on all GPIOs (`P01`/`PA6`, `P03`/`PB2`, `P08`/`PC2` support fully asynchronous detection) |

---

## 5. Dedicated Onboard Pins & Internal Reserves Protection

The following 7 MCU pins are not exposed to the IDC bus and are allocated to board-specific onboard circuitry:

| Pin Name | CORE-U (USB Standard) | CORE-A5 / A24 (Industrial RS-485) | Firmware Constraints |
| :---: | :--- | :--- | :--- |
| **PA1** | CH342K Port 1 TxD | TDA51S485HC DI (RS-485 TxD) | Active as USART0 TxD when `PORTMUX.CTRLB.USART0 = 1` (`Serial.swap(1)`). |
| **PA2** | CH342K Port 1 RxD | TDA51S485HC RO (RS-485 RxD) | Same as above. Hardwired to CCL LUT0-IN2 on RS-485 boards for OTW Break reset. |
| **PA4** | Onboard LED pool / Reserve | TDA51S485HC DE & /RE | Driven by USART0 Alternate `XDIR` auto-direction on RS-485 boards. |
| **PA5** | Onboard LED pool / Reserve | Internal Reserve (N.C.) | Set `PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc` if unconnected. |
| **PA7** | eFuse Fault Flag (`FLG`) | Internal Reserve (N.C.) | Active-LOW with 10kΩ PU on CORE-U; disable input buffer if unused. |
| **PB4** | Onboard LED (`R/Y/W` pool) | Onboard Red LED | Active HIGH output. |
| **PB5** | Onboard LED (`R/Y/W` pool) | Onboard Blue LED | Active HIGH output. |
