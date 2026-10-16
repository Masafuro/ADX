<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ドキュメント疎結合化 ＆ Pinout 編纂再設計提案書
(ADX Documentation Decoupling & Pinout Architecture Proposal)

**文書ID**: PROPOSAL-ADX-DOC-ARCH-001  
**策定日**: 2026-10-16  
**ステータス**: 設計レビュー協議用  

---

## 1. 課題認識と再編の狙い (Why Decouple?)

現在、ADX リポジトリ内のドキュメントは、プロジェクトの急速な進化（CORE-A ➜ CORE-R ➜ CORE-A5）に伴い、以下の**「3つの密結合・重複課題」**を抱えています。

```text
【 現状の課題：密結合と多重管理 】

   [ docs/ja/ADX_pinout.md ] ─── 英語/日本語の両方に同じピン表が重複
   [ docs/en/ADX_pinout.md ] ─── ATtiny1616 固有のピン名 (PB2, PC3等) が規格に癒着
   [ README 各種 ]           ─── 各所にピン表がコピペされ、仕様変更時に修正漏れが発生
   [ 各 CORE 仕様書 ]        ─── 過去の仕様（5V GPIO, VDD等）が混在して陳腐化
```

### 抽出された3大課題
1. **真実の単一ソース (SSOT: Single Source of Truth) の欠如:**
   - ピンアサイン表が `ADX_pinout.md`（日・英）、各種 `README.md`、各ボードの仕様書などに何度もコピペ・再掲されており、改訂時の修正漏れ・不整合リスクが高い。
2. **規格（バス仕様）と実装（特定MCU）の混同:**
   - 共通バス規格であるはずの `ADX_pinout.md` に、ATtiny1616 の端子番号（`PA6`, `PB2`, `PC3`）や `PORTMUX` の設定方法が埋め込まれている。
   - CORE-R（RP2040）や将来の ESP32 系 CORE が登場した際に、規格ドキュメントとして破綻する。
3. **データ（規格定義）と言語（思想・解説）の混在:**
   - ピン番号、ネット名、定格といった「純粋な仕様データ」は本来言語に依存しない（プログラミングコードと同じ）。
   - それが日本語・英語の散文の中に埋め込まれているため、翻訳や同期のコストが無駄に増大している。

---

## 2. 疎結合ドキュメント・アーキテクチャ（4層構造）

ドキュメント全体を以下の **4つの独立したレイヤー** に分離・再編します。

```text
 ┌────────────────────────────────────────────────────────────────────────┐
 │ 【 Layer 1: Core Specification (SSOT) 】                               │
 │   docs/spec/ADX_PINOUT.md (言語非依存 / 英語アルファベット / 仕様純度100%)│
 │   ・普遍的なバス定義（Pin 1〜20）                                      │
 │   ・抽象論理ネット名（VBUS, UART_TX, I2C_SDA, SPI_CS 等）               │
 │   ・電気的・物理的定格（3.3V LVCMOS, 2A, IDC 20P 形状）                 │
 └───────────────────────────────────┬────────────────────────────────────┘
                                     │ 参照 (単一ソース)
         ┌───────────────────────────┴───────────────────────────┐
         ▼                                                       ▼
 ┌─────────────────────────────────┐   ┌─────────────────────────────────┐
 │ 【 Layer 2: Guides & Rationale】│   │ 【 Layer 3: MCU Implementation 】│
 │   docs/ja/guides/pinout_guide.md│   │   docs/spec/mcu_mapping/        │
 │   docs/en/guides/pinout_guide.md│   │    ├─ attiny1616.md (CORE-A5)   │
 │   ・日本語・英語での設計思想解説 │   │    └─ rp2040.md     (CORE-R)    │
 │   ・なぜ VBUS なのか / シールド │   │   ・共通バス ➜ MCU各端子マッピング│
 │   ・CARD基板のパターン設計指針   │   │   ・PORTMUX, タイマー, PWM割当  │
 └─────────────────┬───────────────┘   └────────────────┬────────────────┘
                   │                                    │
                   └─────────────────┬──────────────────┘
                                     │ リンク参照のみ (表の再掲禁止)
                                     ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │ 【 Layer 4: Products & Portals 】                                      │
 │   hardware/CORE-A5/ , hardware/CORE-R/ , hardware/CARD/ , README.md   │
 │   ・各製品の個別仕様書およびポータル                                   │
 │   ・「拡張バスは ADX Pinout 規格に準拠」とリンクを貼るだけ             │
 └────────────────────────────────────────────────────────────────────────┘
```

