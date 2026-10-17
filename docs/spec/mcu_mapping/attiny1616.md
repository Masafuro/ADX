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

This document specifies how the universal signals of the **ADX 20-Pin Bus (v3.0)** are mapped to the physical pins, peripherals, and PORTMUX channels of the **Microchip ATtiny1616** microcontroller.

---

## 2. Bus to MCU Physical Interconnect Table

| IDC Pin | Bus Net Name | ATtiny1616 Pin | QFN Pin | Primary MCU Peripheral & Alternate Functions | Notes |
| :---: | :--- | :--- | :---: | :--- | :--- |
| **1** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) | Pins 1–4 tied to VBUS plane |
| **2** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) | |
| **3** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) | |
| **4** | **VBUS** | - | - | System Power Bus (4.5V–5.0V Input/Output) | |
| **5** | **GND** | - | - | VBUS Primary Ground Return & Zone Barrier | Directly adjacent to VBUS cluster |
| **6** | **SYS_RST** | **PA0** | 19 | **UPDI / RESET** | 1-wire programming & hardware reset |
| **7** | **AIN** | **PA6** | 7 | **ADC0 AIN6** / ADC1 AIN2 / **DAC0 OUT** | 10-bit ADC Input / 8-bit DAC Output |
| **8** | **GND** | - | - | Ground Return / Coplanar Shield | Analog/Clock shield |
| **9** | **EXTCLK** | **PA3** | 2 | **EXTCLK** / ADC0 AIN3 / TCB1 WO | External clock / PWM output |
| **10**| **GND** | - | - | Ground Return / Inter-bus Isolation | Clock/Communication isolation |
| **11**| **UART_TX** | **PB2** | 12 | **USART0 TxD (Alternate)** / TCA0 WO2 | PORTMUX Alternate USART0 |
| **12**| **UART_RX** | **PB3** | 11 | **USART0 RxD (Alternate)** / TCA0 WO0 | PORTMUX Alternate USART0 |
| **13**| **I2C_SCL** | **PB0** | 14 | **TWI0 SCL (Default)** / TCA0 WO0 | Hardware I2C Clock |
| **14**| **I2C_SDA** | **PB1** | 13 | **TWI0 SDA (Default)** / TCA0 WO1 | Hardware I2C Data |
| **15**| **GND** | - | - | Ground Return / Mid-bus Shield | Flanking I2C and SPI buses |
| **16**| **SPI_CS** | **PC3** | 18 | **SPI0 SS (Alternate)** / TCA0 WO3 | Active-LOW Chip Select |
| **17**| **SPI_MOSI**| **PC2** | 17 | **SPI0 MOSI (Alternate)** / EVOUT2 | SPI Controller Out / Peripheral In |
| **18**| **SPI_MISO**| **PC1** | 16 | **SPI0 MISO (Alternate)** / TCD0 WOD | SPI Controller In / Peripheral Out |
| **19**| **SPI_SCK** | **PC0** | 15 | **SPI0 SCK (Alternate)** / TCD0 WOC / TCB0 WO | SPI Serial Clock |
| **20**| **GND** | - | - | Ground Return / Bus Termination Shield | Outer bus termination |

---

## 3. Dedicated Onboard Pins (Non-Exposed to IDC Bus)

The following 7 GPIO pins are reserved for dedicated onboard circuitry or internal reserves:

| MCU Pin | QFN Pin | Dedicated Board Function | Electrical Connection & Notes |
| :--- | :---: | :--- | :--- |
| **PA1** | 20 | **RS-485 TxD** | USART0 Default TxD (DI pin of isolated transceiver). |
| **PA2** | 1 | **RS-485 RxD** | USART0 Default RxD (RO pin) ➜ Hardwired to **CCL LUT0-IN2**. |
| **PA4** | 5 | **RS-485 DE & /RE** | **USART0 XDIR** hardware auto-direction tied directly to both DE and /RE pins. |
| **PB4** | 10 | **Red LED** | Active HIGH onboard status indicator. |
| **PB5** | 9 | **Blue LED** | Active HIGH onboard status/comm indicator. |
| **PA5** | 6 | **Reserved / Internal** | ADC0 AIN5 / VREFA / TCB0 WO / TCD0 WOB (Unconnected / internal reserve). |
| **PA7** | 8 | **Reserved / Internal** | ADC0 AIN7 / CCL LUT1-OUT (Unconnected / internal reserve). |

