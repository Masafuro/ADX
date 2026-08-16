# ADX (Advanced Devices eXtended) - 日本語ドキュメント

[ English (../en/README.md) | **日本語** ]

ADX（Advanced Devices eXtended）は、オープンソースハードウェアの扱いやすさと、産業用途に適した信頼性・耐ノイズ性・保守性を両立することを目指した小型制御プラットフォーム・ハードウェア規格です。

---

## 1. コア仕様 (Specifications)

### 1.1 8748 Form Factor（基板外形規格）
基板設計（mil単位）と筐体・メカ設計（mm単位）の寸法整合性を高め、製造公差に配慮した小型フォームファクタ規格です。
詳細は [8748_formfactor.md](8748_formfactor.md) を参照してください。

* **基板外形寸法**: `87.0 mm × 48.0 mm` (`3425 mil × 1890 mil`)
* **取付穴ピッチ**: `77.0 mm × 38.0 mm` (`3031 mil × 1496 mil`)
* **穴マージン / 取付穴径**: 四隅から `5.0 mm` (`197 mil`) / `Φ3.3 mm` (M3ネジ対応 / `130 mil`)
* **角部加工**: C3 面取り (`120 mil`)

### 1.2 ADX Pinout（共通 20P 拡張インターフェース）
マスター／スレーブ共通の20ピン拡張コネクタ規格です。クロック信号へのGNDシールド配置や、制御ピンを配置することによる信号間クロストークの低減を考慮したピン配置を採用しています。
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

## 2. 開発ボード (Hardware Lineup)

### 2.1 ADX Core-D (LN-485 & ブートローダ開発基板)
LIN-based RS-485（LN-485）通信およびブートローダ開発を目的とした開発ボードです。

* **MCU**: Microchip ATtiny1616-MNR (QFN-20, 5V, 20MHz)
* **通信 (LN-485)**: SP485EEN-L/TR 搭載、5.08mm 3P端子台 (KF142R-5.08-3P)、TVSダイオード保護 (PSM712)、終端抵抗切替ジャンパ
* **USB / デバッグ**: USB Type-C、WCH CH342K（SerialUPDI 書き込み & UART シリアルモニタ対応）
* **クロック**: 12MHz 水晶発振器搭載（EXTCLK 供給ジャンパ切替対応）
* **保護回路**: 1A PTC リセッタブルヒューズ、フェライトビーズ、ESD保護ダイオード
* **詳細仕様**: [../../dev/ADX_Core-D/proposal.md](../../dev/ADX_Core-D/proposal.md)
* **設計・製造データ**: [../../dev/ADX_Core-D/data/](../../dev/ADX_Core-D/data/)

### 2.2 ADX Prototyping CARD (拡張プロトタイピングカード)
ADX規格コネクタに接続し、周辺回路やセンサの試作・検証を行うための拡張基板です。

* **インターフェース**:
  - 下列 20ピン: ADX規格 GPIO / バス信号引き出し
  - 上列 20ピン: ユニバーサルエリア配線用カスタムピン
* **外形**: 8748 Form Factor 準拠
* **詳細仕様**: [../../dev/CARD/Prototyping/proposal.md](../../dev/CARD/Prototyping/proposal.md)
* **設計・製造データ**: [../../dev/CARD/Prototyping/data/](../../dev/CARD/Prototyping/data/)

---

## 3. ディレクトリ構成 (Repository Structure)

```text
ADX/
├── README.md               # ポータル（英語概要・言語ナビゲーション）
├── docs/                   # 仕様・ドキュメント（多言語）
│   ├── en/                 # 英語版ドキュメント
│   │   ├── README.md
│   │   ├── 8748_formfactor.md
│   │   └── ADX_pinout.md
│   └── ja/                 # 日本語版ドキュメント
│       ├── README.md
│       ├── 8748_formfactor.md
│       └── ADX_pinout.md
├── dev/                    # ハードウェア設計データ
│   ├── ADX_Core-D/         # LN-485 ブートローダ開発基板
│   └── CARD/Prototyping/   # プロトタイピング拡張カード
├── memo/                   # 開発メモ・作業レポート
│   └── REPORT/             # 提案・計画書
└── Project_Snapshot.md     # プロジェクト進捗・概要
```

---

## 4. 関連リンク (Links & Resources)

* **Project Snapshot**: [../../Project_Snapshot.md](../../Project_Snapshot.md)
* **ADX Platform**: [https://adxplatform.com/](https://adxplatform.com/)
* **Developer Blog**: [https://dev-blog.adxplatform.com/](https://dev-blog.adxplatform.com/)
