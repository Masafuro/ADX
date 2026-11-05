<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U 技術仕様案 (Hardware & Toolchain Specification Proposal)
〜 Arduino IDE 1.8系 × megaTinyCore 最適化・デュアルType-C・現場で使える「はたらくArduino」標準USBコアボード 〜

---

## 1. 概要とプロダクト位置づけ (Overview & Positioning)

### 1.1 プロダクト概要
**ADX CORE-U** は、ADXエコシステムにおける **「標準デスクトップ開発 兼 現場即応型 USBコアボード」** です。  
幾度もの試作と現場開発（CORE-U_OLD、CORE-D、RPR4等）の知見を経て、**「机の上のプロトタイプを、現場のリアルな箱へ連れ出すための等身大の相棒 ＝ はたらくArduino（The Working Arduino）」** として再定義されました。

最大の特徴は、**「筐体フレンドリー（Case-Friendly）」** であることです。  
制御盤、機器の隙間、カスタム3Dプリントケースに収められた状態でも、一切の物理ボタン操作（BOOTSELやリセットピン差し）を必要とせず、ノートPCからType-Cケーブル1本で安全・確実にスケッチ書き換えとリアルタイムデバッグを実現します。  
さらに、視覚（NeopixelフルカラーLED）と聴覚（パッシブ圧電サウンダー）のデュアルフィードバックを搭載し、ケースの外からでも状態が直感的に把握できます。

### 1.2 競合・他ボードとの位置づけと設計思想
Arduino UNO R3 や UNO R4 Minima との詳細な工学的対比、USBハング耐性の仕組み、電源保護、および「はたらくArduino」としての設計思想の深掘りについては、以下の比較ドキュメントを参照してください：
- 📖 **詳細ドキュメント:** [ADX CORE-U 比較分析・設計思想 (comparison.md)](comparison.md)

---

## 2. 開発ツールチェーン仕様 (Software & Toolchain)

本ボードは、現場での軽快な動作とデュアルポート運用を最大化するため、**Arduino IDE 1.8.x（クラシック版）** および **megaTinyCore** を公式標準開発環境として策定します。

```text
               【 CORE-U 開発・デバッグワークフロー 】

    PC (Arduino IDE 1.8.x / megaTinyCore)
      │
      │ ──[ 1本の Type-C ケーブル (USB-COM) ]──
      ▼
   [ WCH CH342K (Dual USB-UART) ]
      ├─▶ COM Port A (Port 0): SerialUPDI ──────────▶ ATtiny1616 PA0 (UPDI)
      │                                                【スケッチ書込 / 強制リセット】
      │
      └─▶ COM Port B (Port 1): CDC Serial ──────────▶ ATtiny1616 PA1/PA2 (Alt-UART)
                                                       【Serial.swap(1) / 常時デバッグモニタ】
```

### 2.1 推奨IDE: Arduino IDE 1.8.x (Classic) の選定理由
1. **シリアルモニタの並行稼働（ノーウェイト・ノーフリーズ）:**
   - Arduino IDE 1.8系はシリアルモニタが別ウィンドウとして独立稼働します。
   - スケッチ書き込み（Port A: UPDI）を実行する際、**シリアルモニタ（Port B: UART）を開いたまま一切閉じる必要がありません**。
   - ポート競合エラーやIDEフリーズの不安を解消し、再接続の待機時間を極限まで削減した軽快なコード修正・動作検証ループを実現します。
2. **現場ノートPCでの軽快性:**
   - ネット接続が不安定な現場環境でも瞬時に起動し、リソース消費が極めて少ない。
3. **SerialUPDI との枯れた実績:**
   - megaTinyCore 内蔵の `SerialUPDI` ツール（Python / C++ 実装）が 1.8 系で最も安定して動作します。

### 2.2 ハードウェアUARTのPORTMUX切替（CORE-D直系のデュアルUART）
ATtiny1616 の内蔵ポートマルチプレクサ（PORTMUX）を活用し、ハードウェアUSART0を切り替えて運用します：
- **`Serial.swap(1)`（デフォルト推奨）:**  
  USART0 を `PA1 (TX)` / `PA2 (RX)` にルーティングし、CH342K Port 1 と直結。ADXバス側のCARD通信と干渉しない独立したデバッグが可能。
- **`Serial.swap(0)`:**  
  USART0 を `PB2 (TX)` / `PB3 (RX)` にルーティングし、ADX 20Pバス（Pins 7/8）経由でスタックされた外部CARDと通信。

