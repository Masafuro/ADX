<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-UA5 技術仕様案 (Hardware & Toolchain Specification Proposal)
〜 Arduino IDE 1.8系 × megaTinyCore 完全対応・デュアルType-C・筐体フレンドリー標準USBコアボード 〜

---

## 1. 概要とプロダクト位置づけ (Overview & Positioning)

### 1.1 プロダクト概要
**ADX CORE-UA5** は、ADXエコシステムにおける **「標準デスクトップ開発 兼 現場即応型 USBコアボード」** です。  
幾度もの試作と現場開発（CORE-U、CORE-D、RPR4等）の教訓を経て、**「Arduino UNO R3 / UNO R4 Minima への対抗馬」** として再定義されました。

最大の特徴は、**「筐体フレンドリー（Case-Friendly）」** であることです。  
機器内部や配電盤、カスタムケースに組み込まれた状態でも、一切の物理ボタン操作（BOOTSELやリセットピン差し）を必要とせず、ノートPCからType-Cケーブル1本で安全・確実にスケッチ書き換えとリアルタイムデバッグを実現します。

### 1.2 Arduino UNO R3 / R4 Minima との対比・設計思想

| 比較項目 | **Arduino UNO R3** | **Arduino UNO R4 Minima** | **ADX CORE-UA5** |
| :--- | :--- | :--- | :--- |
| **主 MCU** | ATmega328P (8-bit AVR 16MHz) | RA4M1 (32-bit Cortex-M4 48MHz) | **ATtiny1616-MNR** (8-bit AVR 3.3V 10MHz) |
| **USB インターフェース** | 独立 MCU (ATmega16U2) | MCU 内蔵 USB (ブートローダ依存) | **独立 IC: WCH CH342K** (Dual CDC) |
| **ハング時の書き換え耐性** | ◎ (DTRリセット経由で高確率) | △ (MCU暴走時にUSB見失い/手動リセット要) | **◎◎ (UPDIハードウェア直結・強制書込)** |
| **デバッグ・書き込みポート** | 1ポート (排他制御・モニタ切断要) | 1ポート (排他制御・モニタ切断要) | **完全独立 2ポート (UPDI / Serial)** |
| **電源供給構造** | DCジャック / 5V USB (最大500mA) | DCジャック / Type-C (1ポート) | **デュアル Type-C (Power 5V2A / Com 独立)** |
| **拡張構造** | 緩いピンソケット (耐振性低) | 緩いピンソケット (耐振性低) | **ADX 20P IDC (耐振・産業スタック)** |
| **外形規格** | UNO Form Factor (68.6 × 53.4 mm) | UNO Form Factor (68.6 × 53.4 mm) | **ADX 8748** (87.0 × 48.0 mm) |

> [!IMPORTANT]
> **「スペック競争」から「現場UXと不沈性」へのシフト**  
> 近年のMCUは内蔵USBブートローダ（UF2やCDC）に依存する傾向にありますが、組み込み現場において「スケッチのバグや暴走でUSBスタックが死に、PCから認識されなくなる」ことは致命的です。  
> CORE-UA5は、UNO R3が持っていた「独立USBブリッジによる物理的安定性」をさらに推し進め、**ハードウェアUPDI書き込み＋独立UARTデバッグ** という完全2系統を1つのType-Cコネクタで提供します。

---

## 2. 開発ツールチェーン仕様 (Software & Toolchain)

本ボードは、現場での軽快な動作とデュアルポート運用を最大化するため、**Arduino IDE 1.8.x（クラシック版）** および **megaTinyCore** を公式標準開発環境として策定します。

```text
               【 CORE-UA5 開発・デバッグワークフロー 】

    PC (Arduino IDE 1.8.x / megaTinyCore)
      │
      │ ──[ 1本の Type-C ケーブル (USB-COM) ]──
      ▼
   [ WCH CH342K (Dual USB-UART) ]
      ├─▶ COM Port A (Port 1): SerialUPDI ──────────▶ ATtiny1616 PA0 (UPDI)
      │                                                【スケッチ書込 / 強制リセット】
      │
      └─▶ COM Port B (Port 2): CDC Serial ──────────▶ ATtiny1616 PB2/PB3 (UART)
                                                       【Serial.print / デバッグモニタ】
```

### 2.1 推奨IDE: Arduino IDE 1.8.x (Classic) の選定理由
1. **シリアルモニタの完全並行稼働（ノーウェイト・ノーフリーズ）:**
   - Arduino IDE 1.8系はシリアルモニタが別ウィンドウとして独立稼働します。
   - スケッチ書き込み（Port A: UPDI）を実行する際、**シリアルモニタ（Port B: UART）を開いたまま一切閉じる必要がありません**。
   - ポート競合エラーやIDEフリーズ、再接続時の待機時間がゼロとなり、超高速なコード修正・動作検証ループを実現します。
2. **現場ノートPCでの軽快性:**
   - ネット接続が不安定な工場・現場環境でも瞬時に起動し、リソース消費が極めて少ない。
