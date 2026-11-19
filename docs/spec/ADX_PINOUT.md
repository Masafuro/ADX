<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus Specification

**Standard Identifier**: ADX-BUS-SPEC-v4.2  
**Category**: Core Electrical & Interconnect Specification (Single Source of Truth)  
**Status**: Standard (Release)  
**Maintained by**: ADX Hardware Standards Committee  

---

## 1. Overview & Scope

The **ADX 20-Pin Bus** is the universal physical and electrical interconnect standard for the ADX modular hardware ecosystem. It establishes a unified, vibration-resistant backplane interface connecting **Core Boards** (main MCUs and host controllers) and **Card Modules** (expansion addons, sensors, actuators, and communication cards).

This document serves as the authoritative **Single Source of Truth (SSOT)** for physical pin assignments, MCU-independent logical pin identifiers (`S00`, `P01`–`P10`), electrical limits, signal definitions, and mandatory design rules for the 20-pin IDC interface.

---

## 2. Physical & Mechanical Interface

- **Connector Family**: $2 \times 10$-pin (20-pin), 2.54 mm (0.100") pitch shrouded box header or ejector/latch header (MIL-DTL-83503 compliant).
- **Mating Cable**: Standard 20-conductor flat ribbon cable (AWG 28 stranded).
- **Pin Numbering Convention**: Standard zigzag (interleaved) ribbon cable numbering:
  - Row 1 (Odd pins): 1, 3, 5, 7, 9, 11, 13, 15, 17, 19
  - Row 2 (Even pins): 2, 4, 6, 8, 10, 12, 14, 16, 18, 20

```text
                  【 20-Pin Shrouded Box Header (Top View) 】

                             ┌────────[ Center Key ]────────┐
                GND     [ 1] │  ●   ●  │ [ 2]  S00  (OWD / PROG)
       (AIN)    P01     [ 3] │  ●   ●  │ [ 4]  GND
    (EXTCLK)    P02     [ 5] │  ●   ●  │ [ 6]  GND
   (UART_TX)    P03     [ 7] │  ●   ●  │ [ 8]  P04  (UART_RX)
   (I2C_SCL)    P05     [ 9] │  ●   ●  │ [10]  P06  (I2C_SDA)
                GND     [11] │  ●   ●  │ [12]  P07  (SPI_CS)
  (SPI_MOSI)    P08     [13] │  ●   ●  │ [14]  P09  (SPI_MISO)
   (SPI_SCK)    P10     [15] │  ●   ●  │ [16]  GND
                VBUS    [17] │  ●   ●  │ [18]  VBUS
                VBUS    [19] │  ●   ●  │ [20]  VBUS
                             └──────────────────────────────┘
```

---

## 3. Universal Pin Assignment Table

The bus separates every contact into three layers: **Physical IDC Pin** (`1`–`20`), **Logical Pin ID** (`S00`, `P01`–`P10`, plus power rails `GND` / `VBUS`), and **Standard Bus Function** (guaranteed baseline peripheral role).

| IDC Pin | Logical ID | Bus Function (Alt) | Signal Class | Electrical Standard | Primary Function / Guaranteed Role | Shield & Topology Role |
| :---: | :---: | :--- | :--- | :--- | :--- | :--- |
| **1** | **GND** | — | Power / Shield | 0V Return | Cable Outer Edge Shield & Signal Return | Wire #1 (Edge ESD / Mechanical Guard) |
| **2** | **S00** | **OWD** | **System (Bi-dir)** | 3.3V LVCMOS (Weak PU) | System Debug & Factory Flash (UPDI / SWDIO) — **Card N.C.** | Wire #2 (Quasi-static Test Line) |
| **3** | **P01** | **AIN** | General / Analog | 0V – 3.3V Analog / GPIO | High-Precision Analog Input (ADC) / Opt. DAC / 3.3V GPIO | Wire #3 (Coplanar Shielded by Pins 1 & 4) |
| **4** | **GND** | — | Power / Shield | 0V Return | Analog / Clock Isolation Shield | Wire #4 (P02 / EXTCLK Coplanar Guard ①) |
| **5** | **P02** | **EXTCLK** | General / System | 3.3V LVCMOS | External Clock / Debug Clock (SWCLK) / Timer PWM / GPIO | Wire #5 (Clock Waveguide Trace) |
| **6** | **GND** | — | Power / Shield | 0V Return | Clock / Communications Isolation Shield | Wire #6 (P02 / EXTCLK Coplanar Guard ②) |
| **7** | **P03** | **UART_TX** | General / Comm | 3.3V LVCMOS | Asynchronous Serial Transmit (TXD) / 3.3V GPIO | Wire #7 (UART Balanced Pair) |
| **8** | **P04** | **UART_RX** | General / Comm | 3.3V LVCMOS | Asynchronous Serial Receive (RXD) / 3.3V GPIO | Wire #8 (UART Balanced Pair) |
| **9** | **P05** | **I2C_SCL** | General / Comm | 3.3V Open-Drain (PU req.) | I2C Bus Serial Clock / 3.3V GPIO | Wire #9 (I2C Bus Pair) |
| **10** | **P06** | **I2C_SDA** | General / Comm | 3.3V Open-Drain (PU req.) | I2C Bus Serial Data / 3.3V GPIO | Wire #10 (I2C Bus Pair) |
| **11** | **GND** | — | Power / Shield | 0V Return | Mid-bus Shield (I2C / SPI Group Boundary) | Wire #11 (Open-Drain to Push-Pull Barrier) |
| **12** | **P07** | **SPI_CS** | General / Comm | 3.3V LVCMOS | SPI Chip Select (Active LOW) / 3.3V GPIO | Wire #12 (SPI Bus Group) |
| **13** | **P08** | **SPI_MOSI** | General / Comm | 3.3V LVCMOS | SPI Controller Out / Peripheral In / 3.3V GPIO | Wire #13 (SPI Bus Group) |
| **14** | **P09** | **SPI_MISO** | General / Comm | 3.3V LVCMOS | SPI Controller In / Peripheral Out / 3.3V GPIO | Wire #14 (SPI Bus Group) |
| **15** | **P10** | **SPI_SCK** | General / Comm | 3.3V LVCMOS | SPI Serial Clock / 3.3V GPIO | Wire #15 (SPI Bus Group) |
| **16** | **GND** | — | Power / Shield | 0V Return | VBUS Short-Circuit Barrier Wall & SPI Return | Wire #16 (Protective Boundary Wall) |
| **17** | **VBUS** | — | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Panel-Aligned Bottom Cluster) | Wire #17 ($2 \times 2$ Solid Power Plane) |
| **18** | **VBUS** | — | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Panel-Aligned Bottom Cluster) | Wire #18 ($2 \times 2$ Solid Power Plane) |
| **19** | **VBUS** | — | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Panel-Aligned Bottom Cluster) | Wire #19 ($2 \times 2$ Solid Power Plane) |
| **20** | **VBUS** | — | Power | 4.5V – 5.0V DC (2.0A–3.0A) | Main Power Bus (Panel-Aligned Bottom Cluster) | Wire #20 ($2 \times 2$ Solid Power Plane) |

