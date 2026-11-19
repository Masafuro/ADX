<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout Architecture & Design Guide

[ **English** | [日本語](../ja/ADX_pinout.md) ]

> [!IMPORTANT]
> **Single Source of Truth (SSOT):**  
> For the authoritative pin assignment table, strict electrical ratings, logical pin naming rules, software macro conventions, and mechanical dimensions, please refer to the master specification: **[`docs/spec/ADX_PINOUT.md`](../spec/ADX_PINOUT.md)** (`ADX-BUS-SPEC-v4.2`).  
> This guide explains the engineering rationale, logical pin abstraction (`S00`, `P01`–`P10`), panel-aligned power topology, coplanar shielding architecture, and practical rules for addon Card developers.

---

## 1. Overview & Engineering Philosophy

The ADX Common Expansion Bus utilizes a **$2 \times 10$-pin (20-pin) 2.54 mm pitch IDC connector** that forms the standard backplane interface connecting Core Boards (MCU hosts) and Card Modules (expansion addons).

Unlike unshielded pin headers or breadboard wiring, the layout is calculated from the physical wire sequence up to the software abstraction layer to simultaneously achieve:
- **High vibration immunity and retention**
- **Straight-through alignment with industrial wiring duct practices (weak signals on top, heavy power on bottom)**
- **High-current delivery ($2\text{A} \sim 3\text{A}$) without thermal throttling**
- **Coplanar shielding to suppress high-frequency radiated EMI**
- **Fail-safe boundary protection against ribbon cable crimp misalignments**
- **MCU-independent logical pin identifiers (`S00`, `P01`–`P10`) for cross-architecture portability**

```text
                    【 20-Pin Shrouded Box Header (Top View) 】

                             ┌────────[ Center Key ]────────┐
                GND     [ 1] │  ●   ●  │ [ 2]  S00  (OWD / PROG)  ─── 【Outer Edge Shield & System Only】
       (AIN)    P01     [ 3] │  ●   ●  │ [ 4]  GND                ─── 【P01 (AIN) Coplanar Guard】
    (EXTCLK)    P02     [ 5] │  ●   ●  │ [ 6]  GND                ─── 【P02 (EXTCLK/SWCLK) Waveguide】
   (UART_TX)    P03     [ 7] │  ●   ●  │ [ 8]  P04  (UART_RX)     ─── 【UART Balanced Pair】
   (I2C_SCL)    P05     [ 9] │  ●   ●  │ [10]  P06  (I2C_SDA)     ─── 【I2C Open-Drain Pair】
                GND     [11] │  ●   ●  │ [12]  P07  (SPI_CS)      ─── 【I2C / SPI Group Boundary】
  (SPI_MOSI)    P08     [13] │  ●   ●  │ [14]  P09  (SPI_MISO)    ─── 【High-Speed SPI Bus Group】
   (SPI_SCK)    P10     [15] │  ●   ●  │ [16]  GND                ─── 【Protective Boundary Wall】
                VBUS    [17] │  ●   ●  │ [18]  VBUS               ─── 【Panel-Aligned Bottom Power (2×2)】
                VBUS    [19] │  ●   ●  │ [20]  VBUS
                             └──────────────────────────────┘
```

---

## 2. Logical Pin Abstraction (`S00`, `P01`–`P10`)

MCU-native port identifiers like `PA0` or `PB2` are specific to the ATtiny1616; migrating to another Core architecture such as CH32X035 or RP2040 would invalidate schematics, silkscreens, and firmware examples. `ADX-BUS-SPEC-v4.2` introduces an MCU-independent **Logical Pin ID** layer (`S00`, `P01`–`P10`) between the physical IDC pin number (`Pin 1`–`20`) and the peripheral bus role (`UART_TX`, `SPI_CS`, etc.).

### 2.1 Why `S00` and `P01`–`P10`?
1. **Role-Neutral Flat Numbering (No `D`/`A` Split)**:  
   Splitting pins into `D` (digital) and `A` (analog) as in legacy Arduino conventions creates ambiguity on modern microcontrollers where most pins multiplex ADC, PWM, and serial functions. Inspired by flat naming schemes such as Raspberry Pi Pico (`GP0`–`GP28`), ADX uses a neutral `P` prefix for all user-accessible bus pins.
2. **Clear Visual Separation of System Pin `S00`**:  
   Power rails (`GND`, `VBUS`) are left unnumbered. Pin 2 (`OWD / PROG`) is assigned **`S00` (System 00)**, leaving exactly ten user/Card pins cleanly numbered **`P01`–`P10`**. The distinct `S` prefix immediately signals on silkscreens and schematics that `S00` is reserved for system programming and must not be driven by standard Cards.
