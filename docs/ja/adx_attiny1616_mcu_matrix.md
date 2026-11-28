<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX ATtiny1616 ペリフェラル機能 & PORTMUX 統合リファレンス

[ [English](../en/adx_attiny1616_mcu_matrix.md) | **日本語** ]

## 1. 概要 (Overview)
本ドキュメントは、Microchip 社製 **ATtiny1616-MNR (QFN-20)** を搭載した ADX フラッグシップボード **CORE-U (rev6)** における、MCU 全 18 I/O ピンのペリフェラル多重化機能（Multiplexing）および **PORTMUX（ポートマルチプレクサ）レジスタ** の設定仕様・デザインルールを定義する統合リファレンスです。

共通バスの物理ピン配置・論理ピン ID（`P01`〜`P11`）・シールド構造については、[ADX 20ピンバス仕様書 (SSOT: v5.0)](../spec/ADX_PINOUT.md) および [設計・解説ガイド](ADX_pinout.md) を参照してください。本マッピングの英文 SSOT は [docs/spec/mcu_mapping/attiny1616.md](../spec/mcu_mapping/attiny1616.md) です。

---

## 2. ADX CORE-U (rev6) における PORTMUX デザインルール (Design Rules)

`ADX-BUS-SPEC-v5.0` / `CORE-U rev6` では、絶縁 RS-485 / LIN を拡張 CARD 側へ移管したことに伴い、**Alternate `USART0`（`PA1`/`PA2`/`PA4`）を IDC 拡張バス側**、**Default `USART0`（`PB2`/`PB3`）をオンボード USB (`CH342K` Port 1)** に割り当てています。  
ファームウェア開発時は以下の設定ルール（Microchip データシート Table 5-1 / Section 15.3.2 準拠）を遵守してください。

### ① PORTMUX 設定ガイドライン一覧

| レジスタ | ビット | 機能 | ADX 推奨/必須設定 | 設定値 | Arduino (`megaTinyCore`) | 理由・動作解説 |
| :--- | :--- | :--- | :--- | :---: | :--- | :--- |
| **PORTMUX.CTRLB** | **USART0** (bit 0) | USART0 通信ピン | **用途切替** | `0` *(初期)* / `1` | `Serial.swap(0)` / `Serial.swap(1)` | `0` (Default / リセット初期値): **オンボード USB (`PB2`/`PB3` → `CH342K` Port 1)**。`Serial.begin()` のみで即動作。<br>`1` (Alternate): **IDC フィールドバス `P04`/`P05`（`PA1`/`PA2`）＋ `P03` ハード `XDIR`（`PA4`）**。同時に `PB2`/`PB3` を **SoftwareSerial** として再利用可能。 |
| **PORTMUX.CTRLB** | **SPI0** (bit 2) | SPI0 通信ピン | **Alternate 必須** | `1` | `SPI.swap(1)` | Default（`PA1`..`PA4`）は IDC UART / EXTCLK と重複。IDC バスの **`P08`〜`P11`（`PC3`〜`PC0` / Pins 12〜15）** を使用。 |
| **PORTMUX.CTRLB** | **TWI0** (bit 4) | I2C (TWI0) 通信ピン | **Default 必須** | `0` | `Wire.swap(0)` *(既定)* | Default で IDC バスの **`P06`/`P07`（`PB0`/`PB1` / Pins 9, 10）** に接続。Alternate（`PA1`/`PA2`）は IDC UART と衝突するため使用禁止。 |
| **PORTMUX.CTRLC** | **TCA01** (bit 1) | `LED_MCU` PWM 出力 | **Alternate (シングル)** | `1` | `analogWrite(ADX_LED, v)` | `PB4`（`ADX_LED`）へ 16-bit ハード PWM を出力（I2C・UART と無干渉）。 |
| **PORTMUX.CTRLD** | **TCB0** (bit 0) | TCB0 PWM 出力 | **任意** | `1` *(バス出力時)* | — | `0`: `PA5`（オンボード `VBUS_VSENSE`）<br>`1`: **`P11`（`PC0` / IDC Pin 15）** |
| **PORTMUX.CTRLD** | **TCB1** (bit 1) | TCB1 PWM 出力 | **Default** | `0` | — | Default で **`P02`（`PA3` / IDC Pin 4）** に接続。 |
| **PORTMUX.CTRLA** | **LUT0/1** (bit 4,5) | CCL 出力ピン | **用途に応じる** | `0` / `1` | — | `LUT0-OUT (Def)` = `P03`（`PA4`）、`LUT1-OUT (Alt)` = `P10`（`PC1`）。 |
| **PORTMUX.CTRLA** | **EVOUT0〜2** | イベント出力 | **任意** | `0` / `1` | — | `EVOUT0`: `P05`（`PA2`）、`EVOUT2`: `P09`（`PC2`）。 |

