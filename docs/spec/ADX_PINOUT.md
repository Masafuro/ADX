<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus Specification

**Standard Identifier**: ADX-BUS-SPEC-v5.0  
**Category**: Core Electrical & Interconnect Specification (Single Source of Truth)  
**Status**: Standard (Release)  
**Maintained by**: ADX Hardware Standards Committee  

---

## 1. Overview & Scope

The **ADX 20-Pin Bus** is the universal physical and electrical interconnect standard for the ADX modular hardware ecosystem. It establishes a unified, vibration-resistant backplane interface connecting **Core Boards** (main MCUs and host controllers) and **Card Modules** (expansion addons, isolated field-bus transceivers, isolated power supplies, sensors, and actuators).

This document serves as the authoritative **Single Source of Truth (SSOT)** for physical pin assignments, MCU-independent logical pin identifiers (`P01`–`P11`), electrical limits, signal definitions, and mandatory design rules for the 20-pin IDC interface.

> [!WARNING]
> **Breaking Electrical Change (`v4.x` → `v5.0`)**:  
> `ADX-BUS-SPEC-v5.0` omits the legacy system programming line (`S00` / Pin 2) from the 20-pin expansion bus, strengthens coplanar analog shielding (`Pin 1 & Pin 3 GND`), and introduces a dedicated RS-485 Driver Enable line (`P03 / UART_DE` on Pin 6). Because **Pins 3–6 swap signal and `GND` assignments relative to `v4.2`**, `v4.x` prototype boards (e.g., `Reference CARD v0`, `Prototype CARD v0`, `CORE-U rev0–rev4`) and `v5.0` boards (`CORE-U rev6+`) are **electrically incompatible and MUST NOT be cross-mated**. All `v5.0`-compliant PCBs SHOULD clearly print `ADX-BUS v5.0` on silkscreen.

---

## 2. Physical & Mechanical Interface

