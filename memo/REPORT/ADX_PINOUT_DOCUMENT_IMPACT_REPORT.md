<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout 改訂影響調査・ドキュメント整理レポート
(ADX Pinout Revision Impact & Documentation Reorganization Report)

**文書ID**: REPORT-ADX-PINOUT-REV-001  
**作成日**: 2026-10-16  
**ステータス**: レビュー・作業計画協議用  

---

## 1. 背景と改訂概要 (Background & Key Changes)

ADX CORE-A5 の策定およびエコシステム全体の設計見直しに伴い、ADX 共通拡張バス（20ピン IDC コネクタ）の仕様について以下の重要な改訂が行われました。

### 1.1 主要な変更点

1. **幹線電源ネットの呼称および電圧仕様の変更:**
   - **旧仕様:** `VDD`（5V / 2A 供給対応）
   - **新仕様:** **`VBUS`**（約 4.5V 〜 5.0V / 2A 級給受電対応）
   - **背景:** CORE-A5 の前面 5VIN 給電時、逆接保護・過電流保護スイッチ（SY6280AAC）等による順方向降下（約 0.2V〜0.5V）が生じるため、実効電圧は約 4.5V〜4.8V となります。また、「入力専用（VIN）」ではなく「COREからCARDへの給電、およびPOWER CARDからCOREへの受電」という双方向の大動脈ラインであることを明確化するため、呼称を **`VBUS`** に統一します。
2. **GPIO 論理レベルの 3.3V ネイティブ統一:**
   - **旧仕様:** 5V GPIO / 5V LVCMOS（一部 3.3V トレラントやオンボードシフタ混在）
   - **新仕様:** **3.3V LVCMOS ネイティブ**
   - **背景:** CORE-A5 にオンボード 3.3V LDO（ME6211）を搭載し、MCU（ATtiny1616）を 3.3V / 10MHz で定格駆動させることになりました。これにより、CORE-R（RP2040）や将来の ESP32 系 CORE、そして拡張 CARD 群のすべてが **レベルシフタなしの 3.3V ネイティブ直結** で統一されます。
3. **GND ネット呼称の汎用化:**
   - **旧仕様:** `GND_5V`
   - **新仕様:** **`GND`**（5V マイコン前提の名称から、規格共通のシンプルな `GND` へ）

---

## 2. 影響を受けるドキュメント一覧 (Impacted Document Matrix)

リポジトリ全体を調査した結果、改訂の影響を受けるドキュメントを以下の4カテゴリに分類しました。

