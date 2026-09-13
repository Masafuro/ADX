<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ピンアウト v2 改訂に伴う README・ドキュメント修正計画書
(Pinout v2 Documentation & README Revision Plan)

## 1. 背景と改訂の目的 (Background & Objective)

先行して検討・策定を進めてきた **ADX ピンアウト仕様書 (v2)**（[`docs/memo/adx_pinout_v2.md`](../../docs/memo/adx_pinout_v2.md)）および **MCUペリフェラル＆PORTMUX統合リファレンス**（[`docs/memo/adx_attiny1616_mcu_matrix.md`](../../docs/memo/adx_attiny1616_mcu_matrix.md)）により、ADX共通拡張バスの物理配置・電気的シールド構造・MCU多重化機能が確定しました。

本計画書は、確定した v2 仕様に基づき、現在リポジトリ各所に残っている旧ピンアウト（v1）の記述を最新仕様へ整合・同期させるための作業計画および修正方針を定義するものです。

---

## 2. ピンアウト v1 から v2 への主要変更点 (Key Architectural Changes)

| 設計項目 | 旧仕様 (v1: `memo/ADX_pinout.md`) | 新仕様 (v2: `docs/memo/adx_pinout_v2.md`) | 改訂の理由・技術的メリット |
| :--- | :--- | :--- | :--- |
| **コネクタ呼称** | `CN2` (特定基板の部品番号) | **`IDC` / `IDC Pin 1〜20`** | 基板個別の回路図Designatorから独立させ、規格共通バスとしての普遍性を向上。 |
| **電源供給** | VDD × 2ピン (Pins 2, 3) | **VDD × 3ピン (IDC Pins 1〜3)** | フラットケーブル（AWG28）接続時でも **5V / 2A 級** の安定給電を実現。 |
| **短絡防止** | - | **IDC Pin 4 (N.C.)** | VDDゾーン（Pin 3）とGND（Pin 5）の間に緩衝帯を設け、ケーブル圧接時のショート事故を防止。 |
| **クロックシールド** | 片側GND (Pin 6 のみ) | **両側GNDサンドイッチ (IDC Pins 5, 7)** | 高周波 EXTCLK (Pin 6) を両側からGNDで挟み込むコプレーナGNDシールド構造を確立。 |
| **アナログ保護** | 静止制御線（RE/DE）による緩衝 | **完全GNDアイソレーション (IDC Pins 7, 10)** | アナログ信号（Pin 8 DAC0, Pin 9 AIN5）の両隣をGNDでシールドし、デジタル通信ノイズを遮断。 |
| **UART通信** | オンボードRS-485と信号共有 (PA1/PA2) | **PORTMUX排他切替 (IDC Pins 11, 12: PB2/PB3)** | `PORTMUX.CTRLB.USART0` によりオンボードRS-485と外部直接UARTをハードウェアで排他切替。 |
| **ペリフェラル定義** | PWM等の記載が限定的 | **全タイマー (TCD0/TCA0/TCBn) 逆引き対応** | 最大48MHz高周波タイマー TCD0（3ch PWM）や ADC（最大8ch以上）の引き出しを網羅。 |

---

## 3. 対象ドキュメントと改訂スコープ (Revision Scope per Document)

本改訂では、以下のドキュメントを対象とし、段階的に更新を実施します。

```text
ADX/
├── README.md                           # ① ルート英語ポータル（要約・リンク更新）
├── docs/
│   ├── ja/
│   │   ├── README.md                   # ② 日本語ポータル（v2 ピンアサイン表・メリット・早見表の反映）
│   │   ├── ADX_pinout.md               # ④ 正式仕様書（v2仕様書の正式昇格）
│   │   └── adx_attiny1616_mcu_matrix.md# ④ MCU詳細リファレンスの正式配備
│   └── en/
│       ├── README.md                   # ③ 英語ポータル（v2 ピンアサイン表・メリット・早見表の反映）
│       ├── ADX_pinout.md               # ④ 英語版正式仕様書（v2仕様書の正式昇格）
│       └── adx_attiny1616_mcu_matrix.md# ④ 英語版MCU詳細リファレンスの正式配備
└── memo/
    └── PLAN/
        └── PINOUT_V2_DOCS_REVISION_PLAN.md  # 本計画書
```

### ① ルート `README.md` (Global Portal)
- **改訂箇所**: `3.2 ADX Pinout` セクション
- **内容**:
  - コネクタ仕様を `20-pin IDC ribbon connector (IDC Pins 1–20)` に更新。
  - v2 の主要なハイライト（5V/2A電源供給、EXTCLKおよびアナログゾーンの両側GNDシールド、PORTMUXによるUART排他切替）を簡潔に紹介。
  - 正式ドキュメントへのリンクを更新。

### ② `docs/ja/README.md` (Japanese Portal)
- **改訂箇所**: `3.2 ADX Pinout（共通 20P 拡張インターフェース）` セクション
- **内容**:
  - 掲載されている旧ピンアサイン表を、**最新の v2 表（IDC Pin 1〜20）** に全面刷新。
  - 「主な電気的・構造的メリット（電源供給、短絡防止、クロックシールド、アナログアイソレーション、バス配置維持）」を掲載。
  - 「IDC 主要ペリフェラル機能 早見サマリー（PWM出力タイマー、ADC入力、特殊アナログ）」を掲載。
  - 詳細リファレンス（`adx_attiny1616_mcu_matrix.md`）への案内を追加。

