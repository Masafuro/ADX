<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CERN-OHL-P-2.0
-->

# ADX Core-D

## 主な目的
ADXを開発していくにあたり、LIN Based RS-485（LN-485）とそのブートローダーを開発するための基板。最終的には、USBやオシレーターはメイン基板からオミットされるが、LN-485ブートローダの開発では、様々な周辺ハードウェア機能を必要とするために、ブートローダ開発用基板として回路を起こした。

## 主な機能

### 1. マイコン部（MCU）
- **Microchip ATtiny1616-MNR** (QFN-20) 搭載
  - 8-bit AVR マイコン（Flash 16KB, SRAM 2KB, EEPROM 128B）
  - 5V 動作、最大 20MHz 駆動

### 2. LN-485 (LIN-based RS-485) 通信機能
- **MaxLinear SP485EEN-L/TR**（半二重 RS-485 トランシーバー）搭載
- **接続コネクタ**: 5.08mm ピッチ 3P 端子台（KF142R-5.08-3P: A, B, GND）
- **保護回路**: RS-485 バス保護用 TVS ダイオード（PSM712-LF-T7）
- **フェイルセーフバイアス**: A-VDD プルアップ（4.7kΩ） / B-GND プルダウン（4.7kΩ）
- **終端抵抗切り替え**: 2×3P ジャンパピン（H4）による 100Ω 終端抵抗の有効/無効切り替え
- **ドライバ/レシーバ制御**: 1×3P ジャンパピン（H2）による DE / RE_ 端子制御（独立制御 / 連動制御の切り替え）

### 3. USB インターフェース & デバッグ・書き込み機能
- **USB Type-C コネクタ**（TYPE-C 16PIN）による 5V 電源供給および通信
- **WCH CH342K**（デュアル USB-UART ブリッジ）搭載
  - **SerialUPDI 書き込み回路**: USB 経由で ATtiny1616 へのファームウェア書き込み（UPDI プログラミング）に対応
  - **UART シリアル通信**: MCU（PB4/PB5）とのシリアル通信ポートを確保（デバッグログ・モニタ用）
- **ESD 保護**: USB データラインおよび VBUS 保護用ダイオード（SRV05-4-ES）

### 4. 外部高精度クロック源
- **12MHz アクティブ水晶発振器（オシレータ）**（TFOM12M4RHKCNT2T）搭載
- **クロック入力選択**: 1×3P ジャンパピン（H3）により、外部クロックを MCU の PA3 (EXTCLK) に供給可能

### 5. 電源および電源保護回路
- USB VBUS（5V）入力
- **過電流保護**: 1A リセッタブル PTC ヒューズ（SMD0805-100-12）
- **ノイズ対策**: 電源ライン用フェライトビーズ（BLM18PG121SN1D: 120Ω@100MHz）および各 IC へのデカップリングコンデンサ配置

### 6. インジケータ LED
- **電源 LED**: 緑色（KT-0805G, VDD 通電表示）
- **ユーザー LED**: 2 系統
  - 赤色 LED（NCD0805R1, PB2 接続）
  - 白色 LED（KT-0805W, PB3 接続）

### 7. 外部拡張コネクタ（I/O ブレイクアウト）
- **2.54mm ピッチ 2×10P ピンヘッダ（CN2）**: MCU の各機能を外部ブレイクアウト
  - **SPI**: SCK (PC0), MISO (PC1), MOSI (PC2), SS (PC3)
  - **I2C**: SCL (PB0), SDA (PB1)
  - **アナログ / GPIO**: DAC0 (PA6), AIN5 (PA5)
  - **UART / RS-485 信号**: D (PA1), R (PA2), DE (PA4), RE (PA7)
  - **外部クロック / その他**: EXC (PA3), VDD, GND

---

## 8. 実機動作検証
初版基板の実機通電および機能検証（SerialUPDI書き込み、SoftwareSerial通信、GPIO制御、12MHz外部オシレーター動作、RS-485半二重通信等）を実施済みです。詳細な検証結果および実機写真については、以下を参照してください。

- **ハードウェア検証レポート & 実機写真**: [`firmware/tests/ADX_Core-D/README.md`](../../firmware/tests/ADX_Core-D/README.md)
- **RS-485 通信テスト詳細**: [`firmware/tests/ADX_Core-D/RS-485_test/README.md`](../../firmware/tests/ADX_Core-D/RS-485_test/README.md)