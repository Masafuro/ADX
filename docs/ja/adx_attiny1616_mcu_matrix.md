<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ATtiny1616 ペリフェラル機能 & PORTMUX 統合リファレンス

[ [English](../en/adx_attiny1616_mcu_matrix.md) | **日本語** ]

## 1. 概要 (Overview)
本ドキュメントは、Microchip 社製 **ATtiny1616-MNR (QFN-20)** を搭載した ADX Core ボード（`CORE-U`, `CORE-A5` / `CORE-A24` 等）における、MCU 全ピンのペリフェラル多重化機能（Multiplexing）および **PORTMUX（ポートマルチプレクサ）レジスタ** の設定仕様・デザインルールを定義する統合リファレンスです。

共通バスの物理ピン配置・論理ピン ID（`S00`, `P01`〜`P10`）・シールド構造については、[ADX 20ピンバス仕様書 (SSOT: v4.2)](../spec/ADX_PINOUT.md) および [設計・解説ガイド](ADX_pinout.md) を参照してください。本マッピングの英文 SSOT は [docs/spec/mcu_mapping/attiny1616.md](../spec/mcu_mapping/attiny1616.md) です。

---

## 2. ADX ボードにおける PORTMUX デザインルール (Design Rules)

ATtiny1616 は柔軟なピン再配置機能（PORTMUX）を備えていますが、ADX ボードではオンボード回路（`CORE-U` では CH342K デュアル UART・eFuse フラグ・LED、`CORE-A5` では RS-485 トランシーバー・LED）に計 7 ピンが専用接続されています。  
そのため、ファームウェア開発時は以下の **必須設定・禁止設定**（Microchip データシート Table 5-1 / Section 15.3.2 準拠）を遵守してください。

### ① PORTMUX 設定ガイドライン一覧

| レジスタ | ビット | 機能 | ADX 推奨/必須設定 | 設定値 | Arduino (`megaTinyCore`) | 理由・動作解説 |
| :--- | :--- | :--- | :--- | :---: | :--- | :--- |
| **PORTMUX.CTRLB** | **SPI0** (bit 2) | SPI0 通信ピン | **Alternate 必須** | `1` | `SPI.swap(1)` | Default（PA1..PA4）はオンボード回路で占有されているため使用不可。IDC バスの **`P07`〜`P10`（PC3〜PC0 / Pins 12〜15）** を使用。 |
| **PORTMUX.CTRLB** | **TWI0** (bit 4) | I2C (TWI0) 通信ピン | **Default 必須** | `0` | `Wire.swap(0)` (既定) | Default で IDC バスの **`P05`/`P06`（PB0/PB1 / Pins 9, 10）** に接続。Alternate（PA1/PA2）はオンボード回路と衝突するため使用禁止。 |
| **PORTMUX.CTRLB** | **USART0** (bit 0) | USART0 通信ピン | **用途切替** | `0` / `1` | `Serial.swap(0)` / `Serial.swap(1)` | `0` (Default): IDC 外部 UART **`P03`/`P04`（PB2/PB3 / Pins 7, 8）**<br>`1` (Alternate): オンボード **CH342K / RS-485（PA1/PA2）** |
| **PORTMUX.CTRLC** | **TCA00〜05** | TCA0 PWM 出力 | **任意** | 表参照 | — | モード（Normal / Split）および使用ピンに応じて動的設定可能。 |
| **PORTMUX.CTRLD** | **TCB0** (bit 0) | TCB0 PWM 出力 | **任意** | `0` / `1` | — | `0`: PA5（オンボード側）<br>`1`: **`P10`（PC0 / IDC Pin 15）** |
| **PORTMUX.CTRLD** | **TCB1** (bit 1) | TCB1 PWM 出力 | **Default** | `0` | — | Default で **`P02`（PA3 / IDC Pin 5）** に接続。 |
| **PORTMUX.CTRLA** | **LUT0/1** (bit 4,5) | CCL 出力ピン | **用途に応じる** | `0` / `1` | — | オンボード回路との競合を避けて選択。 |
| **PORTMUX.CTRLA** | **EVOUT0〜2** | イベント出力 | **任意** | `0` / `1` | — | IDC 外部へ内部イベントクロック等を出力可能（EVOUT1: `P03`/PB2、EVOUT2: `P08`/PC2）。 |

---

## 3. ATtiny1616-MNR 全 18 ピン マルチプレクス完全マトリクス