3. **Guaranteed Bus Contract vs. MCU-Specific Bonus Capabilities**:  
   Secondary multiplexed features on any pin (for instance, whether `P10 / SPI_SCK` also carries ADC or high-frequency PWM) vary completely by MCU silicon. ADX cleanly separates the two:
   - **Guaranteed Bus Contract**: Every Core board guarantees the baseline peripheral roles (`P01`=AIN, `P02`=EXTCLK/SWCLK, `P03–P04`=UART, `P05–P06`=I2C, `P07–P10`=SPI) plus 3.3V digital GPIO on `P01`–`P10`.
   - **MCU-Specific Bonus Capabilities**: Extra ADC channels, DAC outputs, or specialized timers available on a specific Core board are documented in its individual `mcu_mapping/*.md` reference.

### 2.2 Software Abstraction Convention (`ADX_<BOARD>.h` & `ADX_P01`–`ADX_P10`)
Each Core board provides a lightweight header (`ADX_CORE_U.h`, `ADX_CORE_CX5.h`, etc.) exposing **`ADX_`-prefixed** pin macros:

- **Why `ADX_P01` instead of bare `P01`?**  
  Popular Arduino libraries for 16-bit I2C GPIO expanders (`PCF8575`, `PCA9555`, etc.) define global preprocessor macros `#define P00 0` through `#define P07 7` and `#define P10 8`. Because preprocessor macros bypass even C++ namespaces, ADX headers strictly use **`ADX_P01`–`ADX_P10`** (plus functional aliases `ADX_AIN`, `ADX_CS`, etc.) and never define bare `Pxx` macros.
- **System Pin Safety**: `S00` is left undefined in standard board headers so application firmware cannot accidentally toggle the programming line.

```cpp
#include "ADX_CORE_U.h"   // Swap one line to "ADX_CORE_CX5.h" when changing Core boards

void setup() {
  pinMode(ADX_P07, OUTPUT);       // General-purpose digital I/O (alias: ADX_CS)
}

void loop() {
  int val = analogRead(ADX_P01);  // Guaranteed shielded analog input (alias: ADX_AIN)
  digitalWrite(ADX_P07, val > 512);
}
```

---

## 3. Engineering Rationale & Structural Benefits

### 3.1 Panel-Aligned Power Placement (`VBUS`: Pins 17–20)
Placing the high-current external power bus (`VBUS`) at Pins 17–20 aligns the internal bus directly with the foundational industrial rule: **"Weak signals on top, heavy power on bottom."**

1. **Straight-Through Front I/O Terminal Alignment:**  
   Industrial wiring ducts universally isolate top ducts for instrumentation and communication from bottom ducts for high-current power. By positioning `VBUS` at the lowest row of the backplane connector, a solid power plane runs straight along the bottom edge of the PCB into the lower front-panel power terminals, eliminating any need for power and communication wiring to cross inside the cabinet.
2. **Elimination of On-Board Crossings (Zero Signal Crossings):**  
   Because high-current delivery is confined to the lower edge of the PCB, power routing never crosses or runs parallel to delicate analog inputs (`P01 / AIN`) or high-speed communication pairs (`SPI`, `UART`) routed in the upper half of the board, completely eliminating switching noise radiation and IR-drop interference.
3. **Secondary Physical Safety:**  
   As an inherent physical side-effect of this bottom placement, should a front screw terminal vibrate loose in a vertical slot, the fallen wire hangs or drops safely downward into the lower wiring duct, preventing accidental contact with communication lines above.
4. **$2 \times 2$ Solid Copper Grid:**  
   Pins 17–20 form a symmetrical $2 \times 2$ square, enabling PCB layouts to pour uninterrupted power planes with low IR drop, handling up to $2.0\,\text{A} \sim 3.0\,\text{A}$ continuously with excellent heat sinking.

### 3.2 Protective Boundary Wall (`Pin 16: GND`)
- **Fault Isolation Barrier:**  
  `Pin 16 (GND)` is placed immediately adjacent to the high-power `VBUS` cluster. Should a ribbon cable crimp misalignment or stray wire strand occur across the power boundary, `VBUS` shorts safely to ground, tripping upstream PTC or load-switch protection rather than exposing low-voltage logic (Pins 1–15) to $5\,\text{V}$ power.

### 3.3 Outer Perimeter Guard & Coplanar Shielded Analog (`Pins 1–4`)
- **Outer Edge ESD Protection:**  
  `Pin 1 (GND)` occupies the outermost edge of the flat cable, buffering the entire bus from chassis rubbing and electrostatic discharge (ESD).
- **Analog Isolation:**  
  `P01` (Pin 3: `AIN`) is sandwiched between Pin 1 (`GND`) and Pin 4 (`GND`), forming a coplanar waveguide structure. This topology prevents fast edge switching on `P02` (Pin 5: `EXTCLK`) from coupling into delicate analog transducer readings.