---

## 3. 各レイヤーの責務とファイル構成案

### 3.1 【Layer 1】普遍的バス規格（Core Specification / SSOT）
- **配置:** `docs/spec/ADX_PINOUT.md`
- **特徴:** 
  - 全編シンプルな英語・アルファベット表記。
  - マイコン（ATtiny1616等）に依存しない、純粋な**「バスのインターフェース仕様書」**。
  - ピン番号、抽象ネット名、方向、電気的定格のみを端正に記載。

#### `docs/spec/ADX_PINOUT.md` のイメージ（抜粋案）

```markdown
# ADX 20-Pin Bus Specification (Pinout v2.1)

## 1. Pin Assignment Table

| Pin | Net Name | Signal Type | Electrical Level | Description |
| :---: | :--- | :--- | :--- | :--- |
| **1** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) |
| **2** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) |
| **3** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus (Bidirectional) |
| **4** | **VBUS** | Power | 4.5V – 5.0V DC (2.0A max) | Main Power Bus Boost |
| **5** | **N.C.** | - | - | Isolation Barrier (Short-circuit prevention) |
| **6** | **AOUT** | Analog / GPIO | 3.3V Analog Out / LVCMOS | DAC Output / Analog Channel |
| **7** | **AIN**  | Analog / GPIO | 3.3V Analog In / LVCMOS  | Voltage Reference / Analog Channel |
| **8** | **GND**  | Power / Shield | 0V | Coplanar Shield (Clock guard) |
| **9** | **EXTCLK**| Digital In / GPIO | 3.3V LVCMOS | External High-speed Clock Input |
| **10**| **GND**  | Power / Shield | 0V | Inter-bus Shield (Isolation) |
| **11**| **UART_TX** | Digital Out | 3.3V LVCMOS | Serial Transmit |
| **12**| **UART_RX** | Digital In  | 3.3V LVCMOS | Serial Receive |
| **13**| **I2C_SCL** | Open-Drain  | 3.3V LVCMOS (Pull-up req.) | I2C Clock |
| **14**| **I2C_SDA** | Open-Drain  | 3.3V LVCMOS (Pull-up req.) | I2C Data |
| **15**| **GND**  | Power / Shield | 0V | Mid-bus Shield |
| **16**| **SPI_CS**  | Digital Out | 3.3V LVCMOS | SPI Chip Select |
| **17**| **SPI_MOSI**| Digital Out | 3.3V LVCMOS | SPI Controller Out / Peripheral In |
| **18**| **SPI_MISO**| Digital In  | 3.3V LVCMOS | SPI Controller In / Peripheral Out |
| **19**| **SPI_SCK** | Digital Out | 3.3V LVCMOS | SPI Clock |
| **20**| **GND**  | Power / Shield | 0V | Bus Termination Shield |

## 2. Electrical Specifications
- **VBUS Voltage:** 4.5V to 5.0V DC (tolerates diode drops from 5.0V sources)
- **Logic Level:** Standard 3.3V LVCMOS (Native 3.3V logic across all GPIOs)
- **Connector Type:** 2.54mm pitch 2×10-pin Shrouded IDC Header (MIL-DTL-83503 compliant)
```

---

