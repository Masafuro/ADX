<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Hardware & Form Factor Design Guidelines

[ **English** | [日本語](../ja/adx_formfactor_guidelines.md) ]

---

## 1. Overview & Philosophy

This guideline defines the standard design rules for engineers developing Core boards (e.g., **ADX CORE-U**) and expansion cards (**CARD Series**) in the ADX ecosystem.

This specification balances two essential principles:
1. **Open-Source Freedom & Low Barrier to Entry:**
   - Enabling hobbyists, researchers, and DIY developers to easily create custom add-on cards at low cost with creative freedom.
2. **Equipment Safety & Robustness:**
   - Designing interfaces to actively prevent catastrophic accidents (fire, battery rupture, host PC burnout, power shorts) regardless of which expansion cards are attached.

To achieve this balance, this guideline defines three requirement tiers based on **RFC 2119**.

---

## 2. Requirement Levels (RFC 2119)

| Level | Keyword | Definition & Scope |
| :---: | :---: | :--- |
| **Level 1** | **[MUST] Mandatory**<br>*(Shall / Required)* | **Mandatory constraints required to prevent electrical fires, physical destruction, or danger to human life.**<br>Must be strictly obeyed by all ADX hardware without exception (Official, third-party, and DIY). |
| **Level 2** | **[SHOULD] Official Standard**<br>*(Recommended)* | **Industrial-grade quality and reliability standards met by official ADX products.**<br>Designed for demanding environments and long-term reliability. DIY and specialized cards may deviate. |
| **Level 3** | **[MAY] Developer Freedom**<br>*(Optional)* | **Areas left entirely to the developer's creativity, cost targets, and design requirements.** |

---

## 3. Power & Electrical Safety Rules

### 3.1 Common 5V Power Trunk (`VBUS`) Baseline
- **Voltage Specification:** **DC 5.0V (±10%)** (4.5V to 5.0V operating window; 4.75V–5.25V recommended)
- **Current Rating:** **Up to 2.0A–3.0A continuous** (Delivered via Pins 17–20 $2 \times 2$ bottom power cluster)
- **Bidirectionality:** Power may flow either from Core board (USB Type-C) to CARDs, or from a Power CARD to the Core board.
- **Point of Load (PoL) Rule:** 3.3V logic power is NOT distributed across the 20-pin bus. Cards requiring 3.3V must regulate locally from `VBUS` via an onboard LDO or DC-DC converter.

### 3.2 Power CARD Design Rules (Separation of Responsibilities)

```text
                 [ Double Check-Valve Architecture ]

   [ Core Board (USB Type-C / External Supply) ]
          │
     [ ⛔ Protection & Reverse Blocking (eFuse / Ideal Diode) ⛔ ] ──► (Blocks back-feed into host)
          │
          ▼
   ┌────────────────────────────────────────────────────────┐
   │  Common 5V VBUS Trunk (Pins 17–20 / 2×2 Bottom Grid)   │
   └────────────────────────────────────────────────────────┘
          ▲
          │
     [ ⛔ Reverse Blocking (Diode/FET) ⛔ ] ──► [MUST] Mandatory for Power CARDs!
          │                                   (Blocks VBUS from entering external battery)
   [ ⚡ Protection / Surge / Regulators ⚡ ]
          │
   [ External Power (Dry Batteries / Factory 24V / Telecom -48V) ]
```

1. **[MUST] Reverse-Current Blocking for Power CARDs (Mandatory):**
   - Any CARD injecting power into the 20-pin bus (Pins 17–20: `VBUS`) **MUST provide reverse-current blocking and over-current / short-circuit protection at its output** (Schottky diode, ideal diode, eFuse, or synchronous DCDC with reverse-blocking capability).
   - *(Reason: When a core board such as CORE-U is externally powered on the workbench, 5.0V must be prevented from back-feeding into external batteries or regulators, mitigating **battery rupture and electrolyte leakage risks**).*
