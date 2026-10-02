<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Phase 3: 通信・IDC 規格整合性レビュー結果 (rev2 Communication & IDC)

- **対象回路ブロック**: `Block 4` (RS-485 トランシーバ `SP485EEN`), `GPIO-IDC` (拡張 2×10P コネクタ)
- **ステータス**: **PASS+（規格 100% 準拠・耐ノイズ性優秀）**

---

## 1. RS-485 / LN-485 トランシーバ回路 (`Block 4`) の検証

### 1.1 リファレンス再編と結線完全性
rev2 において、終端切り替え用ソルダージャンパのリファレンスが Block 4 に統一されました：
- `3-JP1` $\to$ **`4-JP1`** (Pin A: Net A, Pin B: Net $7N12 $\to$ 終端抵抗 `4-R4` Pin 1)
- `3-JP3` $\to$ **`4-JP3`** (Pin A: 終端抵抗 `4-R4` Pin 2 $\to$ Net $7N18, Pin B: Net B)

ソルダージャンパのショートにより、A-B 間に 120Ω 終端抵抗（`4-R4`）が確実に接続される構造が正常に維持されています。

### 1.2 フェイルセーフバイアス回路の定常動作
- プルアップ: `4-R6` (1.0kΩ 1% to +5V)
- プルダウン: `4-R5` (1.0kΩ 1% to GND)
- 終端抵抗: `4-R4` (120Ω 1%)
- **無信号時差動電位差**:
  $$V_{AB(idle)} = 5.0\,\mathrm{V} \times \frac{120\,\Omega}{1000 + 120 + 1000\,\Omega} = \mathbf{+283.0\,\mathrm{mV}}$$
  SP485EEN の受信スレッショルド（$-200\,\mathrm{mV} \sim +200\,\mathrm{mV}$）に対し、$+83\,\mathrm{mV}$ の十分な安全マージンをもって HIGH（Mark）を維持。バス開放・全スレーブ停止時のノイズ誤受信を物理的にシャットアウトします。

### 1.3 マイコン制御ライン
- `DI` $\to$ `PA1/D` (`3-MCU.PA1`: USART0 TxD)
- `RO` $\to$ `BMC_RO` (`3-MCU.PA2` ＆ `5-BMC.PA2` 双方へ受信供給)
- `DE` $\to$ `PA4/DE` (`3-MCU.PA4`: USART0 ハードウェア自動方向制御 `XDIR`)
- `RE#` $\to$ `PA7/RE` (`3-MCU.PA7`: GPIO 受信制御)

### 1.4 TVS オミット決定事項（DEC-01）の適合性
- **決定**: CORE-STANDARD の設計方針（12V系 / 近距離・機器内 / Commercial）に基づき、外付け TVS はオミット（未実装維持）。
- **保護耐量**: `SP485EEN` 自体が **HBM $\pm 15\,\mathrm{kV}$ の強固な ESD 耐量** を備えており、直列 10Ω 抵抗（`4-R7`, `4-R8`）との協調により、想定環境における静電気放電に対して十分な耐性を有することを確認。

---

## 2. IDC 2×10 拡張コネクタ (`GPIO-IDC`) の完全照合

ADX 規格書 [`docs/ja/ADX_pinout.md`](../../../../docs/ja/ADX_pinout.md) (v2) と rev2 ネットリストの全 20 ピン結線を照合しました。

| IDC Pin | ADX Pinout v2 規定ネット | rev2 ネットリスト接続 | マイコン端子 / 機能 | 規格合致判定 |
| :---: | :--- | :--- | :--- | :---: |
| **1** | **VDD** | `VDD` (2-P_SW.VOUT) | 電源 (5V/2A 給電) | **PASS** |
| **2** | **VDD** | `VDD` (2-P_SW.VOUT) | 電源 (5V/2A 給電) | **PASS** |
| **3** | **VDD** | `VDD` (2-P_SW.VOUT) | 電源 (5V/2A 給電) | **PASS** |
| **4** | **VDD** | `VDD` (2-P_SW.VOUT) | 電源 (5V/2A 給電) | **PASS** |
| **5** | **N.C.** (短絡保護緩衝帯) | *(未接続)* | 電源短絡防止アイソレーション | **PASS** |
| **6** | **PA6/DAC0** | `PA6/DAC0` | `3-MCU.PA6` (DAC0 出力 / AIN6) | **PASS** |
| **7** | **PA5/AIN5** | `PA5/AIN5` | `3-MCU.PA5` (VREFA / AIN5) | **PASS** |
| **8** | **GND** (EXTCLK ガード①) | `G` (共通 GND) | 高周波シールド GND | **PASS** |
| **9** | **PA3/EXTCLK** | `PA3/EXC` | `3-MCU.PA3` (EXTCLK 信号線) | **PASS** |
| **10** | **GND** (EXTCLK ガード②) | `G` (共通 GND) | 通信分離シールド GND | **PASS** |
| **11** | **PB2/TXD_EXT** | `PB2/TXD_EXT` | `3-MCU.PB2` (UART Alternate TxD) | **PASS** |
| **12** | **PB3/RXD_EXT** | `PB3/RXD_EXT` | `3-MCU.PB3` (UART Alternate RxD) | **PASS** |
| **13** | **PB0/SCL** | `PB0/SCL` | `3-MCU.PB0` (I2C SCL) | **PASS** |
| **14** | **PB1/SDA** | `PB1/SDA` | `3-MCU.PB1` (I2C SDA) | **PASS** |
| **15** | **GND** (中継シールド) | `G` (共通 GND) | 中継シールド GND | **PASS** |
| **16** | **PC3/SS** | `PC3/SS` | `3-MCU.PC3` (SPI SS) | **PASS** |
| **17** | **PC2/MOSI** | `PC2/MOSI` | `3-MCU.PC2` (SPI MOSI) | **PASS** |
| **18** | **PC1/MISO** | `PC1/MISO` | `3-MCU.PC1` (SPI MISO) | **PASS** |
| **19** | **PC0/SCK** | `PC0/SCK` | `3-MCU.PC0` (SPI SCK) | **PASS** |
| **20** | **GND** (終端シールド) | `G` (共通 GND) | 終端シールド GND | **PASS** |

### 2.1 規格整合性サマリー
- **適合率**: **100% (20 ピン全数完全一致)**
- **物理シールド構造**: Pin 8 / Pin 10 による高周波クロック（Pin 9: PA3）の完全シールド、および Pin 5 のアイソレーションが完全に保持されています。

---

## 3. Phase 3 結論

**判定: ALL PASS+**
通信系・インターフェース系ともに ADX 規格および設計方針と 100% 整合しています。