---

## 4. Electrical Requirements & Design Rules

### 4.1 Panel-Aligned Power Bus Architecture (`VBUS`: Pins 17–20)
- **Topology & Enclosure Alignment (Weak-Signal Top, Heavy-Power Bottom)**:  
  In industrial control panels and vertical DIN-rail enclosures, standard wiring practice mandates physical segregation between high-current power wiring (bottom ducts) and sensitive instrumentation/communication signals (top ducts). Pins 17, 18, 19, and 20 are dedicated to `VBUS` at the lowest physical row of the connector to mirror this architecture:
  1. **Direct Straight-Through Routing**: High-current power planes run straight along the bottom edge of the PCB directly into front-panel power terminals and high-current connectors, without traversing or coupling noise into low-voltage signals above.
  2. **Zero Signal Crossings**: Eliminates magnetic and capacitive crosstalk into sensitive analog (`P01 / AIN`) and high-speed communication pairs (`UART`, `SPI`) across both the backplane and module boards.
  3. **Secondary Physical Safety**: As an inherent physical side-effect of this bottom placement, should an external terminal connection come loose in vertical mounts, the loose conductor hangs or drops downward toward the lower wiring duct, preventing accidental contact with communication lines routed above.
- **Operating Voltage Window**: Nominal $5.0\,\text{V} \pm 10\%$ ($4.5\,\text{V} \sim 5.0\,\text{V}$ operating range).
- **Current Carrying Capacity**: Rated for **$2.0\,\text{A} \sim 3.0\,\text{A}$ continuous** across Pins 17–20 combined.
- **$2 \times 2$ Grid Copper Plane**: Pins 17, 18, 19, and 20 form a symmetrical $2 \times 2$ square grid. PCB layouts MUST bond all four pins together into a continuous solid copper pour on outer and inner layers to minimize IR drop and parasitic inductance.
- **Mandatory Sourcing Protection (MUST)**:
  Any module injecting power into `VBUS` (Power Cards, Core Board external supplies, USB inputs) MUST incorporate:
  1. **Reverse-Current Blocking**: Schottky barrier diodes, ideal diode controllers, or P-channel MOSFET switches to prevent reverse back-feeding into external sources.
  2. **Short-Circuit / Over-Current Limitation**: Self-recovering PTC devices, eFuses, or current-limited load switches capable of isolating faults within microsecond thresholds.

