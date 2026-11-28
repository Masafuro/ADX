<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ATtiny1616 Peripheral Functions & PORTMUX Integrated Reference

[ **English** | [日本語](../ja/adx_attiny1616_mcu_matrix.md) ]

## 1. Overview
This document serves as an integrated technical reference defining the full MCU pin multiplexing and **PORTMUX (Port Multiplexer) register configuration rules** for the ADX flagship board **CORE-U (rev6)** based on the Microchip **ATtiny1616-MNR (QFN-20)**.

For physical pinout definitions, Logical Pin IDs (`P01`–`P11`), and ribbon cable shielding structure, refer to the master [ADX 20-Pin Bus Specification (SSOT: v5.0)](../spec/ADX_PINOUT.md) and [Architecture Guide](ADX_pinout.md). The authoritative English mapping reference is [docs/spec/mcu_mapping/attiny1616.md](../spec/mcu_mapping/attiny1616.md).

---

## 2. PORTMUX Design Rules on ADX CORE-U (rev6)

Under `ADX-BUS-SPEC-v5.0` / `CORE-U rev6`, isolated RS-485 / LIN transceivers reside on expansion Cards. Accordingly, **Alternate `USART0` (`PA1`/`PA2`/`PA4`)** is routed to the **IDC expansion bus**, while **Default `USART0` (`PB2`/`PB3`)** connects to the **onboard `CH342K` Port 1 USB-UART**.  
Firmware developers must adhere to the following configuration rules (per Microchip Datasheet Table 5-1 & Section 15.3.2):

### ① PORTMUX Configuration Guidelines

| Register | Bit Field | Function | ADX Rule | Value | Arduino (`megaTinyCore`) | Technical Rationale & Behavior |
| :--- | :--- | :--- | :--- | :---: | :--- | :--- |
| **PORTMUX.CTRLB** | **USART0** (bit 0) | USART0 Communication Pins | **Dynamic** | `0` *(default)* / `1` | `Serial.swap(0)` / `Serial.swap(1)` | `0` (Reset Default): **Onboard USB (`PB2`/`PB3` → `CH342K` Port 1)** — `Serial.begin()` works out of the box.<br>`1` (Alternate): **IDC field bus `P04`/`P05` (`PA1`/`PA2`) + `P03` hardware `XDIR` (`PA4`)**, simultaneously freeing `PB2`/`PB3` for **SoftwareSerial**. |
| **PORTMUX.CTRLB** | **SPI0** (bit 2) | SPI0 Communication Pins | **Mandatory Alternate** | `1` | `SPI.swap(1)` | Default pins (`PA1`–`PA4`) are used by IDC UART/EXTCLK. Must route SPI to **`P08`–`P11` (`PC3`–`PC0` / IDC Pins 12–15)**. |
| **PORTMUX.CTRLB** | **TWI0** (bit 4) | I2C (TWI0) Communication Pins | **Mandatory Default** | `0` | `Wire.swap(0)` *(default)* | Default routes I2C to **`P06`/`P07` (`PB0`/`PB1` / IDC Pins 9, 10)**. Alternate (`PA1`/`PA2`) conflicts with IDC UART and is prohibited. |
| **PORTMUX.CTRLC** | **TCA01** (bit 1) | `LED_MCU` PWM Output | **Alternate (Single)** | `1` | `analogWrite(ADX_LED, v)` | Routes 16-bit hardware PWM to **`PB4` (`ADX_LED`)** without interfering with I2C or UART. |
| **PORTMUX.CTRLD** | **TCB0** (bit 0) | TCB0 PWM Output | **Flexible** | `1` *(for bus)* | — | `0`: `PA5` (Onboard `VBUS_VSENSE`)<br>`1`: **`P11` (`PC0` / IDC Pin 15)** |
| **PORTMUX.CTRLD** | **TCB1** (bit 1) | TCB1 PWM Output | **Default** | `0` | — | Default routes to **`P02` (`PA3` / IDC Pin 4)**. |
| **PORTMUX.CTRLA** | **LUT0/1** (bit 4,5) | CCL Output Pins | **Application Dependent** | `0` / `1` | — | `LUT0-OUT (Def)` = `P03` (`PA4`), `LUT1-OUT (Alt)` = `P10` (`PC1`). |
| **PORTMUX.CTRLA** | **EVOUT0–2** | Event System Output | **Flexible** | `0` / `1` | — | `EVOUT0`: `P05` (`PA2`), `EVOUT2`: `P09` (`PC2`). |

