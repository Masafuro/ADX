<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Pinout 仕様書 (v2)

[ English (../en/ADX_pinout.md) | **日本語** ]

## 1. 概要
本ドキュメントは、Microchip社製 ATtiny1616-MNR を搭載したマイコンボード「ADX」のIDC 2x10 コネクタ（回路図上のリファレンス例: CN2）のピンアウト、およびオンボードペリフェラル（RS-485、CH342K、UPDI）と PORTMUX 機能の接続関係を定義する仕様書（v2）です。本規格では、コネクタ端子番号を **「IDC Pin 1〜20」** と表記します。

## 2. ATtiny1616 ピン占有構造と PORTMUX 切り替え仕様

### ① ボード側占有ピン（IDC非引き出し：計7ピン）
マイコンのGPIO pinのうち、以下の7ピンはオンボード回路専用として配線されます。

- **RS-485 トランシーバー回路（4ピン）**:
  - `PA1`: USART0 Default TxD (DI)
  - `PA2`: USART0 Default RxD (RO)
  - `PA4`: USART0 Default XDIR (DE - Driver Enable)
  - `PA7`: GPIO制御 (RE - Receiver Enable)
- **CH342K / UPDI デバッグ回路（3ピン）**:
  - `PA0`: UPDI / RESET (書き込み・デバッグ専用)
  - `PB4`: Software Serial (CH342K TX/RX通信用)
  - `PB5`: Software Serial (CH342K TX/RX通信用)

### ② PORTMUX レジスタによる UART 排他切替制御
USART0 モジュールは `PORTMUX.CTRLB` レジスタの設定により、通信ピンの位置を動的にハードウェア切替可能です。

- **`PORTMUX.CTRLB.USART0 = 0` (Default位置)**:
  - 通信経路はオンボード **RS-485 トランシーバー（PA1 / PA2）** に接続されます。
  - RS-485自動方向制御（`XDIR` / `PA4`）およびLIN機能（`LINAUTO` によるBreak検出・自動ボーレート調整）が有効に機能します。
- **`PORTMUX.CTRLB.USART0 = 1` (Alternate位置)**:
  - 通信経路は **IDC コネクタ（PB2 / PB3: IDC Pin 11, 12）** へ切り替わり、外部直結UART通信が有効となります。
  - オンボードRS-485とは排他的に切り替わるため、信号衝突なく用途に応じた選択が可能です。

---

## 3. IDC 2x10 コネクタ ピン配置仕様 (v2)

### 主な電気的・構造的メリット
1. **電源供給（Pins 1〜3）**: 3ピンのVDDにより、5V / 2A 級の給電能力を確保。
2. **短絡防止アイソレーション（Pin 4 N.C.）**: VDDゾーン（Pin 3）とGND（Pin 5）の間にN.C.を配置し、フラットケーブル圧接時の位置ズレや異物による電源ショート事故を物理的に防止。
3. **クロックシールド（Pin 6）**: `EXTCLK` (Pin 6) の両隣（Pin 5, 7）をGNDでサンドイッチし、高周波ノイズを完全遮蔽。
4. **アナログゾーン保護とアイソレーション（Pins 8, 9 & Pin 10 GND）**: アナログ信号 (`DAC0`, `AIN5`) を Pin 7 (GND) と新設の Pin 10 (GND) で挟み込むことで、デジタル通信線 (UART/I2C/SPI) からのノイズ干渉を隔離。
5. **バス通信群の位置維持（Pins 11〜20）**: UART (PB2/PB3)、I2C (PB0/PB1)、SPI (PC0〜PC3) の位置を変えることなくスマートに統合。

