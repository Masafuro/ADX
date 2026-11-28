<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Hardware Mapping: Microchip ATtiny1616
(Flagship Target Board: **ADX CORE-U rev6**; legacy/derivative reference: `CORE-A5`)

**Standard Reference**: [ADX 20-Pin Bus Specification (ADX_PINOUT.md)](../ADX_PINOUT.md) (Standard: `ADX-BUS-SPEC-v5.0`)  
**Target MCU**: Microchip ATtiny1616-MNR (QFN-20-EP / SOIC-20)  
**Operating Condition**: 3.3V DC / 10 MHz internal high-frequency oscillator  

---

## 1. Overview

This document specifies how the universal signals and Logical Pin IDs (`P01`–`P11`) of the **ADX 20-Pin Bus (`v5.0`)** map to the physical pins, peripherals, MCU-specific bonus capabilities, and PORTMUX channels of the **Microchip ATtiny1616** microcontroller on the **ADX CORE-U (rev6)** flagship board.

In `v5.0` / `CORE-U rev6`, isolated RS-485 / LIN transceivers and isolated field power (24V/48V) reside on expansion Cards rather than on the Core board. Accordingly:
- **Alternate `USART0` (`PA1` TxD, `PA2` RxD / `CCL LUT0-IN2`, `PA4` `XDIR`)** is routed to the **20-pin IDC expansion bus (`P03`–`P05`)** for full hardware RS-485 direction control, LIN auto-baud (`LINAUTO`), hardware break detection, and sleep wake-up.
- **Default `USART0` (`PB2` TxD, `PB3` RxD)** is routed to the **onboard `CH342K` Port 1 USB-UART** (isolated via `SN74LV4066A` when `USB-COM` is unplugged), making `Serial.begin()` work on USB at reset default while allowing `PB2`/`PB3` to operate as SoftwareSerial simultaneously when `USART0` is swapped to the expansion bus.
- **`PA0` (`UPDI`)** is kept strictly onboard (`CH342K` Port 0 via `SN74LV4066A`) and omitted from the IDC connector to eliminate HV-noise and accidental-reset exposure.

---

## 2. Bus to MCU Physical Interconnect Table (`ADX-BUS-SPEC-v5.0`)

| IDC Pin | Logical ID | Code Macro | Bus Function | ATtiny1616 Pin | QFN Pin | Guaranteed Role & MCU-Specific Bonus Functions |
| :---: | :---: | :--- | :--- | :---: | :---: | :--- |
| **1** | **GND** | — | — | - | - | Cable Outer Edge Shield & `P01 (AIN)` Guard ① |
| **2** | **P01** | `ADX_P01` (`ADX_AIN`) | **AIN** | **PA6** | 7 | **ADC0 AIN6** / ADC1 AIN2 / **DAC0 OUT** / Full Async Int (`Px6`) — Dual-GND shielded 10-bit ADC & 8-bit true DAC |
| **3** | **GND** | — | — | - | - | `P01 (AIN)` Guard ② & `P02 (EXTCLK)` Guard ① |
| **4** | **P02** | `ADX_P02` (`ADX_CLK`) | **EXTCLK** | **PA3** | 2 | **EXTCLK** / **ADC0 AIN3** / TCA0 WO3 (Def) / **TCB1 WO** — Dual-GND shielded clock / PWM / extra `analogRead()` |
| **5** | **GND** | — | — | - | - | `P02 (EXTCLK)` Guard ② & UART Shield |
| **6** | **P03** | `ADX_P03` (`ADX_DE`) | **UART_DE** | **PA4** | 5 | **USART0 XDIR (Alternate)** / **ADC0 AIN4** / ADC1 AIN0 / **TCD0 WOA** (48 MHz 12-bit PWM) / **CCL LUT0-OUT** — Hardware RS-485 DE (Card PD MUST) |
| **7** | **P04** | `ADX_P04` (`ADX_TX`) | **UART_TX** | **PA1** | 20 | **USART0 TxD (Alternate)** / **ADC0 AIN1** / CCL LUT0-IN1 — Hardware Field UART TX / extra `analogRead()` |
| **8** | **P05** | `ADX_P05` (`ADX_RX`) | **UART_RX** | **PA2** | 1 | **USART0 RxD (Alternate)** / **ADC0 AIN2** / **CCL LUT0-IN2** / EVOUT0 / **Full Async Int (`Px2`)** — Hardware Field UART RX, LIN / Break monitor & sleep wake-up |
| **9** | **P06** | `ADX_P06` (`ADX_SCL`) | **I2C_SCL** | **PB0** | 14 | **TWI0 SCL (Default)** / **ADC0 AIN11** / TCA0 WO0 (Def) — Hardware I2C Clock (Power-Down address match) / extra `analogRead()` |
| **10** | **P07** | `ADX_P07` (`ADX_SDA`) | **I2C_SDA** | **PB1** | 13 | **TWI0 SDA (Default)** / **ADC0 AIN10** / TCA0 WO1 (Def) — Hardware I2C Data (Power-Down address match) / extra `analogRead()` |
| **11** | **GND** | — | — | - | - | I2C / SPI Group Boundary Shield |
| **12** | **P08** | `ADX_P08` (`ADX_CS`) | **SPI_CS** | **PC3** | 18 | **SPI0 SS (Alternate)** / ADC1 AIN9\* / TCA0 WO3 (Alt) / CCL LUT1-IN0 — Active-LOW Chip Select |
| **13** | **P09** | `ADX_P09` (`ADX_MOSI`) | **SPI_MOSI** | **PC2** | 17 | **SPI0 MOSI (Alternate)** / ADC1 AIN8\* / EVOUT2 / Full Async Int (`Px2`) — SPI Controller Out / Peripheral In |
| **14** | **P10** | `ADX_P10` (`ADX_MISO`) | **SPI_MISO** | **PC1** | 16 | **SPI0 MISO (Alternate)** / ADC1 AIN7\* / **TCD0 WOD** (48 MHz 12-bit PWM) / CCL LUT1-OUT (Alt) |
| **15** | **P11** | `ADX_P11` (`ADX_SCK`) | **SPI_SCK** | **PC0** | 15 | **SPI0 SCK (Alternate)** / ADC1 AIN6\* / **TCD0 WOC** / TCB0 WO (Alt) |
| **16** | **GND** | — | — | - | - | VBUS Protective Boundary Wall & SPI Return |
| **17–20** | **VBUS** | — | — | - | - | Main Bidirectional High-Current Power Bus (4.5V–5.5V / 5V ±10%, $2 \times 2$ solid plane) |