- **Connector Family**: $2 \times 10$-pin (20-pin), 2.54 mm (0.100") pitch shrouded box header or ejector/latch header (MIL-DTL-83503 compliant).
- **Mating Cable**: Standard 20-conductor flat ribbon cable (AWG 28 stranded).
- **Pin Numbering Convention**: Standard zigzag (interleaved) ribbon cable numbering:
  - Row 1 (Odd pins): 1, 3, 5, 7, 9, 11, 13, 15, 17, 19
  - Row 2 (Even pins): 2, 4, 6, 8, 10, 12, 14, 16, 18, 20

```text
                  【 20-Pin Shrouded / Eject Box Header (Top View) 】

                             ┌────────[ Center Key ]────────┐
                GND     [ 1] │  ●   ●  │ [ 2]  P01  (AIN)
                GND     [ 3] │  ●   ●  │ [ 4]  P02  (EXTCLK)
                GND     [ 5] │  ●   ●  │ [ 6]  P03  (UART_DE / XDIR)
    (UART_TX)   P04     [ 7] │  ●   ●  │ [ 8]  P05  (UART_RX)
    (I2C_SCL)   P06     [ 9] │  ●   ●  │ [10]  P07  (I2C_SDA)
                GND     [11] │  ●   ●  │ [12]  P08  (SPI_CS)
   (SPI_MOSI)   P09     [13] │  ●   ●  │ [14]  P10  (SPI_MISO)
    (SPI_SCK)   P11     [15] │  ●   ●  │ [16]  GND
                VBUS    [17] │  ●   ●  │ [18]  VBUS
                VBUS    [19] │  ●   ●  │ [20]  VBUS
                             └──────────────────────────────┘
```

---

## 3. Universal Pin Assignment Table

The bus separates every contact into three layers: **Physical IDC Pin** (`1`–`20`), **Logical Pin ID** (`P01`–`P11`, plus unnumbered power rails `GND` / `VBUS`), and **Standard Bus Function** (guaranteed baseline peripheral role). All 11 signal pins (`P01`–`P11`) are unrestricted, user/Card-accessible 3.3V signals.

| IDC Pin | Logical ID | Bus Function | Signal Class | Electrical Standard | Primary Function / Guaranteed Role | Shield & Topology Role |
| :---: | :---: | :--- | :--- | :--- | :--- | :--- |
| **1** | **GND** | — | Power / Shield | 0V Return | Cable Outer Edge Shield & Analog Return | Wire #1 (Edge ESD / `P01` Guard ①) |
| **2** | **P01** | **AIN** | General / Analog | 0V – 3.3V Analog / GPIO | High-Precision Analog Input (ADC) / Opt. DAC / 3.3V GPIO | Wire #2 (True Coplanar Shielded by Pins 1 & 3) |
| **3** | **GND** | — | Power / Shield | 0V Return | Analog / Clock Isolation Shield | Wire #3 (`P01` Guard ② / `P02` Guard ①) |
| **4** | **P02** | **EXTCLK** | General / Clock | 3.3V LVCMOS | External Clock / Timer PWM / 3.3V GPIO | Wire #4 (Clock Waveguide Trace) |
| **5** | **GND** | — | Power / Shield | 0V Return | Clock / Communications Isolation Shield | Wire #5 (`P02` Guard ② / UART Guard) |
| **6** | **P03** | **UART_DE** | General / Comm | 3.3V LVCMOS (Card PD req.) | RS-485 Driver Enable (`XDIR` / Active-HIGH) / 3.3V GPIO | Wire #6 (3-Wire Field UART Group) |
| **7** | **P04** | **UART_TX** | General / Comm | 3.3V LVCMOS | Asynchronous Serial Transmit (TXD) / 3.3V GPIO | Wire #7 (3-Wire Field UART Group) |
| **8** | **P05** | **UART_RX** | General / Comm | 3.3V LVCMOS | Asynchronous Serial Receive (RXD / LIN / Break) / 3.3V GPIO | Wire #8 (3-Wire Field UART Group) |
| **9** | **P06** | **I2C_SCL** | General / Comm | 3.3V Open-Drain (PU req.) | I2C Bus Serial Clock / 3.3V GPIO | Wire #9 (I2C Bus Pair) |
| **10** | **P07** | **I2C_SDA** | General / Comm | 3.3V Open-Drain (PU req.) | I2C Bus Serial Data / 3.3V GPIO | Wire #10 (I2C Bus Pair) |
| **11** | **GND** | — | Power / Shield | 0V Return | Mid-bus Shield (I2C / SPI Group Boundary) | Wire #11 (Open-Drain to Push-Pull Barrier) |
| **12** | **P08** | **SPI_CS** | General / Comm | 3.3V LVCMOS | SPI Chip Select (Active LOW) / 3.3V GPIO | Wire #12 (SPI Bus Group) |
| **13** | **P09** | **SPI_MOSI** | General / Comm | 3.3V LVCMOS | SPI Controller Out / Peripheral In / 3.3V GPIO | Wire #13 (SPI Bus Group) |
| **14** | **P10** | **SPI_MISO** | General / Comm | 3.3V LVCMOS | SPI Controller In / Peripheral Out / 3.3V GPIO | Wire #14 (SPI Bus Group) |
| **15** | **P11** | **SPI_SCK** | General / Comm | 3.3V LVCMOS | SPI Serial Clock / 3.3V GPIO | Wire #15 (SPI Bus Group) |
| **16** | **GND** | — | Power / Shield | 0V Return | VBUS Short-Circuit Barrier Wall & SPI Return | Wire #16 (Protective Boundary Wall) |
| **17** | **VBUS** | — | Power | 4.5V – 5.25V DC (2.0A–3.0A) | Main Bidirectional Power Bus (Panel-Aligned Bottom Cluster) | Wire #17 ($2 \times 2$ Solid Power Plane) |
| **18** | **VBUS** | — | Power | 4.5V – 5.25V DC (2.0A–3.0A) | Main Bidirectional Power Bus (Panel-Aligned Bottom Cluster) | Wire #18 ($2 \times 2$ Solid Power Plane) |
| **19** | **VBUS** | — | Power | 4.5V – 5.25V DC (2.0A–3.0A) | Main Bidirectional Power Bus (Panel-Aligned Bottom Cluster) | Wire #19 ($2 \times 2$ Solid Power Plane) |
| **20** | **VBUS** | — | Power | 4.5V – 5.25V DC (2.0A–3.0A) | Main Bidirectional Power Bus (Panel-Aligned Bottom Cluster) | Wire #20 ($2 \times 2$ Solid Power Plane) |

---

## 4. Electrical Requirements & Design Rules

### 4.1 Panel-Aligned Power Bus Architecture (`VBUS`: Pins 17–20)
- **Topology & Enclosure Alignment (Weak-Signal Top, Heavy-Power Bottom)**:  
  In industrial control panels and vertical DIN-rail enclosures, standard wiring practice mandates physical segregation between high-current power wiring (bottom ducts) and sensitive instrumentation/communication signals (top ducts). Pins 17, 18, 19, and 20 are dedicated to `VBUS` at the lowest physical row of the connector to mirror this architecture:
  1. **Direct Straight-Through Routing**: High-current power planes run straight along the bottom edge of the PCB directly into front-panel power terminals and high-current connectors, without traversing or coupling noise into low-voltage signals above.
  2. **Zero Signal Crossings**: Eliminates magnetic and capacitive crosstalk into sensitive analog (`P01 / AIN`) and high-speed communication pairs (`UART`, `SPI`) across both the backplane and module boards.
  3. **Secondary Physical Safety**: As an inherent physical side-effect of this bottom placement, should an external terminal connection come loose in vertical mounts, the loose conductor hangs or drops downward toward the lower wiring duct, preventing accidental contact with communication lines routed above.
- **Operating Voltage Window**: Nominal $5.0\,\text{V}$ ($4.5\,\text{V} \sim 5.25\,\text{V}$ operating range across bidirectional sourcing/sinking paths).
- **Current Carrying Capacity**: Rated for **$2.0\,\text{A} \sim 3.0\,\text{A}$ continuous** across Pins 17–20 combined.
- **$2 \times 2$ Grid Copper Plane**: Pins 17, 18, 19, and 20 form a symmetrical $2 \times 2$ square grid. PCB layouts MUST bond all four pins together into a continuous solid copper pour on outer and inner layers to minimize IR drop and parasitic inductance.
- **Mandatory Sourcing Protection (MUST)**:
  Any module injecting power into `VBUS` (Power Cards, Core Board external supplies, USB inputs) MUST incorporate:
  1. **Reverse-Current Blocking**: Schottky barrier diodes, ideal diode controllers, eFuses with true reverse-current blocking, or P-channel MOSFET switches to prevent reverse back-feeding into external sources.
  2. **Short-Circuit / Over-Current Limitation**: Self-recovering PTC devices, eFuses, or current-limited load switches capable of isolating faults within microsecond thresholds.

### 4.2 Protective Boundary Wall (`Pin 16: GND`)
- **Fault Isolation Barrier**: Pin 16 is dedicated as a solid `GND` return directly flanking the high-power `VBUS` cluster (Pins 17–20).
- **Crimp Misalignment Protection**: In flat ribbon cabling, should a manufacturing crimp misalignment or conductor fray occur along the power boundary, `VBUS` shorts directly to `GND` (Pin 16). This safely triggers upstream over-current protection rather than exposing low-voltage logic lines (Pins 1–15) to $5\,\text{V}$ bus power.

### 4.3 Core-Side Programming Convention & Field RS-485 Direction Control (`P03`)

#### 4.3.1 Omission of Programming Lines from the 20-Pin Bus (Core-Side Responsibility)
Starting with `ADX-BUS-SPEC-v5.0`, the 20-pin IDC bus is strictly dedicated to **field expansion I/O and bidirectional power**. Microcontroller programming and hardware debug lines (`UPDI`, `SWDIO`, `SWCLK`, `RESET`) are **intentionally omitted** from the IDC connector:
- **HV Noise & Accidental Reset Immunity**: On MCUs such as the ATtiny1616, the `UPDI` pin (`PA0`) doubles as a high-voltage (12V) programming entry and reset line. Keeping `PA0` completely off external ribbon cabling prevents HV transients, ESD, and EMI from triggering unintended resets or programming-mode lockups in harsh industrial, outdoor, and automotive-cabin environments.
- **Core-Board Self-Containment**: Each Core board owns its own programming/debug mechanism internally (for example, `CORE-U` integrates a USB-isolated `CH342K` Port 0 `SerialUPDI` channel; 32-bit Core boards may use native USB ROM ISP / UF2 bootloaders and/or onboard SWD test pads). This specification does not mandate a specific onboard connector format.
- **Zero Card-Side Debug Exceptions**: Because neither `OWD` nor `SWCLK` occupies the 20-pin header, expansion Cards no longer require legacy `S00` N.C. restrictions or `P02` $0\,\Omega$ DNP isolation resistors. All 11 pins (`P01`–`P11`) are unrestricted expansion signals.

#### 4.3.2 Field RS-485 / LIN 3-Wire UART Group (`P03`, `P04`, `P05`) & Mandatory Card Pull-Down (MUST)
To allow isolated RS-485 / LIN expansion Cards to operate simultaneously alongside I2C (`P06`–`P07`), SPI (`P08`–`P11`), Analog (`P01`), and Clock (`P02`) without pin conflicts or auto-direction IC limitations (such as LIN Break / low-baud dropout):
- **`P03` (Pin 6: `UART_DE`)**: Active-HIGH RS-485 Driver Enable (`XDIR` / `DE` tied to `/RE` on half-duplex transceivers). On MCUs with hardware RS-485 support (e.g., ATtiny1616 `PA4 / XDIR`, STM32 `USARTx_DE`, RP2040 `PIO`), `P03` provides deterministic hardware guard-time and automatic bus release; on other MCUs, standard GPIO control via TX-complete interrupt is supported seamlessly.
- **`P04` (Pin 7: `UART_TX`)** and **`P05` (Pin 8: `UART_RX`)**: Primary asynchronous serial pair optimized for external field transceivers, LIN auto-baud (`LINAUTO`), and break detection / wake-up (`P05`).
- **Mandatory Card-Side Pull-Down on `P03` (MUST)**:  
  During MCU reset, boot, or unprogrammed states, `P03 (UART_DE)` is High-Z. Every RS-485 expansion Card connecting to `P03` **MUST populate a pull-down resistor ($10\,\text{k}\Omega \sim 100\,\text{k}\Omega$ to `GND`) on `P03`** so the transceiver defaults deterministically to **Receive mode (`DE = LOW`)** and never jams a shared field bus.

#### 4.3.3 System Reset & Wake-Up Architecture
System-level resets and recovery are governed via Core-board onboard programming interfaces (e.g., `CORE-U` `USB-COM` Port 0 `UPDI`), primary `VBUS` power cycling, internal watchdog timers (WDT), or verified protocol commands / Break signaling over `P05 (UART_RX)`.

### 4.4 True Coplanar Shielded Analog & Clock Channels (`P01` [Pin 2] & `P02` [Pin 4])
- **Triple Odd-Row Ground Guard (`Pins 1, 3, 5: GND`)**:
  - `P01` (Pin 2: `AIN`) is flanked directly on both ribbon wires by **Pin 1 (`GND`)** and **Pin 3 (`GND`)**, forming a true coplanar waveguide shield for low-level analog measurements ($0\,\text{V} \sim 3.3\,\text{V}$ ADC, optional DAC0 output, or 3.3V GPIO).
  - `P02` (Pin 4: `EXTCLK`) is flanked directly on both ribbon wires by **Pin 3 (`GND`)** and **Pin 5 (`GND`)**, suppressing harmonic EMI from external clock sources, clock outputs, or high-frequency PWM signals and isolating them from both `P01 (AIN)` and the UART group (`Pins 6–8`).
  - On standard 2×10 vertical headers, Pins 1, 3, and 5 occupy three consecutive contacts along the odd-pin edge, enabling a continuous solid `GND` copper pour on the PCB.

### 4.5 Digital Signal Integrity & Logic Levels
- **Bus Group Segregation**: Pin 5 (`GND`) isolates `P02 (EXTCLK)` from the 3-wire UART group (`P03`–`P05`), and Pin 11 (`GND`) separates open-drain, pull-up dependent I2C lines (`P06`–`P07`) from high-speed push-pull SPI lines (`P08`–`P11`).
- **Native 3.3V LVCMOS Levels**: All 11 digital I/O lines (`P01`–`P11` / Pins 2, 4, 6–10, 12–15) operate strictly at **3.3V LVCMOS**:
  - $V_{IL} \le 0.8\,\text{V}$
  - $V_{IH} \ge 2.0\,\text{V}$
  - $V_{OL} \le 0.4\,\text{V}$
  - $V_{OH} \ge 2.4\,\text{V}$
- **Point of Load (PoL) Regulation Rule**: 3.3V power is NOT distributed across the 20-pin bus. Addon Cards requiring 3.3V logic supply MUST receive `VBUS` (Pins 17–20) and regulate down locally using dedicated on-card LDO or DC-DC stages.

---

## 5. Logical Pin Naming & Software Abstraction (`P01`–`P11`)

Because ADX Core boards span heterogeneous MCU architectures (8-bit AVR, 32-bit RISC-V, ARM Cortex-M, etc.), MCU-native port labels such as `PA1`, `PA4`, or `PB10` are strictly board-internal implementation details and MUST NOT be used as the cross-board bus identifier.

### 5.1 Three-Layer Naming Model
1. **Physical Pin (`Pin 1`–`Pin 20`)**: Used for IDC connector numbering, ribbon cable wire index, footprint pads, and continuity testing.
2. **Logical Pin ID (`P01`–`P11`)**: The MCU-independent bus coordinate used in schematics, PCB silkscreens, Card documentation, and firmware APIs.
   - **Power Rails Excluded**: `GND` and `VBUS` are not numbered.
   - **Uniform Role-Neutral Series (`P01`–`P11`)**: All 11 signal contacts on the connector follow one clean sequential series from Pin 2 (`P01`) to Pin 15 (`P11`), with no legacy `S00` system-pin exception and no Arduino-style `D`/`A` split.
   - **Fixed Two-Digit Zero Padding**: `P01`–`P11` maintain uniform 3-character alignment across tables, schematics, and PCB silkscreens.
3. **Bus Function (`AIN`, `EXTCLK`, `UART_*`, `I2C_*`, `SPI_*`)**: The guaranteed baseline peripheral role overlaid onto each Logical Pin ID.

### 5.2 Guaranteed Baseline vs. MCU-Specific Capabilities
Multiplexed secondary functions on a given pin depend on the host MCU silicon:
- **Guaranteed Bus Contract (This Document)**: Every ADX-compliant Core board guarantees the primary peripheral role in Section 3 (`P01`=AIN, `P02`=EXTCLK/Clock/PWM, `P03–P05`=3-wire UART with `DE`/`TX`/`RX`, `P06–P07`=I2C, `P08–P11`=SPI) plus 3.3V digital GPIO capability on all 11 pins (`P01`–`P11`). Universal expansion Cards intended to work on *any* Core board MUST rely only on this baseline contract.
- **MCU-Specific Bonus Capabilities (`mcu_mapping/*.md`)**: Additional peripheral functions available on specific Core boards (e.g., extra `ADC0` channels on `P02`–`P07`, `DAC0` on `P01`, `TCD0` 48 MHz PWM & `CCL LUT0-OUT` on `P03`, `CCL LUT0-IN2` & full-async wake on `P05` for ATtiny1616; extra 12-bit `ADC` channels on `P08`–`P11` for CH32X035) are documented in each controller's mapping specification.

### 5.3 General-Purpose I/O (GPIO) Repurposing Rules
- **`P01`–`P11` (All 11 Signal Pins)**: When a Card does not utilize the corresponding baseline function (AIN, EXTCLK, UART/RS-485, I2C, or SPI), those pins MAY be freely used as standard 3.3V general-purpose digital I/O.
  - *(Note 1: On multi-card backplanes where another Card populates I2C pull-up resistors on `P06`/`P07`, account for the external pull-up load.)*
  - *(Note 2: On backplanes containing an RS-485 Card, `P03` carries the mandatory Card-side pull-down resistor.)*

### 5.4 Software Header & Macro Convention (`ADX_<BOARD>.h`)
To write portable Arduino / C / C++ firmware across different Core boards, each Core board SHOULD provide a board-specific header named `ADX_<BOARD>.h` (e.g., `ADX_CORE_U_rev6.h`, `ADX_CORE_CX5.h`) using underscores rather than hyphens for C identifier compatibility.

1. **Mandatory `ADX_` Prefix (MUST)**:
   - Logical pin macros MUST be prefixed as **`ADX_P01`** through **`ADX_P11`**.
   - **No Bare `P01`–`P11` Macros**: Headers MUST NOT define unprefixed `P00`–`P11` identifiers. Common Arduino I2C expander libraries (such as `PCF8575` and `PCA9555`) define global `#define P00 0` through `#define P07 7` and `#define P10 8`, which collide with bare `Pxx` macros and bypass C++ namespaces at the preprocessor stage.
2. **Standard Bus Function Aliases (SHOULD)**:
   - Board headers SHOULD expose matching `ADX_`-prefixed functional aliases mapped to the corresponding `ADX_Pxx` pin:
     - `ADX_AIN` (`ADX_P01`), `ADX_CLK` (`ADX_P02`)
     - `ADX_DE` (`ADX_P03`), `ADX_TX` (`ADX_P04`), `ADX_RX` (`ADX_P05`)
     - `ADX_SCL` (`ADX_P06`), `ADX_SDA` (`ADX_P07`)
     - `ADX_CS` (`ADX_P08`), `ADX_MOSI` (`ADX_P09`), `ADX_MISO` (`ADX_P10`), `ADX_SCK` (`ADX_P11`)
3. **Board-Specific Definitions**:
   - Onboard peripherals unique to a Core board (such as USB debug UART pins, power telemetry channels, status LEDs, or mode detection inputs) use the same `ADX_` prefix (e.g., `ADX_LED`, `ADX_USB_COM_DET`, `ADX_VBUS_VSENSE`, `ADX_VBUS_ISENSE`).

```cpp
// Example: Portable user sketch using ADX_<BOARD>.h (ADX-BUS-SPEC-v5.0)
#include "ADX_CORE_U_rev6.h"   // Swap header when migrating Core boards

void setup() {
  pinMode(ADX_P08, OUTPUT);       // Use Logical Pin ID for general I/O (or ADX_CS)
}

void loop() {
  int val = analogRead(ADX_P01);  // Guaranteed coplanar-shielded analog pin (or ADX_AIN)
  digitalWrite(ADX_P08, val > 512);
}
```

### 5.5 Silkscreen Labeling Guideline
Where PCB silkscreen space adjacent to the 20-pin IDC header is constrained, designers SHOULD label contacts using the compact Logical Pin IDs (`P01`–`P11`, `GND`, `VBUS`) alongside the bus revision tag (`ADX-BUS v5.0`). Where space permits, primary bus functions MAY be appended (e.g., `P01/AIN`, `P03/DE`, `P04/TX`, `P08/CS`).

---

## 6. Implementation Mapping per Controller Architecture

For detailed pin multiplexing (PORTMUX), MCU-specific bonus functions (additional ADC/PWM/DAC/CCL channels), and register-level configurations for each supported controller, refer to the dedicated specification references:

- **Microchip ATtiny1616 (`CORE-U` rev6 Flagship; legacy/derivative reference: `CORE-A5`)**: [`mcu_mapping/attiny1616.md`](mcu_mapping/attiny1616.md)
- **WCH CH32X035 (`CORE-CX5` Reference)**: [`mcu_mapping/ch32x035.md`](mcu_mapping/ch32x035.md)
- **Raspberry Pi RP2040 (`CORE-R`)**: *(Planned)*

---

## 7. Document Architecture & Loose Coupling Note

To maintain absolute specification integrity, this document (`docs/spec/ADX_PINOUT.md`) is maintained strictly in **English as the Single Source of Truth (SSOT)**.

Explanatory architecture guides in Japanese ([`docs/ja/ADX_pinout.md`](../ja/ADX_pinout.md)) and English ([`docs/en/ADX_pinout.md`](../en/ADX_pinout.md)) provide background design rationale, historical context, and application guidelines without duplicating authoritative tables, ensuring all future specification revisions are maintained in exactly one place.

---

## 8. Revision History

| Version | Date | Description |
| :---: | :---: | :--- |
| **v4.0** | 2026-10-16 | Initial unified release of the 20-Pin Bus Specification with Gravity Fail-Safe power architecture, Pin 2 OWD, and coplanar analog shield. |
| **v4.1** | 2026-11-03 | Explicitly define Pins 2 and 5 as system management lines. Establish Card-side $0\,\Omega$ DNP isolation recommendation for Pin 5 to support both 1-wire and 2-wire serial programming topologies. Refine power topology rationale to emphasize industrial panel alignment (weak-signal top, heavy-power bottom) and straight-through non-crossing layout. |
| **v4.2** | 2026-11-19 | Introduce MCU-independent Logical Pin IDs (`S00` for system `OWD`, `P01`–`P10` for the 10 user/Card signal pins; power rails `GND`/`VBUS` unnumbered) with 100% electrical and physical backward compatibility to v4.1. Formalize collision-safe `ADX_<BOARD>.h` C/C++ macro conventions (`ADX_P01`–`ADX_P10`). |
| **v5.0** | 2026-11-28 | **Major Breaking Revision (Flagship Field-Bus Architecture)**: Omit `S00` (`OWD / UPDI`) from the 20-pin IDC bus to eliminate HV-noise / accidental-reset vectors and move programming interfaces to Core-board internal responsibility (removing Card-side `S00` N.C. and `P02` $0\,\Omega$ DNP rules). Compact Pins 1–5 to `GND / P01(AIN) / GND / P02(EXTCLK) / GND` for true dual-GND coplanar shielding. Introduce dedicated RS-485 Driver Enable **`P03` (`UART_DE` / `ADX_DE`)** on Pin 6 with mandatory Card-side pull-down ($10\,\text{k}\Omega$–$100\,\text{k}\Omega$), keeping Pins 7–20 physically identical to `v4.2`. Renumber all 11 unrestricted signal pins sequentially as **`P01`–`P11`** (`ADX_P01`–`ADX_P11`). *Electrically incompatible with `v4.x` boards on Pins 3–6.* |
