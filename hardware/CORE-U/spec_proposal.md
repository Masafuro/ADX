<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U 技術仕様書 (rev4 ベース)
〜 「マイコンボードには機能を載せない」・完全 SMD 化・3D プリント筐体／現場組み込み向け USB コアボード 〜

---

## 1. 概要とプロダクト位置づけ (Overview & Positioning)

### 1.1 プロダクト概要
**ADX CORE-U (rev4)** は、3D プリンターによる自作筐体や、現場の制御ボックス・カスタム装置へ電子制御を組み込むための **「筐体組み込み特化型 USB コアボード（はたらく Arduino）」** です。

度重なる設計見直し（rev0〜rev4）を経て、rev4 では以下の設計哲学に収束しました：

1. **「マイコンボード側には機能を限りなく載せない」**  
   オンボードのブザー、microSD、電源スイッチ端子台、追加の個別ピンヘッダ（XH / HY2.0 / 汎用 2.54mm ヘッダ等）をすべて廃止。拡張機能は **ADX 20P IDC リボンケーブル 1 本に一本化** し、メインボードの混乱と出荷検品工数を最小化します。
2. **「3D プリンターユーザー／組み込み開発のリアルな課題」への集中**  
   10W 級（5V 2.0A）の外部給電と PC デバッグを安全に両立しつつ、マイコンがハングしても物理的に復旧でき、シリアルモニタを開いたまま書き込みできる開発性を備えます。
3. **「全 SMD 化による低コスト・量産性」**  
   IDC コネクタや Type-C コネクタを含め表面実装（SMD）ベースで構成し、JLCPCB PCBA 込みで低原価・安定した製造性を確保します。

### 1.2 関連ドキュメント
* **rev4 コンセプト・荒見積り:** [rev4/concept.md](rev4/concept.md)
* **市場・ユーザー再定義メモ:** [rev2/consider.md](rev2/consider.md)
* **競合比較分析（rev4 準拠）:** [comparison.md](comparison.md)

---

## 2. 開発ツールチェーン仕様 (Software & Toolchain)

本ボードは、軽快な動作とデュアル CDC ポート運用を活かすため、**Arduino IDE 1.8.x（クラシック版）** および **megaTinyCore** を公式推奨開発環境とします。

```text
               【 CORE-U (rev4) 開発・給電ワークフロー 】

    [ 5V 3A 推奨 ACアダプタ ] ──▶ USB-PWR (Type-C: 5V 電源専用) ──▶ 5V 2.0A / 3.3V 生成
                                                                     (POW_LED 緑点灯)

    [ PC (Arduino IDE 1.8.x) ] ──▶ USB-COM (Type-C: 通信専用 / 非給電)
                                      │
                                      ▼
                                 [ WCH CH342K (Dual USB-UART, 3.3V 動作) ]
                                   ├─▶ Port 0: SerialUPDI ──▶ PA0 (UPDI: 強制書込)
                                   └─▶ Port 1: CDC Serial ──▶ PA1/PA2 (Alt-UART)
```

### 2.1 標準ワークフロー（完全分離された 2 本の Type-C）
ユーザーに対する接続手順は極めてシンプルです：
1. **5V 3A（または 2A 以上）の USB-C AC アダプタ** を電源用ポート（`USB-PWR`）に接続し、`POW_LED`（緑）の点灯を確認する。
2. **PC** を通信用ポート（`USB-COM`）に接続し、Arduino IDE 1.8.x + `megaTinyCore` で開発する。

### 2.2 独立 2 ポート構成のメリット (`CH342K` + `SerialUPDI`)
* **シリアルモニタを開いたまま書き込み可能:** Port 0（SerialUPDI）と Port 1（UART デバッグモニタ）が独立しているため、書き込みのたびにモニタを閉じる必要がありません。
* **ハング時の確実な復旧:** MCU のソフトウェア USB に依存せず、`CH342K` 経由のハードウェア UPDI で外部から書き込むため、スケッチ暴走時や密閉ケース内でも物理リセットボタン不要で書き換え可能です。
* **HV-UPDI 不要・工場書き込みゼロ:** `PA0` を RESET へヒューズ変更せず UPDI のまま出荷するため、工場でのブートローダ事前書き込み（`optiboot_x` 等）や 12V HV パルスによる復旧リスクを排除できます。

