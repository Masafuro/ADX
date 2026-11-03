<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout Architecture & Design Guide

[ **English** | [日本語 (../ja/ADX_pinout.md)](../ja/ADX_pinout.md) ]

> [!IMPORTANT]
> **Single Source of Truth (SSOT):**  
> For the authoritative pin assignment table, strict electrical ratings, signal timing, and mechanical dimensions, please refer to the master specification: **[`docs/spec/ADX_PINOUT.md`](../spec/ADX_PINOUT.md)**.  
> This guide explains the engineering rationale, panel-aligned power topology, coplanar shielding architecture, and practical rules for addon Card developers.

---

## 1. Overview & Engineering Philosophy

The ADX Common Expansion Bus utilizes a **$2 \times 10$-pin (20-pin) 2.54 mm pitch IDC connector** that forms the standard backplane interface connecting Core Boards (MCU hosts) and Card Modules (expansion addons).

Unlike unshielded pin headers or breadboard wiring, the layout is meticulously calculated down to the physical wire sequence in ribbon cables to simultaneously achieve:
- **High vibration immunity and retention**
- **Straight-through alignment with industrial wiring duct practices (weak signals on top, heavy power on bottom)**
- **High-current delivery ($2\text{A} \sim 3\text{A}$) without thermal throttling**
- **Coplanar shielding to suppress high-frequency radiated EMI**
- **Fail-safe boundary protection against ribbon cable crimp misalignments**

```text
                  【 20-Pin Shrouded Box Header (Top View) 】

                         ┌───────[ Center Key ]───────┐
              GND   [ 1]  │  ●   ●  │  [ 2]  OWD / PROG  ─── 【Outer Edge Shield & System Debug】
              AIN   [ 3]  │  ●   ●  │  [ 4]  GND         ─── 【AIN Guard & Clock Separation】
           EXTCLK   [ 5]  │  ●   ●  │  [ 6]  GND         ─── 【EXTCLK / SWCLK Coplanar Waveguide】
          UART_TX   [ 7]  │  ●   ●  │  [ 8]  UART_RX     ─── 【UART Balanced Pair】
          I2C_SCL   [ 9]  │  ●   ●  │  [10]  I2C_SDA     ─── 【I2C Open-Drain Pair】
              GND   [11]  │  ●   ●  │  [12]  SPI_CS      ─── 【I2C / SPI Group Boundary】
         SPI_MOSI   [13]  │  ●   ●  │  [14]  SPI_MISO    ─── 【High-Speed SPI Bus Group】
          SPI_SCK   [15]  │  ●   ●  │  [16]  GND         ─── 【Protective Boundary Wall】
             VBUS   [17]  │  ●   ●  │  [18]  VBUS        ─── 【Panel-Aligned Bottom Power ($2 \times 2$)】
             VBUS   [19]  │  ●   ●  │  [20]  VBUS
                         └────────────────────────────┘
```

---

## 2. Engineering Rationale & Structural Benefits

### 2.1 Panel-Aligned Power Placement (`VBUS`: Pins 17–20)
Placing the high-current external power bus (`VBUS`) at Pins 17–20 aligns the internal bus directly with the foundational industrial rule: **"Weak signals on top, heavy power on bottom."**

1. **Straight-Through Front I/O Terminal Alignment:**  
   Industrial wiring ducts universally isolate top ducts for instrumentation and communication from bottom ducts for high-current power. By positioning `VBUS` at the lowest row of the backplane connector, a solid power plane runs straight along the bottom edge of the PCB into the lower front-panel power terminals, eliminating any need for power and communication wiring to cross inside the cabinet.
2. **Elimination of On-Board Crossings (Zero Signal Crossings):**  
   Because high-current delivery is confined to the lower edge of the PCB, power routing never crosses or runs parallel to delicate analog inputs (`AIN`) or high-speed communication pairs (`SPI`, `UART`) routed in the upper half of the board, completely eliminating switching noise radiation and IR-drop interference.
3. **Secondary Physical Safety:**  
   As an inherent physical side-effect of this bottom placement, should a front screw terminal vibrate loose in a vertical slot, the fallen wire hangs or drops safely downward into the lower wiring duct, preventing accidental contact with communication lines above.
4. **$2 \times 2$ Solid Copper Grid:**  
   Pins 17–20 form a symmetrical $2 \times 2$ square, enabling PCB layouts to pour uninterrupted power planes with low IR drop, handling up to $2.0\,\text{A} \sim 3.0\,\text{A}$ continuously with excellent heat sinking.

### 2.2 Protective Boundary Wall (`Pin 16: GND`)
- **Fault Isolation Barrier:**  
  `Pin 16 (GND)` is placed immediately adjacent to the high-power `VBUS` cluster. Should a ribbon cable crimp misalignment or stray wire strand occur across the power boundary, `VBUS` shorts safely to ground, tripping upstream PTC or load-switch protection rather than exposing low-voltage logic (Pins 1–15) to $5\,\text{V}$ power.

