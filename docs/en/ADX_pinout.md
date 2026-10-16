<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout Architecture & Design Guide

[ **English** | [日本語 (../ja/ADX_pinout.md)](../ja/ADX_pinout.md) ]

> [!IMPORTANT]
> **Single Source of Truth (SSOT):**  
> For the authoritative pin assignment table, strict electrical limits, and connector dimensions, please refer to the master specification: **[`docs/spec/ADX_PINOUT.md`](../spec/ADX_PINOUT.md)**.  
> This guide explains the engineering rationale, shielding topologies, and design rules for addon Card developers.

---

## 1. Overview & Engineering Philosophy

The ADX Common Expansion Bus utilizes a **2×10-pin (20-pin) 2.54 mm pitch IDC connector** that forms the standard backplane interface connecting Core Boards (MCU hosts) and Card Modules (expansion addons).

Unlike unshielded pin headers or breadboard wires common in hobbyist electronics, the ADX bus layout is carefully optimized down to each individual pin to simultaneously achieve:
- **High vibration immunity and anti-drop retention**
- **High-current delivery without thermal throttling**
- **Strict suppression of high-frequency radiated EMI**
- **Physical fail-safe protection against ribbon cable crimp misalignments**

---

## 2. Design Rationale & Structural Benefits

```text
                  【 20-Pin Shrouded Header (Top View) 】

                        ┌───────[ Center Key ]───────┐
           VBUS   [ 1]  │  ●   ●  │  [ 2]  VBUS
           VBUS   [ 3]  │  ●   ●  │  [ 4]  VBUS
           N.C.   [ 5]  │  ●   ●  │  [ 6]  AOUT / DAC
           AIN    [ 7]  │  ●   ●  │  [ 8]  GND
         EXTCLK   [ 9]  │  ●   ●  │  [10]  GND
        UART_TX   [11]  │  ●   ●  │  [12]  UART_RX
        I2C_SCL   [13]  │  ●   ●  │  [14]  I2C_SDA
            GND   [15]  │  ●   ●  │  [16]  SPI_CS
       SPI_MOSI   [17]  │  ●   ●  │  [18]  SPI_MISO
        SPI_SCK   [19]  │  ●   ●  │  [20]  GND
                        └────────────────────────────┘
```

### 2.1 Bidirectional Power Backbone `VBUS` (Pins 1–4: 2×2 Block)
- **High-Current Highway (4.5V – 5.0V / 2.0A continuous):** Allocating 4 dedicated pins ensures low IR voltage drop even over thin AWG 28 flat ribbon cable.
- **2×2 Solid Copper Plane:** Arranged in a $2 \times 2$ grid, PCB artwork can easily pour a continuous solid copper plane, maximizing heat dissipation and reducing inductance.
- **Bidirectional Powering:** Core boards can supply Cards, or dedicated Power Cards (e.g., DC 12V/24V industrial step-down or battery packs) can inject power into `VBUS` to energize the entire stack.

### 2.2 Short-Circuit Prevention Barrier (Pin 5 N.C.)
- **Isolation Buffer:** Placed strategically between the high-current `VBUS` power zone (Pins 1–4) and sensitive analog/GPIO signals (Pin 6+).
- **Fail-Safe Protection:** If flat cable crimping becomes misaligned or strands bridge adjacent contacts, high-current supply lines are prevented from shorting directly into MCU I/O ports.

### 2.3 Universal 3.3V LVCMOS Native Logic
- **Full GPIO Standardization:** All digital communication and I/O lines operate strictly at **3.3V LVCMOS native levels**.
- **Zero Level Shifter Overhead:** Addon Cards can interface directly with modern 3.3V sensors and ICs without requiring complex onboard level shifting.

### 2.4 Coplanar Clock Shielding (Pins 8–10)
- **GND Coplanar Sandwich:** The high-speed `EXTCLK` (Pin 9) is flanked on both sides by `GND` (Pins 8 and 10).
- **EMI Suppression:** Radiated emissions from clock harmonics are effectively shielded, isolating sensitive analog inputs (Pins 6, 7) from digital communications (Pins 11–20).

### 2.5 Segregated Digital Communication Cluster (Pins 11–20)
- Pin 10 (`GND`) isolates clock lines from serial buses.
- Pin 15 (`GND`) shields the open-drain I2C bus from the high-speed SPI bus.
- Pin 20 (`GND`) terminates the cable edge.

---

## 3. Design Guidelines for Addon Cards

When designing custom ADX-compatible Card modules:
1. **Power Copper Pour:** Connect Pins 1–4 together using multiple vias to a common power plane. Keep Pin 5 strictly unconnected (no copper, pad only).
2. **I2C Pull-Up Resistors:** Host Core boards may not implement onboard pull-ups on Pin 13 (`I2C_SCL`) and Pin 14 (`I2C_SDA`). Ensure your bus stack provides appropriate pull-ups ($2.2\,\text{k}\Omega \sim 4.7\,\text{k}\Omega$ to 3.3V).
3. **High-Speed Return Paths:** Route high-speed SPI signals in close proximity to Pin 8, 10, 15, or 20 GND pins to minimize loop area.

---

## 4. Controller-Specific Hardware Mappings

For exact pin multiplexing, timer allocations, and register configurations on specific MCU hosts, refer to:
- **Microchip ATtiny1616 (ADX CORE-A5):**  
  👉 [`docs/spec/mcu_mapping/attiny1616.md`](../spec/mcu_mapping/attiny1616.md)
