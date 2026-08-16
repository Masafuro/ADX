# ADX Licensing Policy

The ADX (Advanced Devices eXtended) Project uses a multi-licensing model to provide the most suitable open-source terms for each component of the ecosystem:

| Component | Scope | License | SPDX Identifier | License Text |
| :--- | :--- | :--- | :--- | :--- |
| **Documentation** | Specifications, manuals, guides, schematics diagrams in docs | **CC BY 4.0** | `CC-BY-4.0` | [LICENSES/CC-BY-4.0.txt](LICENSES/CC-BY-4.0.txt) |
| **Hardware** | KiCad schematics, PCB layouts, Gerber, BOM, 3D CAD models | **CERN-OHL-P-v2** | `CERN-OHL-P-2.0` | [LICENSES/CERN-OHL-P-2.0.txt](LICENSES/CERN-OHL-P-2.0.txt) |
| **Firmware & Code** | Drivers, BSP, bootloaders, sample sketches, utility scripts | **MIT License** | `MIT` | [LICENSES/MIT.txt](LICENSES/MIT.txt) |

---

## 1. Documentation License (CC BY 4.0)

All documentation, specification documents (including the 8748 Form Factor and ADX Pinout specifications), illustrations, and explanatory text within the `docs/` and `memo/` directories are licensed under the **Creative Commons Attribution 4.0 International License**.

* You are free to share (copy and redistribute) and adapt (remix, transform, and build upon) the material for any purpose, including commercial use.
* **Attribution Requirement:** You must give appropriate credit to the **ADX Project Contributors**, provide a link to the license, and indicate if changes were made.
* See [LICENSES/CC-BY-4.0.txt](LICENSES/CC-BY-4.0.txt) for the full license text.

---

## 2. Hardware License (CERN-OHL-P-v2)

All hardware design files, schematic files, PCB layout files, manufacturing packages (Gerber, drill files), Bill of Materials (BOM), and mechanical CAD files located in the `hardware/` directory are licensed under the **CERN Open Hardware Licence Version 2 – Permissive (CERN-OHL-P-2.0)**.

* You may manufacture, distribute, and sell physical products based on these designs.
* You may modify the design files without being required to release your modifications under the same license (permissive nature).
* **Attribution & Notice:** You must retain copyright notices and license headers on all distributed design files.
* See [LICENSES/CERN-OHL-P-2.0.txt](LICENSES/CERN-OHL-P-2.0.txt) for the full license text.

---

## 3. Firmware & Software License (MIT License)

All source code, drivers, board support packages (BSP), bootloader code, example applications, and scripts located in the `firmware/` directory are licensed under the **MIT License**.

```text
Copyright (c) 2026 ADX Project Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
* See [LICENSES/MIT.txt](LICENSES/MIT.txt) for the full license text.

---

## 4. Brand & Trademark Notice

The "ADX" name, logo, and project badges located in `logo/` are trademarks/identifiers of the ADX Project. 

* Open-source licenses grant rights to copyright and hardware design, but **do not grant permission to use the project logo or brand name in a way that implies official endorsement or certification** of third-party hardware.
* Third-party products compatible with ADX standards are encouraged to use phrases such as *"Designed for ADX"* or *"ADX 8748 Compatible"*, rather than naming their product *"Official ADX..."*.