### 3.4 Clock Radiated EMI Suppression (`Pins 4–6`)
- High-speed clock line `P02` (Pin 5: `EXTCLK`) is flanked by Pin 4 (`GND`) and Pin 6 (`GND`), forming a coplanar waveguide structure. This topology traps high-frequency harmonics from external system clocks and synchronous debug clocks (`SWCLK`), preventing radiated EMI and crosstalk into adjacent analog or communication pairs.

### 3.5 System Debug & Production Programming Pair (`S00` [Pin 2] & `P02` [Pin 5])
- **Integration of 1-Wire and 2-Wire Programming Topologies:**  
  To provide universal programming support across microcontrollers from 8-bit to 32-bit RISC-V/ARM architectures, `S00` and `P02` form a dedicated system management pair:
  - **CORE-U / CORE-A5 (ATtiny1616)**: `S00` (Pin 2) operates as `PA0` (UPDI) for 1-wire factory programming.
  - **CORE-CX5 (CH32X035)**: `S00` (Pin 2) operates as `PC18` (SWDIO) and `P02` (Pin 5) as `PC19` (SWCLK). **Mating the 20-pin IDC Eject Header to the factory test fixture completes both power injection and 2-wire SDI programming with zero pogo pins required.**
- **Production & Field Lifecycle:**  
  Enables automated testing and initial firmware/bootloader flashing via the 20-pin IDC header during assembly, while end users perform field maintenance over communication ports.

---

## 4. Design Guidelines for Addon Cards

Developers designing ADX-compliant expansion Cards should observe the following guidelines:

1. **Power Ingestion & PoL Regulation:**  
   Tie all four pins of `VBUS` (Pins 17–20) to solid copper planes. **3.3V logic power is not distributed over the bus**; Cards requiring 3.3V must regulate locally from `VBUS` using an onboard LDO or DC-DC stage.
2. **`S00` (Pin 2: `OWD`) Rule (MUST):**  
   Standard Cards **MUST leave `S00` (Pin 2) unconnected (N.C.)**. Direct connections to push-buttons or hardware pull-downs to GND are strictly prohibited.
3. **`P02` (Pin 5: `EXTCLK`) Isolation Recommendation (RECOMMENDED / SHOULD):**  
   Where a Card module feeds an external clock into the host MCU or connects monitoring/GPIO circuitry to `P02`, the Card **SHOULD insert a series $0\,\Omega$ resistor (or solder jumper) to allow complete electrical disconnection (DNP: Do Not Populate)**.  
   - **Interoperability Rationale**: Populating the $0\,\Omega$ resistor enables clock injection or GPIO use on ATtiny1616 hosts, while depopulating (DNP) ensures transparent compatibility with Core boards utilizing `P02` as `SWCLK` (e.g., CORE-CX5) without signal collisions.
4. **General-Purpose GPIO Repurposing (`P01`, `P03`–`P10`):**  
   When UART, I2C, or SPI is unused on a Card, the corresponding pins (`P03`–`P10`, plus `P01`) may be freely used as standard 3.3V digital GPIOs. *(Account for external I2C pull-ups on `P05`/`P06` in multi-card setups.)*
5. **I2C Pull-Up Resistors:**  
   `P05` (Pin 9: `I2C_SCL`) and `P06` (Pin 10: `I2C_SDA`) may not have pull-up resistors on the Core board. Provide one pair of pull-ups ($2.2\,\text{k}\Omega \sim 4.7\,\text{k}\Omega$ to 3.3V) on the Card or bus backplane.
6. **Return Paths:**  
   Route high-speed SPI traces (`P07`–`P10` / Pins 12–15) and clock lines close to neighboring ground pins (Pins 11, 16) to ensure continuous RF return paths.
7. **Silkscreen Labeling:**  
   Use **`S00`, `P01`–`P10`, `GND`, `VBUS`** as primary labels near the 20-pin header, appending alternate bus functions (`P03/TX`, `P07/CS`, etc.) where board area allows.

---

## 5. Controller-Specific Peripheral References

For low-level register configuration, MCU-specific bonus capabilities (additional ADC/PWM/DAC channels), and PORTMUX settings, refer to the authoritative mapping specifications:

- **Microchip ATtiny1616 (`CORE-U`, `CORE-A5`):** 👉 [`docs/spec/mcu_mapping/attiny1616.md`](../spec/mcu_mapping/attiny1616.md) (Full Matrix: [`adx_attiny1616_mcu_matrix.md`](adx_attiny1616_mcu_matrix.md))
- **WCH CH32X035 (`CORE-CX5`):** 👉 [`docs/spec/mcu_mapping/ch32x035.md`](../spec/mcu_mapping/ch32x035.md)
- **Raspberry Pi RP2040 (`CORE-R`):** *(Planned)*