2. **[MUST] Protective Barrier (`Pin 16: GND`) & RS-485 `P03 (UART_DE)` Pull-Down:**
   - **Pin 16 (`GND`)** directly flanks the `VBUS` power cluster (Pins 17–20) as a short-circuit protective wall and MUST be solidly bonded to the PCB ground plane along with shield grounds (Pins 1, 3, 5, 11).
   - Every RS-485 expansion Card using **`P03` (Pin 6: `UART_DE`)** **MUST populate a $10\,\text{k}\Omega \sim 100\,\text{k}\Omega$ pull-down resistor to GND on `P03`** so the transceiver defaults to Receive mode (`DE = LOW`) during MCU reset or unprogrammed states.
3. **[SHOULD] Transformer Isolation for High-Voltage Inputs (DC 12V/24V/48V):**
   - **Official ADX Grade:** To suppress common-mode noise, ground differentials, and lightning surges in factory panels, use a **transformer-isolated DC-DC converter (isolated 5V output)**.
   - **DIY / Hobby Grade [MAY]:** In controlled single-ground indoor environments, low-cost non-isolated buck converters are acceptable. Battery cards may also be non-isolated.
4. **[SHOULD] Input Port Protection:**
   - External power inputs (24V/48V or battery packs) should feature surge suppression (TVS/varistors), overcurrent protection (resettable PTC/fuse), and reverse-polarity protection (P-MOSFET).

---

## 4. Mechanical & Connector Rules

### 4.1 20-Pin Expansion Connector (`ADX-BUS-SPEC-v5.0`)
- **[MUST] Electrical Pinout Adherence:**
  - All boards must strictly adhere to the [ADX 20-Pin Bus Specification (SSOT: v5.0)](../spec/ADX_PINOUT.md) and [Architecture Guide](ADX_pinout.md).
  - *(Note: `v5.0` omits legacy `S00` and swaps signal/GND pins on Pins 3–6 relative to `v4.2`; `v4.x` prototypes and `v5.0` hardware are electrically incompatible. Print `ADX-BUS v5.0` clearly on PCB silkscreens.)*
- **Core-Side Programming Separation:**
  - Because programming/debug lines (`UPDI`/`SWD`) are omitted from the 20-pin header in `v5.0`, legacy `S00` N.C. restrictions and `P02` $0\,\Omega$ DNP isolation resistors are no longer required; all 11 pins (`P01`–`P11`) are unrestricted expansion signals.
- **Connector Type Selection:**
  - **Official Standard [SHOULD]:** To prevent dislodging under heavy vibration and avoid misaligned blind insertion in tight spaces, use **MIL-DTL-83503 compliant Latched Eject Headers** or shrouded box headers.
  - **DIY / General Add-ons [MAY]:** Standard boxed shrouded headers or simple pin headers may be used where low profile or minimal cost is prioritized.

### 4.2 Board Dimensions & Stacking
- **Official Standard [SHOULD]: 8858 Form Factor (Official Alpha / Current Standard):**
  - Board Outline: **`88.0 mm × 58.0 mm`**
  - Mounting Holes: **4× M3 clearance holes (Φ3.30 mm NPTH)**
  - Hole Pitch: **`76.0 mm × 46.0 mm`** (`6.0 mm` margin from board edges, $\Phi 10.0\,\text{mm}$ washer keep-out)
  - 20P Vertical IDC Center: **`(44.0, 51.0) mm`** (centered on long edge, `7.0 mm` from top edge)
  - Corner Treatment: **C0 (Right angles, continuous straight V-cut ready)**
  - Underside Structure: **SMD-only zone outside 66.0 mm I/O Edge (`36.0 × 8.0 mm × 2`)** (enables bottom slide-in rails, snap-fits, and optional optical markers)
  - Optical Alignment Markers [MAY / Official SHOULD]: Corner `M1–M4` zones (pitch **`76.0 × 30.0 mm`**, X-aligned with M3 mounting holes)
  - *(Refer to [8858 Form Factor Specification](8858_formfactor.md) for full CAD details; legacy: [8854 Form Factor](8854_formfactor.md) / [8748 Form Factor](8748_formfactor.md)).*
