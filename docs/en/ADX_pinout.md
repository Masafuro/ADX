<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout Architecture & Design Guide

[ **English** | [日本語](../ja/ADX_pinout.md) ]

> [!IMPORTANT]
> **Single Source of Truth (SSOT):**  
> For the authoritative pin assignment table, strict electrical ratings, logical pin naming rules, software macro conventions, and mechanical dimensions, refer to the master specification: **[`docs/spec/ADX_PINOUT.md`](../spec/ADX_PINOUT.md)** (`ADX-BUS-SPEC-v5.0`).  
> This guide explains the engineering rationale, logical pin abstraction (`P01`–`P11`), panel-aligned power topology, coplanar shielding architecture, and practical rules for addon Card developers.

> [!WARNING]
> **Breaking Electrical Change (`v4.x` → `v5.0`):**  
> `ADX-BUS-SPEC-v5.0` omits the legacy `S00` (Pin 2: OWD/UPDI) line from the expansion bus and introduces `P03 (UART_DE)` on Pin 6. Because **Pins 3–6 swap signal and `GND` contacts relative to `v4.2`**, `v4.x` prototype hardware (`Reference CARD v0`, `Prototype CARD v0`, `CORE-U rev0–rev4`) and `v5.0` hardware (`CORE-U rev6+`) are **electrically incompatible and must not be cross-mated**. Print `ADX-BUS v5.0` clearly on new PCB silkscreens.

---

## 1. Overview & Engineering Philosophy

The ADX Common Expansion Bus utilizes a **$2 \times 10$-pin (20-pin) 2.54 mm pitch IDC connector** forming the standard backplane interface between Core Boards (MCU hosts) and Card Modules (isolated field-bus transceivers, isolated power supplies, sensors, and actuators).

The layout is engineered from physical ribbon wire ordering up to software abstraction to simultaneously achieve:
- **High vibration immunity and retention** (latched MIL-DTL-83503 Eject Headers)
- **Straight-through alignment with industrial wiring duct practices** (weak signals on top, heavy power on bottom)
- **High-current bidirectional power delivery ($2\text{A} \sim 3\text{A}$)** without thermal throttling
- **True dual-GND coplanar shielding** for analog and clock lines
- **HV-noise and accidental-reset immunity** by keeping programming lines (`UPDI` / `SWD`) strictly off the ribbon cable
- **Simultaneous collision-free RS-485 (`DE`/`TX`/`RX`) + I2C + SPI + AIN + EXTCLK operation**
- **MCU-independent logical pin identifiers (`P01`–`P11`)** for cross-architecture portability

```text
                    【 20-Pin Shrouded / Eject Box Header (Top View) — v5.0 】

                             ┌────────[ Center Key ]────────┐
                GND     [ 1] │  ●   ●  │ [ 2]  P01  (AIN)           ─── 【Outer Edge Shield & AIN Guard ①】
                GND     [ 3] │  ●   ●  │ [ 4]  P02  (EXTCLK)        ─── 【AIN Guard ② & EXTCLK Guard ①】
                GND     [ 5] │  ●   ●  │ [ 6]  P03  (UART_DE/XDIR)  ─── 【EXTCLK Guard ② & RS-485 DE】
    (UART_TX)   P04     [ 7] │  ●   ●  │ [ 8]  P05  (UART_RX)       ─── 【3-Wire Field UART / LIN Group】
    (I2C_SCL)   P06     [ 9] │  ●   ●  │ [10]  P07  (I2C_SDA)       ─── 【I2C Open-Drain Pair】
                GND     [11] │  ●   ●  │ [12]  P08  (SPI_CS)        ─── 【I2C / SPI Group Boundary】
   (SPI_MOSI)   P09     [13] │  ●   ●  │ [14]  P10  (SPI_MISO)      ─── 【High-Speed SPI Bus Group】
    (SPI_SCK)   P11     [15] │  ●   ●  │ [16]  GND                  ─── 【Protective Boundary Wall】
                VBUS    [17] │  ●   ●  │ [18]  VBUS                 ─── 【Panel-Aligned Bottom Power (2×2)】
                VBUS    [19] │  ●   ●  │ [20]  VBUS
                             └──────────────────────────────┘
```

---

## 2. Logical Pin Abstraction (`P01`–`P11`)

MCU-native port identifiers like `PA1` or `PA4` are specific to the ATtiny1616; migrating to another Core architecture such as CH32X035 or RP2040 would invalidate schematics, silkscreens, and firmware examples. `ADX-BUS-SPEC-v5.0` defines a clean MCU-independent **Logical Pin ID** layer (`P01`–`P11`) between the physical IDC pin number (`Pin 1`–`20`) and the baseline bus role (`UART_DE`, `UART_TX`, `SPI_CS`, etc.).

### 2.1 Why a Pure `P01`–`P11` Sequence?
1. **Role-Neutral Flat Numbering (No `D`/`A` Split)**:  
   Splitting pins into `D` (digital) and `A` (analog) creates semantic mismatch on modern microcontrollers where pins multiplex ADC, PWM, and serial functions. ADX uses a neutral `P` prefix for all bus signal pins.