---

## 3. ATtiny1616-MNR 全 18 ピン マルチプレクス完全マトリクス (`v5.0`)

データシート（Table 5-1）および `ADX-BUS-SPEC-v5.0` / `CORE-U rev6` に基づく全 GPIO の多重化機能一覧です。

| ピン名 | QFN | SOIC | ADX 接続先 (`Logical ID` / IDC Pin) | ADC0 | ADC1 | PWM / タイマー | 通信 (USART/SPI/I2C) | アナログ / 特殊 | その他 (CCL/PTC/EV) |
| :--- | :---: | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **PA0** | 19 | 16 | **オンボード専用 (CH342K P0 UPDI)** | AIN0 | - | - | - | RESET / UPDI | LUT0-IN0 |
| **PA1** | 20 | 17 | **`P04` / Pin 7 (`UART_TX`)** | **AIN1** | - | - | **USART0 TxD(Alt)** / SPI MOSI(Def) | - | LUT0-IN1 |
| **PA2** | 1 | 18 | **`P05` / Pin 8 (`UART_RX`)** | **AIN2** | - | - | **USART0 RxD(Alt)** / SPI MISO(Def) | - | **EVOUT0** / **LUT0-IN2** / **Async(Px2)** |
| **PA3** | 2 | 19 | **`P02` / Pin 4 (`EXTCLK`)** | **AIN3** | - | TCA0 WO3(Def) / **TCB1 WO** | USART0 XCK(Alt) / SPI SCK(Def) | **EXTCLK** | - |
| **PA4** | 5 | 2 | **`P03` / Pin 6 (`UART_DE`)** | **AIN4** | **AIN0** | TCA0 WO4(Def) / **TCD0 WOA** | **USART0 XDIR(Alt)** / SPI SS(Def) | - | X0/Y0 / **LUT0-OUT(Def)** |
| **PA5** | 6 | 3 | **オンボード (`ADX_VBUS_VSENSE`)** | **AIN5** | AIN1 | TCA0 WO5(Def) / TCB0 WO(Def) / TCD0 WOB | - | VREFA / AC0 OUT | X1/Y1 |
| **PA6** | 7 | 4 | **`P01` / Pin 2 (`AIN / DAC`)** | **AIN6** | **AIN2** | - | - | **DAC0 OUT** / AC0–2 AIN | X2/Y2 / **Async(Px6)** |
| **PA7** | 8 | 5 | **オンボード (`ADX_VBUS_ISENSE`)** | **AIN7** | AIN3 | - | - | AC0–2 AIN | X3/Y3 / LUT1-OUT(Def) |
| **PB0** | 14 | 11 | **`P06` / Pin 9 (`I2C_SCL`)** | **AIN11** | - | **TCA0 WO0(Def)** | **TWI0 SCL(Def)** / USART0 XDIR(Def) | AC0/1 AIN | X5/Y5 |
| **PB1** | 13 | 10 | **`P07` / Pin 10 (`I2C_SDA`)** | **AIN10** | - | **TCA0 WO1(Def)** | **TWI0 SDA(Def)** / USART0 XCK(Def) | AC0 AIN | X4/Y4 |
| **PB2** | 12 | 9 | **オンボード (`ADX_USB_TX`: CH342K P1)** | - | - | TCA0 WO2(Def) | **USART0 TxD(Def)** / SoftSerial TX | AC1 OUT / TOSC2 | EVOUT1 / Async(Px2) |
| **PB3** | 11 | 8 | **オンボード (`ADX_USB_RX`: CH342K P1)** | - | - | TCA0 WO0(Alt) | **USART0 RxD(Def)** / SoftSerial RX | AC0 OUT / TOSC1 | - |
| **PB4** | 10 | 7 | **オンボード (`ADX_LED`: ステータス灯)** | AIN9 | - | **TCA0 WO1(Alt)** | - | AC1/2 AIN | X13/Y13 / LUT0-OUT(Alt) |
| **PB5** | 9 | 6 | **オンボード (`ADX_USB_COM_DET`)** | AIN8 | - | TCA0 WO2(Alt) | - | CLKOUT / AC1/2 AIN | X12/Y12 / 深睡眠復帰(BOTHEDGES) |
| **PC0** | 15 | 12 | **`P11` / Pin 15 (`SPI_SCK`)** | - | **AIN6** | **TCB0 WO(Alt)** / **TCD0 WOC** | **SPI0 SCK(Alt)** | - | X6/Y6 |
| **PC1** | 16 | 13 | **`P10` / Pin 14 (`SPI_MISO`)** | - | **AIN7** | **TCD0 WOD** | **SPI0 MISO(Alt)** | - | X7/Y7 / LUT1-OUT(Alt) |
| **PC2** | 17 | 14 | **`P09` / Pin 13 (`SPI_MOSI`)** | - | **AIN8** | - | **SPI0 MOSI(Alt)** | - | **EVOUT2** / X8/Y8 / **Async(Px2)** |
| **PC3** | 18 | 15 | **`P08` / Pin 12 (`SPI_CS`)** | - | **AIN9** | **TCA0 WO3(Alt)** | **SPI0 SS(Alt)** | - | X9/Y9 / LUT1-IN0 |