### 2.3 ハードウェア UART の PORTMUX 切替
ATtiny1616 の PORTMUX 機能により、用途に応じて USART0 を切り替えます：
* **`Serial.swap(1)`（PC デバッグ標準）:** `PA1 (TX)` / `PA2 (RX)` を `CH342K` Port 1 に接続し、ADX バス側と干渉しないデバッグモニタとして使用。
* **`Serial.swap(0)`（拡張バス通信）:** `PB2 (TX)` / `PB3 (RX)` を ADX 20P IDC（Pins 7/8）に接続し、外部拡張ボードと通信。

---

## 3. 基本電気仕様 (Electrical Specifications)

| 項目 | 定格 / 仕様 | 備考 |
| :--- | :--- | :--- |
| **主電源入力 (`USB-PWR`)** | **5.0V DC (定格 2.0A / 10W)** | Type-C (`01-USB_Com1`)、CC 5.1kΩ プルダウン×2、TVS 保護 |
| **パワースイッチ / eFuse** | **Diodes `AP22814AW5-7`** | ハイサイドロードスイッチ（連続最大 3A、RDS(on) 50mΩ、過電流・短絡・過熱・逆流保護） |
| **通信ポート (`USB-COM`)** | **USB 2.0 (データ通信専用)** | Type-C (`01-USB_Com`)、VBUS 非接続（PC からの給電・逆流を物理遮断）、`USBLC6-2SC6` ESD 保護 |
| **内部ロジック電源** | **3.3V DC (最大 500mA)** | Microne `ME6211C33M5G-N` LDO（`VBUS_PROT` より生成） |
| **主 MCU** | **Microchip `ATtiny1616-MNR`** | QFN-20-EP (3×3mm)、8-bit AVR (3.3V / 10MHz)、Flash 16KB / SRAM 2KB / EEPROM 256B |
| **USB ブリッジ IC** | **WCH `CH342K`** | ESSOP-10、デュアル USB-UART |
| **拡張インターフェース** | **ADX 20P IDC (`CN1`) のみ** | `321020MG0CBK00A01`（2.54mm ピッチ 2×10P SMD、3.3V LVCMOS 信号＋`VBUS_PROT`×4） |
| **オンボード LED** | **計 4 個 (電源×1 + GPIO×3)** | `POW_LED` (緑)、GPIO 駆動用 `LED_R` (赤) / `LED_Y` (黄) / `LED_W` (白) |

---

## 4. 回路アーキテクチャ (Circuit Architecture)

```text
                      【 ADX CORE-U rev4 回路ブロック図 】

    [ USB-PWR (Type-C: 01-USB_Com1) ] ──▶ [ TVS: SMF5.0CA-MS ]
    (5V 電源入力専用 / 2.0A 定格)               │
                                                ▼ (USB_VBUS)
                                    ┌─────────────────────────────────────┐
                                    │ 03-eFuse: AP22814AW5-7              │
                                    │  - Pin 5 (IN)  : USB_VBUS           │
                                    │  - Pin 4 (EN)  : 10kΩ Pull-up       │
                                    │  - Pin 3 (FLG) ──[10kΩ]──▶ PA7/FLG  │
                                    │  - Pin 1 (OUT) : VBUS_PROT          │
                                    └──────────────────┬──────────────────┘
                                                       │
                                                       ├──▶ CN1 (Pins 17-20: VBUS_PROT / 最大2.0A)
                                                       ├──▶ POW_LED (電源表示: 緑)
                                                       └──▶ [ 04-LDO: ME6211C33M5G-N (3.3V) ]
                                                                     │
                                                                     ├─▶ MCU VDD (3.3V)
                                                                     └─▶ U1: CH342K 電源 (3.3V)

    [ USB-COM (Type-C: 01-USB_Com) ] ───▶ [ USBLC6-2SC6 (ESD保護) ]
    (通信専用: VBUS ピン未接続)                 │
                                                ▼
                                       [ U1: WCH CH342K ]
                                         ├─▶ Port 0 ──▶ PA0/UPDI (SerialUPDI)
                                         └─▶ Port 1 ──▶ PA1/TX, PA2/RX (Alt-UART)

    [ オンボード視覚 UI ]
     └── 余剰 GPIO (PA4 / PA5 / PB4 / PB5 のうち 3 本) ──▶ LED_R (赤), LED_Y (黄), LED_W (白)
```