> **Unconnected Pin (N.C.) Treatment Rule (PA5 / PA7):**  
> To eliminate parasitic shoot-through current and noise vulnerability without adding external components, firmware MUST disable the input buffers or enable internal pull-ups on startup:  
> `PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;`  
> `PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;`  
> In PCB layout, leave the pads completely isolated or route to small internal test lands with no external stubs.


---

## 4. PORTMUX & Direction Control Configuration Rules

To route serial and timer peripherals to the universal 20-pin IDC bus without conflicting with the dedicated onboard RS-485 transceiver, firmware MUST apply the following register settings:

| Register | Bit / Field | Required Setting | Value | Rationale |
| :--- | :--- | :--- | :---: | :--- |
| **`PORTMUX.CTRLB`** | **`SPI0`** (bit 2) | **Alternate** | `1` | Default (PA1–PA4) conflicts with RS-485. Routes SPI to **PC0–PC3 (Pins 16–19)**. |
| **`PORTMUX.CTRLB`** | **`TWI0`** (bit 4) | **Default** | `0` | Default routes I2C to **PB0/PB1 (Pins 13, 14)**. Alternate conflicts with RS-485. |
| **`PORTMUX.CTRLB`** | **`USART0`** (bit 0) | **Dynamic** | `0` or `1` | `0`: Onboard RS-485 (`PA1/PA2`)<br>`1`: External IDC Bus (`PB2/PB3`, Pins 11, 12) |
| **`PORTMUX.CTRLD`** | **`TCB1`** (bit 1) | **Default** | `0` | Default routes TCB1 PWM output to Pin 9 (`PA3`). |

### 4.1 RS-485 Hardware Auto-Direction Control (`XDIR`)
- By hardwiring `DE` (Active HIGH) and `/RE` (Active LOW) together on the PCB and driving them from `PA4 (XDIR)`, the transceiver hardware automatically:
  - Asserts `DE=1` and `/RE=1` one baud period prior to data transmission (Driver ON, Receiver OFF), preventing self-reception echo.
  - De-asserts `DE=0` and `/RE=0` immediately upon stop bit completion (Driver OFF, Receiver ON).
- Firmware transmits data via standard serial write without manual GPIO direction switching overhead.

### 4.2 USART0 Dynamic Switching Operation
- **Factory Test & Programming**: Set `PORTMUX.CTRLB |= PORTMUX_USART0_bm` (value `1`) to route serial debug output to IDC Pins 11 and 12 for automated PC test fixtures.
- **Field Operation**: Set `PORTMUX.CTRLB &= ~PORTMUX_USART0_bm` (value `0`) to engage the isolated RS-485 transceiver on PA1/PA2.
- **OTW Break Reset Autonomy**: Because `PA2` (RS-485 RX) is hardwired directly to CCL LUT0 input, incoming break pulses trigger software reset regardless of the current `PORTMUX.CTRLB` state.

---

## 5. Factory Calibration & VDD Self-Testing

The CORE-A5 board does not route internal 3.3V power to external pins. Factory automated inspection verifies the onboard 3.3V LDO output voltage via the ATtiny1616 internal ADC:
1. Initialize ADC0 with `ADC_REFSEL_INTREF_gc` (1.1V internal bandgap).
2. Sample the internal VDD channel (`ADC_MUXPOS_INTREF_gc` with VDD reference or scaled VDD).
3. Transmit the calculated VDD value via USART0 Alternate (IDC Pin 11) to the test fixture. Valid acceptance window: $3.20\,\text{V} \le V_{DD} \le 3.40\,\text{V}$.