※ピン機能表記の `(Def)` は Default 位置、`(Alt)` は Alternate（PORTMUX 切替後）を示します。

---

## 4. ADX バス 逆引きリファレンス (ペリフェラル別)

拡張 Card 開発やファームウェア実装時に、論理ピン ID（`P01`〜`P11` / コードマクロ `ADX_P01`〜`ADX_P11`）から ATtiny1616 固有の追加機能を素早く引くための逆引き表です。

### ① PWM / タイマー出力機能

ATtiny1616 には 3 種類の独立したタイマー（TCA0, TCBn, TCD0）が搭載されており、`P01`〜`P11` のうち **7 本** からハードウェア PWM 出力が可能です（うち **3 本が 48 MHz 12-bit `TCD0`** 対応）。

| Logical ID | IDC Pin | コードマクロ | MCU ピン | 対応タイマー・PWM チャネル | PORTMUX 設定条件 | 備考・特徴 |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P02`** | Pin 4 | `ADX_P02` | PA3 | **TCB1 WO**<br>TCA0 WO3 (Def) | `PORTMUX.CTRLD.TCB1 = 0`<br>`PORTMUX.CTRLC.TCA03 = 0` | 16-bit PWM (TCB1) / EXTCLK 兼用 |
| **`P03`** | Pin 6 | `ADX_P03` | PA4 | **TCD0 WOA**<br>TCA0 WO4 (Def) | TCD0 制御レジスタ有効化<br>`PORTMUX.CTRLC.TCA04 = 0` | **高周波 12-bit PWM (TCD0)** / `UART_DE` 兼用 |
| **`P06`** | Pin 9 | `ADX_P06` | PB0 | **TCA0 WO0 (Def)** | `PORTMUX.CTRLC.TCA00 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / I2C SCL 兼用 |
| **`P07`** | Pin 10 | `ADX_P07` | PB1 | **TCA0 WO1 (Def)** | `PORTMUX.CTRLC.TCA01 = 0` | 16-bit PWM (Normal) / 8-bit PWM (Split) / I2C SDA 兼用 |
| **`P08`** | Pin 12 | `ADX_P08` | PC3 | **TCA0 WO3 (Alt)** | `PORTMUX.CTRLC.TCA03 = 1` | 8-bit PWM (Split) / SPI CS 兼用 |
| **`P10`** | Pin 14 | `ADX_P10` | PC1 | **TCD0 WOD** | TCD0 制御レジスタ有効化 | **高周波 12-bit PWM (TCD0)** / SPI MISO 兼用 |
| **`P11`** | Pin 15 | `ADX_P11` | PC0 | **TCD0 WOC**<br>**TCB0 WO (Alt)** | TCD0 制御レジスタ有効化<br>`PORTMUX.CTRLD.TCB0 = 1` | **高周波 12-bit PWM (TCD0)** / 16-bit PWM (TCB0) / SPI SCK 兼用 |