| カテゴリ | 重要度 | ドキュメント名 | パス | 主な影響箇所・改訂要件 |
| :--- | :---: | :--- | :--- | :--- |
| **A. 規格正式仕様書** | **最優先** | **ADX Pinout 仕様書 (日本語)** | [`docs/ja/ADX_pinout.md`](../../docs/ja/ADX_pinout.md) | ・ピンアサイン表（VDD ➜ VBUS, GND_5V ➜ GND）<br>・5V/2A ➜ VBUS (4.5〜5.0V / 2A級)<br>・GPIO 3.3V LVCMOS ネイティブ規定 |
| | **最優先** | **ADX Pinout Specification (英語)** | [`docs/en/ADX_pinout.md`](../../docs/en/ADX_pinout.md) | ・日本語版と 1:1 対応の英文改訂 |
| | **高** | **MCUペリフェラル統合リファレンス (日)** | [`docs/ja/adx_attiny1616_mcu_matrix.md`](../../docs/ja/adx_attiny1616_mcu_matrix.md) | ・VDD_5V / GND_5V 記述の更新<br>・ATtiny1616 の 3.3V / 10MHz 駆動前提への補足 |
| | **高** | **MCU Peripheral Reference (英)** | [`docs/en/adx_attiny1616_mcu_matrix.md`](../../docs/en/adx_attiny1616_mcu_matrix.md) | ・日本語版と 1:1 対応の英文改訂 |
| | **中** | **Pinout 仕様書ドラフト (旧作業メモ)** | [`docs/memo/adx_pinout_v2.md`](../../docs/memo/adx_pinout_v2.md) | ・アーカイブ化または最新同期 |
| **B. ポータル・概要** | **高** | **ハードウェア総合 README** | [`hardware/README.md`](../../hardware/README.md) | ・1.1 表内の「5V給電」表記 ➜ 「VBUS / 3.3V ロジック」<br>・リンクおよび規格概要の整合 |
| | **中** | **ルート英語ポータル** | [`README.md`](../../README.md) | ・3.2 ADX Pinout セクションの概要文更新 |
| | **中** | **日本語ポータル** | [`docs/ja/README.md`](../../docs/ja/README.md) | ・2.3 節の 20ピンバス概要文更新 |
| | **中** | **英語ポータル** | [`docs/en/README.md`](../../docs/en/README.md) | ・日本語ポータルと 1:1 対応の英文改訂 |
| | **中** | **Project Snapshot** | [`Project_Snapshot.md`](../../Project_Snapshot.md) | ・電源幹線および GPIO 電圧記述の更新 |
| **C. 各ハードウェア仕様**| **高** | **CORE-A5 構想書 ＆ 技術仕様検討書** | [`hardware/CORE-A5/concept.md`](../../hardware/CORE-A5/concept.md)<br>[`hardware/CORE-A5/spec_study.md`](../../hardware/CORE-A5/spec_study.md) | ・**改訂完了済み**（VBUS, ネイティブ 3.3V GPIO, 10MHz 駆動反映済） |
| | **中** | **CORE-R 技術仕様書** | [`hardware/CORE-R/spec.md`](../../hardware/CORE-R/spec.md) | ・「ADX Pinout v3 準拠 / 3.3V ネイティブ」の記述整合（既に 3.3V 寄りだが VBUS 呼称等を確認） |
| | **中** | **Prototyping CARD 仕様書** | [`hardware/CARD/Prototyping/proposal.md`](../../hardware/CARD/Prototyping/proposal.md) | ・引き出しピンネット名（VDD ➜ VBUS, GND_5V ➜ GND）の整合 |
| | **低** | **CORE-A (rev2) 構想書** | [`hardware/CORE-A/concept.md`](../../hardware/CORE-A/concept.md) | ・歴史的経緯（5V時代）としての位置づけ注記 |
| **D. 過去計画・レポート**| **低** | **前回の改訂計画書** | [`memo/PLAN/PINOUT_V2_DOCS_REVISION_PLAN.md`](../PLAN/PINOUT_V2_DOCS_REVISION_PLAN.md) | ・過去ログとして保管（改訂不要） |

---

## 3. 具体的な改訂方針とピンアサイン表の変更案 (Pinout Revision Detail)

### 3.1 IDC 2x10 コネクタ ピン配置表（改訂案）

`docs/ja/ADX_pinout.md` および `docs/en/ADX_pinout.md` のピンアサイン表を以下のように更新します。

