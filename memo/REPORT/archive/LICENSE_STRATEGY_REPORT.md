# ADX プロジェクト ライセンス戦略および記載方針レポート
(ADX Multi-License Strategy & Implementation Guide)

本ドキュメントは、ADX（Advanced Devices eXtended）プロジェクトにおけるマルチライセンス構成の妥当性評価、リポジトリへの具体的な記載方法、およびオープンソースハードウェア（OSHW）としてのベストプラクティスをまとめたレポートです。

---

## 1. 提案されたライセンス構成の評価

ユーザーより提示された以下のライセンス構成は、オープンソースハードウェア（OSHW）のグローバルスタンダード（OSHWA推奨、Raspberry Pi、Arduino、CERN等のエコシステム）において、**最も調和が取れ、実用性が高く、普及に最適な「パーミッシブ型3層構造（Tri-License Architecture）」** です。

```text
ADX Project
├── docs       : CC BY 4.0       (仕様書・解説ドキュメント・図解)
├── hardware   : CERN-OHL-P-v2   (回路図・PCB・BOM・CADデータ)
└── firmware   : MIT             (ドライバ・サンプルコード・BSP)
```

### レイヤー別選定理由とメリット

| レイヤー | 適用ライセンス | SPDX識別子 | 選定理由とメリット |
| :--- | :--- | :--- | :--- |
| **Documentation**<br>(仕様書・記事) | **CC BY 4.0**<br>(Creative Commons Attribution 4.0) | `CC-BY-4.0` | **文章・図解に特化した世界標準ライセンス**。<br>著作者クレジットを表示すれば、商用利用・翻訳・二次利用（派生仕様の作成）が自由に認められます。コミュニティによる仕様書の普及や多言語化を強力に後押しします。 |
| **Hardware**<br>(回路図・基板・CAD) | **CERN-OHL-P-v2**<br>(CERN Open Hardware Licence v2 - Permissive) | `CERN-OHL-P-2.0` | **ハードウェア専用に設計されたオープンライセンス**。<br>ソフトウェア用ライセンス（MIT/GPL）では曖昧だった「製造」「製品（物理物）」「CADデータ」の定義が法的に明確化されています。<br>**`-P`（Permissive）** はコピーレフト義務（派生基板の全公開強制）を課さないため、サードパーティ企業がADX準拠の産業用モジュールや商用ボードを独自開発しやすく、エコシステム拡大の参入障壁を最小化できます。 |
| **Firmware**<br>(コード・ドライバ) | **MIT License** | `MIT` | **組み込みソフトウェア界のデファクトスタンダード**。<br>極めてシンプルかつ制約が少なく、プロプライエタリな商用ファームウェアや各種RTOS（FreeRTOS, Zephyr等）への組み込みが容易です。開発者の心理的ハードルを最低限に抑えます。 |

---

## 2. ライセンスの記載・管理方式（2つのアプローチ）

GitHubリポジトリにおいてマルチライセンスを明記する方法には、主に以下の2種類があります。

### アプローチ1：REUSE / SPDX 準拠方式【強く推奨・プロフェッショナル】
Linux FoundationやFSFE（Free Software Foundation Europe）が提唱する **REUSE仕様** に準拠した構造です。自動ライセンススキャンツール（FOSSA, GitHub Licensee等）との親和性が最高です。

```text
ADX/
├── LICENSE.md              # リポジトリ全体のライセンス案内（マルチライセンス概要）
├── LICENSES/               # 各ライセンスの正式条文テキスト
│   ├── CC-BY-4.0.txt
│   ├── CERN-OHL-P-2.0.txt
│   └── MIT.txt
├── docs/                   # ファイル先頭に SPDX-License-Identifier: CC-BY-4.0
├── dev/ (または hardware/)  # 回路図・BOM等に SPDX-License-Identifier: CERN-OHL-P-2.0
└── firmware/               # ソースコードに SPDX-License-Identifier: MIT
```

### アプローチ2：ディレクトリ個別配置方式【シンプル・直感的】
各主要ディレクトリ直下にそれぞれの `LICENSE` ファイルを配置する方式です。

```text
ADX/
├── LICENSE.md              # ルート（全体のマルチライセンス要約と各ディレクトリへの案内）
├── docs/
│   └── LICENSE.md          # CC BY 4.0 全文
├── dev/ (hardware/)
│   └── LICENSE.md          # CERN-OHL-P-2.0 全文
└── firmware/
    └── LICENSE.md          # MIT 全文
```

> [!TIP]
> **推奨方針**:
> ルートに包括的な **`LICENSE.md`** を設置し、主要ライセンス条文を `LICENSES/` ディレクトリに集約する **アプローチ1（REUSE準拠）** を基本としつつ、各ディレクトリ（`docs/`、`dev/`、`firmware/`）のREADMEやヘッダーにもライセンスを明記する形が最もトラブルが少なく堅牢です。

---

## 3. 具体的な記載テンプレート

そのままリポジトリに適用できるMarkdownおよびテキストの記述例です。