> **TCD0 (Timer Type D) について:**  
> 最大 48 MHz で駆動可能な高周波タイマーです。`v5.0` では **`P03`（Pin 6: WOA）**、**`P10`（Pin 14: WOD）**、**`P11`（Pin 15: WOC）** の **3 チャンネル** を ADX バスから取り出せます。

---

### ② アナログ機能 (ADC0 / ADC1 / DAC0 / コンパレータ)

`v5.0` では ATtiny1616 の IDC バス **全 11 ピン (`P01`〜`P11`) がすべて ADC 計測に対応** します（うち Arduino 標準 `analogRead()` 対応の **`ADC0` が 7 本** に増加！）。

| Logical ID | IDC Pin | コードマクロ | MCU ピン | アナログ機能 | Arduino (`megaTinyCore`) `analogRead()` | 説明 |
| :---: | :---: | :--- | :---: | :--- | :---: | :--- |
| **`P01`** | Pin 2 | `ADX_P01` | PA6 | **DAC0 OUT**<br>**ADC0 AIN6** / ADC1 AIN2<br>AC0–2 AIN | **◎ 標準対応** | **全 Core 保証の真のコプレーナ保護アナログ枠**。<br>**8-bit DAC 真アナログ出力** / 10-bit ADC 入力 |
| **`P02`** | Pin 4 | `ADX_P02` | PA3 | **ADC0 AIN3** | **◎ 標準対応** | 追加 10-bit ADC 入力（EXTCLK 未使用時） |
| **`P03`** | Pin 6 | `ADX_P03` | PA4 | **ADC0 AIN4** / ADC1 AIN0 | **◎ 標準対応** | 追加 10-bit ADC 入力（`UART_DE` 未使用時） |
| **`P04`** | Pin 7 | `ADX_P04` | PA1 | **ADC0 AIN1** | **◎ 標準対応** | 追加 10-bit ADC 入力（UART TX 未使用時） |
| **`P05`** | Pin 8 | `ADX_P05` | PA2 | **ADC0 AIN2** | **◎ 標準対応** | 追加 10-bit ADC 入力（UART RX 未使用時） |
| **`P06`** | Pin 9 | `ADX_P06` | PB0 | **ADC0 AIN11** / AC0,1 AIN | **◎ 標準対応** | 追加 10-bit ADC 入力（I2C SCL 未使用時） |
| **`P07`** | Pin 10 | `ADX_P07` | PB1 | **ADC0 AIN10** / AC0 AIN | **◎ 標準対応** | 追加 10-bit ADC 入力（I2C SDA 未使用時） |
| **`P08`** | Pin 12 | `ADX_P08` | PC3 | **ADC1 AIN9** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI CS 兼用） |
| **`P09`** | Pin 13 | `ADX_P09` | PC2 | **ADC1 AIN8** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI MOSI 兼用） |
| **`P10`** | Pin 14 | `ADX_P10` | PC1 | **ADC1 AIN7** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI MISO 兼用） |
| **`P11`** | Pin 15 | `ADX_P11` | PC0 | **ADC1 AIN6** | △ ADC1 専用\* | ADC1 10-bit ADC 入力（SPI SCK 兼用） |

> **\*`ADC1`（`P08`〜`P11` / `PC0`〜`PC3`）の注意点:**  
> `megaTinyCore` の標準 `analogRead()` は `ADC0` のみに対応しています。`P08`〜`P11` をアナログ入力として読む場合は、`ADC1` レジスタ（`ADC1.MUXPOS`, `ADC1.COMMAND`, `ADC1.RES` 等）を直接操作してください。

---

### ③ イベントシステム (EVSYS)・CCL・スリープ起床