データシート（Table 5-1: PORT Function Multiplexing）および `ADX-BUS-SPEC-v4.2` に基づく、全 GPIO の多重化機能一覧です。

| ピン名 | QFN | SOIC | ADX 接続先 (`Logical ID` / IDC Pin) | ADC0 | ADC1 | PWM / タイマー | 通信 (USART/SPI/I2C) | アナログ / 特殊 | その他 (CCL/PTC/EV) |
| :--- | :---: | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **PA0** | 19 | 16 | **`S00` / Pin 2 (`OWD / UPDI`)** | AIN0 | - | - | - | RESET / UPDI | LUT0-IN0 |
| **PA1** | 20 | 17 | **オンボード (CH342K / RS-485 TxD)** | AIN1 | - | - | USART0 TxD(Alt) / SPI MOSI(Def) / *TWI SDA(Alt)* | - | LUT0-IN1 |
| **PA2** | 1 | 18 | **オンボード (CH342K / RS-485 RxD)** | AIN2 | - | - | USART0 RxD(Alt) / SPI MISO(Def) / *TWI SCL(Alt)* | - | EVOUT0 / LUT0-IN2 |
| **PA3** | 2 | 19 | **`P02` / Pin 5 (`EXTCLK`)** | **AIN3** | - | TCA0 WO3(Def) / **TCB1 WO** | USART0 XCK(Alt) / SPI SCK(Def) | **EXTCLK** | - |
| **PA4** | 5 | 2 | **オンボード (RS-485 DE&/RE / LED候補)** | AIN4 | AIN0 | TCA0 WO4(Def) / **TCD0 WOA** | **USART0 XDIR(Alt)** / SPI SS(Def) | - | X0/Y0 / LUT0-OUT(Def) |
| **PA5** | 6 | 3 | **オンボード (LED候補 / 内部予約)** | AIN5 | AIN1 | TCA0 WO5(Def) / **TCB0 WO(Def)** / **TCD0 WOB** | - | **VREFA** / AC0 OUT / AC2 AINN0 | X1/Y1 |
| **PA6** | 7 | 4 | **`P01` / Pin 3 (`AIN / DAC`)** | **AIN6** | **AIN2** | - | - | **DAC0 OUT** / AC0–2 AIN | X2/Y2 |
| **PA7** | 8 | 5 | **オンボード (eFuse FLG / 内部予約)** | AIN7 | AIN3 | - | - | AC0–2 AIN | X3/Y3 / LUT1-OUT(Def) |
| **PB0** | 14 | 11 | **`P05` / Pin 9 (`I2C_SCL`)** | **AIN11** | - | **TCA0 WO0(Def)** | **TWI0 SCL(Def)** / USART0 XDIR(Def) | AC0/1 AIN | X5/Y5 |
| **PB1** | 13 | 10 | **`P06` / Pin 10 (`I2C_SDA`)** | **AIN10** | - | **TCA0 WO1(Def)** | **TWI0 SDA(Def)** / USART0 XCK(Def) | AC0 AIN | X4/Y4 |
| **PB2** | 12 | 9 | **`P03` / Pin 7 (`UART_TX`)** | - | - | **TCA0 WO2(Def)** | **USART0 TxD(Def)** | AC1 OUT / TOSC2 | EVOUT1 |
| **PB3** | 11 | 8 | **`P04` / Pin 8 (`UART_RX`)** | - | - | **TCA0 WO0(Alt)** | **USART0 RxD(Def)** | AC0 OUT / TOSC1 | - |
| **PB4** | 10 | 7 | **オンボード (LED)** | AIN9 | - | TCA0 WO1(Alt) | - | AC1/2 AIN | X13/Y13 / LUT0-OUT(Alt) |
| **PB5** | 9 | 6 | **オンボード (LED)** | AIN8 | - | TCA0 WO2(Alt) | - | CLKOUT / AC1/2 AIN | X12/Y12 |
| **PC0** | 15 | 12 | **`P10` / Pin 15 (`SPI_SCK`)** | - | **AIN6** | **TCB0 WO(Alt)** / **TCD0 WOC** | **SPI0 SCK(Alt)** | - | X6/Y6 |
| **PC1** | 16 | 13 | **`P09` / Pin 14 (`SPI_MISO`)** | - | **AIN7** | **TCD0 WOD** | **SPI0 MISO(Alt)** | - | X7/Y7 / LUT1-OUT(Alt) |
| **PC2** | 17 | 14 | **`P08` / Pin 13 (`SPI_MOSI`)** | - | **AIN8** | - | **SPI0 MOSI(Alt)** | - | EVOUT2 / X8/Y8 |
| **PC3** | 18 | 15 | **`P07` / Pin 12 (`SPI_CS`)** | - | **AIN9** | **TCA0 WO3(Alt)** | **SPI0 SS(Alt)** | - | X9/Y9 / LUT1-IN0 |

