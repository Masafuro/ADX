<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ATtiny1616 ペリフェラル機能 & PORTMUX 統合リファレンス

[ English (../en/adx_attiny1616_mcu_matrix.md) | **日本語** ]

## 1. 概要 (Overview)
本ドキュメントは、Microchip社製 **ATtiny1616-MNR (QFN-20)** を搭載したマイコンボード「ADX」における、MCU全ピンのペリフェラル多重化機能（Multiplexing）および **PORTMUX（ポートマルチプレクサ）レジスタ** の設定仕様・デザインルールを定義する統合リファレンスです。

物理ピン配置やリボンケーブルのシールド構造については、[ADX Pinout 仕様書 (v2)](ADX_pinout.md) を参照してください。本ドキュメントでは、外部拡張用の 2×10ピン IDCコネクタ（基板回路図上のリファレンス例: CN2）の各端子番号を **「IDC Pin 1〜20」** と表記します。

---

## 2. ADX ボードにおける PORTMUX デザインルール (Design Rules)

ATtiny1616 は柔軟なピン再配置機能（PORTMUX）を備えていますが、ADX ボードではオンボード回路（RS-485 トランシーバー、BMCブート制御 / インジケータLED、UPDI）に計7ピンが専用接続されています。
そのため、ファームウェア開発時は以下の **必須設定・禁止設定** を遵守する必要があります。

### ① PORTMUX 設定ガイドライン一覧

| レジスタ | ビット | 機能 | ADX推奨/必須設定 | 設定値 | 理由・動作解説 |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **PORTMUX.CTRLB** | **SPI0** (bit 2) | SPI0 通信ピン | **Alternate 必須** | `1` | Default（PA1..PA4）は RS-485 で占有されているため使用不可。IDC コネクタの **PC0〜PC3 (IDC Pin 16〜19)** を使用。 |
| **PORTMUX.CTRLB** | **TWI0** (bit 4) | I2C (TWI0) 通信ピン | **Default 必須** | `0` | Default で IDC コネクタの **PB0/PB1 (IDC Pin 13, 14)** に接続。Alternate（PA1/PA2）は RS-485 と衝突するため使用禁止。 |
| **PORTMUX.CTRLB** | **USART0** (bit 0) | USART0 通信ピン | **用途切替** | `0` / `1` | `0`: オンボード **RS-485 (PA1/PA2)** に接続<br>`1`: IDC 外部 **UART (PB2/PB3: IDC Pin 11, 12)** に接続 |
| **PORTMUX.CTRLC** | **TCA00〜05** | TCA0 PWM 出力 | **任意** | 表参照 | モード（Normal / Split）および使用ピンに応じて動的設定可能。 |
| **PORTMUX.CTRLD** | **TCB0** (bit 0) | TCB0 PWM 出力 | **任意** | `0` / `1` | `0`: **IDC Pin 7 (PA5)**<br>`1`: **IDC Pin 19 (PC0)** |
| **PORTMUX.CTRLD** | **TCB1** (bit 1) | TCB1 PWM 出力 | **Default** | `0` | Default で **IDC Pin 9 (PA3)** に接続。 |
| **PORTMUX.CTRLA** | **LUT0/1** (bit 4,5) | CCL 出力ピン | **用途に応じる** | `0` / `1` | オンボード回路との競合を避けて選択。 |
| **PORTMUX.CTRLA** | **EVOUT0〜2** | イベント出力 | **任意** | `0` / `1` | IDC 外部へ内部イベントクロック等を出力可能（EVOUT1: PB2 / Pin 11, EVOUT2: PC2 / Pin 17）。 |

---

## 3. ATtiny1616-MNR 全18ピン マルチプレクス完全マトリクス

データシート（Table 5-1: PORT Function Multiplexing）に基づく、全GPIOの多重化機能一覧です。

