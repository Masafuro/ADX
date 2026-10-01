<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Hardware & Form Factor Design Guidelines

[ **English** | [日本語 (../ja/adx_formfactor_guidelines.md)](../ja/adx_formfactor_guidelines.md) ]

---

## 1. Overview & Philosophy

This guideline defines the standard design rules for engineers developing Core boards (e.g., **ADX CORE-A**) and expansion cards (**CARD Series**) in the ADX ecosystem.

This specification balances two essential principles:
1. **Open-Source Freedom & Low Barrier to Entry:**
   - Enabling hobbyists, researchers, and DIY developers to easily create custom add-on cards at low cost with creative freedom.
2. **Absolute Industrial Safety & Robustness:**
   - Guaranteeing that no matter what expansion card is plugged in, catastrophic accidents (fire, battery rupture, PC port burnout, high-voltage shorts) are physically and electrically prevented.

To achieve this balance, this guideline defines three requirement tiers based on **RFC 2119**.

---

## 2. Requirement Levels (RFC 2119)

| Level | Keyword | Definition & Scope |
| :---: | :---: | :--- |
| **Level 1** | **[MUST] Mandatory**<br>*(Shall / Required)* | **Absolute constraints required to prevent electrical fires, physical destruction, or danger to human life.**<br>Must be strictly obeyed by all ADX hardware without exception (Official, third-party, and DIY). |
| **Level 2** | **[SHOULD] Official Standard**<br>*(Recommended)* | **Industrial-grade quality and reliability standards met by official ADX products.**<br>Designed for automotive, extreme temperatures, and critical infrastructure. DIY and specialized cards may deviate. |
| **Level 3** | **[MAY] Developer Freedom**<br>*(Optional)* | **Areas left entirely to the developer's creativity, cost targets, and design requirements.** |

---

## 3. Power & Electrical Safety Rules

### 3.1 Common 5V Power Trunk (VDD) Baseline
- **Voltage Specification:** **DC 5.0V (±10%)** (4.5V to 5.5V)
- **Current Rating:** **Up to 2.0A** (Delivered via 2×2 pin block)
- **Bidirectionality:** Power may flow either from Core board (USB Type-C) to CARDs, or from a Power CARD to the Core board.

### 3.2 Power CARD Design Rules (Separation of Responsibilities)

```
                 [ Double Check-Valve Architecture ]

   [ USB Type-C 6P ]
          │
     [ ⛔ Back-Feed Blocking (LM66100) ⛔ ] ──► (Prevents VDD back-powering PC host)
          │
          ▼
   ┌────────────────────────────────────────────────────────┐
   │  Common 5V VDD Trunk (Pins 1–4)                        │
   └────────────────────────────────────────────────────────┘
          ▲
          │
     [ ⛔ Reverse Blocking (Diode/FET) ⛔ ] ──► [MUST] Mandatory for Power CARDs!
          │                                   (Blocks VDD from entering external battery)
   [ ⚡ Protection / Surge / Regulators ⚡ ]
          │
   [ External Power (Dry Batteries / Factory 24V / Telecom -48V) ]
```

1. **[MUST] Reverse-Current Blocking for Power CARDs (Mandatory):**
   - Any CARD injecting power into the 20-pin bus (Pins 1–4: VDD) **MUST provide a reverse-current blocking mechanism at its output** (Schottky diode, ideal diode, or synchronous DCDC with reverse-blocking capability).
   - *(Reason: When CORE-A is plugged into a PC via USB Type-C on the workbench, the 5.0V USB rail must be 100% prevented from back-feeding into external alkaline batteries, eliminating **battery rupture and electrolyte leakage fires**).*
2. **[MUST] Strict Isolation of Pin 5 (N.C.):**
   - Pin 5 acts as a short-circuit prevention isolation buffer between the high-current VDD zone (Pins 1–4) and sensitive signals (Pin 6 onwards). **No traces, copper pours, or vias may connect to Pin 5**.
   - Prevents fatal short-circuits during ribbon cable crimping misalignment or manufacturing tolerances.
3. **[SHOULD] Transformer Isolation for High-Voltage Inputs (DC 12V/24V/48V):**
   - **Official ADX Grade:** To suppress common-mode noise, ground differentials, and lightning surges in factory panels, use a **transformer-isolated DC-DC converter (isolated 5V output)**.
   - **DIY / Hobby Grade [MAY]:** In controlled single-ground indoor environments, low-cost non-isolated buck converters are acceptable. Battery cards may also be non-isolated.