| Logical ID | IDC Pin | コードマクロ | MCU ピン | 特殊機能 | PORTMUX / 設定 | 説明 |
| :---: | :---: | :--- | :---: | :--- | :--- | :--- |
| **`P01`** | Pin 2 | `ADX_P01` | PA6 | **完全非同期割込 (`Px6`)** | `PORTA.PIN6CTRL` | Power-Down からの任意エッジ起床・センサーイベント監視 |
| **`P02`** | Pin 4 | `ADX_P02` | PA3 | **EXTCLK** | CLKCTRL 設定 | 外部クロック発振器入力（システムクロック源） |
| **`P03`** | Pin 6 | `ADX_P03` | PA4 | **CCL `LUT0-OUT`** | `PORTMUX.CTRLA.LUT0 = 0` | CCL 演算結果の外部出力（`UART_DE` 未使用時） |
| **`P05`** | Pin 8 | `ADX_P05` | PA2 | **CCL `LUT0-IN2` / `EVOUT0` / 完全非同期割込 (`Px2`)** | `CCL` / `USART0 SFDEN` | RS-485/LIN Break ハード監視・Standby/Power-Down 起床 |
| **`P06`/`P07`** | Pin 9,10 | `ADX_P06`/`P07` | PB0/PB1 | **TWI0 アドレス一致起床** | `TWI0.SCTRLA` | Power-Down 睡眠中の I2C スレーブアドレス一致ハード起床 |
| **`P09`** | Pin 13 | `ADX_P09` | PC2 | **`EVOUT2` / 完全非同期割込 (`Px2`)** | `PORTMUX.CTRLA.EVOUT2 = 1` | イベントシステム出力・完全非同期エッジ検知 |
| `P01`〜`P11` | 全信号 | — | PA/PB/PC | **外部割り込み** | `PORTx.PINnCTRL` | 全 11 ピンでピン変化割込対応（`BOTHEDGES`/`LEVEL` で Power-Down 起床可） |

---

## 5. オンボード専用ピンの役割と保護（CORE-U rev6）

IDC バスへ引き出されない 7 ピンは、すべて CORE-U rev6 のオンボード中核機能に割り当てられています（旧 CORE-A5 等の派生ボード構成は `hardware/archive/CORE-A5/` を参照）。

| ピン名 | CORE-U rev6 マクロ | 接続先・役割 | ファームウェア上の注意点 |
| :---: | :--- | :--- | :--- |
| **PA0** | *(非公開)* | `SN74LV4066A` → `CH342K` Port 0 (**UPDI**) | オンボード専用（IDC 非露出・47kΩ PU）。スリープ中も UPDI 強制介入可能。 |
| **PB2** | `ADX_USB_TX` | `SN74LV4066A` → `CH342K` Port 1 RX | リセット初期値 (`Serial.swap(0)`) でハード UART TX。`Serial.swap(1)` 時は SoftwareSerial TX。 |
| **PB3** | `ADX_USB_RX` | `SN74LV4066A` → `CH342K` Port 1 TX | 同上（MCU 側 100kΩ PU）。Mode 1/2 は `SN74LV4066A` で完全隔離。 |
| **PA5** | `ADX_VBUS_VSENSE` | `VBUS_PROT` 1/2 分圧入力 | `ADC0 AIN5`（0〜6.60V）。全 3 Mode で幹線電圧を常時監視可能。 |
| **PA7** | `ADX_VBUS_ISENSE` | `TP181A2-CR` 出力（10mΩ シャント） | `ADC0 AIN7`（1.00 V/A）。Mode 1 では `≈ 0 mA`、Mode 2/3 では `≥ 6 mA`。 |
| **PB4** | `ADX_LED` | `LED_MCU`（`2N7002`・5V 駆動 110℃ LED） | Active-HIGH、`TCA0 WO1*` ハード PWM 対応（100kΩ PD）。 |
| **PB5** | `ADX_USB_COM_DET` | `USB_COM_EN`（`10kΩ` 保護抵抗経由） | Active-HIGH。`PORT_ISC_BOTHEDGES_gc` で Power-Down 深睡眠から `USB-COM` 挿入直接復帰可能。 |
