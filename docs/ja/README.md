<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended) - 日本語ドキュメント

[ English (../en/README.md) | **日本語** ]

**ADX（Advanced Devices eXtended）**は、筐体適合性と現場据付性を重視したオープンソースのハードウェア規格およびモジュラー制御プラットフォームです。

一般的なプロトタイピング基板は卓上での取り扱いに優れている一方、実際の筐体へ組み込む際には、ネジ締結クリアランスの不足、多方向への配線突出、ピンヘッダ直接スタックによる公差集積といった構造的課題が生じやすい側面があります。ADXは、**8748 Form Factor** と **IDCリボンケーブルによるフレキシブルな拡張バス**を通じてこれらの課題を解決し、Arduino等の親しみやすい開発体験を維持しながら、家庭用3Dプリンタ製ケース、防水ボックス、電工ボックス、スロットインラック等へのスムーズな実装を実現します。

---

## 1. 主な設計特徴 (Key Design Features)

* **ワッシャー対応の確実なM3ネジ締結**:
  取付穴（$\Phi 3.3\text{ mm}$）の周囲に $5.0\text{ mm}$ のマージンを確保。標準的なワッシャーを併用でき、基板パターンを痛めることなく適切なトルクで確実に固定可能。
* **一貫したI/F辺の定義**:
  外部配線や端子台の引き出し方向を特定の一辺に集約し、筐体側の開口加工や防水・防塵設計を簡素化。
* **IDCリボンケーブルによる「柔結合」**:
  拡張基板との接続に20ピンIDCリボンケーブル（ADX Pinout）を採用。剛結合スタックと異なり、寸法公差を吸収しつつ、筐体内での配置自由度（スタック、横並び、L字等）を確保し、デイジーチェーン接続に対応。
* **多様な筐体への適合性**:
  市販の防水ボックス（タカチ等）や電工ボックスへの取付、家庭用3Dプリンタでのケース作製、専用ラックへのスロットイン構造に柔軟に対応。

---

## 2. ハードウェアラインナップ (Hardware Lineup)

ADXエコシステムは、共通の8748フォームファクタと20ピン拡張バスを共有する **Coreボード（メインMCU基板）** と **CARD（機能拡張基板）** で構成されます。

### 2.1 Core シリーズ (MCU基板)

| ボード名 | 概要 | 状態・リソース |
| :--- | :--- | :--- |
| **ADX CORE-U** | Arduino Unoに近い扱いやすさを持ち、そのまま筐体に組み込める汎用・標準プロトタイピングボード。 | 開発中 |
| **ADX Core-D** | LN-485（LIN-based RS-485）トランシーバを搭載し、高耐ノイズ・長距離通信環境に適した産業通信ボード。 | [仕様書](../../hardware/ADX_Core-D/proposal.md) / [設計データ](../../hardware/ADX_Core-D/data/) |

### 2.2 CARD シリーズ (拡張基板)

| ボード名 | 概要 | 状態・リソース |
| :--- | :--- | :--- |
| **ADX Prototyping CARD** | 8748フォームファクタ準拠、下列20ピンADXバス引き出しおよび上列20ピンユニバーサル試作エリアを備えた拡張基板。 | [仕様書](../../hardware/CARD/Prototyping/proposal.md) / [設計データ](../../hardware/CARD/Prototyping/data/) |

---

## 3. コア仕様 (Specifications)

### 3.1 8748 Form Factor（基板外形規格）
基板設計（mil単位）と筐体・メカ設計（mm単位）の寸法整合性を高め、製造公差に配慮した小型フォームファクタ規格です。
詳細は [8748_formfactor.md](8748_formfactor.md) を参照してください。

