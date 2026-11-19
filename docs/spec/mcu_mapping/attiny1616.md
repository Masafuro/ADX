<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Hardware Mapping: Microchip ATtiny1616
(Target Boards: ADX CORE-U, ADX CORE-A5 / CORE-A24)

**Standard Reference**: [ADX 20-Pin Bus Specification (ADX_PINOUT.md)](../ADX_PINOUT.md) (Standard: `ADX-BUS-SPEC-v4.2`)  
**Target MCU**: Microchip ATtiny1616-MNR (QFN-20 / SOIC-20)  
**Operating Condition**: 3.3V DC / 10 MHz internal high-frequency oscillator  

---

## 1. Overview

This document specifies how the universal signals and Logical Pin IDs (`S00`, `P01`–`P10`) of the **ADX 20-Pin Bus (v4.2)** map to the physical pins, peripherals, MCU-specific bonus capabilities, and PORTMUX channels of the **Microchip ATtiny1616** microcontroller.

---

## 2. Bus to MCU Physical Interconnect Table

| IDC Pin | Logical ID | Code Macro | Bus Function | ATtiny1616 Pin | QFN Pin | Guaranteed Role & MCU-Specific Bonus Functions |
| :---: | :---: | :--- | :--- | :---: | :---: | :--- |
| **1** | **GND** | — | — | - | - | Cable Outer Edge Ground Shield |
| **2** | **S00** | *(unexposed)* | **OWD** | **PA0** | 19 | **UPDI / RESET** (Configured as UPDI) — 1-wire factory programming & debug (**Card N.C.**) |
| **3** | **P01** | `ADX_P01` (`ADX_AIN`) | **AIN** | **PA6** | 7 | **ADC0 AIN6** / ADC1 AIN2 / **DAC0 OUT** / AC0–2 — Coplanar shielded 10-bit ADC & 8-bit true DAC |
| **4** | **GND** | — | — | - | - | Analog / Clock Separation Shield |
| **5** | **P02** | `ADX_P02` (`ADX_CLK`) | **EXTCLK** | **PA3** | 2 | **EXTCLK** / **ADC0 AIN3** / TCA0 WO3 (Def) / **TCB1 WO** — Clock / PWM / extra `analogRead()` |
| **6** | **GND** | — | — | - | - | Clock / Communication Separation Shield |
| **7** | **P03** | `ADX_P03` (`ADX_TX`) | **UART_TX** | **PB2** | 12 | **USART0 TxD (Default)** / TCA0 WO2 (Def) / EVOUT1 — Hardware UART TX (no ADC) |
| **8** | **P04** | `ADX_P04` (`ADX_RX`) | **UART_RX** | **PB3** | 11 | **USART0 RxD (Default)** / TCA0 WO0 (Alt) — Hardware UART RX (no ADC) |
| **9** | **P05** | `ADX_P05` (`ADX_SCL`) | **I2C_SCL** | **PB0** | 14 | **TWI0 SCL (Default)** / **ADC0 AIN11** / TCA0 WO0 (Def) — Hardware I2C Clock / extra `analogRead()` |
| **10** | **P06** | `ADX_P06` (`ADX_SDA`) | **I2C_SDA** | **PB1** | 13 | **TWI0 SDA (Default)** / **ADC0 AIN10** / TCA0 WO1 (Def) — Hardware I2C Data / extra `analogRead()` |
| **11** | **GND** | — | — | - | - | I2C / SPI Group Boundary Shield |
| **12** | **P07** | `ADX_P07` (`ADX_CS`) | **SPI_CS** | **PC3** | 18 | **SPI0 SS (Alternate)** / ADC1 AIN9\* / TCA0 WO3 (Alt) — Active-LOW Chip Select |
| **13** | **P08** | `ADX_P08` (`ADX_MOSI`) | **SPI_MOSI** | **PC2** | 17 | **SPI0 MOSI (Alternate)** / ADC1 AIN8\* / EVOUT2 — SPI Controller Out / Peripheral In |
| **14** | **P09** | `ADX_P09` (`ADX_MISO`) | **SPI_MISO** | **PC1** | 16 | **SPI0 MISO (Alternate)** / ADC1 AIN7\* / **TCD0 WOD** (48 MHz 12-bit PWM) |
| **15** | **P10** | `ADX_P10` (`ADX_SCK`) | **SPI_SCK** | **PC0** | 15 | **SPI0 SCK (Alternate)** / ADC1 AIN6\* / **TCD0 WOC** / TCB0 WO (Alt) |
| **16** | **GND** | — | — | - | - | VBUS Protective Boundary Wall & SPI Return |
| **17–20** | **VBUS** | — | — | - | - | Main High-Current Power Bus (4.5V–5.0V, $2 \times 2$ solid plane) |