4. **[SHOULD] Input Port Protection:**
   - External power inputs (24V/48V or battery packs) should feature surge suppression (TVS/varistors), overcurrent protection (resettable PTC/fuse), and reverse-polarity protection (P-MOSFET).

---

## 4. Mechanical & Connector Rules

### 4.1 20-Pin Expansion Connector (ADX Pinout v2)
- **[MUST] Electrical Pinout Adherence:**
  - All boards must strictly adhere to the [ADX Pinout Specification (v2)](ADX_pinout.md).
- **Connector Type Selection:**
  - **Official Standard [SHOULD]:** To prevent dislodging under heavy automotive/industrial vibration and eliminate misaligned blind insertion in the field, use **MIL-DTL-83503 compliant Latched Eject Headers (牛角 / Ejector)**.
  - **DIY / General Add-ons [MAY]:** Standard boxed shrouded headers or simple pin headers may be used where low profile or minimal cost is prioritized.

### 4.2 Board Dimensions & Stacking
- **Official Standard [SHOULD]: 8748 Form Factor:**
  - Board Outline: **`87.0 mm × 48.0 mm`**
  - Mounting Holes: **4× M3 clearance holes (Φ3.3 mm)**
  - Hole Pitch: **`77.0 mm × 38.0 mm`** (`5.0 mm` margin from board edges)
  - Corner Treatment: **C3 Chamfer**
  - *(Refer to [8748 Form Factor Specification](8748_formfactor.md) for full CAD details).*
- **Third-Party / Custom Shapes [MAY]:**
  - Custom outline shapes (compact daughterboards, full-size carrier shields, L-shapes) are fully permitted as long as the 20-pin IDC interface aligns electrically.

---

## 5. Thermal & Environmental Rules

1. **[MUST] Heat Source Separation (Core Board Protection):**
   - Major thermal generators (high-voltage step-down regulators, relay coils, high-power switches) must reside on the CARD, isolated from CORE-A via PCB thermal relief slots and ground-plane partitioning.
2. **Temperature Rating Selection:**
   - **Official Standard [SHOULD]: Industrial Grade (-40°C to +105°C):**
     - Official industrial CARDs should use X7R capacitors, thick-film resistors, and industrial-rated ICs rated for -40°C to +105°C.
   - **DIY / General Add-ons [MAY]: Commercial Grade (0°C to 70°C / -20°C to 85°C):**
     - Common consumer components are fully acceptable for indoor experiments, hobby prototypes, and education.

---

## 6. Optional Features & Signal Routing [MAY]

The following signals may be utilized flexibly depending on application requirements:

1. **Battery Level / Bus Voltage Telemetry:**
   - External raw battery voltage can be divided down and routed to Pin 7 (`PA5/AIN5`) for analog voltage monitoring by the MCU.
2. **BMC-Coupled Sleep Shutdown:**
   - For ultra-low-power standby, connect Pin 18 (`PC3`) to the Enable (EN) pin of the CARD's DC-DC converter to completely shut down peripheral power rails during MCU sleep. (May be tied high if unused).

---

## 7. Developer Pre-Production Checklist

Before releasing or sending a PCB for fabrication (JLCPCB PCBA):

- [ ] **[MUST]** Did you place a reverse-blocking diode or FET at the output of the Power CARD?
- [ ] **[MUST]** Is Pin 5 (N.C.) completely isolated with no copper pours or traces touching it?
- [ ] **[MUST]** Does the 20-pin IDC connector pin assignment strictly match ADX Pinout v2?
- [ ] **[SHOULD]** Did you use a transformer-isolated DC-DC converter for industrial 24V/48V inputs?
- [ ] **[SHOULD]** Did you select a latched Eject Header if intended for high-vibration applications?
- [ ] **[SHOULD]** Does the board outline match the 8748 pitch (`77.0 mm × 38.0 mm`) for enclosure compatibility?

---

## 8. License

This guideline is published under the [Creative Commons Attribution 4.0 International License (CC-BY-4.0)](../../LICENSES/CC-BY-4.0.txt). Compatible hardware may be freely designed, fabricated, and commercially sold.