| IDC ピン | ネット名 | マイコンピン | 主な機能 / 役割 | フラットケーブル構造・シールド機能 |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **VDD** | - | **電源 (5V/2A供給対応)** | #1 (電源ゾーン) |
| **2** | **VDD** | - | **電源 (5V/2A供給対応)** | #2 (電源ゾーン) |
| **3** | **VDD** | - | **電源増強 (5V/2A供給対応)** | #3 (電源ゾーン) |
| **4** | *N.C.* | - | **アイソレーション (電源・GND短絡防止緩衝帯)** | #4 (緩衝帯) |
| **5** | **GND_5V** | - | **GND (EXTCLKガード①)** | #5 (GNDシールド) |
| **6** | **PA3/EXTCLK** | PA3 | **EXTCLK** / AIN3 | #6 (EXTCLK 信号線) |
| **7** | **GND_5V** | - | **GND (EXTCLKガード②)** | #7 (GNDシールド) |
| **8** | PA6/DAC0 | PA6 | **DAC0** 出力 / AIN6 | #8 (アナログゾーン) |
| **9** | PA5/AIN5 | PA5 | **VREFA** / AIN5 | #9 (アナログゾーン) |
| **10** | **GND_5V** | - | **GND (アナログ/デジタル離隔ガード - 新設)** | #10 (GNDシールド) |
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

> **注記 (Pin Header / Cable Mapping):**  
> ※ IDCコネクタのピン番号は、MIL規格IDC圧接コネクタの標準的な千鳥足（ジグザグ）ナンバリング（フラットケーブル芯線 #1〜#20 とピン番号 1〜20 が 1:1 に対応する配列）を想定しています。基板CAD設計（シンボル・フットプリント作成）時は、DIP型（列順ナンバリング）との取り違えにご注意ください。

---

## 4. IDC 主要ペリフェラル機能 早見サマリー

IDCコネクタ（IDC Pin 1〜20）の各ピンから利用可能な代表的ペリフェラル機能（PWM出力、アナログ入力、特殊機能）の早見一覧です。

| IDC ピン | マイコンピン | 主な通信機能 | PWM 出力可能タイマー | アナログ / 特殊機能 |
| :---: | :---: | :--- | :--- | :--- |
| **Pin 6** | PA3 | - | TCB1 / TCA0 WO3 | **EXTCLK** (外部クロック) / ADC0 AIN3 |
| **Pin 8** | PA6 | - | - | **DAC0** 出力 / ADC0 AIN6 / ADC1 AIN2 |
| **Pin 9** | PA5 | - | **TCD0 WOB** (高周波) / TCB0 / TCA0 WO5 | **VREFA** (外部基準電圧) / ADC0 AIN5 / ADC1 AIN1 |
| **Pin 11** | PB2 | **UART TxD** (Alt) | TCA0 WO2 | EVOUT1 (イベント出力) |
| **Pin 12** | PB3 | **UART RxD** (Alt) | TCA0 WO0 (Alt: Split) | TOSC1 |
| **Pin 13** | PB0 | **I2C SCL** | TCA0 WO0 | ADC0 AIN11 |
| **Pin 14** | PB1 | **I2C SDA** | TCA0 WO1 | ADC0 AIN10 |
| **Pin 16** | PC3 | **SPI SS** | TCA0 WO3 (Alt: Split) | ADC1 AIN9 |
| **Pin 17** | PC2 | **SPI MOSI** | - | EVOUT2 / ADC1 AIN8 |
| **Pin 18** | PC1 | **SPI MISO** | **TCD0 WOD** (高周波) | ADC1 AIN7 |
| **Pin 19** | PC0 | **SPI SCK** | **TCD0 WOC** (高周波) / TCB0 (Alt) | ADC1 AIN6 |

---

## 5. 関連ドキュメント
各ピンの完全なペリフェラル多重化マトリクス（CCL, PTC, コンパレータ等）および、PORTMUX レジスタ（CTRLA〜CTRLD）の詳細設定ガイドライン・デザインルールについては、以下を参照してください。

- **MCUペリフェラル詳細・PORTMUX仕様書**: [`adx_attiny1616_mcu_matrix.md`](adx_attiny1616_mcu_matrix.md)
