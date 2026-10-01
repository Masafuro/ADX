<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX (Advanced Devices eXtended) - 日本語ドキュメント

[ English (../en/README.md) | **日本語** ]

**ADX（Advanced Devices eXtended）**は、**「Enclosure-Friendly（筐体適合性）」** をコアフィロソフィーとして掲げる、オープンソースのハードウェア規格およびモジュラー制御プラットフォームです。

一般的なプロトタイピング基板は卓上（ブレッドボード）での取り扱いに優れている一方、実際の筐体や過酷な現場へ組み込もうとした瞬間に、ネジ締結部へのワッシャー干渉、多方向への配線突出、ピンヘッダの剛体スタックによる寸法公差の集積といった物理的・構造的な壁に直面します。

ADXは、**8748 Form Factor**（`87.0 mm × 48.0 mm`）と **IDCリボンケーブルによるフレキシブルな拡張バス**を通じてこれらの課題を解消します。音響・PA機器のように確実かつ直感的に配線できる完全絶縁フィールドネットワーク（ADX CORE-A）と専用アナライザ（RPR4 Smart Probe）、そして多彩な電源・センサー拡張（CARDシリーズ）が、3Dプリントケースや市販防水ボックスに美しく収まるハードウェアエコシステムの新しい可能性を切り拓きます。

---

## 1. 主な設計特徴 (Key Design Features)

* **筐体適合性を極めた寸法設計（8748 Form Factor）**:
  外形 `87.0 mm × 48.0 mm`、取付穴ピッチ `77.0 mm × 38.0 mm`（M3ネジ対応）。四隅に $5.0\text{ mm}$ のゆとりあるマージンを確保し、標準的なネジワッシャーを噛ませても基板パターンを傷つけることなく強固にトルク締め可能。
* **一貫したI/F辺の定義**:
  外部端子台やコネクタの引き出し方向を特定の一辺に集約し、筐体側の開口パネル加工や防滴・防水設計を劇的にシンプル化。
* **IDCリボンケーブルによる「柔結合」**:
  拡張基板（CARD）との接続に20ピンIDCリボンケーブル（ADX Pinout）を採用。剛結合ピンヘッダと異なり、筐体内での寸法公差を柔軟に吸収しつつ、スタック・横並び・折りたたみなど自由なレイアウトを実現。
* **安全な双方向5V電源幹線（ダブル・チェックバルブ方式）**:
  CoreボードはType-C（LM66100理想ダイオード保護）から受電してIDC拡張バスへ5V/2A+を供給することも、Power CARDからIDC拡張バス経由でCoreボードへ5Vを逆受電することも可能。双方向の安全性を担保するため、CORE-A側のPC逆流阻止ダイオードとPower CARD側の逆流阻止回路（MUST要件）による「ダブル・チェックバルブ」で電源衝突や逆流事故を防止。

---

## 2. ハードウェアラインナップ (Hardware Lineup)

ADXエコシステムは、同一の8748フォームファクタと20ピン拡張バスを共有する **Coreボード（メインMCU基板）**、**スマートツール**、および **CARD（機能拡張基板）** で構成されます。

### 2.1 Core シリーズ (メインMCU基板)

| ボード名 | 役割と電源アーキテクチャ | 通信・主要機能 | 用途・ステータス |
| :--- | :--- | :--- | :--- |
| **ADX CORE-A**<br>*(Absolute / 絶対標準機)*<br>**想定売価: $28** | **クリーン5V専任 / 自己発熱ゼロ**<br>USB Type-C 6P（理想ダイオードLM66100によるPC逆流阻止）。20P牛角ヘッダからの双方向5V受給電。 | **完全絶縁 RS-485 (2500V耐圧)**<br>Mornsun TDA51S485HC搭載。前面スプリング端子台直結。車載AEC-Q102 LED（-40℃〜+110℃）。BMC（ATtiny412）常時監視。 | 机上開発・室内IoTから、夏の車内、完全密閉防水ケース、工場配電盤、-48V通信インフラまで網羅。<br>*(rev1 ネットリスト・見積FIX、試作準備中)* |
| *ADX Core-D*<br>*(R&D 実証ボード)* | *DC 5V USB / 端子台* | *LN-485プロトコルおよびSerialUPDIブートローダの実機検証用ボード（Phase 1〜5検証完了）。* | *内部開発・実験用（製品ラインナップ外）* |

### 2.2 スマートツール (アナライザ ＆ プログラマ)

| ツール名 | 役割と構成 | 主な機能・特徴 | 想定売価・ステータス |
| :--- | :--- | :--- | :--- |
| **RPR4 Smart Probe** | **完全絶縁 RS-485 ポケットアナライザ**<br>Raspberry Pi RP2040 ＋ 絶縁RS-485（TDA51S485HC）。 | **RP2040 PIO による超高速解析**<br>Auto-Baud自動検出、Modbus RTU / DMX / LN-485自動デコード、専用PWAロジアナ、スマホからのワンタップOTW書き換え。 | **$25.00**<br>*(rev1 ネットリストFIX、開発中)* |

### 2.3 CARD シリーズ (拡張基板)