2. **Zero Reserved-Pin Exceptions**:  
   With programming lines (`S00`) moved to Core-board internal responsibility in `v5.0`, all 11 non-power contacts on the 20-pin header form one unrestricted, sequential **`P01`–`P11`** series.
3. **Guaranteed Bus Contract vs. MCU-Specific Bonus Capabilities**:  
   - **Guaranteed Bus Contract**: Every Core board guarantees the baseline peripheral roles (`P01`=AIN, `P02`=EXTCLK/Clock/PWM, `P03–P05`=3-wire UART `DE`/`TX`/`RX`, `P06–P07`=I2C, `P08–P11`=SPI) plus 3.3V digital GPIO on all 11 pins (`P01`–`P11`).
   - **MCU-Specific Bonus Capabilities**: Extra ADC channels, DAC outputs, CCL inputs/outputs, or specialized timers are documented in each controller's `mcu_mapping/*.md` reference.

### 2.2 Software Abstraction Convention (`ADX_<BOARD>.h` & `ADX_P01`–`ADX_P11`)
Each Core board provides a lightweight header (`ADX_CORE_U_rev6.h`, `ADX_CORE_CX5.h`, etc.) exposing **`ADX_`-prefixed** pin macros:

- **Why `ADX_P01` instead of bare `P01`?**  
  Popular Arduino libraries for 16-bit I2C GPIO expanders (`PCF8575`, `PCA9555`, etc.) define global preprocessor macros `#define P00 0`–`#define P07 7` and `#define P10 8`. Because preprocessor macros bypass C++ namespaces, ADX headers strictly define **`ADX_P01`–`ADX_P11`** (plus functional aliases `ADX_AIN`, `ADX_DE`, `ADX_TX`, `ADX_CS`, etc.) and never define bare `Pxx` macros.

```cpp
#include "ADX_CORE_U_rev6.h"   // Swap one line when migrating Core boards

void setup() {
  pinMode(ADX_P08, OUTPUT);       // General-purpose digital I/O (alias: ADX_CS)
}

void loop() {
  int val = analogRead(ADX_P01);  // Guaranteed dual-GND shielded analog input (alias: ADX_AIN)
  digitalWrite(ADX_P08, val > 512);
}
```

---

## 3. Engineering Rationale & Structural Benefits

### 3.1 Panel-Aligned Power Placement (`VBUS`: Pins 17–20)
Placing the high-current external power bus (`VBUS`) at Pins 17–20 aligns the internal bus directly with the foundational industrial rule: **"Weak signals on top, heavy power on bottom."**

1. **Straight-Through Front I/O Terminal Alignment:** Solid power planes run straight along the bottom edge of the PCB into lower front-panel power terminals without crossing upper instrumentation/communication ducts.
2. **Zero Signal Crossings:** High-current delivery stays confined to the lower edge of the PCB, eliminating switching noise radiation and IR-drop coupling into `P01 (AIN)` and upper serial buses.
3. **Secondary Physical Safety:** Should a lower power conductor vibrate loose in a vertical mount, gravity drops it into the lower duct away from upper communication terminals.
4. **$2 \times 2$ Solid Copper Grid & `4.5V–5.5V` (`5V ±10%`) Bus Tolerance:** Pins 17–20 form a symmetrical $2 \times 2$ square pour supporting $2.0\,\text{A} \sim 3.0\,\text{A}$ continuous bidirectional power transfer with minimal IR drop. Standardizing mandatory operating tolerance across **`4.5V – 5.5V DC` (`5.0V ±10%`)** covers both USB Type-C `vSafe5V` sources (up to `5.50V`) and heavy-load IR drops (down to `4.5V`), while regulated sourcing modules are encouraged to target `4.75V – 5.25V`.

### 3.2 Protective Boundary Wall (`Pin 16: GND`)
`Pin 16 (GND)` sits directly above the `VBUS` cluster. In case of ribbon cable crimp misalignment or frayed strands, `VBUS` shorts directly to `GND`, tripping upstream eFuse/PTC protection rather than exposing 3.3V logic lines (Pins 1–15) to 5V.

### 3.3 Triple Odd-Row Ground Guard (`Pins 1, 3, 5: GND`)
- **PCB & Ribbon Synergy:** Pins 1, 3, and 5 occupy three consecutive odd-row contacts along the upper edge of the 2×10 header, tying cleanly into a single continuous ground pour.
- **True Dual-GND Coplanar Analog Shield (`P01` / Pin 2):** Wire #2 (`AIN`) is flanked directly by Wire #1 (`GND`) and Wire #3 (`GND`), providing stronger noise immunity than `v4.2` (where Wire #2 was `S00`).
- **Clock Waveguide (`P02` / Pin 4):** Wire #4 (`EXTCLK`) is flanked by Wire #3 (`GND`) and Wire #5 (`GND`), trapping clock and PWM EMI between ground returns.