- **Third-Party / Custom Shapes [MAY]:**
  - Custom outline shapes (compact daughterboards, full-size carrier shields, L-shapes) are fully permitted as long as the 20-pin IDC interface aligns electrically.

---

## 5. Thermal & Environmental Rules

1. **[MUST] Heat Source Separation (Core Board Protection):**
   - Major thermal generators (high-voltage step-down regulators, relay coils, high-power switches) must reside on the CARD, isolated from the core board via PCB thermal relief slots and ground-plane partitioning.
2. **Temperature Rating Selection:**
   - **High-Reliability Standard [SHOULD]: Industrial Grade (-40°C to +105°C):**
     - Official high-reliability CARDs should use X7R capacitors, thick-film resistors, and industrial-rated ICs rated for -40°C to +105°C.
   - **DIY / General Add-ons [MAY]: Commercial Grade (0°C to 70°C / -20°C to 85°C):**
     - Common consumer components are fully acceptable for indoor experiments, hobby prototypes, and education.

---

## 6. Optional Features & Signal Routing [MAY]

The following signals may be utilized flexibly depending on application requirements using MCU-independent Logical Pin IDs (`P01`–`P11` / code macros `ADX_P01`–`ADX_P11`):

1. **Battery Level / Bus Voltage Telemetry:**
   - External raw battery voltage can be divided down (within 0–3.3V) and routed to **`P01` (IDC Pin 2 / `AIN`)** for analog voltage monitoring by the MCU.
2. **General-Purpose GPIO & Power Control Repurposing:**
   - When SPI, UART, or I2C is unused on a Card, those pins (e.g., **`P08` [IDC Pin 12 / `SPI_CS`]**) may be freely repurposed as 3.3V digital GPIOs—such as driving LEDs, reading switches, or controlling an on-card DC-DC Enable (EN) pin for low-power sleep.

---

## 7. Developer Pre-Production Checklist

Before releasing or sending a PCB for fabrication (JLCPCB PCBA):

- [ ] **[MUST]** Did you place reverse-current blocking and autonomous short-circuit/over-current protection on any Power CARD injecting into `VBUS`?
- [ ] **[MUST]** Are Pins 17–20 (`VBUS`) bonded into a $2 \times 2$ solid copper pour, and are Pin 16 (`GND` barrier wall) plus Pins 1, 3, 5, 11 tied solidly to the ground plane?
- [ ] **[MUST]** If the Card is an RS-485 transceiver using **`P03` (Pin 6: `UART_DE`)**, did you place a $10\,\text{k}\Omega \sim 100\,\text{k}\Omega$ pull-down resistor to GND on `P03` for fail-safe Receive default?
- [ ] **[MUST]** Does the 20-pin IDC connector pin assignment (`P01`–`P11`, `GND`, `VBUS`) strictly match **`ADX-BUS-SPEC-v5.0`** ([`docs/spec/ADX_PINOUT.md`](../spec/ADX_PINOUT.md)), with `ADX-BUS v5.0` printed on silkscreen to avoid mating with legacy `v4.x` boards?
- [ ] **[SHOULD]** Did you use a transformer-isolated DC-DC converter for industrial 24V/48V inputs?
- [ ] **[SHOULD]** Did you select a latched Eject Header if intended for high-vibration applications?
- [ ] **[SHOULD]** Does the board outline match the 8858 pitch (`76.0 mm × 46.0 mm`) (or legacy 8854 `76.0 × 42.0 mm` / 8748 `77.0 × 38.0 mm`) for enclosure compatibility?

---

## 8. License

This guideline is published under the [Creative Commons Attribution 4.0 International License (CC-BY-4.0)](../../LICENSES/CC-BY-4.0.txt). Compatible hardware may be freely designed, fabricated, and commercially sold.