> **\*Note on `ADC0` vs. `ADC1` under Arduino (`megaTinyCore`):**  
> Under `v5.0`, ATtiny1616 exposes **all 11 bus pins (`P01`–`P11`) with ADC capability**, split across two hardware converters:
> - **`ADC0` (`P01`–`P07`: 7 pins)**: Directly supported by standard Arduino `analogRead()` in `megaTinyCore` (+3 additional `ADC0` pins compared to `v4.2`, thanks to placing `PA1`/`PA2`/`PA4` on the bus).
> - **`ADC1` (`P08`–`P11` / `PC0`–`PC3`: 4 pins)**: Standard `megaTinyCore` `analogRead()` only addresses `ADC0`. Reading `PC0`–`PC3` as analog inputs requires direct `ADC1` register access (or a board-header helper function).

---

## 3. Dedicated Onboard Pins (CORE-U rev6 — Non-Exposed to IDC Bus)

The remaining 7 GPIO pins are allocated to onboard USB programming/communication (completely isolated via `SN74LV4066A` in Mode 1/2), AI power telemetry, status indication, and 3-Mode detection — using all 18 I/O pins with zero waste:

| MCU Pin | QFN Pin | Code Macro | CORE-U rev6 Dedicated Function | Electrical & Firmware Notes |
| :---: | :---: | :--- | :--- | :--- |
| **PA0** | 19 | *(unexposed)* | **Onboard SerialUPDI** (`CH342K` Port 0 via `SN74LV4066A`) | Strictly onboard (omitted from IDC bus for HV-noise immunity). MCU-side `47kΩ` weak pull-up. Accessible in all sleep modes (§33.3.9). |
| **PB2** | 12 | `ADX_USB_TX` | **Onboard USB UART TX** (`CH342K` Port 1 RX via `SN74LV4066A`) | `USART0 TxD (Default)` at reset; simultaneously usable as **SoftwareSerial TX** when `Serial.swap(1)` is active. |
| **PB3** | 11 | `ADX_USB_RX` | **Onboard USB UART RX** (`CH342K` Port 1 TX via `SN74LV4066A`) | `USART0 RxD (Default)` at reset; MCU-side `100kΩ` pull-up prevents false BREAK when isolated. Usable as **SoftwareSerial RX** when `Serial.swap(1)` is active. |
| **PA5** | 6 | `ADX_VBUS_VSENSE` | **Trunk Voltage Telemetry (`VBUS_PROT` 1/2 divider)** | `ADC0 AIN5` (`0.00V–6.60V`). Active in all 3 Modes. |
| **PA7** | 8 | `ADX_VBUS_ISENSE` | **Supply Current Telemetry (`TP181A2-CR` 100 V/V + 10mΩ)** | `ADC0 AIN7` (`1.00 V/A`, `0.00A–3.30A`). Reads `≈ 0 mA` in Mode 1 (eFuse reverse blocking) vs. `≥ 6 mA` in Mode 2/3. |
| **PB4** | 10 | `ADX_LED` | **Onboard User / AI Status LED (`LED_MCU`)** | Active-HIGH via `2N7002` (5V-driven automotive 110°C `HVRTB-3433EPX`, `100kΩ` PD). Supports `TCA0 WO1*` 16-bit hardware PWM. |
| **PB5** | 9 | `ADX_USB_COM_DET` | **Mode 3 (`USB-COM`) Connection Detect** | Active-HIGH digital input (`USB_COM_EN` via `10kΩ` series resistor). Supports `BOTHEDGES`/`LEVEL` wake-up directly from Power-Down deep sleep; paired with `PA7` for full 3-Mode self-identification. |