3. **SerialUPDI との枯れた実績:**
   - megaTinyCore 内蔵の `SerialUPDI` ツール（Python / C++ 実装）が 1.8 系で最も安定して動作します。

### 2.2 コアライブラリ: megaTinyCore 推奨設定

| 設定項目 | 設定値 | 理由・備考 |
| :--- | :--- | :--- |
| **Board** | `ATtiny1616` (megaTinyCore) | Microchip 8-bit AVR 0-series |
| **Chip** | `ATtiny1616` | Flash 16KB / SRAM 2KB / EEPROM 256B |
| **Clock Source** | `10 MHz (Internal)` | 3.3V 駆動時 Safe Operating Area 保証、水晶発振子不要 |
| **Programmer** | `SerialUPDI (230400 or 460800 baud)` | CH342K Port A 経由の高速1ピン書き込み |
| **Port Selection** | **Port A (COMx)**: 書き込み用<br>**Port B (COMy)**: シリアルモニタ用 | OS側で2つのシリアルポートとして認識 |
| **BOD Voltage** | `2.6V` または `2.9V` | 3.3V 電源変動時の確実なフェイルセーフ保護 |

---

## 3. 基本電気仕様 (Electrical Specifications)

| 項目 | 定格 / 仕様 | 備考 |
| :--- | :--- | :--- |
| **主電源入力 (USB-PWR)** | **5.0V DC (最大 2.0A)** | Type-C (CCプルダウン 5.1kΩ×2)、過電流PTC・TVS保護 |
| **通信・デバッグポート (USB-COM)**| **USB 2.0 Full-Speed** | Type-C (USBLC6-2SC6 ESD保護、CH342K直結) |
| **内部ロジック電圧** | **3.3V DC** (最大 500mA) | Microne ME6211C33M5G-N 高速低ノイズ LDO |
| **拡張バス論理電圧** | **3.3V LVCMOS** | ADX 20P IDC (Pins 1〜16) へネイティブ直結 |
| **主電源幹線 (VBUS)** | **5.0V DC** (最大 2.0A) | ADX 20P IDC (Pins 17〜20) へ給電 |
| **主 MCU** | **Microchip ATtiny1616-MNR** | 20-pin QFN (3×3mm), -40℃〜+105℃ |
| **USB ブリッジ IC** | **WCH CH342K** | 10-pin ESSOP, デュアルUART出力 |
| **外形寸法** | **87.0 mm × 48.0 mm** | ADX 8748 フォームファクタ準拠 |

---

## 4. 回路ブロック ＆ アーキテクチャ (Circuit Architecture)

```text
                              【 ADX CORE-UA5 回路ブロック図 】

    [ USB-PWR (Type-C) ] ────▶ [ PTCヒューズ / TVS ] ──┬──▶ VBUS (5V/2A) ──▶ ADX 20P バス (Pins 17-20)
    (給電専用 5V 2A)                                   │
                                                      └──▶ [ ME6211 (3.3V LDO) ]
                                                                     │
                                                                     ├─▶ MCU VDD (3.3V)
                                                                     ├─▶ CH342K VIO (3.3V)
                                                                     └─▶ ADX バス ロジック電源

    [ USB-COM (Type-C) ] ────▶ [ USBLC6-2SC6 (ESD) ]
    (通信・デバッグ専用)              │
                                      ▼
                             [ WCH CH342K (Dual USB-UART) ]
                                      │
                                      ├─▶ Port 1 (TXD1/RXD1) ──[ SerialUPDI回路 ]──▶ PA0 (UPDI)
                                      │                          (ダイオード+抵抗)
                                      └─▶ Port 2 (TXD2/RXD2) ──────────────────────▶ PB2 (TX) / PB3 (RX)
                                                                                     (Serial デバッグ)
```

### 4.1 デュアル Type-C による電源・通信の完全分離
- **USB-PWR (Power ONLY):**
  - ACアダプタ、モバイルバッテリー、現場のUSB 5V電源から受電。
  - ADX拡張バス（VBUS）に直結され、スタックされたリレーCARD、モーターCARD、各種センサーCARDへ最大2Aの電力を安定供給。
- **USB-COM (Com ONLY):**
  - ノートPC等を接続。PC側からの電力逆流を防止、または最小限のVBUS検知のみとし、PCのUSBポートを保護。
  - **最大のメリット:** 現場で主電源（USB-PWR）を入れたまま、ノートPC（USB-COM）を安全に抜き差し可能。PCを抜いてもMCUや周辺機器は再起動せず稼働を継続可能。

### 4.2 SerialUPDI 接続回路
CH342K の Port 1 と ATtiny1616 の PA0 (UPDI) は、SpenceKonde 推奨の標準 SerialUPDI 回路（1N4148相当のショットキーダイオード ＋ 4.7kΩ抵抗）で結合されます。
- TXD1 からダイオード経由で UPDI ピンへ駆動
- RXD1 は UPDI ピンから直接受信
- これにより、専用デバッガ（Atmel-ICE等）不要で、標準COMポート経由の1線高速UPDI通信を実現。

---