* **基板外形寸法**: `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **取付穴ピッチ**: `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`)
* **穴マージン / 取付穴径**: 四隅から `5.0 mm` (`197 mil`) / `Φ3.3 mm` (M3ネジ対応 / `130 mil`)
* **角部加工**: C3 面取り (`120 mil`)

### 3.2 ADX Pinout（共通 20P 拡張インターフェース）
マスター／スレーブ共通の20ピン拡張コネクタ規格です。クロック信号へのGNDシールド配置や、制御ピン配置による信号間クロストーク低減を考慮したピン配置を採用しています。
詳細は [ADX_pinout.md](ADX_pinout.md) を参照してください。

**コネクタ仕様**: 2×10ピン 2.54mmピッチ ピンヘッダ (CN2)

| Pin | ネット名 | GPIO | アナログ / 特殊機能 | 通信 / 制御 | 備考 |
|:---:|:---|:---:|:---|:---|:---|
| **1** | *N.C.* | - | - | - | 未接続 |
| **2** | `VDD` | - | - | - | 電源 (5V) |
| **3** | `VDD` | - | - | - | 電源 (5V) |
| **4** | *N.C.* | - | - | - | 未接続 |
| **5** | `PA3/EXTCLK` | PA3 | AIN3 / PTC | **EXTCLK (外部クロック)** | ※1 |
| **6** | `GND_5V` | - | - | - | GND（EXTCLK シールド） |
| **7** | `PA6/DAC0` | PA6 | AIN6 / PTC / **DAC0出力** | - | TCD0 WOC / AC入力 |
| **8** | `PA5/AIN5` | PA5 | AIN5 / PTC / **VREFA** | - | TCA0 WO5 / AC0出力 |
| **9** | `PA7/RE` | PA7 | - | UART `\RE` 制御 | アナログ信号への干渉低減 (※2) |
| **10** | `PA4/DE` | PA4 | - | UART `XDIR (DE)` | I2C信号への干渉低減 (※3) |
| **11** | `PB0/SCL` | PB0 | - | **I2C SCL** | - |
| **12** | `PB1/SDA` | PB1 | - | **I2C SDA** | - |
| **13** | `GND_5V` | - | - | - | GND（RxD シールド） |
| **14** | `PA2/R` | PA2 | - | **UART RxD** (受信) | - |
| **15** | `PA1/D` | PA1 | - | **UART TxD** (送信) | - |
| **16** | `PC3/SS` | PC3 | - | **SPI SS** | - |
| **17** | `PC2/MOSI` | PC2 | - | **SPI MOSI** | - |
| **18** | `PC1/MISO` | PC1 | - | **SPI MISO** | - |
| **19** | `PC0/SCK` | PC0 | - | **SPI SCK** | - |
| **20** | `GND_5V` | - | - | - | GND（SCK シールド） |

*※1: マスター時は外部クロック入力、スレーブ時は AIN3 (アナログ入力) として利用可能。*  
*※2: 通信切替時以外は論理固定となる制御ピンを配置し、隣接するアナログ入力（AIN5）へのクロストークを低減。*  
*※3: 送信制御ピンを配置し、隣接するI2Cクロックライン（SCL）への影響を低減。*

---

## 4. ディレクトリ構成 (Repository Structure)

```text
ADX/
├── README.md               # ポータル（英語概要・言語ナビゲーション）
├── LICENSE.md              # ライセンス方針・商標規定
├── LICENSES/               # REUSE準拠ライセンス正式条文 (CC-BY-4.0, CERN-OHL-P-2.0, MIT)
├── docs/                   # 仕様・ドキュメント（多言語、CC BY 4.0）
│   ├── en/                 # 英語版ドキュメント
│   └── ja/                 # 日本語版ドキュメント
├── hardware/               # ハードウェア設計データ (CERN-OHL-P-v2)
│   ├── ADX_Core-D/         # LN-485 ブートローダ開発基板
│   └── CARD/Prototyping/   # プロトタイピング拡張カード
├── firmware/               # ファームウェア・ドライバ・サンプル (MIT)
├── logo/                   # ブランドロゴ・アセット
├── memo/                   # 開発メモ・計画書
│   ├── what_is_adx.md      # コア思想・背景
│   ├── PLAN/               # 計画書
│   └── REPORT/             # 技術レポート・提案書
└── Project_Snapshot.md     # プロジェクト進捗・概要
```

---

## 5. 関連リンク (Links & Resources)

* **Project Snapshot**: [../../Project_Snapshot.md](../../Project_Snapshot.md)
* **ADX Platform**: [https://adxplatform.com/](https://adxplatform.com/)
* **Developer Blog**: [https://dev-blog.adxplatform.com/](https://dev-blog.adxplatform.com/)

---

## 6. ライセンス (License)

ADXプロジェクトは、用途に応じた3層マルチライセンス構成を採用しています：

* **仕様書・ドキュメント** (`docs/`, `memo/`): [CC BY 4.0](../../LICENSES/CC-BY-4.0.txt) (`CC-BY-4.0`)
* **ハードウェア設計データ** (`hardware/`): [CERN-OHL-P-v2](../../LICENSES/CERN-OHL-P-2.0.txt) (`CERN-OHL-P-2.0`)
* **ファームウェア・コード** (`firmware/`): [MIT License](../../LICENSES/MIT.txt) (`MIT`)

詳細は [../../LICENSE.md](../../LICENSE.md) を参照してください。