---

## 3. ATtiny1616-MNR Complete 18-Pin Multiplexing Matrix (`v5.0`)

Complete multiplexed pin capability matrix based on Microchip Datasheet Table 5-1 aligned with `ADX-BUS-SPEC-v5.0` / `CORE-U rev6`:

| Pin Name | QFN | SOIC | ADX Connection (`Logical ID` / IDC Pin) | ADC0 | ADC1 | PWM / Timers | Comm (USART/SPI/I2C) | Analog / Special | Other (CCL/PTC/EV) |
| :--- | :---: | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **PA0** | 19 | 16 | **Onboard Only (CH342K P0 UPDI)** | AIN0 | - | - | - | RESET / UPDI | LUT0-IN0 |
| **PA1** | 20 | 17 | **`P04` / Pin 7 (`UART_TX`)** | **AIN1** | - | - | **USART0 TxD(Alt)** / SPI MOSI(Def) | - | LUT0-IN1 |
| **PA2** | 1 | 18 | **`P05` / Pin 8 (`UART_RX`)** | **AIN2** | - | - | **USART0 RxD(Alt)** / SPI MISO(Def) | - | **EVOUT0** / **LUT0-IN2** / **Async(Px2)** |
| **PA3** | 2 | 19 | **`P02` / Pin 4 (`EXTCLK`)** | **AIN3** | - | TCA0 WO3(Def) / **TCB1 WO** | USART0 XCK(Alt) / SPI SCK(Def) | **EXTCLK** | - |
| **PA4** | 5 | 2 | **`P03` / Pin 6 (`UART_DE`)** | **AIN4** | **AIN0** | TCA0 WO4(Def) / **TCD0 WOA** | **USART0 XDIR(Alt)** / SPI SS(Def) | - | X0/Y0 / **LUT0-OUT(Def)** |
| **PA5** | 6 | 3 | **Onboard (`ADX_VBUS_VSENSE`)** | **AIN5** | AIN1 | TCA0 WO5(Def) / TCB0 WO(Def) / TCD0 WOB | - | VREFA / AC0 OUT | X1/Y1 |
| **PA6** | 7 | 4 | **`P01` / Pin 2 (`AIN / DAC`)** | **AIN6** | **AIN2** | - | - | **DAC0 OUT** / AC0–2 AIN | X2/Y2 / **Async(Px6)** |
| **PA7** | 8 | 5 | **Onboard (`ADX_VBUS_ISENSE`)** | **AIN7** | AIN3 | - | - | AC0–2 AIN | X3/Y3 / LUT1-OUT(Def) |
| **PB0** | 14 | 11 | **`P06` / Pin 9 (`I2C_SCL`)** | **AIN11** | - | **TCA0 WO0(Def)** | **TWI0 SCL(Def)** / USART0 XDIR(Def) | AC0/1 AIN | X5/Y5 |
| **PB1** | 13 | 10 | **`P07` / Pin 10 (`I2C_SDA`)** | **AIN10** | - | **TCA0 WO1(Def)** | **TWI0 SDA(Def)** / USART0 XCK(Def) | AC0 AIN | X4/Y4 |
| **PB2** | 12 | 9 | **Onboard (`ADX_USB_TX`: CH342K P1)** | - | - | TCA0 WO2(Def) | **USART0 TxD(Def)** / SoftSerial TX | AC1 OUT / TOSC2 | EVOUT1 / Async(Px2) |
| **PB3** | 11 | 8 | **Onboard (`ADX_USB_RX`: CH342K P1)** | - | - | TCA0 WO0(Alt) | **USART0 RxD(Def)** / SoftSerial RX | AC0 OUT / TOSC1 | - |
| **PB4** | 10 | 7 | **Onboard (`ADX_LED`: Status LED)** | AIN9 | - | **TCA0 WO1(Alt)** | - | AC1/2 AIN | X13/Y13 / LUT0-OUT(Alt) |
| **PB5** | 9 | 6 | **Onboard (`ADX_USB_COM_DET`)** | AIN8 | - | TCA0 WO2(Alt) | - | CLKOUT / AC1/2 AIN | X12/Y12 / Sleep Wake (BOTHEDGES) |
| **PC0** | 15 | 12 | **`P11` / Pin 15 (`SPI_SCK`)** | - | **AIN6** | **TCB0 WO(Alt)** / **TCD0 WOC** | **SPI0 SCK(Alt)** | - | X6/Y6 |
| **PC1** | 16 | 13 | **`P10` / Pin 14 (`SPI_MISO`)** | - | **AIN7** | **TCD0 WOD** | **SPI0 MISO(Alt)** | - | X7/Y7 / LUT1-OUT(Alt) |
| **PC2** | 17 | 14 | **`P09` / Pin 13 (`SPI_MOSI`)** | - | **AIN8** | - | **SPI0 MOSI(Alt)** | - | **EVOUT2** / X8/Y8 / **Async(Px2)** |
| **PC3** | 18 | 15 | **`P08` / Pin 12 (`SPI_CS`)** | - | **AIN9** | **TCA0 WO3(Alt)** | **SPI0 SS(Alt)** | - | X9/Y9 / LUT1-IN0 |