### 4.1 電源と通信の完全分離（ORing Controller レス）
* rev2〜rev3 初期で検討されていたパワーマルチプレクサ（`TPS2116`）を廃し、**`USB-COM` 側の VBUS を電源ラインから物理的に切り離す** 構成を採用しました。
* これにより、PC の USB ポートから大電流（2A）を引いて PC 側が遮断される事故や、逆流・電源優先度の混乱が構造上発生しなくなります。
* `CH342K` もボード側 `3V3` から給電する前提とすることで、主電源 OFF 時のマイコン側へのバックパワー（回り込み電流）を防ぎます（※`PC` 側 Type-C の `CC1`/`CC2` にはホスト認識用の `5.1kΩ` プルダウンをそれぞれ配置）。

### 4.2 拡張の一本化（`CN1`: ADX 20P IDC）
* 外部拡張はすべて `CN1`（`2.54mm` ピッチ `2×10P` SMD ボックスヘッダ）に集約しました。
* ブザー、スイッチ、センサー、モータードライバ、あるいは開発用の外部デバッガ等を接続したい場合は、すべて `CN1` に繋がる ADX 拡張モジュール側で受け持ちます。
* `3V3` 電源は `CN1` には出さず（`VBUS_PROT` と `GND` のみ）、拡張ボード側で 3.3V が必要な場合は拡張側で降圧する、あるいは 5V 駆動機器を直接扱う設計としています。

### 4.3 オンボード LED によるミニマルな状態表示
* 密閉前の動作確認や最低限のエラー・ステータス通知用として、GPIO から駆動できる **赤 (`LED_R`)・黄 (`LED_Y`)・白 (`LED_W`)** の 3 色ディスクリート LED と、電源表示 **緑 (`POW_LED`)** のみを搭載します。

---

## 5. ピン配分と ADX 20P IDC アサイン (Pin Assignment)

### 5.1 ATtiny1616-MNR（20ピン QFN）割り当て状況
rev4 ネットリスト（作業中データ）における 20 ピンの割り当ては以下の通りです：

| ピン番号 | ポート名 | 接続先 | 主な役割 |
| :---: | :--- | :--- | :--- |
| **1** | `PA2/RX` | `CH342K` 側予定 | USART0 Alt-RX（PC デバッグ受信） |
| **2** | `PA3/EXTCLK` | `CN1` Pin 5 | 外部クロック / 汎用 GPIO |
| **3** | `GND` | `GND` | システムグラウンド |
| **4** | `VDD` | `3V3` | 内部ロジック電源 (3.3V) |
| **5** | `PA4/PWM` | オンボード LED 候補 | PWM / CCL LUT0 / AIN4 / GPIO |
| **6** | `PA5/AIN` | オンボード LED 候補 | AIN5 / VREFA / GPIO |
| **7** | `PA6/AIN` | `CN1` Pin 3 | ADX バス標準アナログ入力 |
| **8** | `PA7/FLG` | `03-eFuse` Pin 3 | eFuse 過電流・過熱フラグ入力（`03-R2` 10kΩ プルアップ） |
| **9** | `PB5/PWM` | オンボード LED 候補 | TCB1 PWM / GPIO |
| **10** | `PB4/NEOP` | オンボード LED 候補 | GPIO（旧 NeoPixel 枠） |
| **11** | `PB3/RX` | `CN1` Pin 8 | USART0 Default-RX（外部拡張 UART） |
| **12** | `PB2/TX` | `CN1` Pin 7 | USART0 Default-TX（外部拡張 UART） |
| **13** | `PB1/SDA` | `CN1` Pin 10 | I2C SDA (TWI0) |
| **14** | `PB0/SCL` | `CN1` Pin 9 | I2C SCL (TWI0) |
| **15** | `PC0/SCK` | `CN1` Pin 15 | SPI SCK (SPI0) |
| **16** | `PC1/MISO` | `CN1` Pin 14 | SPI MISO (SPI0) |
| **17** | `PC2/MOSI` | `CN1` Pin 13 | SPI MOSI (SPI0) |
| **18** | `PC3/CS` | `CN1` Pin 12 | SPI CS (SPI0) |
| **19** | `PA0/UPDI` | `CN1` Pin 2 & `CH342K` | UPDI 書込 ＆ ADX バス共有 |
| **20** | `PA1/TX` | `CH342K` 側予定 | USART0 Alt-TX（PC デバッグ送信） |
| **21** | `EP` | `GND` | サーマルパッド |