※ピン機能表記の `(Def)` は Default 位置、`(Alt)` は Alternate（PORTMUX 切替後）を示します。

---

## 4. ADX バス 逆引きリファレンス (ペリフェラル別)

拡張 Card 開発やファームウェア実装時に、論理ピン ID（`P01`〜`P10` / コードマクロ `ADX_P01`〜`ADX_P10`）から ATtiny1616 固有の追加機能を素早く引くための逆引き表です。

### ① PWM / タイマー出力機能

ATtiny1616 には 3 種類の独立したタイマー（TCA0, TCBn, TCD0）が搭載されており、`P01`〜`P10` のうち 8 本から PWM 出力が可能です。

| Logical ID | IDC Pin | コードマクロ | MCU ピン | 対応タイマー・PWM チャネル | PORTMUX 設定条件 | 備考・特徴 |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P02`** | Pin 5 | `ADX_P02` | PA3 | **TCB1 WO**<br>TCA0 WO3 (Def) | `PORTMUX.CTRLD.TCB1 = 0`<br>`PORTMUX.CTRLC.TCA03 = 0` | 16-bit PWM (TCB1) / EXTCLK 兼用 |
| **`P03`** | Pin 7 | `ADX_P03` | PB2 | **TCA0 WO2 (Def)** | `PORTMUX.CTRLC.TCA02 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / UART TxD 兼用 |
| **`P04`** | Pin 8 | `ADX_P04` | PB3 | **TCA0 WO0 (Alt)** | `PORTMUX.CTRLC.TCA00 = 1` | 8-bit PWM (Split) / UART RxD 兼用 |
| **`P05`** | Pin 9 | `ADX_P05` | PB0 | **TCA0 WO0 (Def)** | `PORTMUX.CTRLC.TCA00 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / I2C SCL 兼用 |
| **`P06`** | Pin 10 | `ADX_P06` | PB1 | **TCA0 WO1 (Def)** | `PORTMUX.CTRLC.TCA01 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / I2C SDA 兼用 |
| **`P07`** | Pin 12 | `ADX_P07` | PC3 | **TCA0 WO3 (Alt)** | `PORTMUX.CTRLC.TCA03 = 1` | 8-bit PWM (Split) / SPI CS 兼用 |
| **`P09`** | Pin 14 | `ADX_P09` | PC1 | **TCD0 WOD** | TCD0 制御レジスタ有効化 | **高周波 12-bit PWM (TCD0)** / SPI MISO 兼用 |
| **`P10`** | Pin 15 | `ADX_P10` | PC0 | **TCD0 WOC**<br>**TCB0 WO (Alt)** | TCD0 制御レジスタ有効化<br>`PORTMUX.CTRLD.TCB0 = 1` | **高周波 12-bit PWM (TCD0)** / 16-bit PWM (TCB0) / SPI SCK 兼用 |

> **TCD0 (Timer Type D) について:**  
> 最大 48 MHz（PLL または非同期クロック）で駆動可能な高周波タイマーです。ADX バスからは **`P09`（Pin 14: WOD）**、**`P10`（Pin 15: WOC）** の 2 チャンネルを取り出すことができ、高分解能 PWM や電源制御・ハーフブリッジ駆動等に適しています。

---

### ② アナログ機能 (ADC0 / ADC1 / DAC0 / コンパレータ)

ADX 規格で全 Core 共通保証されるアナログ枠は **`P01`（Pin 3）** ですが、ATtiny1616 実チップ上は **計 8 本**（うち Arduino 標準 `analogRead()` 対応は **`ADC0` の 4 本**）でアナログ計測が可能です。