| ボード名 | 概要・電源トポロジー | 状態・リソース |
| :--- | :--- | :--- |
| **AAA 2S Power CARD** | 単4アルカリ電池×2本から低暗電流昇圧DCDCでクリーンな5Vを生成。BMC連動スリープで待機電流ナノアンペア化。 | 設計検討中 |
| **DC 12V〜48V Power CARD** | 工場配電盤（24V）、車載（12V/24V）、通信基地局（-48V）等の広入力をトランス絶縁5V変換。サージ保護集約。 | 設計検討中 |
| **ADX Prototyping CARD** | 8748フォームファクタ準拠、20P Eject Header引き出しおよびユニバーサル試作エリアを備えた拡張基板。 | [仕様書](../../hardware/CARD/Prototyping/proposal.md) / [設計データ](../../hardware/CARD/Prototyping/data/) |

---

## 3. コア仕様 (Specifications)

### 3.1 8748 Form Factor（基板外形規格）
基板設計（mil単位）と筐体・メカ設計（mm単位）の寸法整合性を高め、製造公差に配慮した小型フォームファクタ規格です。
詳細は [8748_formfactor.md](8748_formfactor.md) を参照してください。

* **基板外形寸法**: `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **取付穴ピッチ**: `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`)
* **穴マージン / 取付穴径**: 四隅から `5.0 mm` (`197 mil`) / `Φ3.3 mm` (M3ネジ対応 / `130 mil`)
* **角部加工**: C3 面取り (`120 mil`)

### 3.2 ADX Pinout（共通 20P 拡張インターフェース / v2）
マスター／スレーブおよび各種ドータボード間で共通利用可能な 20ピン IDC コネクタ規格です。
電源供給能力の強化（5V / 2A級）、フラットケーブル圧接時の短絡防止、EXTCLK およびアナログゾーンの完全両側 GND シールド構造を採用しています。
詳細は [ADX_pinout.md](ADX_pinout.md) を参照してください。

**コネクタ仕様**: 2×10ピン 2.54mmピッチ IDC圧接コネクタ（IDC Pin 1〜20）

| IDC ピン | ネット名 | マイコンピン | 主な機能 / 役割 | フラットケーブル構造・シールド機能 |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **VDD** | - | **電源 (5V/2A供給対応)** | #1 (電源ゾーン: 2×2ブロック) |
| **2** | **VDD** | - | **電源 (5V/2A供給対応)** | #2 (電源ゾーン: 2×2ブロック) |
| **3** | **VDD** | - | **電源 (5V/2A供給対応)** | #3 (電源ゾーン: 2×2ブロック) |
| **4** | **VDD** | - | **電源増強 (5V/2A供給対応)** | #4 (電源ゾーン: 2×2ブロック) |
| **5** | *N.C.* | - | **アイソレーション (電源・信号短絡防止緩衝帯)** | #5 (緩衝帯) |
| **6** | PA6/DAC0 | PA6 | **DAC0** 出力 / AIN6 | #6 (アナログゾーン) |
| **7** | PA5/AIN5 | PA5 | **VREFA** / AIN5 | #7 (アナログゾーン) |
| **8** | **GND_5V** | - | **GND (EXTCLKガード①)** | #8 (GNDシールド) |
| **9** | **PA3/EXTCLK** | PA3 | **EXTCLK** / AIN3 | #9 (EXTCLK 信号線) |
| **10** | **GND_5V** | - | **GND (EXTCLKガード② / デジタル通信分離)** | #10 (GNDシールド) |
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

### 3.3 ハードウェア命名規則・グレード分類基準
ADXエコシステムにおけるハードウェア型番の命名規則、および「一般・製品グレード」と「プロ専用・自己責任DIYグレード（`-P`）」の安全判定基準を定めています。
詳細は [hardware_naming_rules.md](hardware_naming_rules.md) を参照してください。

### 3.4 ADX 開発ガイドライン (MUST / SHOULD / MAY)
サードパーティやDIY開発者が安全に互換カードを設計するための包括的な設計指針です。
RFC 2119に基づく3段階の要件定義により、オープンソースとしての自由度と産業機器としての安全性を両立します。
詳細は [adx_formfactor_guidelines.md](adx_formfactor_guidelines.md) を参照してください。

* **【MUST】必須要件**: Power CARDの逆流阻止義務、Pin 5アイソレーション帯の厳守。
* **【SHOULD】公式標準**: ラッチ付き Eject Header、産業用トランス絶縁DCDC、8748寸法、-40℃〜+105℃耐熱。
* **【MAY】自由選択**: 非絶縁DCDC、ボックスヘッダ、自由な基板形状、独自センサーの実装。

> ※ 全ピンのPWM出力、ADC、PORTMUXレジスタ仕様の詳細は [adx_attiny1616_mcu_matrix.md](adx_attiny1616_mcu_matrix.md) を参照してください。

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
│   ├── CORE-A/             # ADX CORE-A (Absolute/完全絶縁RS-485/5V専任)
│   ├── ADX_Core-D/         # LN-485 ブートローダ開発基板 (R&D検証用)
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

