<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CERN-OHL-P-2.0
-->

# ADX ハードウェアリポジトリ (Hardware Repository)

本ディレクトリは、ADXエコシステムにおけるハードウェア設計データ（回路図、PCBアートワーク、BOM、製造データ等）を管理・公開するリポジトリです。

---

## 1. ハードウェアラインナップ

ADXのハードウェアは、**8858 Form Factor（正式 α 版）**（`88.0 mm × 58.0 mm`、旧 8854 / 8748）および共通の **20ピン IDC 拡張バス（`ADX-BUS-SPEC-v5.0`）** を共有する「Coreシリーズ（メインボード）」と「CARDシリーズ（機能拡張基板）」で構成されます。

### 1.1 Core シリーズ (メインMCUボード)

| ボード名 | グレード | 保護・通信構造 | 状態・概要 | リンク |
| :--- | :--- | :--- | :--- | :--- |
| **ADX CORE-U (rev6)** | **Flagship** | **3 Mode 耐環境 / 独立 Dual Type-C / ラッチ付 IDC 20P** | **ADX フラッグシップ Core ボード（仕様確定・設計中）。** ATtiny1616 (3.3V 10MHz) ＋ `CH342K`（Mode 1/2 完全 0mW 遮断）、Mode 1 `-40℃〜+85℃` 理論仕様、AI 電力観測、3 Mode 自己認識・深睡眠起床、`ADX-BUS-SPEC-v5.0` 準拠。絶縁 RS-485 / 24V・48V 電源は拡張 CARD 側へ委譲。 | [技術仕様書 (rev6)](CORE-U/rev6/spec.md) / [耐熱ベンチマーク](CORE-U/rev6/plc_thermal_benchmark_report.md) / [深睡眠考察](CORE-U/rev6/deep_sleep_potential.md) |
| **ADX CORE-U (rev0〜rev5)** | **Legacy / Pre-rev6** | デュアル Type-C（旧 `v4.2` 試作世代） | rev6 への進化過程における前世代仕様群。 | [rev5 仕様案](CORE-U/rev5/spec_proposal.md) / [rev4 仕様案](CORE-U/spec_proposal.md) / [比較・設計思想](CORE-U/comparison.md) |
| **ADX CORE-G** | **Archive / Study** | Type-C 1本 / Grove×3 | Grove / M5Unit 特化プロトタイピング機検討ログ。 | [構想書](archive/CORE-G/concept.md) |
| **ADX CORE-A5 / CORE-A** | **Derivative / Archive** | 2500V 絶縁RS-485 / VBUS幹線受電 | 付加価値型派生ボード参考（旧現場向けモデル。CORE-U rev6 フラッグシップ化に伴い主軸から外しアーカイブ参照）。 | [旧構想書](archive/CORE-A5/concept.md) / [旧仕様書](archive/CORE-A5/spec_study.md) |
| **ADX CORE-CX5** | **Archive / Research** | **デュアル Type-C（通信/電源分離）** | USB Core 試作研究モデル。WCH CH32X035（32-bit RISC-V 48MHz）、USB-PHY 内蔵。 | [研究構想書](OLD/CORE-CX5/concept.md) / [Mapping 仕様](../docs/spec/mcu_mapping/ch32x035.md) |
| **ADX CORE-U (旧版)** | **Archive** | **PC保護回路** | 旧USB構想ボード。ATtiny1616初期試作。 | [旧検討ノート](archive/CORE-U_OLD/concept.md) |
| **ADX CORE-A** | **Archive** | **完全絶縁RS-485 / 5V給電** | 初代現場向けモデル（CORE-A5/A24 PROへ発展継承）。 | [旧構想書](archive/CORE-A/concept.md) |
| **ADX CORE-S** | **Archive** | **非絶縁** (共通GND) | 低価格製造を追求した試作機（旧 `-P` 区分 / アーカイブ）。 | [詳細・レビュー](archive/CORE-S/README.md) |
| **ADX Core-D** | **Development** | **非絶縁** | LN-485（LIN-based RS-485）ブートローダおよびファームウェア開発専用ボード。 | [仕様書](archive/CORE-D/proposal.md) |

### 1.2 CARD シリーズ (拡張アドオン基板)

| ボード名 | 概要 | リンク |
| :--- | :--- | :--- |
| **ADX Reference CARD v1** | 8858 Form Factor・`ADX-BUS-SPEC-v5.0`（`4.5V~5.5V`）・THT Eject Header (`C9039`)・光学マーカーを具現化した公式リファレンスボード（`v1` 仕様確定・CAD 更新待ち）。 | [仕様書](CARD/Reference/concept.md) |
| **ADX Prototype CARD** | 2×20P ADX バス引き出し ＋ THT / SMD ユニバーサル試作エリアを備えたプロトタイピング拡張基板（v0 は旧 8854 試作）。 | [仕様書](CARD/Prototype/proposal.md) |

### 1.3 フォームファクタ & 規格仕様

- **8858 フォームファクタ定義 (正式 α 版・現行標準)**: [8858 仕様書](../docs/ja/8858_formfactor.md) / [8858 検討書](CARD/Reference/8858ff_plan.md) *(旧仕様: [8854 仕様書](../docs/ja/8854_formfactor.md) / [8748 仕様書](../docs/ja/8748_formfactor.md))*
- **ADX 20ピンバス規格 (SSOT)**: [普遍的バス規格書 (英語・アルファベット)](../docs/spec/ADX_PINOUT.md)（Standard: `ADX-BUS-SPEC-v5.0`）
- **ADX ピン配置 設計ガイド**: [日本語仕様解説](../docs/ja/ADX_pinout.md) / [English Architecture Guide](../docs/en/ADX_pinout.md)

---

## 2. 命名規則・グレード分類（無印 vs `PRO`）

ADXのハードウェアは、適用領域および試験可能性（Testability）に応じて以下の2つのグレードに明確に分類されています。

```text
ADX [FAMILY]-[TYPE] [GRADE]
```

1. **Standard Grade（無印・サフィックスなし）**:
   - 例: `ADX CORE-G`, `ADX CORE-U`, `ADX CARD-RELAY`
   - **要件**: USB Type-C や Grove（HY2.0）など民生・卓上向けインターフェースの手軽さとコストを優先。eFuse・ESD保護など基本的な安全保護は備えるが、現場常設の耐振動や業務用全数定格試験は保証対象外。
   - **用途**: 机上開発、プロトタイピング、M5Unit 連携、一般エントリー。
2. **PRO Grade（`PRO` 付与型）**:
   - 例: `ADX CORE-A PRO`, `ADX CORE-A24 PRO`, `ADX CARD-RELAY PRO`
   - **要件**: 民生 USB ボトルネックを排し、スプリングクランプ端子・ラッチ付き IDC・絶縁 RS-485 等で構成。全 BOM の動作温度定格を確定し、耐熱・耐振動・連続通電の定格試験条件を策定できる堅牢設計。
   - **用途**: 制御盤、設備組込、過酷環境での現場常設。

※ 詳細な分類判定基準については、[ハードウェア命名規則・グレード分類基準](../docs/ja/hardware_naming_rules.md) を参照してください。

---

## 3. ライセンス

本ディレクトリ内のすべてのハードウェア設計データは、**CERN-OHL-P-v2（CERN Open Hardware Licence Version 2 - Strongly Reciprocal）** のもとで公開されています。
詳細は [LICENSE.md](../LICENSE.md) および [LICENSES/CERN-OHL-P-2.0.txt](../LICENSES/CERN-OHL-P-2.0.txt) を参照してください。