> **Legacy / Derivative Reference Note (`CORE-A5`)**:  
> Legacy integrated RS-485 derivative boards (such as `CORE-A5`, designed under `v4.x` before `CORE-U rev6` unified flagship deployment) used `PA1`/`PA2`/`PA4` onboard for an integrated RS-485 transceiver and `PB2`/`PB3` on the IDC bus. That topology is preserved in `hardware/archive/CORE-A5/` as a value-added derivative reference and does not apply to `ADX-BUS-SPEC-v5.0` Core boards.

---

## 4. PORTMUX & Direction Control Configuration Rules

According to Microchip Datasheet Table 5-1 and Section 15.3.2 (`PORTMUX.CTRLB` / `CTRLC` / `CTRLD`), firmware for `CORE-U rev6` (`ADX-BUS-SPEC-v5.0`) applies the following register settings:

| Register | Bit / Field | Required Setting | Value | Arduino (`megaTinyCore`) | Rationale |
| :--- | :--- | :--- | :---: | :--- | :--- |
| **`PORTMUX.CTRLB`** | **`USART0`** (bit 0) | **Dynamic** | `0` *(default)* or `1` | `Serial.swap(0)` / `Serial.swap(1)` | `0` (Reset Default): **Onboard USB `CH342K` (`PB2`/`PB3`)** — `Serial.begin()` works out of the box.<br>`1` (Alternate): **IDC Field Bus `P04`/`P05` (`PA1`/`PA2`) + `P03` `XDIR` (`PA4`)**. |
| **`PORTMUX.CTRLB`** | **`SPI0`** (bit 2) | **Alternate** | `1` | `SPI.swap(1)` | Default (`PA1`–`PA4`) conflicts with IDC UART/EXTCLK. Routes SPI to **`P08`–`P11` (`PC3`–`PC0`)**. |
| **`PORTMUX.CTRLB`** | **`TWI0`** (bit 4) | **Default** | `0` | `Wire.swap(0)` *(default)* | Default routes I2C to **`P06`/`P07` (`PB0`/`PB1`)**. Alternate (`PA1`/`PA2`) conflicts with IDC UART. |
| **`PORTMUX.CTRLC`** | **`TCA01`** (bit 1) | **Alternate** *(when PWM on LED)* | `1` | `analogWrite(ADX_LED, v)` | Routes `TCA0 WO1` (single mode) to **`PB4` (`ADX_LED`)** without interfering with I2C (`PB0`/`PB1`) or USB UART (`PB2`/`PB3`). |
| **`PORTMUX.CTRLD`** | **`TCB1`** (bit 1) | **Default** | `0` | — | Default routes `TCB1` PWM output to **`P02` (`PA3`)**. |
| **`PORTMUX.CTRLD`** | **`TCB0`** (bit 0) | **Optional Alt** | `1` | — | Alternate routes `TCB0` PWM output to **`P11` (`PC0`)**. |