| ピン名 | QFN | SOIC | ADX接続先 | ADC0 | ADC1 | PWM / タイマー | 通信 (USART/SPI/I2C) | アナログ / 特殊 | その他 (CCL/PTC/EV) |
| :--- | :---: | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **PA0** | 19 | 16 | **オンボード (UPDI/RESET)** | AIN0 | - | - | - | RESET / UPDI | LUT0-IN0 |
| **PA1** | 20 | 17 | **オンボード (RS-485 TxD)** | AIN1 | - | - | USART0 TxD (Def) / SPI MOSI(Def) / *TWI SDA(Alt)* | - | LUT0-IN1 |
| **PA2** | 1 | 18 | **オンボード (RS-485 RxD)** | AIN2 | - | - | USART0 RxD (Def) / SPI MISO(Def) / *TWI SCL(Alt)* | - | EVOUT0 / LUT0-IN2 |
| **PA3** | 2 | 19 | **IDC Pin 9** | AIN3 | - | TCA0 WO3(Def) / **TCB1 WO** | USART0 XCK (Def) / SPI SCK (Def) | **EXTCLK** | - |
| **PA4** | 5 | 2 | **オンボード (RS-485 DE)** | AIN4 | AIN0 | TCA0 WO4(Def) / **TCD0 WOA** | USART0 XDIR (Def) / SPI SS (Def) | - | X0/Y0 / LUT0-OUT(Def) |
| **PA5** | 6 | 3 | **IDC Pin 7** | AIN5 | AIN1 | TCA0 WO5(Def) / **TCB0 WO(Def)** / **TCD0 WOB** | - | **VREFA** / AC0 OUT / AC2 AINN0 | X1/Y1 |
| **PA6** | 7 | 4 | **IDC Pin 6** | AIN6 | AIN2 | - | - | **DAC0 OUT** / AC0-2 AIN | X2/Y2 |
| **PA7** | 8 | 5 | **オンボード (RS-485 RE)** | AIN7 | AIN3 | - | - | AC0-2 AIN | X3/Y3 / LUT1-OUT(Def) |
| **PB0** | 14 | 11 | **IDC Pin 13** | AIN11 | - | **TCA0 WO0(Def)** | **TWI0 SCL (Def)** / USART0 XDIR(Alt) | AC0/1 AIN | X5/Y5 |
| **PB1** | 13 | 10 | **IDC Pin 14** | AIN10 | - | **TCA0 WO1(Def)** | **TWI0 SDA (Def)** / USART0 XCK(Alt) | AC0 AIN | X4/Y4 |
| **PB2** | 12 | 9 | **IDC Pin 11** | - | - | **TCA0 WO2(Def)** | **USART0 TxD (Alt)** | AC1 OUT / TOSC2 | EVOUT1 |
| **PB3** | 11 | 8 | **IDC Pin 12** | - | - | **TCA0 WO0(Alt)** | **USART0 RxD (Alt)** | AC0 OUT / TOSC1 | - |
| **PB4** | 10 | 7 | **オンボード (赤色LED)** | AIN9 | - | TCA0 WO1(Alt) | - | AC1/2 AIN | X13/Y13 / LUT0-OUT(Alt) |
| **PB5** | 9 | 6 | **オンボード (BOOT_REQ)** | AIN8 | - | TCA0 WO2(Alt) | - | CLKOUT / AC1/2 AIN | X12/Y12 |
| **PC0** | 15 | 12 | **IDC Pin 19** | - | AIN6 | **TCB0 WO(Alt)** / **TCD0 WOC** | **SPI0 SCK (Alt)** | - | X6/Y6 |
| **PC1** | 16 | 13 | **IDC Pin 18** | - | AIN7 | **TCD0 WOD** | **SPI0 MISO (Alt)** | - | X7/Y7 / LUT1-OUT(Alt) |
| **PC2** | 17 | 14 | **IDC Pin 17** | - | AIN8 | - | **SPI0 MOSI (Alt)** | - | EVOUT2 / X8/Y8 |
| **PC3** | 18 | 15 | **IDC Pin 16** | - | AIN9 | **TCA0 WO3(Alt)** | **SPI0 SS (Alt)** | - | X9/Y9 / LUT1-IN0 |