### 2.3 Outer Perimeter Guard & Coplanar Shielded Analog (`Pins 1–4`)
- **Outer Edge ESD Protection:**  
  `Pin 1 (GND)` occupies the outermost edge of the flat cable, buffering the entire bus from chassis rubbing and electrostatic discharge (ESD).
- **Analog Isolation:**  
  Pin 3 (`AIN`) is sandwiched between Pin 1 (`GND`) and Pin 4 (`GND`), forming a coplanar waveguide structure. This topology prevents fast edge switching on Pin 5 (`EXTCLK`) from coupling into delicate analog transducer readings.

### 2.4 Clock Radiated EMI Suppression (`Pins 4–6`)
- High-speed clock line Pin 5 (`EXTCLK`) is flanked by Pin 4 (`GND`) and Pin 6 (`GND`), forming a coplanar waveguide structure. This topology traps high-frequency harmonics from external system clocks and synchronous debug clocks (`SWCLK`), preventing radiated EMI and crosstalk into adjacent analog or communication pairs.

### 2.5 System Debug & Production Programming Pair (`Pins 2 & 5`)
- **Integration of 1-Wire and 2-Wire Programming Topologies:**  
  To provide universal programming support across microcontrollers from 8-bit to 32-bit RISC-V/ARM architectures, Pins 2 and 5 form a dedicated system management pair:
  - **CORE-A5 (ATtiny1616)**: Pin 2 operates as `PA0` (UPDI) for 1-wire factory programming.
  - **CORE-CX5 (CH32X035)**: Pin 2 operates as `PC18` (SWDIO) and Pin 5 as `PC19` (SWCLK). **Mating the 20-pin IDC Eject Header to the factory test fixture completes both power injection and 2-wire SDI programming with zero pogo pins required.**
- **Production & Field Lifecycle:**  
  Enables automated testing and initial bootloader (UF2) flashing via the 20-pin IDC Eject Header during assembly, while end users perform driverless drag-and-drop sketch maintenance over USB in the field.

---

## 3. Design Guidelines for Addon Cards

Developers designing ADX-compliant expansion Cards should observe the following guidelines:

1. **Power Ingestion & PoL Regulation:**  
   Tie all four pins of `VBUS` (Pins 17–20) to solid copper planes. **3.3V logic power is not distributed over the bus**; Cards requiring 3.3V must regulate locally from `VBUS` using an onboard LDO or DC-DC stage.
2. **Pin 2 (OWD) Rule (MUST):**  
   Standard Cards **MUST leave Pin 2 unconnected (N.C.)**. Direct connections to push-buttons or hardware pull-downs to GND are strictly prohibited.
3. **Pin 5 (EXTCLK) Isolation Recommendation (RECOMMENDED / SHOULD):**  
   Where a Card module feeds an external clock into the host MCU or connects monitoring circuitry to Pin 5, the Card **SHOULD insert a series $0\,\Omega$ resistor (or solder jumper) to allow complete electrical disconnection (DNP: Do Not Populate)**.  
   - **Interoperability Rationale**: Populating the $0\,\Omega$ resistor supplies external clock on ATtiny1616 hosts, while depopulating (DNP) ensures transparent compatibility with Core boards utilizing Pin 5 as `SWCLK` (e.g., CORE-CX5) or timer PWM outputs without signal collisions.
4. **I2C Pull-Up Resistors:**  
   Pins 9 (`I2C_SCL`) and 10 (`I2C_SDA`) may not have pull-up resistors on the Core board. Provide one pair of pull-ups ($2.2\,\text{k}\Omega \sim 4.7\,\text{k}\Omega$ to 3.3V) on the Card or bus backplane.
5. **Return Paths:**  
   Route high-speed SPI traces (Pins 12–15) and clock lines close to neighboring ground pins (Pins 11, 16) to ensure continuous RF return paths.

---

## 4. Controller-Specific Peripheral References

For low-level register configuration, timer mapping, and PORTMUX settings, refer to the authoritative mapping specifications:

- **Microchip ATtiny1616 (CORE-A5):** 👉 [`docs/spec/mcu_mapping/attiny1616.md`](../spec/mcu_mapping/attiny1616.md)
- **WCH CH32X035 (CORE-CX5):** 👉 [`docs/spec/mcu_mapping/ch32x035.md`](../spec/mcu_mapping/ch32x035.md)
- **Raspberry Pi RP2040 (CORE-R):** 👉 [`docs/spec/mcu_mapping/rp2040.md`](../spec/mcu_mapping/rp2040.md)