> **\*Note on `ADC1` (`P07`–`P10` / `PC0`–`PC3`) under Arduino (`megaTinyCore`):**  
> ATtiny1616 exposes **8 ADC-capable pins** on the ADX bus, split across two hardware converters:
> - **`ADC0` (`P01`, `P02`, `P05`, `P06`)**: Directly supported by standard Arduino `analogRead()` in `megaTinyCore`.
> - **`ADC1` (`P07`, `P08`, `P09`, `P10`)**: Standard `megaTinyCore` `analogRead()` only addresses `ADC0`. Reading `PC0`–`PC3` as analog inputs requires direct `ADC1` register access (or a board-header helper function).

---

## 3. Dedicated Onboard Pins (Non-Exposed to IDC Bus)

The remaining 7 GPIO pins are reserved for onboard circuitry specific to each Core board variant:

| MCU Pin | QFN Pin | CORE-U (USB Standard, rev4) | CORE-A5 / A24 (Industrial RS-485) |
| :---: | :---: | :--- | :--- |
| **PA1** | 20 | **CH342K Port 1 TxD** (`USART0` Alt TxD) | **RS-485 TxD** (`USART0` Alt TxD → transceiver DI) |
| **PA2** | 1 | **CH342K Port 1 RxD** (`USART0` Alt RxD) | **RS-485 RxD** (`USART0` Alt RxD → RO & CCL LUT0-IN2) |
| **PA4** | 5 | **Onboard LED / Reserve** | **RS-485 DE & /RE** (`USART0` Alt `XDIR` auto-direction) |
| **PA5** | 6 | **Onboard LED / Reserve** (`VREFA` / `ADC0 AIN5`) | **Reserved / Internal (N.C.)** |
| **PA7** | 8 | **eFuse Fault Flag (`ADX_EFUSE_FLG`)** (Active-LOW, 10kΩ PU) | **Reserved / Internal (N.C.)** |
| **PB4** | 10 | **Onboard LED (`LED_R` / `Y` / `W` pool)** | **Onboard Red LED** (Active HIGH) |
| **PB5** | 9 | **Onboard LED (`LED_R` / `Y` / `W` pool)** | **Onboard Blue LED** (Active HIGH) |

> **Unconnected Pin (N.C.) Treatment Rule:**  
> To eliminate parasitic shoot-through current and noise vulnerability on any unused onboard pins (e.g., `PA5` / `PA7` on CORE-A5), firmware MUST disable input buffers during initialization:  
> `PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;`  
> `PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;`

---

## 4. PORTMUX & Direction Control Configuration Rules

According to Microchip Datasheet Table 5-1 and Section 15.3.2 (`PORTMUX.CTRLB`), firmware MUST apply the following register settings so bus peripherals align with `P01`–`P10` without conflicting with onboard `PA1`/`PA2`/`PA4` circuits:

