<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# ADX Firmware & Embedded Software

This directory contains official drivers, board support packages (BSP), bootloader code, and sample applications for ADX-compliant hardware modules and controller boards.

---

## 1. Directory Roadmap

As firmware modules are developed, they will be organized as follows:

```text
firmware/
├── README.md               # Firmware overview and development guide (this file)
├── bsp/                    # Board Support Packages & HAL definitions
├── drivers/                # Peripheral drivers (LN-485, DAC, ADC, I2C, SPI)
├── bootloaders/            # SerialUPDI & custom bootloader sources
├── examples/               # Example sketches and reference applications
└── tests/                  # Hardware verification, bring-up, and diagnostic tests
    └── ADX_Core-D/         # Hardware bring-up & test suite for ADX Core-D
```

---

## 2. Hardware Verification & Tests

- [ADX Core-D Verification Suite](./tests/ADX_Core-D/README.md): Hardware bring-up, SerialUPDI/SoftwareSerial communication tests, and verification report for the ATtiny1616-based ADX Core-D board.

---

## 3. License

All software, firmware, and code within this directory are released under the **MIT License**.

See [../LICENSE.md](../LICENSE.md) and [../LICENSES/MIT.txt](../LICENSES/MIT.txt) for full license details.