### 3.2 【Layer 2】言語別・設計思想解説（Guides & Rationale）
- **配置:** `docs/ja/guides/pinout_guide.md` / `docs/en/guides/pinout_guide.md`
- **特徴:**
  - なぜ Pins 1〜4 が VBUS なのか（2×2 正方形ベタプレーンによる配線・放熱の最適化）。
  - なぜ Pin 5 が N.C. なのか（ケーブル圧接不良による電源-GPIO短絡防止）。
  - なぜ Pin 8, 10 で EXTCLK をサンドイッチしているのか（EMI放射抑制）。
  - 拡張 CARD を自作・設計するエンジニア向けの配慮事項やパターン設計ルール。
  - ピン表自体は記載せず、「規格定義は [`ADX_PINOUT.md`](../spec/ADX_PINOUT.md) を参照」と誘導。

---

### 3.3 【Layer 3】MCU 個別実装マッピング（MCU Specific Mappings）
- **配置:** `docs/spec/mcu_mapping/`
  - `attiny1616.md` (CORE-A5 向けマッピング ＆ PORTMUX レジスタ詳細)
  - `rp2040.md` (CORE-R 向けマッピング ＆ PIO/GPIO 割り当て)
- **特徴:**
  - 共通バス（VBUS, UART_TX, SPI_MOSI 等）が、各マイコンの「どの物理ピン」「どの内部ペリフェラル」に繋がっているかを記載。
  - これにより、ATtiny1616 固有の話題（PORTMUX や TCD0）が共通バス規格に混ざるのを完全に防止。

---

### 3.4 【Layer 4】製品仕様書 ＆ ポータル（Products & Portals）
- **対象:** 各種 `README.md`, `hardware/CORE-A5/`, `hardware/CARD/`
- **設計ルール（重要）:**
  - **ピン配置テーブルの丸ごとコピペを原則禁止**。
  - 「共通拡張バス（ADX Pinout v2.1 準拠）」と表記し、`docs/spec/ADX_PINOUT.md` へのリンクを貼るのみとする。
  - これにより、今後ピン規格のマイナーアップデートや解説追記があっても、各ボードの仕様書が陳腐化・矛盾することを永久に防ぎます。

---

## 4. 移行・リファクタリング計画 (Migration Roadmap)

```text
  【 Step 1: Layer 1 の新設 (SSOT 確立) 】
    └─ docs/spec/ADX_PINOUT.md を作成（アルファベット表記・普遍的バス規格）

  【 Step 2: Layer 3 の切り出し (MCU 密結合の解消) 】
    └─ 既存の adx_attiny1616_mcu_matrix.md を docs/spec/mcu_mapping/attiny1616.md に再配置

  【 Step 3: Layer 2 の整理 (日・英 ガイド) 】
    ├─ docs/ja/ADX_pinout.md ➜ docs/ja/guides/pinout_guide.md へ整理（解説特化）
    └─ docs/en/ADX_pinout.md ➜ docs/en/guides/pinout_guide.md へ整理（解説特化）

  【 Step 4: Layer 4 のリンク疎結合化 (コピペ表の撤廃) 】
    ├─ 各種 README.md 内のインラインピン表を Layer 1 へのリンクに置換
    └─ 古い作業メモ（docs/memo/adx_pinout_v2.md 等）を archive 配下へ退避
```

---

## 5. まとめ ＆ レビューのお願い

この「4層疎結合アーキテクチャ」により：
- **仕様の単一性 (SSOT):** ピンアサインの変更・確認は `docs/spec/ADX_PINOUT.md` の1箇所だけで完結します。
- **マルチMCUへの対応力:** ATtiny1616、RP2040、ESP32 が対等な実装例として綺麗に並び立ちます。
- **ドキュメントの長寿命化:** 各製品の仕様書に過去のピン配置が残って腐る問題が根本的に解決します。

この構成方針で進めてよろしいでしょうか？
よろしければ、まずは **Step 1: `docs/spec/ADX_PINOUT.md`（英語・普遍的バス規格マスター）** の作成から着手いたします。