### 4.1 USART0 Dynamic Switching & Concurrent USB SoftwareSerial
- **Onboard USB Debug / `AI-Firmata` (`Serial.swap(0)`, Reset Default)**: `PORTMUX.CTRLB.USART0 = 0` routes hardware `USART0` to `PB2`/`PB3` (`CH342K` Port 1). Standard `Serial.begin()` communicates directly with PC / AI / Android GOT with zero `swap()` boilerplate.
- **IDC Field RS-485 / LIN (`Serial.swap(1)`)**: Set `PORTMUX.CTRLB |= PORTMUX_USART0_bm` (`Serial.swap(1)`) to route `USART0` to `P04`/`P05` (`PA1`/`PA2`) and enable hardware RS-485 auto-direction (`USART0.CTRLA = USART_RS485_EXT_gc`) on **`P03` (`PA4 / XDIR`)** plus `LINAUTO` auto-baud (`USART_RXMODE_LINAUTO_gc`).
  - Because `PA2` (`P05`) is hardwired directly to `CCL LUT0-IN2` and supports full-asynchronous edge wake-up (`Px2`), incoming Break pulses or start bits can monitor/reset or wake the MCU from Power-Down/Standby sleep.
  - While `Serial.swap(1)` is active, `PB2`/`PB3` remain independent GPIOs, allowing simultaneous **SoftwareSerial** debug logging to `USB-COM` whenever `ADX_USB_COM_DET` (`PB5`) reads `HIGH`.

---

## 5. Reference Board Header Snippet (`ADX_CORE_U_rev6.h`)

Following `ADX-BUS-SPEC-v5.0` Section 5.4, portable Arduino / C++ sketches for `CORE-U rev6` use collision-safe `ADX_`-prefixed macros:

```cpp
#pragma once
#include <Arduino.h>

// ADX 20-Pin Bus (v5.0) Logical Pin IDs (P01-P11, S00 omitted)
#define ADX_P01          PIN_PA6  // AIN / DAC0
#define ADX_P02          PIN_PA3  // EXTCLK / TCB1 PWM
#define ADX_P03          PIN_PA4  // UART_DE (USART0 Alt XDIR) / TCD0 WOA / LUT0-OUT
#define ADX_P04          PIN_PA1  // UART_TX (USART0 Alt TxD)
#define ADX_P05          PIN_PA2  // UART_RX (USART0 Alt RxD / LUT0-IN2)
#define ADX_P06          PIN_PB0  // I2C_SCL
#define ADX_P07          PIN_PB1  // I2C_SDA
#define ADX_P08          PIN_PC3  // SPI_CS
#define ADX_P09          PIN_PC2  // SPI_MOSI
#define ADX_P10          PIN_PC1  // SPI_MISO
#define ADX_P11          PIN_PC0  // SPI_SCK

// Standard Bus Function Aliases
#define ADX_AIN          ADX_P01
#define ADX_CLK          ADX_P02
#define ADX_DE           ADX_P03
#define ADX_TX           ADX_P04
#define ADX_RX           ADX_P05
#define ADX_SCL          ADX_P06
#define ADX_SDA          ADX_P07
#define ADX_CS           ADX_P08
#define ADX_MOSI         ADX_P09
#define ADX_MISO         ADX_P10
#define ADX_SCK          ADX_P11

// CORE-U rev6 Onboard USB Serial (SoftwareSerial pins when Serial.swap(1) is active)
#define ADX_USB_TX       PIN_PB2  // USART0 Def TxD
#define ADX_USB_RX       PIN_PB3  // USART0 Def RxD

// CORE-U rev6 Onboard AI Observability, 3-Mode Detection & Status LED
#define ADX_VBUS_VSENSE  PIN_PA5  // ADC0 AIN5 (0.50 V/V, 0.0-6.6V)
#define ADX_VBUS_ISENSE  PIN_PA7  // ADC0 AIN7 (1.00 V/A, 0.0-3.3A, TP181A2-CR)
#define ADX_LED          PIN_PB4  // Active-HIGH (TCA0 WO1* Hardware PWM, 100k PD)
#define ADX_USB_COM_DET  PIN_PB5  // Active-HIGH (USB-COM Detect / Mode 3 / Sleep Wake)
```

---

## 6. Factory Calibration & Self-Testing

ATtiny1616 Core boards do not route internal 3.3V power onto the 20-pin bus. Automated factory inspection on `CORE-U rev6` connects via onboard `USB-COM` (`CH342K` Port 0 `UPDI` + Port 1 `UART`), flashes the test/production image, and verifies:
1. Internal 3.3V LDO output via ATtiny1616 internal `VDD` ADC channel (`ADC_REFSEL_INTREF_gc` 1.1V bandgap; valid window: $3.20\,\text{V} \le V_{DD} \le 3.40\,\text{V}$).
2. `ADX_VBUS_VSENSE` (`PA5`), `ADX_VBUS_ISENSE` (`PA7`), `ADX_USB_COM_DET` (`PB5`), and `ADX_LED` (`PB4`) integrity.
3. Continuity across `P01`–`P11` on the 20-pin IDC Eject Header.