| Logical ID | IDC Pin | コードマクロ | MCU ピン | アナログ機能 | Arduino (`megaTinyCore`) `analogRead()` | 説明 |
| :---: | :---: | :--- | :---: | :--- | :---: | :--- |
| **`P01`** | Pin 3 | `ADX_P01` | PA6 | **DAC0 OUT**<br>**ADC0 AIN6** / ADC1 AIN2<br>AC0–2 AIN | **◎ 標準対応** | **全 Core 保証のコプレーナ保護アナログ枠**。<br>**8-bit DAC 真アナログ出力** / 10-bit ADC 入力 |
| **`P02`** | Pin 5 | `ADX_P02` | PA3 | **ADC0 AIN3** | **◎ 標準対応** | 追加 10-bit ADC 入力（EXTCLK 未使用時） |
| **`P05`** | Pin 9 | `ADX_P05` | PB0 | **ADC0 AIN11** / AC0,1 AIN | **◎ 標準対応** | 追加 10-bit ADC 入力（I2C SCL 未使用時） |
| **`P06`** | Pin 10 | `ADX_P06` | PB1 | **ADC0 AIN10** / AC0 AIN | **◎ 標準対応** | 追加 10-bit ADC 入力（I2C SDA 未使用時） |
| **`P07`** | Pin 12 | `ADX_P07` | PC3 | **ADC1 AIN9** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI CS 兼用） |
| **`P08`** | Pin 13 | `ADX_P08` | PC2 | **ADC1 AIN8** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI MOSI 兼用） |
| **`P09`** | Pin 14 | `ADX_P09` | PC1 | **ADC1 AIN7** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI MISO 兼用） |
| **`P10`** | Pin 15 | `ADX_P10` | PC0 | **ADC1 AIN6** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI SCK 兼用） |

> **\*`ADC1`（`P07`〜`P10` / `PC0`〜`PC3`）の注意点:**  
> `megaTinyCore` の標準 `analogRead()` は `ADC0` のみに対応しています。`P07`〜`P10` をアナログ入力として読む場合は、`ADC1` レジスタ（`ADC1.MUXPOS`, `ADC1.COMMAND`, `ADC1.RES` 等）を直接操作してください。

---

### ③ イベントシステム (EVSYS) & 割り込み・クロック出力

| Logical ID | IDC Pin | コードマクロ | MCU ピン | 特殊機能 | PORTMUX / 設定 | 説明 |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P02`** | Pin 5 | `ADX_P02` | PA3 | **EXTCLK** | CLKCTRL 設定 | 外部クロック発振器入力（システムクロック源） |
| **`P03`** | Pin 7 | `ADX_P03` | PB2 | **EVOUT1** | `PORTMUX.CTRLA.EVOUT1 = 1` | イベントシステム出力（タイマーイベントや周波数のモニタ出力） |
| **`P08`** | Pin 13 | `ADX_P08` | PC2 | **EVOUT2** | `PORTMUX.CTRLA.EVOUT2 = 1` | イベントシステム出力 |
| `P01`〜`P10` | 全信号 | — | PA/PB/PC | **外部割り込み** | `PORTx.PINnCTRL` | 全 GPIO でピン変化割り込み対応（※`P03`/PB2、`P08`/PC2、`P01`/PA6 は完全非同期検知に対応） |

---

## 5. オンボード専用・内部予約ピンの設計意図と保護

IDC に引き出されていない 7 ピン（オンボード専用回路および内部予約）の役割です（ボード種別により一部割当が異なります）。

| ピン名 | CORE-U (USB 標準) | CORE-A5 / A24 (RS-485 産業用) | ファームウェア上の注意点 |
| :---: | :--- | :--- | :--- |
| **PA1** | CH342K Port 1 TxD | TDA51S485HC DI (RS-485 送信) | `PORTMUX.CTRLB.USART0 = 1` (`Serial.swap(1)`) 時に USART0 送信ピンとして機能。 |
| **PA2** | CH342K Port 1 RxD | TDA51S485HC RO (RS-485 受信) | 同上。RS-485 機では CCL LUT0-IN2 に直結（OTW Break リセット対応）。 |
| **PA4** | オンボード LED 候補 / 予備 | TDA51S485HC DE & /RE | RS-485 機では USART0 Alternate の `XDIR` 出力（自動方向切替）。 |
| **PA5** | オンボード LED 候補 / 予備 | 内部予約 (N.C.) | 未使用時は `PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc` で入力バッファ遮断。 |
| **PA7** | eFuse 過電流フラグ (`FLG`) | 内部予約 (N.C.) | CORE-U では Active-LOW（10kΩ プルアップ）。未使用機は入力バッファ遮断。 |
| **PB4** | オンボード LED (`R/Y/W` 枠) | オンボード赤色 LED | Active HIGH 出力。 |
| **PB5** | オンボード LED (`R/Y/W` 枠) | オンボード青色 LED | Active HIGH 出力。 |