> **Note:** オンボード 3 色 LED（`LED_R`, `LED_Y`, `LED_W`）への具体的なピン割り当ては、余剰 GPIO 4 本（`PA4`, `PA5`, `PB4`, `PB5`）の中から配線時に確定します。

### 5.2 ADX 20P IDC (`CN1`: `321020MG0CBK00A01`) ピン配列

| Pin | ネット名 | Pin | ネット名 |
| :---: | :--- | :---: | :--- |
| **1** | `GND` | **2** | `PA0/UPDI` |
| **3** | `PA6/AIN` | **4** | `GND` |
| **5** | `PA3/EXTCLK` | **6** | `GND` |
| **7** | `PB2/TX` | **8** | `PB3/RX` |
| **9** | `PB0/SCL` | **10** | `PB1/SDA` |
| **11** | `GND` | **12** | `PC3/CS` |
| **13** | `PC2/MOSI` | **14** | `PC1/MISO` |
| **15** | `PC0/SCK` | **16** | `GND` |
| **17** | `VBUS_PROT` (5V) | **18** | `VBUS_PROT` (5V) |
| **19** | `VBUS_PROT` (5V) | **20** | `VBUS_PROT` (5V) |

---

## 6. 主要部品構成 (BOM) ＆ 概算コスト

rev4 ネットリスト（[Netlist_PCB1_2026-11-18_wip.enet](rev4/Netlist_PCB1_2026-11-18_wip.enet)）に基づく主要部品一覧です：

| リファレンス | カテゴリ | 型番 / メーカー | パッケージ | LCSC / JLCPCB区分 |
| :--- | :--- | :--- | :--- | :--- |
| `MCU` | 主 MCU | `ATTINY1616-MNR` (Microchip) | QFN-20-EP (3×3) | `C507118` (Extended) |
| `U1` | USB-UART (2ch) | `CH342K` (WCH) | ESSOP-10 | `C2826608` (Extended) |
| `03-eFuse` | ハイサイドスイッチ | `AP22814AW5-7` (Diodes Inc) | SOT-25 | `C2680321` (Extended) |
| `04-LDO` | 3.3V LDO | `ME6211C33M5G-N` (Microne) | SOT-23-5 | `C82942` (Extended) |
| `02_TVS` | 5V TVS ダイオード | `SMF5.0CA-MS` (MSKSEMI) | SOD-123FL | `C7473196` (Extended) |
| `USB_Serge` | USB ESD 保護 | `USBLC6-2SC6` (TECH PUBLIC) | SOT-23-6 | `C2827654` (Extended) |
| `01-USB_Com` / `Com1` | USB Type-C 16P | `UC09-0B16F00P11` (BXCONN) | SMD Right Angle | `C36936538` (Extended) |
| `CN1` | 拡張 IDC 20P | `321020MG0CBK00A01` (JILN) | SMD 2.54mm 2×10P | `C601970` (Extended) |
| `POW_LED` | 電源 LED (緑) | `KT-0805G` (KENTO) | 0805 | `C2297` (Basic) |
| `LED_Y` | 汎用 LED (黄) | `KT-0805Y` (KENTO) | 0805 | `C2296` (Basic) |
| `LED_R` | 汎用 LED (赤) | `NCD0805R1` (国星光電) | 0805 | `C84256` (Basic) |
| `LED_W` | 汎用 LED (白) | `KT-0805W` (KENTO) | 0805 | `C34499` (Basic) |

### 荒見積り状況 (JLCPCB PCBA 込・送料込み)
* **120 pcs 合計:** `$460.57`
* **1 pc あたり原価:** `$3.83`
* **想定販売価格 (原価 4 倍算):** **`$15.35`**

> ※ `Netlist_PCB1_2026-11-18_wip.enet` 時点では `01-USB_Com1`、`U1` (`CH342K`)、`MCU` 電源ピン (3/4/21)、各 LED 及びその電流制限抵抗が配線途中（未接続）のため、最終アートワーク前にネット接続・LED 抵抗追加を完了させます。