### 4.2 Protective Boundary Wall (`Pin 16: GND`)
- **Fault Isolation Barrier**: Pin 16 is dedicated as a solid `GND` return directly flanking the high-power `VBUS` cluster (Pins 17–20).
- **Crimp Misalignment Protection**: In flat ribbon cabling, should a manufacturing crimp misalignment or conductor fray occur along the power boundary, `VBUS` shorts directly to `GND` (Pin 16). This safely triggers upstream over-current protection rather than exposing low-voltage logic lines (Pins 1–15) to $5\,\text{V}$ bus power.

### 4.3 System Debug & Factory Programming Architecture (`S00` [Pin 2] & `P02` [Pin 5])

#### 4.3.1 Primary Programming Channels
The ADX bus standard accommodates both 1-wire and 2-wire serial programming topologies to ensure universal compatibility across microcontrollers (8-bit architectures through 32-bit RISC-V/ARM controllers):
- **`S00` (Pin 2: `OWD / PROG`)**: 3.3V LVCMOS bidirectional programming data line (`UPDI` / `SWDIO`) with an onboard weak pull-up ($10\,\text{k}\Omega \sim 47\,\text{k}\Omega$ to 3.3V) on the Core board.
- **`P02` (Pin 5: `EXTCLK / PROG_CLK`)**: 3.3V LVCMOS clock and timer line. On architectures requiring 2-wire serial debug (e.g., WCH CH32X035 `SWCLK`, RP2040 `SWCLK`), `P02` acts as the high-speed synchronous debug clock during automated test bench programming. Pin 5 is shielded by Pin 4 (`GND`) and Pin 6 (`GND`), suppressing clock switching radiation (EMI) across the ribbon cable.

#### 4.3.2 Card-Side Isolation & DNP Recommendation Rules (MUST / RECOMMENDED)
To guarantee cross-architecture interoperability among different Core boards without signal collision, boot interference, or programming disruption:
1. **`S00` (Pin 2: `OWD`) Isolation (MUST)**:
   - Addon expansion Cards **MUST leave `S00` (Pin 2) unconnected (N.C.)** under normal operation.
   - Dedicated factory test fixtures, programming harnesses, or specialized diagnostic adapters are the sole modules permitted to drive `S00`.
   - **Direct Grounding Prohibited**: Cards MUST NOT connect push-buttons or hardware pull-down switches directly between `S00` (Pin 2) and GND.
2. **`P02` (Pin 5: `EXTCLK`) DNP Isolation Recommendation (RECOMMENDED / SHOULD)**:
   - Where an expansion Card module features an on-card clock source (e.g., an external oscillator feeding clock into host MCUs such as ATtiny1616 master cards) or connects monitoring/GPIO circuitry to `P02`, the Card **SHOULD insert a series $0\,\Omega$ resistor (or solder jumper) to allow complete electrical disconnection (DNP: Do Not Populate)**.
   - **Interoperability Rationale**: When such a Card is mated with Core controllers operating from internal oscillators and utilizing `P02` as `SWCLK` (e.g., CORE-CX5), depopulating the $0\,\Omega$ resistor prevents bus collisions during 2-wire debugging and ensures transparent plug-and-play compatibility across all Core variants.

#### 4.3.3 System Reset Architecture
System-level resets are governed via primary `VBUS` power cycling, internal watchdog timers (WDT), or verified protocol commands over communication buses (USB / RS-485).

### 4.4 Coplanar Shielded Analog Channel (`P01` [Pin 3: `AIN`])
- **Coplanar Ground Guard**: `P01` (Pin 3: `AIN`) is flanked by Pin 1 (`GND`) on the outer cable perimeter and Pin 4 (`GND`) on the inner boundary.
- **Noise Immunity**: This topology forms a coplanar waveguide shield that isolates high-impedance, delicate analog measurements from adjacent high-speed switching noise (such as `P02 / EXTCLK` on Pin 5) and external electrostatic interference.
- **Signal Range**: $0\,\text{V} \sim 3.3\,\text{V}$ single-ended analog input for ADC acquisition. Where host MCUs support pin multiplexing (e.g., ATtiny1616), `P01` may optionally function as a true analog DAC output, or as a standard 3.3V digital GPIO.