---

## 3. 基本電気仕様 (Electrical Specifications)

| 項目 | 定格 / 仕様 | 備考 |
| :--- | :--- | :--- |
| **主電源入力 (USB-PWR)** | **5.0V DC (最大 2.0A)** | Type-C (CCプルダウン 5.1kΩ×2)、TVS保護、平滑コンデンサ |
| **パワースイッチ IC** | **Diodes AP22814AW5-7** | 固定電流制限（代表値3A）、逆流防止、過熱保護、短絡保護、ソフトスタート |
| **通信・デバッグポート (USB-COM)**| **USB 2.0 Full-Speed** | Type-C (USBLC6-2SC6 ESD保護、CH342K直結) |
| **内部ロジック電圧** | **3.3V DC** (最大 500mA) | Microne ME6211C33M5G-N 高速低ノイズ LDO |
| **拡張バス論理電圧** | **3.3V LVCMOS** | ADX 20P IDC (Pins 1〜16) へネイティブ直結 |
| **主電源幹線 (VBUS)** | **5.0V DC** (保護後 VBUS_PROT) | ADX 20P IDC (Pins 17〜20) へ給電（最大 2.0A） |
| **主 MCU** | **Microchip ATtiny1616-MNR** | 20-pin QFN (3×3mm), -40℃〜+105℃ 定格品 |
| **USB ブリッジ IC** | **WCH CH342K** | 10-pin ESSOP, デュアルUART出力 |
| **視覚インジケータ** | **Neopixel (シリアルRGB LED)** | 1ピン制御、フルカラー 24bit RGB |
| **聴覚インジケータ** | **パッシブ圧電サウンダー** | TCB1 タイマーPWM直結、`tone()` 関数対応 (SFN-1407PA7.6) |
| **外形寸法** | **87.0 mm × 48.0 mm** | ADX 8748 フォームファクタ準拠 |

---

## 4. 回路ブロック ＆ アーキテクチャ (Circuit Architecture)

```text
                              【 ADX CORE-U 回路ブロック図 】

    [ USB-PWR (Type-C) ] ────▶ [ TVS: SMF5.0CA ]
    (給電専用 5V 最大2.0A)           │
                                     ▼ (USB_VBUS)
                         ┌─────────────────────────────────────────────────┐
                         │ U3: AP22814AW5-7 (過電流保護パワースイッチ)     │
                         │  - Pin 5 (IN)  : USB_VBUS                       │
                         │  - Pin 4 (EN)  : 10kΩ プルアップ (常時オン)      │
                         │  - Pin 3 (FLG) ──[ 10kΩ Pullup ]──▶ PA7 (過電流検知/CCL)
                         │  - Pin 1 (OUT) : VBUS_PROT                      │
                         └───────────────────────┬─────────────────────────┘
                                                 │
                                                 ├──▶ ADX 20P バス (Pins 17-20: VBUS幹線 / 最大2.0A)
                                                 │
                                                 └──▶ [ ME6211 (3.3V LDO) ]
                                                                │
                                                                ├─▶ MCU VDD (3.3V)
                                                                ├─▶ CH342K VIO (3.3V)
                                                                ├─▶ Neopixel / ブザー電源
                                                                └─▶ ADX バス ロジック電源

    [ USB-COM (Type-C) ] ────▶ [ USBLC6-2SC6 (ESD) ]
    (通信・デバッグ専用)              │
                                      ▼
                             [ WCH CH342K (Dual USB-UART) ]
                                      │
                                      ├─▶ Port 0 (TXD0/RXD0) ──[ SerialUPDI回路 ]──▶ PA0 (UPDI)
                                      │                          (ダイオード+抵抗)
                                      └─▶ Port 1 (TXD1/RXD1) ──────────────────────▶ PA1/PA2 (Alt-UART)
                                                                                     (Serial.swap(1))

    [ オンボード UI / 自己診断 ]
     ├── PB4 ──[ 100Ω: R2 ]──▶ Neopixel RGB LED (SK6812SMINI-ER)
     │                          │ (内部波形整形出力 DOUT)
     │                          ▼
     │                       [ CN2: 外部LED出力ポート (XH-4AW / 2.54mm 4P) ]
     │                        ├── Pin 1: 5V (VBUS_PROT: 最大2.0A)
     │                        ├── Pin 2: DOUT (NEOP_DO)
     │                        ├── Pin 3: DOUT (NEOP_DO) ※ WS2813/WS2815 バックアップ(BI)直結対応
     │                        └── Pin 4: GND
     │
     ├── PB5 ──[ 100Ω: R1 ]──▶ パッシブ圧電サウンダー (SFN-1407PA7.6, tone()対応)
     └── PA7 (FLG監視) ◀────── 10kΩ Pullup to 3V3 ── AP22814A Pin 3 FLG (過電流・過熱検知)

    [ 外部センサ / 拡張 I/O ポート (U1: ZX-HY2.0-4PWZ / 2.0mm 4P Grove互換) ]
     ├── Pin 1: GND
     ├── Pin 2: 3V3 (内部ロジック 3.3V 電源)
     ├── Pin 3: PA4 (CCL LUT0-OUT / TCA0 WO4 PWM / AIN4 / GPIO)
     └── Pin 4: PA5 (AIN5 / VREFA / GPIO)
```