```markdown
| IDC ピン | ネット名 | マイコンピン (CORE-A5例) | 主な機能 / 役割 | フラットケーブル構造・シールド機能 |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **VBUS** | - | **システム幹線電源 (4.5V〜5.0V / 2A級給受電)** | #1 (電源ゾーン: 2×2ブロック) |
| **2** | **VBUS** | - | **システム幹線電源 (4.5V〜5.0V / 2A級給受電)** | #2 (電源ゾーン: 2×2ブロック) |
| **3** | **VBUS** | - | **システム幹線電源 (4.5V〜5.0V / 2A級給受電)** | #3 (電源ゾーン: 2×2ブロック) |
| **4** | **VBUS** | - | **システム幹線電源増強 (4.5V〜5.0V / 2A級給受電)** | #4 (電源ゾーン: 2×2ブロック) |
| **5** | *N.C.* | - | **アイソレーション (電源・信号短絡防止緩衝帯)** | #5 (緩衝帯) |
| **6** | PA6/DAC0 | PA6 | **DAC0** 出力 / AIN6 (3.3V ロジック) | #6 (アナログゾーン) |
| **7** | PA5/AIN5 | PA5 | **VREFA** / AIN5 (3.3V ロジック) | #7 (アナログゾーン) |
| **8** | **GND** | - | **GND (EXTCLKガード①)** | #8 (GNDシールド) |
| **9** | **PA3/EXTCLK** | PA3 | **EXTCLK** / AIN3 (3.3V ロジック) | #9 (EXTCLK 信号線) |
| **10** | **GND** | - | **GND (EXTCLKガード② / デジタル通信分離)** | #10 (GNDシールド) |
| **11** | **PB2/TXD_EXT** | PB2 | **UART TxD (Alternate)** (3.3V LVCMOS) | #11 (UARTペア) |
| **12** | **PB3/RXD_EXT** | PB3 | **UART RxD (Alternate)** (3.3V LVCMOS) | #12 (UARTペア) |
| **13** | PB0/SCL | PB0 | **I2C SCL** (3.3V LVCMOS) | #13 (I2Cペア) |
| **14** | PB1/SDA | PB1 | **I2C SDA** (3.3V LVCMOS) | #14 (I2Cペア) |
| **15** | **GND** | - | **GND (中継シールド)** | #15 |
| **16** | PC3/SS | PC3 | **SPI SS** (3.3V LVCMOS) | #16 (SPI群) |
| **17** | PC2/MOSI | PC2 | **SPI MOSI** (3.3V LVCMOS) | #17 (SPI群) |
| **18** | PC1/MISO | PC1 | **SPI MISO** (3.3V LVCMOS) | #18 (SPI群) |
| **19** | PC0/SCK | PC0 | **SPI SCK** (3.3V LVCMOS) | #19 (SPI群) |
| **20** | **GND** | - | **GND (終端シールド)** | #20 |
```

### 3.2 設計メリット解説文の修正箇所
`docs/ja/ADX_pinout.md` の「主な電気的・構造的メリット」の解説文を更新：
- **項目1 (電源供給):**  
  「4ピンのVDDにより、5V / 2A 超の安定給電能力を確保」  
  ➜ **「4ピンの VBUS により、4.5V〜5.0V / 2A 級の双方向大動脈給受電能力を確保。CORE前面端子からの給電およびPOWER CARDからの受電の両方に対応」**
- **項目2 (短絡防止):**  
  「VDD電源ゾーン」 ➜ **「VBUS電源ゾーン」**
- **論理レベルの明確化（新設節）:**  
  すべての GPIO ピン（Pin 6〜19）は **3.3V LVCMOS ネイティブ** であり、相手側 CARD は 3.3V 前提で設計可能であることを明記。

---

## 4. ドキュメント整理・改訂ロードマップ (Execution Roadmap)

混乱を避け、一貫性を保ちながらドキュメントを同期させるための推奨改訂ステップです。

```text
  【 Step 1: 正式規格仕様書の改訂 】(最優先)
    ├─ docs/ja/ADX_pinout.md (ピン表・VBUS・3.3Vロジック規定)
    ├─ docs/en/ADX_pinout.md (同英語版)
    ├─ docs/ja/adx_attiny1616_mcu_matrix.md (ネット名・3.3V/10MHz補足)
    └─ docs/en/adx_attiny1616_mcu_matrix.md (同英語版)
         │
         ▼
  【 Step 2: ポータル・総合ドキュメントの更新 】
    ├─ hardware/README.md (ハードウェア総合案内)
    ├─ docs/ja/README.md ＆ docs/en/README.md (言語別ポータル)
    ├─ README.md (ルート英語ポータル)
    └─ Project_Snapshot.md (プロジェクトスナップショット)
         │
         ▼
  【 Step 3: 各ハードウェア仕様書の整合性確認 】
    ├─ hardware/CARD/Prototyping/proposal.md (バス引き出しピン名)
    └─ hardware/CORE-R/spec.md (Pinout v3/VBUS整合)
```

---

## 5. まとめ ＆ 次のアクション

本レポートにより、改訂の影響範囲と修正すべき具体的内容が完全に特定されました。
この方針で合意いただけましたら、**Step 1（正式規格仕様書 `docs/ja/ADX_pinout.md` および `docs/en/ADX_pinout.md` の改訂）** から順次作業を開始できます。