### 4.5 Digital Signal Integrity & Logic Levels
- **Clock Isolation Sandwich**: `P02` (Pin 5: `EXTCLK`) is guarded by Pin 4 (`GND`) and Pin 6 (`GND`), suppressing high-frequency clock harmonic radiation (EMI from `EXTCLK` or synchronous debug clock `SWCLK`).
- **Bus Group Segregation**: Pin 11 (`GND`) separates open-drain, pull-up dependent lines (I2C: `P05`–`P06`) from high-speed push-pull lines (SPI: `P07`–`P10`).
- **Native 3.3V LVCMOS Levels**: All digital I/O lines (`S00`, `P01`–`P10` / Pins 2–3, 5, 7–10, 12–15) operate strictly at **3.3V LVCMOS**:
  - $V_{IL} \le 0.8\,\text{V}$
  - $V_{IH} \ge 2.0\,\text{V}$
  - $V_{OL} \le 0.4\,\text{V}$
  - $V_{OH} \ge 2.4\,\text{V}$
- **Point of Load (PoL) Regulation Rule**: 3.3V power is NOT distributed across the 20-pin bus. Addon Cards requiring 3.3V logic supply MUST receive `VBUS` (Pins 17–20) and regulate down locally using dedicated on-card LDO or DC-DC stages.

---

## 5. Logical Pin Naming & Software Abstraction (`S00`, `P01`–`P10`)

Because ADX Core boards span heterogeneous MCU architectures (8-bit AVR, 32-bit RISC-V, ARM Cortex-M, etc.), MCU-native port labels such as `PA0`, `PB2`, or `PC19` are strictly board-internal implementation details and MUST NOT be used as the cross-board bus identifier.

### 5.1 Three-Layer Naming Model
1. **Physical Pin (`Pin 1`–`Pin 20`)**: Used for IDC connector numbering, ribbon cable wire index, footprint pads, and continuity testing.
2. **Logical Pin ID (`S00`, `P01`–`P10`)**: The MCU-independent bus coordinate used in schematics, PCB silkscreens, Card documentation, and firmware APIs.
   - **Power Rails Excluded**: `GND` and `VBUS` are not numbered.
   - **`S00` (`S` = System)**: Reserved exclusively for system maintenance (`OWD / PROG`). The distinct `S` prefix visually separates it from user-accessible pins.
   - **`P01`–`P10` (`P` = Pin)**: Role-neutral numbering for the 10 user/Card-accessible signal pins. Rather than splitting pins into `D` (digital) and `A` (analog) as in legacy Arduino conventions—which causes semantic ambiguity when multifunction pins carry ADC, PWM, and serial buses—ADX assigns a single flat `P01`–`P10` series.
   - **Fixed Two-Digit Zero Padding**: `S00` and `P01`–`P10` maintain uniform 3-character alignment across tables, schematics, and PCB silkscreens.
3. **Bus Function (`AIN`, `EXTCLK`, `UART_*`, `I2C_*`, `SPI_*`)**: The guaranteed baseline peripheral role overlaid onto each Logical Pin ID.

### 5.2 Guaranteed Baseline vs. MCU-Specific Capabilities
Multiplexed secondary functions on a given pin (for example, whether `P10 / SPI_SCK` also supports ADC or high-frequency timer PWM) depend entirely on the host MCU silicon:
- **Guaranteed Bus Contract (This Document)**: Every ADX-compliant Core board guarantees the primary peripheral role in Section 3 (`P01`=AIN, `P02`=EXTCLK/SWCLK, `P03–P04`=UART, `P05–P06`=I2C, `P07–P10`=SPI) plus 3.3V digital GPIO capability on `P01`–`P10`. Universal expansion Cards intended to work on *any* Core board MUST rely only on this baseline contract.
- **MCU-Specific Bonus Capabilities (`mcu_mapping/*.md`)**: Additional peripheral functions available on specific Core boards (e.g., extra `ADC0` channels on `P02`/`P05`/`P06` or `DAC0` on `P01` for ATtiny1616; extra `ADC` channels on `P07`–`P10` for CH32X035) are documented in each controller's mapping specification.