*(Def) denotes Default pin location; (Alt) denotes Alternate position selected via PORTMUX.*

---

## 4. ADX Bus Quick Reference by Peripheral

Quick lookup by Logical Pin ID (`P01`–`P11` / code macros `ADX_P01`–`ADX_P11`) for ATtiny1616-specific bonus peripheral capabilities.

### ① PWM / Timer Output Capabilities

ATtiny1616 integrates 3 independent timer peripherals (TCA0, TCBn, TCD0), offering hardware PWM outputs on **7 bus pins** (including **3 channels of 48 MHz 12-bit `TCD0`**):

| Logical ID | IDC Pin | Code Macro | MCU Pin | Compatible Timer Channels | PORTMUX Setting | Features & Notes |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P02`** | Pin 4 | `ADX_P02` | PA3 | **TCB1 WO**<br>TCA0 WO3 (Def) | `PORTMUX.CTRLD.TCB1 = 0`<br>`PORTMUX.CTRLC.TCA03 = 0` | 16-bit PWM (TCB1) / Shared with EXTCLK |
| **`P03`** | Pin 6 | `ADX_P03` | PA4 | **TCD0 WOA**<br>TCA0 WO4 (Def) | Enable TCD0 control register<br>`PORTMUX.CTRLC.TCA04 = 0` | **High-Frequency 12-bit PWM (TCD0)** / Shared with `UART_DE` |
| **`P06`** | Pin 9 | `ADX_P06` | PB0 | **TCA0 WO0 (Def)** | `PORTMUX.CTRLC.TCA00 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / Shared with I2C SCL |
| **`P07`** | Pin 10 | `ADX_P07` | PB1 | **TCA0 WO1 (Def)** | `PORTMUX.CTRLC.TCA01 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / Shared with I2C SDA |
| **`P08`** | Pin 12 | `ADX_P08` | PC3 | **TCA0 WO3 (Alt)** | `PORTMUX.CTRLC.TCA03 = 1` | 8-bit PWM (Split) / Shared with SPI CS |
| **`P10`** | Pin 14 | `ADX_P10` | PC1 | **TCD0 WOD** | Enable TCD0 control register | **High-Frequency 12-bit PWM (TCD0)** / Shared with SPI MISO |
| **`P11`** | Pin 15 | `ADX_P11` | PC0 | **TCD0 WOC**<br>**TCB0 WO (Alt)** | Enable TCD0 control register<br>`PORTMUX.CTRLD.TCB0 = 1` | **High-Frequency 12-bit PWM (TCD0)** / 16-bit PWM (TCB0) / Shared with SPI SCK |

> **About TCD0 (Timer Type D):**  
> Operates at up to 48 MHz. Under `v5.0`, the ADX bus exposes **3 channels**: **`P03` (Pin 6: WOA)**, **`P10` (Pin 14: WOD)**, and **`P11` (Pin 15: WOC)**.

---

### ② Analog Functions (ADC0 / ADC1 / DAC0 / Comparators)

Under `v5.0`, **all 11 bus pins (`P01`–`P11`) are ADC-capable** on ATtiny1616 (**7 pins on `ADC0`** directly usable with standard `analogRead()`):

| Logical ID | IDC Pin | Code Macro | MCU Pin | Analog Capability | Arduino (`megaTinyCore`) `analogRead()` | Description |
| :---: | :---: | :--- | :---: | :--- | :---: | :--- |
| **`P01`** | Pin 2 | `ADX_P01` | PA6 | **DAC0 OUT**<br>**ADC0 AIN6** / ADC1 AIN2<br>AC0–2 AIN | **Standard** | **Universal dual-GND coplanar-shielded analog channel**.<br>**8-bit true DAC output** / 10-bit ADC input |
| **`P02`** | Pin 4 | `ADX_P02` | PA3 | **ADC0 AIN3** | **Standard** | Extra 10-bit ADC input (when EXTCLK is unused) |
| **`P03`** | Pin 6 | `ADX_P03` | PA4 | **ADC0 AIN4** / ADC1 AIN0 | **Standard** | Extra 10-bit ADC input (when `UART_DE` is unused) |
| **`P04`** | Pin 7 | `ADX_P04` | PA1 | **ADC0 AIN1** | **Standard** | Extra 10-bit ADC input (when UART TX is unused) |
| **`P05`** | Pin 8 | `ADX_P05` | PA2 | **ADC0 AIN2** | **Standard** | Extra 10-bit ADC input (when UART RX is unused) |
| **`P06`** | Pin 9 | `ADX_P06` | PB0 | **ADC0 AIN11** / AC0,1 AIN | **Standard** | Extra 10-bit ADC input (when I2C SCL is unused) |
| **`P07`** | Pin 10 | `ADX_P07` | PB1 | **ADC0 AIN10** / AC0 AIN | **Standard** | Extra 10-bit ADC input (when I2C SDA is unused) |
| **`P08`** | Pin 12 | `ADX_P08` | PC3 | **ADC1 AIN9** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI CS) |
| **`P09`** | Pin 13 | `ADX_P09` | PC2 | **ADC1 AIN8** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI MOSI) |
| **`P10`** | Pin 14 | `ADX_P10` | PC1 | **ADC1 AIN7** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI MISO) |
| **`P11`** | Pin 15 | `ADX_P11` | PC0 | **ADC1 AIN6** | ADC1 Only\* | ADC1 10-bit ADC input (Shared with SPI SCK) |

> **\*Note on `ADC1` (`P08`–`P11` / `PC0`–`PC3`):**  
> Standard `megaTinyCore` `analogRead()` only operates on `ADC0`. Reading `P08`–`P11` as analog inputs requires direct `ADC1` register configuration (`ADC1.MUXPOS`, `ADC1.COMMAND`, `ADC1.RES`, etc.).

---

### ③ Event System (EVSYS), CCL & Sleep Wake-Up

| Logical ID | IDC Pin | Code Macro | MCU Pin | Special Feature | Configuration | Description |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P01`** | Pin 2 | `ADX_P01` | PA6 | **Full Async Int (`Px6`)** | `PORTA.PIN6CTRL` | Arbitrary-edge Power-Down wake-up & sensor event input |
| **`P02`** | Pin 4 | `ADX_P02` | PA3 | **EXTCLK** | CLKCTRL | External oscillator clock input (System clock source) |
| **`P03`** | Pin 6 | `ADX_P03` | PA4 | **CCL `LUT0-OUT`** | `PORTMUX.CTRLA.LUT0 = 0` | Direct CCL LUT0 output (when `UART_DE` is unused) |
| **`P05`** | Pin 8 | `ADX_P05` | PA2 | **CCL `LUT0-IN2` / `EVOUT0` / Full Async Int (`Px2`)** | `CCL` / `USART0 SFDEN` | Hardware RS-485/LIN Break monitoring & Standby/Power-Down wake-up |
| **`P06`/`P07`** | Pin 9,10 | `ADX_P06`/`P07` | PB0/PB1 | **TWI0 Address-Match Wake** | `TWI0.SCTRLA` | Hardware I2C slave address-match wake-up from Power-Down |
| **`P09`** | Pin 13 | `ADX_P09` | PC2 | **`EVOUT2` / Full Async Int (`Px2`)** | `PORTMUX.CTRLA.EVOUT2 = 1` | Event system output & fully asynchronous edge interrupt |
| `P01`–`P11` | All I/O | — | PA/PB/PC | **Pin Interrupts** | `PORTx.PINnCTRL` | Pin change interrupt on all 11 GPIOs (`BOTHEDGES`/`LEVEL` wakes Power-Down) |