### 4.1 デュアル Type-C による電源・通信の独立系統化
- **USB-PWR (Power ONLY):**
  - ACアダプタ、モバイルバッテリー、現場のUSB 5V電源から受電。
  - パワースイッチ IC（AP22814A）を経由して ADX拡張バス（VBUS）に直結され、スタックされたリレーCARD、モーターCARD、各種センサーCARDへ最大 2.0A の電力を安定供給。
- **USB-COM (Com ONLY):**
  - ノートPC等を接続。PC側からの電力逆流を防止、PCのUSBポートを保護。
  - **最大のメリット:** 現場で主電源（USB-PWR）を入れたまま、ノートPC（USB-COM）を安全に抜き差し可能。PCを抜いてもMCUや周辺機器は再起動せず稼働を継続可能。

### 4.2 現場フィードバック（視覚 ＋ 聴覚）
- **Neopixel フルカラー LED (`PB4`):**
  - わずか 1 ピンで 1677 万色を制御。
  - 状態定義例:
    - **緑 (点灯/ブレス):** 正常稼働
    - **青 (点滅):** PC との USB 通信中
    - **橙 (点滅):** UPDI ファームウェア書き込み中
    - **赤 (高速点滅):** 過電流 FLG トリップ / システムエラー
- **パッシブ圧電サウンダー (`PB5`):**
  - タイマーB（TCB1）の PWM 出力に直結。`tone()` 関数で自在なトーンを生成。
  - 密閉ケース内や制御盤の奥に据え付けられていても、**「電源投入音（ピピッ）」「書込完了音（ピロリ♪）」「過電流警告音（ピー）」** を耳で確実に確認可能。

### 4.3 ハードウェア自律保護 ＆ CCL 連携 (`PA7`)
- AP22814A の Pin 3（FLG）は、過電流または過熱が発生した際にオープンドレインで LOW に落ちます。
- これを ATtiny1616 の `PA7`（CCL LUT1-OUT）に接続することで、ソフトウェアの割り込み処理だけでなく、**CCL（カスタムロジック）を用いたハードウェア自律インターロック**（過電流トリップ時に高速（ナノ秒オーダー）で外部信号を遮断する保護）が構築可能です。

---

## 5. ATtiny1616 全20ピン アサイン一覧 (Pin Assignment)

ATtiny1616 (20ピン QFN/SOIC) の全ペリフェラルを無駄なく引き出す実用的なピン配分です。