### 5.3 General-Purpose I/O (GPIO) Repurposing Rules
- **`P01`, `P03`–`P10`**: When a Card does not utilize the corresponding serial bus (UART, I2C, or SPI), those pins MAY be freely used as standard 3.3V general-purpose digital I/O. *(Note: On multi-card backplanes where another Card populates I2C pull-up resistors on `P05`/`P06`, account for the external pull-up load.)*
- **`P02` (`EXTCLK`)**: May be used as 3.3V GPIO or PWM output, subject to the Card-side $0\,\Omega$ DNP isolation recommendation in Section 4.3.2 when 2-wire SWD Core compatibility is required.
- **`S00` (`OWD`)**: Strictly reserved for system debug/programming. MUST NOT be used as GPIO by standard Cards.

### 5.4 Software Header & Macro Convention (`ADX_<BOARD>.h`)
To write portable Arduino / C / C++ firmware across different Core boards, each Core board SHOULD provide a board-specific header named `ADX_<BOARD>.h` (e.g., `ADX_CORE_U.h`, `ADX_CORE_CX5.h`) using underscores rather than hyphens for C identifier compatibility.

1. **Mandatory `ADX_` Prefix (MUST)**:
   - Logical pin macros MUST be prefixed as **`ADX_P01`** through **`ADX_P10`**.
   - **No Bare `P01`–`P10` Macros**: Headers MUST NOT define unprefixed `P00`–`P10` identifiers. Common Arduino I2C expander libraries (such as `PCF8575` and `PCA9555`) define global `#define P00 0` through `#define P07 7` and `#define P10 8`, which collide with bare `Pxx` macros and bypass C++ namespaces at the preprocessor stage.
2. **Standard Bus Function Aliases (SHOULD)**:
   - Board headers SHOULD expose matching `ADX_`-prefixed functional aliases mapped to the corresponding `ADX_Pxx` pin:
     - `ADX_AIN` (`ADX_P01`), `ADX_CLK` (`ADX_P02`)
     - `ADX_TX` (`ADX_P03`), `ADX_RX` (`ADX_P04`)
     - `ADX_SCL` (`ADX_P05`), `ADX_SDA` (`ADX_P06`)
     - `ADX_CS` (`ADX_P07`), `ADX_MOSI` (`ADX_P08`), `ADX_MISO` (`ADX_P09`), `ADX_SCK` (`ADX_P10`)
3. **System Pin Protection (SHOULD)**:
   - `S00` (`OWD`) SHOULD remain undefined in standard application headers to prevent accidental runtime manipulation of the programming line.
4. **Board-Specific Definitions**:
   - Onboard peripherals unique to a Core board (such as status LEDs or eFuse fault flags) use the same `ADX_` prefix (e.g., `ADX_LED_R`, `ADX_EFUSE_FLG`).

```cpp
// Example: Portable user sketch using ADX_<BOARD>.h
#include "ADX_CORE_U.h"   // Swap to "ADX_CORE_CX5.h" when migrating Core boards

void setup() {
  pinMode(ADX_P07, OUTPUT);       // Use Logical Pin ID for general I/O (or ADX_CS)
}

void loop() {
  int val = analogRead(ADX_P01);  // Guaranteed shielded analog pin (or ADX_AIN)
  digitalWrite(ADX_P07, val > 512);
}
```

### 5.5 Silkscreen Labeling Guideline
Where PCB silkscreen space adjacent to the 20-pin IDC header is constrained, designers SHOULD label contacts using the compact Logical Pin IDs (`S00`, `P01`–`P10`, `GND`, `VBUS`). Where space permits, secondary bus functions MAY be appended (e.g., `P01/AIN`, `P03/TX`, `P07/CS`).

---

## 6. Implementation Mapping per Controller Architecture

For detailed pin multiplexing (PORTMUX), MCU-specific bonus functions (additional ADC/PWM/DAC channels), and register-level configurations for each supported controller, refer to the dedicated specification references:

- **Microchip ATtiny1616 (`CORE-U`, `CORE-A5`)**: [`mcu_mapping/attiny1616.md`](mcu_mapping/attiny1616.md)
- **WCH CH32X035 (`CORE-CX5`)**: [`mcu_mapping/ch32x035.md`](mcu_mapping/ch32x035.md)
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
| **v4.2** | 2026-11-19 | Introduce MCU-independent Logical Pin IDs (`S00` for system `OWD`, `P01`–`P10` for the 10 user/Card signal pins; power rails `GND`/`VBUS` unnumbered) with 100% electrical and physical backward compatibility to v4.1. Formalize the guaranteed bus contract vs. MCU-specific bonus capabilities, general-purpose GPIO repurposing rules, silkscreen labeling guidelines, and collision-safe `ADX_<BOARD>.h` C/C++ macro conventions (`ADX_P01`–`ADX_P10`, explicitly avoiding bare `Pxx` macros). |