### 3.1 ルート `README.md` の記載例

`README.md` の末尾に以下のセクションを設けます。

```markdown
## License

ADX Project is released under a tri-license structure tailored for documentation, open hardware, and embedded software:

* **Documentation & Specifications** (`docs/`, `memo/`):  
  [Creative Commons Attribution 4.0 International (CC BY 4.0)](LICENSES/CC-BY-4.0.txt)  
  *SPDX-License-Identifier: `CC-BY-4.0`*

* **Hardware Design & Production Files** (`dev/`, `hardware/`):  
  [CERN Open Hardware Licence Version 2 - Permissive (CERN-OHL-P-v2)](LICENSES/CERN-OHL-P-2.0.txt)  
  *SPDX-License-Identifier: `CERN-OHL-P-2.0`*

* **Firmware, Drivers & Software** (`firmware/`, sample code):  
  [MIT License](LICENSES/MIT.txt)  
  *SPDX-License-Identifier: `MIT`*

See [LICENSE.md](LICENSE.md) for full license notices and terms.
```

---

### 3.2 ルート `LICENSE.md` の記載例

リポジトリルートに配置する `LICENSE.md` のフルテンプレートです。

```markdown
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
* **Attribution Requirement:** You must give appropriate credit to the **ADX Project**, provide a link to the license, and indicate if changes were made.

---

## 2. Hardware License (CERN-OHL-P-v2)

All hardware design files, schematic files, PCB layout files, manufacturing packages (Gerber, drill files), Bill of Materials (BOM), and mechanical CAD files located in the `dev/` (and `hardware/`) directories are licensed under the **CERN Open Hardware Licence Version 2 – Permissive (CERN-OHL-P-2.0)**.

* You may manufacture, distribute, and sell physical products based on these designs.
* You may modify the design files without being required to release your modifications under the same license (permissive nature).
* **Attribution & Notice:** You must retain copyright notices and license headers on all distributed design files.

---

## 3. Firmware & Software License (MIT License)

All source code, drivers, board support packages (BSP), bootloader code, example applications, and scripts located in the `firmware/` and code directories are licensed under the **MIT License**.

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

---

## 4. Brand & Trademark Notice

The "ADX" name, logo, and project badges located in `logo/` are trademarks/identifiers of the ADX Project. 

* Open-source licenses grant rights to copyright and hardware design, but **do not grant permission to use the project logo or brand name in a way that implies official endorsement or certification** of third-party hardware.
* Third-party products compatible with ADX standards are encouraged to use phrases such as *"Designed for ADX"* or *"ADX 8748 Compatible"*, rather than naming their product *"Official ADX..."*.
```

---

### 3.3 各ファイル内での SPDX ヘッダー記述例

各ファイル先頭に1〜2行のSPDXタグを挿入することで、ファイル単位での権利関係が誰の目にも一目瞭然になります。

#### ① ファームウェア・C/C++・Python コード (`.c`, `.h`, `.cpp`, `.py`)
```c
/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 */
```

#### ② 仕様書・Markdown ドキュメント (`.md`)
```markdown
<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->
```

#### ③ KiCad 回路図・PCB (Page Settings の Comment フィールド / ネットリスト)
KiCadの **Page Settings (図面枠の設定)** の `Comments` や `Title block` に以下を記入します：
* **License:** `CERN-OHL-P-2.0`
* **Comment 1:** `Licensed under CERN-OHL-P-v2. See LICENSE.md`
* **Comment 2:** `SPDX-License-Identifier: CERN-OHL-P-2.0`

---

## 4. 商標・ロゴ（Brand Asset）に関する推奨留意事項

オープンソースハードウェアにおいて頻出するトラブルとして、**「第三者が低品質な互換基板を作り、公式ロゴをそのまま載せて販売する」** ケースがあります。

これを防止するため、以下の切り分けを推奨します：
1. **回路・パターン・仕様・コード**: 完全オープン（商用製造・改変自由）
2. **ロゴ画像・公式マーク（`logo/`）**: `All Rights Reserved` または `CC BY-ND`（改変禁止・商標保持）とし、LICENSE.md に **「Trademark Notice（商標・ブランド利用規約）」** を明記する。

これにより、「誰でも自由にADX互換ボードを作って販売できるが、公式製品と誤認させるようなロゴの無断利用は防ぐ」という健全なエコシステム運営が可能になります。

---

## 5. 今後の導入ステップ（ロードマップ）

1. **`memo/REPORT/LICENSE_STRATEGY_REPORT.md`（本レポート）の確認・合意**
2. **`LICENSES/` ディレクトリの作成**：
   - `LICENSES/CC-BY-4.0.txt`
   - `LICENSES/CERN-OHL-P-2.0.txt`
   - `LICENSES/MIT.txt`
3. **ルート `LICENSE.md` の作成**（本レポートの3.2項のテンプレートを配置）
4. **`README.md`（英語・日本語）への License セクションの追記**
5. **既存ファイル（仕様書・dev配下の基板データ・コード）へのSPDXヘッダーの順次適用**