| ピン番号 | ポート名 | 接続先・機能分類 | 主な機能 / ペリフェラル / 役割 |
| :---: | :--- | :--- | :--- |
| **1** | **PA2** | **CH342K Port 1 TXD1** | **USART0 Alt-RX** (`Serial.swap(1)` PCデバッグ受信) |
| **2** | **PA3** | **ADX Bus Pin 5** | **EXTCLK** (外部クロック入力 / タイマーイベント) |
| **3** | **GND** | **システムグラウンド** | 電源 GND |
| **4** | **VDD** | **内部ロジック電源** | 3.3V DC (ME6211 LDO 出力) |
| **5** | **PA4** | **U1 (HY2.0) Pin 3** | **CCL LUT0-OUT** / TCA0 WO4 (PWM) / AIN4 / GPIO |
| **6** | **PA5** | **U1 (HY2.0) Pin 4** | **AIN5** / VREFA (高精度アナログ入力 / GPIO) |
| **7** | **PA6** | **ADX Bus Pin 3** | **AIN** (ADXバス標準 10-bit アナログ入力) |
| **8** | **PA7** | **AP22814A FLG 監視** | **過電流・過熱検知** / CCL LUT1-OUT / 10kΩプルアップ (to 3V3) |
| **9** | **PB5** | **オンボード圧電サウンダー**| **TCB1 WO (PWM)** / `tone()` 直結 (SFN-1407PA7.6) |
| **10** | **PB4** | **オンボード Neopixel** | **シリアル RGB LED** (SK6812SMINI-ER / tinyNeoPixel) |
| **11** | **PB3** | **ADX Bus Pin 8** | **UART_RX** (USART0 Default-RX / 外部CARD通信) |
| **12** | **PB2** | **ADX Bus Pin 7** | **UART_TX** (USART0 Default-TX / 外部CARD通信) |
| **13** | **PB1** | **ADX Bus Pin 10** | **I2C_SDA** (TWI0 データ) |
| **14** | **PB0** | **ADX Bus Pin 9** | **I2C_SCL** (TWI0 クロック) |
| **15** | **PC0** | **ADX Bus Pin 15** | **SPI_SCK** (SPI0 クロック) |
| **16** | **PC1** | **ADX Bus Pin 14** | **SPI_MISO** (SPI0 主入力) |
| **17** | **PC2** | **ADX Bus Pin 13** | **SPI_MOSI** (SPI0 主出力) |
| **18** | **PC3** | **ADX Bus Pin 12** | **SPI_CS** (SPI0 チップセレクト) |
| **19** | **PA0** | **ADX Pin 2 ＆ CH342K** | **UPDI / RESET** (SerialUPDI書込 ＆ ADX OWD/PROG) |
| **20** | **PA1** | **CH342K Port 1 RXD1** | **USART0 Alt-TX** (`Serial.swap(1)` PCデバッグ送信) |
| **21** | **EP** | **サーマルパッド** | 放熱 ＆ システムグラウンド (GND) |

### 5.1 外部 LED 出力ポート (CN2: XH-4AW)
基板端に配置される **JST XH 2.54mm ピッチ 4極ライトアングルコネクタ**（LCSC: C21273）です。
- **4 線式高信頼 LED（WS2813 / WS2815 等）直接接続対応:**  
  Pin 2 と Pin 3 を基板上でショート（どちらも `NEOP_DO`）接続。WS2813 のバックアップデータライン（BI）はコントローラ側で DI と同電位に繋ぐ仕様であるため、市販の 4 線式 LED テープを **無改造で直結接続** 可能です。
- **標準 3 芯 LED（WS2812B / SK6812 等）との互換性:**  
  1-2-4 番、または 1-3-4 番の結線でそのまま通常 3 芯 LED が点灯します。
- **コネクタ定格 3A（ボード給電上限 2.0A に対して安全マージンを確保）:**  
  LED テープ（最大 2.0A 給電）の点灯でもコネクタ端子の発熱や接触抵抗による電圧降下を最小限に抑えます。
- **端子割り当て:**
  - **Pin 1:** `VBUS_PROT` (5.0V DC 主電源 / 最大 2.0A)
  - **Pin 2:** `DOUT` (オンボード SK6812 内部整形済シリアル出力)
  - **Pin 3:** `DOUT` (Pin 2 直結 / WS2813 バックアップ BI 接続用)
  - **Pin 4:** `GND` (大電流リターン・信号共通グラウンド)

### 5.2 外部センサ / 拡張 I/O ポート (U1: ZX-HY2.0-4PWZ)
基板端に配置される **HY2.0 (PH 2.0mm ピッチ) 4極コネクタ**（LCSC: C7429589）です。M5Stack（Grove互換）や Seeed Studio のエコシステムとピン互換性を持ちます。
- **3.3V 系の独立供給:**  
  CN2（5V 大電流系）から完全に電気的に分離され、内部 3.3V LDO からクリーンなロジック電源を供給します。
- **端子割り当て:**
  - **Pin 1:** `GND` (システムグラウンド)
  - **Pin 2:** `3V3` (内部ロジック 3.3V 電源出力)
  - **Pin 3:** `PA4` (**CCL LUT0-OUT** / TCA0 WO4 PWM / AIN4 / 汎用 GPIO)
  - **Pin 4:** `PA5` (**AIN5** / VREFA 高精度アナログ入力 / 汎用 GPIO)