※ピン機能表記の `(Def)` は Default 位置、`(Alt)` は Alternate（PORTMUX切替後）を示します。

---

## 4. IDC コネクタ 逆引きリファレンス (ペリフェラル別)

ドータボード開発やファームウェア実装時に、IDCコネクタのピン番号から利用可能な機能を素早く確認するための逆引き表です。

### ① PWM / タイマー出力機能

ATtiny1616 には 3 種類の独立したタイマー（TCA0, TCBn, TCD0）が搭載されており、IDC コネクタから多彩な PWM 出力が可能です。

| IDC ピン | マイコンピン | 対応タイマー・PWMチャネル | PORTMUX 設定条件 | 備考・特徴 |
| :---: | :---: | :--- | :--- | :--- |
| **Pin 7** | PA5 | **TCB0 WO (Def)**<br>**TCD0 WOB**<br>TCA0 WO5 (Def) | `PORTMUX.CTRLD.TCB0 = 0`<br>TCD0 制御レジスタ有効化<br>`PORTMUX.CTRLC.TCA05 = 0` | 16-bit PWM (TCB0) / **高周波 12-bit PWM (TCD0)** / 16-bit PWM (TCA0) |
| **Pin 9** | PA3 | **TCB1 WO**<br>TCA0 WO3 (Def) | `PORTMUX.CTRLD.TCB1 = 0`<br>`PORTMUX.CTRLC.TCA03 = 0` | 16-bit PWM (TCB1) / EXTCLK兼用 |
| **Pin 11** | PB2 | **TCA0 WO2 (Def)** | `PORTMUX.CTRLC.TCA02 = 0` | 16-bit PWM (TCA0 Normal) / 8-bit PWM (Split) / UART TxD兼用 |
| **Pin 12** | PB3 | **TCA0 WO0 (Alt)** | `PORTMUX.CTRLC.TCA00 = 1` | 8-bit PWM (TCA0 Split mode) / UART RxD兼用 |
| **Pin 13** | PB0 | **TCA0 WO0 (Def)** | `PORTMUX.CTRLC.TCA00 = 0` | 16-bit PWM (TCA0 Normal) / 8-bit PWM (Split) / I2C SCL兼用 |
| **Pin 14** | PB1 | **TCA0 WO1 (Def)** | `PORTMUX.CTRLC.TCA01 = 0` | 16-bit PWM (TCA0 Normal) / 8-bit PWM (Split) / I2C SDA兼用 |
| **Pin 16** | PC3 | **TCA0 WO3 (Alt)** | `PORTMUX.CTRLC.TCA03 = 1` | 8-bit PWM (TCA0 Split mode) / SPI SS兼用 |
| **Pin 18** | PC1 | **TCD0 WOD** | TCD0 制御レジスタ有効化 | **高周波 12-bit PWM (TCD0)** / SPI MISO兼用 |
| **Pin 19** | PC0 | **TCD0 WOC**<br>**TCB0 WO (Alt)** | TCD0 制御レジスタ有効化<br>`PORTMUX.CTRLD.TCB0 = 1` | **高周波 12-bit PWM (TCD0)** / 16-bit PWM (TCB0) / SPI SCK兼用 |

> **TCD0 (Timer Type D) について:**
> 最大48MHz（PLLまたは非同期クロック）で駆動可能な高周波タイマーです。IDC コネクタからは **IDC Pin 7 (WOB)**、**IDC Pin 18 (WOD)**、**IDC Pin 19 (WOC)** の計3チャンネルを取り出すことができ、高分解能PWMや電源制御・ハーフブリッジ駆動等に適しています。

---

### ② アナログ機能 (ADC0 / ADC1 / DAC0 / コンパレータ)

IDC コネクタには、高精度なアナログ入出力機能が集約されています。