---

## 5. Dedicated Onboard Pins (CORE-U rev6)

The following 7 MCU pins are not exposed to the IDC bus and serve core onboard functions on `CORE-U rev6` (for legacy derivative `CORE-A5` mapping, see `hardware/archive/CORE-A5/`):

| Pin Name | CORE-U rev6 Macro | Hardware Connection | Firmware Notes |
| :---: | :--- | :--- | :--- |
| **PA0** | *(unexposed)* | `SN74LV4066A` → `CH342K` Port 0 (**UPDI**) | Strictly onboard (47kΩ PU). Always accessible in all sleep modes (§33.3.9). |
| **PB2** | `ADX_USB_TX` | `SN74LV4066A` → `CH342K` Port 1 RX | Reset-default hardware `USART0` TX (`Serial.swap(0)`); SoftwareSerial TX when `Serial.swap(1)`. |
| **PB3** | `ADX_USB_RX` | `SN74LV4066A` → `CH342K` Port 1 TX | Same as above (MCU-side 100kΩ PU); isolated in Mode 1/2 via `SN74LV4066A`. |
| **PA5** | `ADX_VBUS_VSENSE` | `VBUS_PROT` 1/2 divider | `ADC0 AIN5` (0–6.60V). Monitors trunk voltage across all 3 Modes. |
| **PA7** | `ADX_VBUS_ISENSE` | `TP181A2-CR` output (10mΩ shunt) | `ADC0 AIN7` (1.00 V/A). Reads `≈ 0 mA` in Mode 1 vs. `≥ 6 mA` in Mode 2/3. |
| **PB4** | `ADX_LED` | `LED_MCU` (`2N7002`, 5V 110°C LED) | Active-HIGH, `TCA0 WO1*` hardware PWM (100kΩ PD). |
| **PB5** | `ADX_USB_COM_DET` | `USB_COM_EN` (via `10kΩ` series resistor) | Active-HIGH. `PORT_ISC_BOTHEDGES_gc` wakes directly from Power-Down sleep upon `USB-COM` insertion. |