| Register | Bit / Field | Required Setting | Value | Arduino (`megaTinyCore`) | Rationale |
| :--- | :--- | :--- | :---: | :--- | :--- |
| **`PORTMUX.CTRLB`** | **`SPI0`** (bit 2) | **Alternate** | `1` | `SPI.swap(1)` | Default (`PA1`–`PA4`) conflicts with onboard UART/RS-485. Routes SPI to **`P07`–`P10` (`PC3`–`PC0`)**. |
| **`PORTMUX.CTRLB`** | **`TWI0`** (bit 4) | **Default** | `0` | `Wire.swap(0)` (default) | Default routes I2C to **`P05`/`P06` (`PB0`/`PB1`)**. Alternate (`PA1`/`PA2`) conflicts with onboard circuits. |
| **`PORTMUX.CTRLB`** | **`USART0`** (bit 0) | **Dynamic** | `0` or `1` | `Serial.swap(0)` / `Serial.swap(1)` | `0` (Default): **IDC Bus `P03`/`P04` (`PB2`/`PB3`)**<br>`1` (Alternate): **Onboard CH342K / RS-485 (`PA1`/`PA2`)** |
| **`PORTMUX.CTRLD`** | **`TCB1`** (bit 1) | **Default** | `0` | — | Default routes TCB1 PWM output to **`P02` (`PA3`)**. |
| **`PORTMUX.CTRLD`** | **`TCB0`** (bit 0) | **Optional Alt** | `1` | — | Alternate routes TCB0 PWM output to **`P10` (`PC0`)**. |

### 4.1 USART0 Dynamic Switching Operation
- **IDC Bus UART (`P03` / `P04`)**: Set `PORTMUX.CTRLB &= ~PORTMUX_USART0_bm` (`Serial.swap(0)`) to communicate with external expansion Cards over Pins 7 and 8.
- **Onboard Serial / RS-485 (`PA1` / `PA2`)**: Set `PORTMUX.CTRLB |= PORTMUX_USART0_bm` (`Serial.swap(1)`) to route USART0 to the CH342K PC debug monitor (`CORE-U`) or isolated RS-485 transceiver (`CORE-A5` / `CORE-A24`).
- **RS-485 Hardware Auto-Direction (`XDIR` on `PA4`)**: When `PORTMUX.CTRLB.USART0 = 1` on RS-485 Core boards, `PA4` (`XDIR`) automatically drives tied `DE` and `/RE` pins without manual GPIO toggling. Because `PA2` is hardwired directly to CCL LUT0-IN2, incoming OTW break pulses trigger reset regardless of the current `PORTMUX.CTRLB` state.

---

## 5. Reference Board Header Snippet (`ADX_CORE_U.h`)

Following `ADX-BUS-SPEC-v4.2` Section 5.4, portable Arduino sketches for `CORE-U` use collision-safe `ADX_`-prefixed macros:

```cpp
#pragma once
#include <Arduino.h>

// ADX 20-Pin Bus (v4.2) Logical Pin IDs (S00 / PA0 intentionally unexposed)
#define ADX_P01   PIN_PA6
#define ADX_P02   PIN_PA3
#define ADX_P03   PIN_PB2
#define ADX_P04   PIN_PB3
#define ADX_P05   PIN_PB0
#define ADX_P06   PIN_PB1
#define ADX_P07   PIN_PC3
#define ADX_P08   PIN_PC2
#define ADX_P09   PIN_PC1
#define ADX_P10   PIN_PC0

// Standard Bus Function Aliases
#define ADX_AIN   ADX_P01
#define ADX_CLK   ADX_P02
#define ADX_TX    ADX_P03
#define ADX_RX    ADX_P04
#define ADX_SCL   ADX_P05
#define ADX_SDA   ADX_P06
#define ADX_CS    ADX_P07
#define ADX_MOSI  ADX_P08
#define ADX_MISO  ADX_P09
#define ADX_SCK   ADX_P10
```

---

## 6. Factory Calibration & VDD Self-Testing

ATtiny1616 Core boards do not route internal 3.3V power onto the 20-pin bus. Factory automated inspection verifies the onboard 3.3V LDO output voltage via the ATtiny1616 internal ADC:
1. Initialize ADC0 with `ADC_REFSEL_INTREF_gc` (1.1V internal bandgap).
2. Sample the internal VDD channel.
3. Transmit the calculated VDD value via USART0 Default (`P03` / IDC Pin 7) to the test fixture. Valid acceptance window: $3.20\,\text{V} \le V_{DD} \le 3.40\,\text{V}$.