| IDC ピン | マイコンピン | アナログ機能 | 説明 |
| :---: | :---: | :--- | :--- |
| **Pin 6** | PA6 | **DAC0 OUT**<br>ADC0 AIN6 / ADC1 AIN2<br>AC0/1/2 AIN | **8-bit DAC 出力**（内蔵DACのアナログ電圧出力ピン）<br>アナログコンパレータ入力 |
| **Pin 7** | PA5 | **VREFA** (外部参照電圧)<br>ADC0 AIN5 / ADC1 AIN1<br>AC0 OUT | **外部基準電圧入力 (VREFA)**<br>アナログコンパレータ AC0 出力 |
| **Pin 9** | PA3 | **ADC0 AIN3** | 汎用 10-bit ADC 入力（外部クロック未使用時） |
| **Pin 13** | PB0 | ADC0 AIN11 / AC0,1 AIN | 10-bit ADC 入力 / コンパレータ入力（I2C SCL兼用） |
| **Pin 14** | PB1 | ADC0 AIN10 / AC0 AIN | 10-bit ADC 入力 / コンパレータ入力（I2C SDA兼用） |
| **Pin 16** | PC3 | **ADC1 AIN9** | ADC1 10-bit ADC 入力（SPI SS兼用） |
| **Pin 17** | PC2 | **ADC1 AIN8** | ADC1 10-bit ADC 入力（SPI MOSI兼用） |
| **Pin 18** | PC1 | **ADC1 AIN7** | ADC1 10-bit ADC 入力（SPI MISO兼用） |
| **Pin 19** | PC0 | **ADC1 AIN6** | ADC1 10-bit ADC 入力（SPI SCK兼用） |

---

### ③ イベントシステム (EVSYS) & クロック出力

| IDC ピン | マイコンピン | 特殊機能 | PORTMUX / 設定 | 説明 |
| :---: | :---: | :--- | :--- | :--- |
| **Pin 9** | PA3 | **EXTCLK** | CLKCTRL 設定 | 外部クロック発振器入力（システムクロック源） |
| **Pin 11** | PB2 | **EVOUT1** | `PORTMUX.CTRLA.EVOUT1 = 1` | イベントシステム出力（タイマーイベントや周波数のモニタ出力） |
| **Pin 17** | PC2 | **EVOUT2** | `PORTMUX.CTRLA.EVOUT2 = 1` | イベントシステム出力 |
| 全信号ピン | PA/PB/PC | **外部割り込み** | `PORTx.PINnCTRL` | 全GPIOでピン変化割り込み対応（※PB2/PC2は完全非同期検知に対応） |

---

## 5. オンボード専用ピン (7ピン) の設計意図と保護

IDC に引き出されていない 7 ピン（オンボード専用回路）の機能と、ボード上で固定されている役割です。

| ピン名 | 接続先・用途 | 状態・制約 | ファームウェア上の注意点 |
| :--- | :--- | :--- | :--- |
| **PA0** | UPDI プログラミング / RESET | デバッグ専用（オンボードTHT引き出し） | ヒューズ設定で GPIO 化しないこと（書き込み不可リスク防止）。 |
| **PA1** | SP485EEN DI (TxD) | オンボード RS-485 送信 | `PORTMUX.CTRLB.USART0 = 0` 時に USART0 送信ピンとして機能。 |
| **PA2** | SP485EEN RO (RxD) | オンボード RS-485 受信 | `PORTMUX.CTRLB.USART0 = 0` 時に USART0 受信ピンとして機能。 |
| **PA4** | SP485EEN DE (Driver Enable) | 送信イネーブル | USART0 の `XDIR` 自動方向制御ピンとして機能。 |
| **PA7** | SP485EEN /RE (Receiver Enable) | 受信イネーブル | GPIO 出力として制御（Lowで受信有効、Highで受信停止）。 |
| **PB4** | オンボード赤色LED | Active HIGH (Highで点灯) | GPIO 出力。用途はオープン（ブートローダー待機中のインジケータ等に推奨）。 |
| **PB5** | オンボードBMC BOOT_REQ | ブートリクエスト入力（プルダウン＋直列抵抗実装） | 起動時にサンプリング。HIGH ならタイムアウトなしでブートローダー突入、LOW なら即座に通常スケッチ実行。 |