### 3.4 Core-Side Programming Architecture (Omission of `S00` / `UPDI`)
- **HV Noise & Reset Immunity:** On ATtiny1616, `PA0 (UPDI/RESET)` is a 12V HV-programming-capable pin. Omitting it from the 20-pin ribbon cable eliminates a major EMI/HV-transient failure vector in industrial, outdoor, and automotive-cabin deployments.
- **Self-Contained Core Boards:** Each Core board provides its own programming/debug path internally (e.g., `CORE-U rev6` integrates USB-isolated `CH342K` Port 0 `SerialUPDI` for sketch flashing, bootloader updates, fuse management, and memory dumps; other MCUs may use USB ROM ISP/UF2 and/or onboard SWD pads).
- **No Card-Side Exceptions:** Both legacy Card rules (`S00` N.C. and `P02` $0\,\Omega$ DNP isolation) are completely eliminated.

### 3.5 Dedicated 3-Wire Field UART & Hardware RS-485 Control (`P03`–`P05` / Pins 6–8)
With isolated RS-485 / LIN transceivers delegated to expansion Cards, grouping **`P03 (UART_DE / XDIR)`** (Pin 6), **`P04 (UART_TX)`** (Pin 7), and **`P05 (UART_RX)`** (Pin 8) enables simultaneous collision-free RS-485 + I2C + SPI + AIN + EXTCLK stacking and avoids the low-baud / LIN-Break pitfalls of standalone auto-direction RS-485 ICs.

---

## 4. Design Guidelines for Addon Cards

Developers designing `v5.0`-compliant expansion Cards should observe the following guidelines:

1. **Power Ingestion (`4.5V–5.5V` Tolerance MUST), Shielding & PoL Regulation:**  
   Tie all four `VBUS` pins (Pins 17–20) to solid copper planes and bond Pin 16 (`GND` barrier wall) plus shield grounds (Pins 1, 3, 5, 11) solidly to the ground plane. **Every Card MUST safely tolerate the full `4.5V – 5.5V` (`5.0V ±10%`) `VBUS` window**; narrow-range 5V components rated strictly for `4.75V – 5.25V` MUST NOT be tied directly to `VBUS` without local voltage conditioning. **3.3V power is not distributed over the bus**; regulate locally from `VBUS` on the Card, avoiding `6.0V` absolute-maximum LDOs in favor of **`≥ 10V` operating-input regulators (recommended `12V – 30V` class)** for hot-plug ringing margin (or a buck DC-DC converter for high-current 3.3V Cards).
2. **Mandatory `P03 (UART_DE)` Pull-Down on RS-485 Cards (MUST):**  
   To prevent unprogrammed or resetting Core boards (High-Z `P03`) from jamming a shared RS-485 field bus, every RS-485 Card **MUST place a $10\,\text{k}\Omega \sim 100\,\text{k}\Omega$ pull-down resistor from `P03` to `GND`**, forcing the transceiver into Receive mode (`DE = LOW`) by default.
3. **Unrestricted GPIO Repurposing (`P01`–`P11`):**  
   When AIN, EXTCLK, UART, I2C, or SPI is unused on a Card, **all 11 signal pins (`P01`–`P11`)** may be freely repurposed as standard 3.3V digital GPIOs. *(Account for external I2C pull-ups on `P06`/`P07` or RS-485 pull-downs on `P03` in multi-card stacks.)*
4. **I2C Pull-Up Resistors:**  
   `P06` (Pin 9: `I2C_SCL`) and `P07` (Pin 10: `I2C_SDA`) may not have pull-ups populated on the Core board. Provide one pair of pull-ups ($2.2\,\text{k}\Omega \sim 4.7\,\text{k}\Omega$ to 3.3V) on the Card or bus backplane.
5. **Return Paths:**  
   Route high-speed SPI traces (`P08`–`P11` / Pins 12–15) and clock traces (`P02` / Pin 4) adjacent to neighboring ground pins (Pins 3, 5, 11, 16) for tight RF return paths.
6. **Silkscreen Labeling:**  
   Use **`P01`–`P11`, `GND`, `VBUS`** as primary labels alongside **`ADX-BUS v5.0`** to prevent accidental mating with legacy `v4.x` prototypes, appending alternate bus functions (`P03/DE`, `P04/TX`, `P08/CS`, etc.) and **`4.5V~5.5V`** power labeling where space allows.

---

## 5. Controller-Specific Peripheral References

For low-level register configuration, MCU-specific bonus capabilities, and PORTMUX settings, refer to the authoritative mapping specifications:

- **Microchip ATtiny1616 (`CORE-U` rev6 Flagship):** 👉 [`docs/spec/mcu_mapping/attiny1616.md`](../spec/mcu_mapping/attiny1616.md) (Full Matrix: [`adx_attiny1616_mcu_matrix.md`](adx_attiny1616_mcu_matrix.md))
- **WCH CH32X035 (`CORE-CX5` Reference):** 👉 [`docs/spec/mcu_mapping/ch32x035.md`](../spec/mcu_mapping/ch32x035.md)
- **Raspberry Pi RP2040 (`CORE-R`):** *(Planned)*