### ③ `docs/en/README.md` (English Portal)
- **改訂箇所**: `3.2 ADX Pinout (Unified 20-Pin Interface)` セクション
- **内容**:
  - `docs/ja/README.md` と 1:1 で対応するよう、v2 表、主要メリット、ペリフェラル早見サマリーの英語版を反映。

### ④ 正式仕様書への昇格 (`docs/ja/` & `docs/en/`)
- `docs/memo/adx_pinout_v2.md` をベースに、正式仕様書 [`docs/ja/ADX_pinout.md`](../../docs/ja/ADX_pinout.md) を改訂。
- 英語版仕様書 [`docs/en/ADX_pinout.md`](../../docs/en/ADX_pinout.md) も同様に v2 へ更新。
- MCU詳細リファレンス [`adx_attiny1616_mcu_matrix.md`](../../docs/memo/adx_attiny1616_mcu_matrix.md) を正式ドキュメントディレクトリへ配置。

---

## 4. `README.md` 掲載用ドラフト案 (Draft Proposals)

### 4.1 `docs/ja/README.md` 向け差し替え原稿案

```markdown
### 3.2 ADX Pinout（共通 20P 拡張インターフェース / v2）
マスター／スレーブおよび各種ドータボード間で共通利用可能な 20ピン IDC コネクタ規格です。
電源供給能力の強化（5V / 2A級）、フラットケーブル圧接時の短絡防止、EXTCLK およびアナログゾーンの完全両側 GND シールド構造を採用しています。
詳細は [ADX_pinout.md](ADX_pinout.md) を参照してください。

**コネクタ仕様**: 2×10ピン 2.54mmピッチ IDC圧接コネクタ（IDC Pin 1〜20）

| IDC ピン | ネット名 | マイコンピン | 主な機能 / 役割 | フラットケーブル構造・シールド機能 |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **VDD** | - | **電源 (5V/2A供給対応)** | #1 (電源ゾーン) |
| **2** | **VDD** | - | **電源 (5V/2A供給対応)** | #2 (電源ゾーン) |
| **3** | **VDD** | - | **電源増強 (5V/2A供給対応)** | #3 (電源ゾーン) |
| **4** | *N.C.* | - | **アイソレーション (電源・GND短絡防止緩衝帯)** | #4 (緩衝帯) |
| **5** | **GND_5V** | - | **GND (EXTCLKガード①)** | #5 (GNDシールド) |
| **6** | **PA3/EXTCLK** | PA3 | **EXTCLK** / AIN3 | #6 (EXTCLK 信号線) |
| **7** | **GND_5V** | - | **GND (EXTCLKガード②)** | #7 (GNDシールド) |
| **8** | PA6/DAC0 | PA6 | **DAC0** 出力 / AIN6 | #8 (アナログゾーン) |
| **9** | PA5/AIN5 | PA5 | **VREFA** / AIN5 | #9 (アナログゾーン) |
| **10** | **GND_5V** | - | **GND (アナログ/デジタル離隔ガード - 新設)** | #10 (GNDシールド) |
| **11** | **PB2/TXD_EXT** | PB2 | **UART TxD (Alternate)** | #11 (UARTペア) |
| **12** | **PB3/RXD_EXT** | PB3 | **UART RxD (Alternate)** | #12 (UARTペア) |
| **13** | PB0/SCL | PB0 | **I2C SCL** | #13 (I2Cペア) |
| **14** | PB1/SDA | PB1 | **I2C SDA** | #14 (I2Cペア) |
| **15** | GND_5V | - | **GND (中継シールド)** | #15 |
| **16** | PC3/SS | PC3 | **SPI SS** | #16 (SPI群) |
| **17** | PC2/MOSI | PC2 | **SPI MOSI** | #17 (SPI群) |
| **18** | PC1/MISO | PC1 | **SPI MISO** | #18 (SPI群) |
| **19** | PC0/SCK | PC0 | **SPI SCK** | #19 (SPI群) |
| **20** | GND_5V | - | **GND (終端シールド)** | #20 |

> ※ 全ピンのPWM出力、ADC、PORTMUXレジスタ仕様の詳細は [adx_attiny1616_mcu_matrix.md](adx_attiny1616_mcu_matrix.md) を参照してください。
```

---

## 5. 作業ロードマップ (Execution Roadmap)

- **Step 1: 計画書の確定・承認** (`memo/PLAN/PINOUT_V2_DOCS_REVISION_PLAN.md`) ★現在地
- **Step 2: 各言語 README の改訂**
  - `docs/ja/README.md` の改訂
  - ルート `README.md` の改訂
  - `docs/en/README.md` の改訂
- **Step 3: 正式仕様書への反映**
  - `docs/ja/ADX_pinout.md` の更新
  - `docs/en/ADX_pinout.md` の更新
  - `adx_attiny1616_mcu_matrix.md` の本配備
- **Step 4: リンク・整合性検証**
  - 全ドキュメント間の相対パスリンク、表記ゆれの最終確認