- **主なユースケース:**
  - M5Stack / Grove 互換のアナログセンサ（光・温度・圧力等）の直結
  - 小型サーボモータの直接駆動（PA4 の TCA0 16-bit PWM 出力）
  - タクトスイッチや人感 PIR センサによる LED 点灯モード切替
  - CCL（内蔵カスタムロジック）LUT0 によるマイクロ秒未満のハードウェア自律信号出力

---

## 6. 部品選定とサプライチェーン (BOM & SCM)

JLCPCB PCBA による少量〜量産製造を前提とし、**「いつでも在庫があり、安価に即納できる部品」** のみを厳選しています。

| 部品カテゴリ | 型番 / メーカー | パッケージ | JLCPCB区分 | 選定理由 |
| :--- | :--- | :--- | :--- | :--- |
| **MCU** | **ATtiny1616-MNR** (Microchip) | QFN-20 (3×3mm) | Extended (C507118) | 超小型・高信頼性、-40℃〜+105℃ 定格品、内蔵高精度発振器、CCL内蔵 |
| **USB-UART** | **CH342K** (WCH) | ESSOP-10 | Extended (C2826608) | 1チップでデュアルUART内蔵、ドライバ極めて安定 |
| **パワースイッチ** | **AP22814AW5-7** (Diodes Inc) | SOT-25 | Extended (C2680321) | 固定電流制限（代表値3A）、逆流防止、短絡・過熱保護、FLGオープンドレイン |
| **フルカラーLED** | **SK6812SMINI-ER** (OPSCO) | SMD2835-4P (3.5×2.8mm)| Extended (C5142047) | 4ピン、800kbps、-40℃〜+85℃、高信頼性 |
| **圧電サウンダー**| **SFN-1407PA7.6** (S&S) | $\phi 14\text{mm} \times 7.5\text{mm}$ | Extended (C360603) | 5mA超低消費、-40℃〜+105℃耐熱、85dB大音量、MCU直結 |
| **外部LEDコネクタ**| **XH-4AW** | 2.54mm 4P ライトアングル | Extended (C21273) | 定格3A（最大2.0A給電対応）、WS2813/WS2815/3芯LED両対応 |
| **外部センサコネクタ**| **ZX-HY2.0-4PWZ** | 2.00mm 4P ライトアングル | Extended (C7429589) | M5Stack/Grove互換、3.3V系センサ・サーボ直結 |
| **3.3V LDO** | **ME6211C33M5G-N** (Microne) | SOT-23-5 | Basic (C82942) | 500mA出力、低ドロップアウト、低ノイズ |
| **TVSダイオード** | **SMF5.0CA-MS** | SOD-123FL | Basic (C7473196) | 5V双方向TVS、サージ保護 |
| **ESD保護** | **USBLC6-2SC6** (Tech Public) | SOT-23-6 | Extended (C2827654) | USB 2.0 高速信号ライン保護 |
| **USBコネクタ** | **Type-C 16P (COM) / 6P (PWR)** | SMD (スルーホールタブ付)| Extended (C36936538/54)| 頑丈な高耐久 Type-C レセプタクル |

---

## 7. 将来ロードマップとエコシステムにおける棲み分け

- **ADX CORE-U (本機):**
  - **位置づけ:** USB デスクトップ開発標準機、ホビー・試作・卓上実験、Arduino UNO R3/R4代替。
  - **強み:** ノートPCをつなぐだけですぐ動く、シリアルモニタを開いたまま瞬時に書き込める快適な開発体験。LEDとブザーによる現場フィードバック。
- **ADX CORE-A24 (構想中):**
  - **位置づけ:** 設備・現場制御盤向け。DC24V 入力 ＋ 絶縁RS-485 特化型（旧CORE-A5の24V進化版）。
- **ADX RPR4 Smart Probe / RS-485 幹線:**
  - **位置づけ:** 現場配備後のリモート運用・メンテ。USB接続を不要とし、RS-485長距離バス経由での一括監視・Web MIDI / PWA 連携。

---

## 8. まとめ

ADX CORE-U は、決して派手な高スペック競争を目指したボードではありません。  
しかし、**「現場のトラブルに強い」「現場でPC片手に触らず書き換えられる」「モニタを閉じずに何度でもリトライできる」「十分な電力（5V 2.0A）をCARDに供給できる」「ケースの外から音と光で状態がわかる」** という、ものづくりの現場で本当に求められている価値を極限まで研ぎ澄ました、現場で使える「はたらくArduino」の標準コアボードです。