## 5. ADX 20ピンバス ピンマッピング (ADX-BUS-SPEC-v4.1 準拠)

ATtiny1616 (20ピン) のリソースを、ADX標準バス仕様に無駄なく完全マッピングしています。

| バスピン | 信号名 | ATtiny1616 ピン | 主な代替機能 / 役割 |
| :---: | :--- | :--- | :--- |
| **1** | **GND** | GND | システムグラウンド |
| **2** | **OWD / PROG** | — (または予備GPIO) | ワンワイヤー / 外部拡張プログラミング |
| **3** | **AIN** | **PA6** (Pin 7) | 10-bit ADC 入力 (アナログ入力) |
| **4** | **GND** | GND | システムグラウンド |
| **5** | **EXTCLK** | **PA3** (Pin 2) | 外部クロック入力 / タイマーイベント入力 |
| **6** | **GND** | GND | システムグラウンド |
| **7** | **UART_TX** | **PB2** (Pin 12) | Hardware USART0 TX (CH342K Port 2 と共有/引出) |
| **8** | **UART_RX** | **PB3** (Pin 11) | Hardware USART0 RX (CH342K Port 2 と共有/引出) |
| **9** | **I2C_SCL** | **PB0** (Pin 14) | TWI / I2C クロック |
| **10** | **I2C_SDA** | **PB1** (Pin 13) | TWI / I2C データ |
| **11** | **GND** | GND | システムグラウンド |
| **12** | **SPI_CS** | **PC3** (Pin 18) | SPI チップセレクト |
| **13** | **SPI_MOSI** | **PC2** (Pin 17) | SPI 主出力 |
| **14** | **SPI_MISO** | **PC1** (Pin 16) | SPI 主入力 |
| **15** | **SPI_SCK** | **PC0** (Pin 15) | SPI クロック |
| **16** | **GND** | GND | システムグラウンド |
| **17** | **VBUS** | VBUS (+5V) | 主幹線電源 (USB-PWR 直結、最大 2.0A) |
| **18** | **VBUS** | VBUS (+5V) | 主幹線電源 |
| **19** | **VBUS** | VBUS (+5V) | 主幹線電源 |
| **20** | **VBUS** | VBUS (+5V) | 主幹線電源 |

---

## 6. 部品選定とサプライチェーン (BOM & SCM)

JLCPCB PCBA による少量〜量産製造を前提とし、**「いつでも在庫があり、安価に即納できる部品」** のみを厳選しています。

| 部品カテゴリ | 型番 / メーカー | パッケージ | JLCPCB区分 | 選定理由 |
| :--- | :--- | :--- | :--- | :--- |
| **MCU** | **ATtiny1616-MNR** (Microchip) | QFN-20 (3×3mm) | Extended (在庫潤沢) | 超小型・高信頼性、105℃耐熱、内蔵高精度発振器 |
| **USB-UART** | **CH342K** (WCH) | ESSOP-10 | Extended (超安価) | 1チップでデュアルUART内蔵、ドライバ安定 |
| **3.3V LDO** | **ME6211C33M5G-N** (Microne) | SOT-23-5 | Basic / Extended | 500mA出力、低ドロップアウト、低ノイズ |
| **ESD保護** | **USBLC6-2SC6** (Tech Public) | SOT-23-6 | Basic / Extended | USB 2.0 高速信号ライン保護 |
| **USBコネクタ** | **Type-C 16P (COM) / 6P (PWR)** | SMD | Basic / Extended | 頑丈なスルーホールタブ付き Type-C |

> [!NOTE]
> **ATtiny3216 / AVR-DD への将来的なアップグレード:**  
> 現時点では JLCPCB における調達性とコストから `ATtiny1616-MNR` を選定していますが、基板アートワーク上は将来的な `ATtiny3216` やピン互換ファミリへの乗せ換えを考慮した設計とします。

---

## 7. 将来ロードマップとエコシステムにおける棲み分け

- **ADX CORE-UA5 (本機):**
  - **位置づけ:** USB デスクトップ開発標準機、ホビー・試作・卓上実験、Arduino UNO R3/R4代替。
  - **強み:** ノートPCをつなぐだけですぐ動く、シリアルモニタを開いたまま瞬時に書き込める最高の開発体験。
- **ADX CORE-A24 (構想中):**
  - **位置づけ:** 工場・産業制御盤向け。DC24V 入力 ＋ 絶縁RS-485 特化型（旧CORE-A5の24V進化版）。
- **ADX RPR4 Smart Probe / RS-485 幹線:**
  - **位置づけ:** 現場配備後のリモート運用・メンテ。USB接続を不要とし、RS-485長距離バス経由での一括監視・Web MIDI / PWA 連携。

---

## 8. まとめ

ADX CORE-UA5 は、決して派手な高スペック競争を目指したボードではありません。  
しかし、**「壊れない」「現場でPC片手に触らず書き換えられる」「モニタを閉じずに何度でもリトライできる」「十分な電力をCARDに供給できる」** という、ものづくりの現場で本当に求められている価値を極限まで研ぎ澄ました、極めて実用的で頼もしいコアボードです。

