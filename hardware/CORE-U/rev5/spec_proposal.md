<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U (rev5) 技術仕様書
〜 **「AI で書かせて、AI で動かす」AI フレンドリー脊髄マイコンボード** 〜

> **AI's First Blink. No Arduino IDE. No BOOTSEL Button. Full Power Observability.**

- **ステータス**: 設計仕様案（rev5 / 検討レビュー用）
- **ベース構想**: [CORE-U_AI_friendly_microcontroller_board.md](../CORE-U_AI_friendly_microcontroller_board.md)
- **前世代仕様**: [spec_proposal.md](../spec_proposal.md)（rev4）

---

## 1. 概要と rev4 → rev5 の核心的アップデート

**ADX CORE-U (rev5)** は、VS Code や Antigravity などの AI エージェントが人間の物理操作ゼロでハードウェアを制御・開発・観測・復旧するために再設計された、**第 4 世代 AI フレンドリー・マイコンボード**です。

rev4 までの「全 SMD 化」「8858 筐体フレンドリー」「20P IDC 拡張一本化」「デュアル Type-C による PC 保護」「CH342K ＋ ATtiny1616 の完全独立 2 ポート」という優れた骨格を継承しつつ、**余剰 4 ピンと電源トポロジを AI のための器官として全面再編成**しました。

### rev4 → rev5 の 4 大変更点

| 項目 | rev4（旧） | **rev5（新：AI フレンドリー再編成）** | 設計意図 |
| :--- | :--- | :--- | :--- |
| **開発体験・ソフトウェア** | Arduino IDE 1.8.x 推奨 | **Arduino IDE 撤廃。VS Code / Antigravity Native（A/B パターン両立）** | 工場出荷の **AI-Firmata（MCP 直接操作）** と **CLI＋UPDI 自動焼込** を AI が 2 秒で自在往復 |
| **オンボード LED** | 単色 LED ×3（赤・黄・白）で 3 ピン消費 | **1 ピン フルカラー RGB LED ×1（`PB4`）＋ 電源/バス LED（GPIO 非消費）** | 1 ピンで「AI's First Blink」と AI の感情・ステータス表現を実現し、残り 3 ピンを解放 |
| **電源トポロジ ＆ 制御** | `LDO` が eFuse の後ろ・`EN` 固定プルアップ | **`LDO` を eFuse 手前へ分離・`VBUS_EN`（`PB5`）で AI が 5V 2A バスを ON/OFF 制御** | 外部ショート時も MCU/CH342K が生存して AI に報告可能。外部 CARD の強制パワーサイクル＆フェイルセーフを実現 |
| **物理観測（Observability）** | eFuse `FLG` のみ（電圧・電流は盲目） | **`VBUS_VSENSE`（`PA5`）＋ `VBUS_ISENSE`（`PA4`：ハイサイド電流計）を標準内蔵** | AI 最大の弱点「物理データの不足」を解消。I2C を汚さず電圧・電流・電力（W）をリアルタイム観測 |

---

## 2. 開発ツールチェーン ＆ UX 仕様（A / B パターン両立）

```text
           ┌──────────────────────────────────────────────────────────┐
           │     Host PC : VS Code / Antigravity (AI エージェント)     │
           └──────────────┬──────────────────────────┬────────────────┘
                          │                          │
        [ A パターン: adx-cli / UPDI ]     [ B パターン: adx-mcp / AI-Firmata ]
        コード生成 → 1秒ビルド              箱出し 0 秒で直接操作・リアルタイム観測
        → 強制焼込 → 暴走時自己修復         (RGB LED / 電源ON-OFF / V・I・W / I2C / SPI)
                          │                          │
                          ▼                          ▼
     [ PC ] ──▶ USB-COM (VBUS 非接続) ──▶ [ WCH CH342K (Dual CDC) ]
                                            ├─▶ Port 0: SerialUPDI ──▶ PA0 (UPDI 強制書込)
                                            └─▶ Port 1: UART       ──▶ PA1/PA2 (対話・ログ)

     [ 5V 3A ACアダプタ ] ──▶ USB-PWR (5V 2A 定格) ──▶ 本体 3.3V (常時生存) ＆ 外部 5V 2A バス (AI 制御)
```

### 2.1 B パターン：工場出荷標準（`ADX AI-Firmata` / MCP）
- 箱から出して接続した瞬間から、コンパイル不要で Antigravity が `Port 1` 経由で直接ボードを操作可能。
- オンボード RGB LED（`AI_LED`）により、拡張 CARD ゼロの状態でも 60 秒以内に **「AI が L チカさせた（AI's First Blink）」** を完結できる。
- 標準コマンド例: `set_rgb(r,g,b)`, `set_bus_power(on/off)`, `get_telemetry()`（VBUS 電圧・電流・電力・eFuse 異常・VDD・チップ温度）, `gpio_rw`, `pwm_set`, `adc_read`, `i2c_scan/xfer`, `spi_xfer`。

### 2.2 A パターン：AI 自律ビルド ＆ UPDI 強制焼込（`adx-cli`）
- Arduino IDE を不要とし、VS Code / Antigravity から CLI ワンコマンドで `build → flash (Port 0 UPDI) → monitor (Port 1 UART)` を自動実行。
- スケッチ暴走やハングが発生しても、物理ボタン操作ゼロで AI が `Port 0 (UPDI)` から 2 秒で復旧・再書込、あるいは B パターン（`AI-Firmata`）への復元が可能。
- 完成後は `USB-COM` を抜去するだけで、`USB-PWR`（5V 2A）単体のスタンドアロン装置として 8858 筐体内で自律稼働する。

---

## 3. 基本電気仕様 (Electrical Specifications)

| 項目 | 定格 / 仕様 | 備考 |
| :--- | :--- | :--- |
| **主電源入力 (`USB-PWR`)** | **5.0V DC (定格 2.0A / 10W)** | Type-C 16P、CC 5.1kΩ×2、TVS (`SMF5.0CA-MS`) 保護 |
| **通信ポート (`USB-COM`)** | **USB 2.0 (データ専用・VBUS 非接続)** | PC からの給電・逆流を物理遮断、`USBLC6-2SC6` ESD 保護 |
| **内部ロジック電源 (`3V3`)** | **3.3V DC (最大 500mA)** | `ME6211C33M5G-N` LDO（**eFuse 手前の `USB_VBUS` から常時生成**） |
| **外部バス電源スイッチ / eFuse** | **Diodes `AP22814AW5-7`** | 連続最大 3A、過電流/短絡/過熱/逆流保護。**`PB5 (VBUS_EN)` によりソフト ON/OFF 可（初期 OFF）** |
| **外部バス電流計測 (`VBUS_ISENSE`)** | **0.00A 〜 3.30A（1.00 V/A）** | `10mΩ` 1% シャント ＋ ハイサイド電流センスアンプ（100 V/V：`INA180A3` / `TP181A2`）→ `PA4 (AIN4)` |
| **外部バス電圧計測 (`VBUS_VSENSE`)** | **0.00V 〜 6.60V（分圧比 1/2）** | `10kΩ + 10kΩ` (1%) 分圧 → `PA5 (AIN5)` |
| **主 MCU** | **Microchip `ATtiny1616-MNR`** | QFN-20-EP (3×3mm)、8-bit AVR (3.3V / 10MHz)、Flash 16KB / SRAM 2KB / 内蔵温度計・VDD 計測 |
| **USB ブリッジ IC** | **WCH `CH342K`** | ESSOP-10、デュアル USB-UART（Port 0: SerialUPDI / Port 1: UART） |
| **オンボード RGB LED (`AI_LED`)** | **フルカラー RGB ×1 (`WS2812B-2020`)** | `PB4` 1 ピン駆動。`USB_VBUS` からダイオード 1 本降圧（~4.3V）給電で 3.3V ロジック規格内動作 |
| **オンボード表示 LED (GPIO 非消費)** | **計 2 個 (`POW_LED` / `BUS_LED`)** | `POW_LED` (緑：`3V3` 通電表示) / `BUS_LED` (黄：`VBUS_PROT` 外部バス通電表示) |
| **拡張インターフェース** | **ADX 20P IDC (`CN1`) のみ** | `321020MG0CBK00A01`（SMD 2×10P、`ADX-BUS-SPEC-v4.2` 準拠） |

---

## 4. 回路アーキテクチャ (Circuit Architecture)

```text
                          【 ADX CORE-U rev5 回路ブロック図 】

    [ USB-PWR (Type-C) ] ──▶ [ TVS: SMF5.0CA-MS ] ──▶ (USB_VBUS: 常時 5V)
                                                          │
       ┌──────────────────────────────────────────────────┼────────────────────────────┐
       │                                                  │                            │
       ▼                                                  ▼                            ▼
  [ 04-LDO: ME6211 (3.3V) ]                     [ D1: 1N4148W (VF≈0.7V) ]    [ 03-eFuse: AP22814AW5-7 ]
       │  (外部ショート時も常時生存)                     │ (~4.3V 電源)          │ Pin 4 (EN) ◀──[100kΩ PD]── PB5 (VBUS_EN)
       ├──▶ MCU VDD (3.3V)                              ▼                      │ Pin 3 (FLG) ──[10kΩ PU]──▶ PA7 (EFUSE_FLG)
       ├──▶ U1: CH342K (3.3V)                  [ RGB LED: WS2812B-2020 ]       │ Pin 1 (OUT) ──▶ (VBUS_EFUSE)
       ├──▶ U2: INA180A3 / TP181A2 (3.3V)        ▲ DIN ◀──[330Ω]── PB4         │                    │
       └──▶ POW_LED (緑: 本体通電)                └ (4.7kΩ ダミー負荷)          │                    ├──▶ BUS_LED (黄: バス通電)
                                                                               │                    │    ※シャント上流に配置
                                                                               │                    ▼
                                                                               │         [ R_SHUNT: 10mΩ 1% 1206 ]
                                                                               │           ├──▶ U2 (INA180A3: 100V/V) ──▶ PA4 (VBUS_ISENSE)
                                                                               │           ▼
                                                                               │         (VBUS_PROT: 外部負荷 5V 2A)
                                                                               │           ├──▶ [ 10kΩ / 10kΩ 分圧 ] ──▶ PA5 (VBUS_VSENSE)
                                                                               │           └──▶ CN1 (Pins 17-20: ADX 20P IDC)

    [ USB-COM (Type-C) ] ──▶ [ USBLC6-2SC6 (ESD) ] ──▶ [ U1: WCH CH342K ]
    (VBUS 非接続・CC 5.1kΩ×2)                            ├─▶ Port 0 ──▶ PA0/UPDI (SerialUPDI)
                                                         └─▶ Port 1 ──▶ PA1/TX, PA2/RX (Alt-UART)
```

### 4.1 電源トポロジ分離 ＆ フェイルセーフ（`VBUS_EN`）
- **MCU/通信系の独立生存**: `04-LDO (3.3V)` を `AP22814` の手前（`USB_VBUS`）へ移動したことで、外部 20P IDC 側でショートや過負荷が発生し eFuse が遮断しても、ATtiny1616 と CH342K は稼働を維持し、AI エージェントへ正確に障害を通知できます。
- **フェイルセーフ初期 OFF**: `AP22814` の `EN` ピン（`PB5`）は `100kΩ` で GND へプルダウンします。これにより、電源投入直後・リセット時・UPDI ファームウェア書込中は外部 5V バスが確実に OFF となり、AI のテスト中や起動過渡期におけるモーター・ヒーター等の意図しない暴走を防ぎます。
- **パワーサイクル機能**: 外部 CARD や I2C デバイスがハングした場合でも、AI が `VBUS_EN = LOW → HIGH` で外部 5V 電源を物理再起動できます。

### 4.2 ハイサイド電流計測（`VBUS_ISENSE` on `PA4`）の設計根拠
- **配置位置**: `AP22814` の出力と `CN1` の間に `10mΩ`（1206、1%、0.5W 以上）のシャント抵抗を直列配置します。`BUS_LED` はシャント上流に置くことで、純粋に **外部 20P IDC 負荷の消費電流のみ** を計測します。
- **スケールと分解能**:
  - 電流センスアンプのゲインを **100 V/V**（`INA180A3` または `TP181A2`、コモンモード入力 -0.2V〜26V、3.3V 単電源動作）とすることで、変換係数はちょうど **$1.00\,\text{V / A}$** になります。
  - **フルスケール**: `VDD (3.3V)` 基準で **$0.00\,\text{A} \sim 3.30\,\text{A}$**（`AP22814` の 3A 保護領域まで飽和せず測定可能、分解能 約 `3.22 mA/LSB`）。
  - **動的オートレンジ**: ATtiny1616 の内部基準電圧を `2.5V`（0〜2.5A、約 `2.44 mA/LSB`）や `1.1V`（0〜1.1A、約 `1.07 mA/LSB`）へ切り替えることで、軽負荷センサーの消費電流も高精度に観測可能です。
  - **自動ゼロ点キャリブレーション**: 起動直後（`VBUS_EN = OFF` の無負荷状態）に ADC 値を読み取ることで、アンプ固有の入力オフセット電圧（±10〜50mA 相当）をソフトウェア側で全自動補正できます。
- **電力損失と発熱**: 定格 2.0A 時の電圧降下はわずか **$20\,\text{mV}$**、発熱は **$0.04\,\text{W}$**（最大 3.0A 時でも $30\,\text{mV}$ / $0.09\,\text{W}$）と極めて小さく、10W 供給性能を損ないません。

### 4.3 バス電圧計測（`VBUS_VSENSE` on `PA5`）の設計根拠
- シャント下流（実際に `CN1` へ供給される `VBUS_PROT`）を `10kΩ + 10kΩ`（1% 精度）で **1/2 分圧** し、`0.1µF` フィルタコンデンサ経由で `PA5 (AIN5)` へ入力します。
- **測定範囲**: `0.00V 〜 6.60V`（5.00V 時に ADC 入力 2.50V、分解能 約 `6.45 mV/LSB`）。
- **診断用途**:
  - `VBUS_EN = ON` 時：AC アダプタやケーブルの電圧降下（例：5.0V → 4.3V）を検知し、電力（$P = V \times I$）をリアルタイム算出。
  - `VBUS_EN = OFF` 時：電圧が 0V へ落ちているかを確認し、外部 CARD 側からの逆流・外部給電の有無を検知。

### 4.4 1 ピン フルカラー RGB LED（`AI_LED` on `PB4`）の 3.3V ロジック整合
- 5.0V 給電の WS2812B に 3.3V ロジック信号を入力すると、仕様上の $V_{IH} = 0.7 \times V_{DD} = 3.5\text{V}$ を下回り不安定になる問題があります。
- rev5 では、**`USB_VBUS (5.0V)` から汎用ダイオード `1N4148W`（Basic 部品、$V_F \approx 0.7\text{V}$）経由で約 4.3V を生成**し、LED 電源へ供給します（無負荷時の電圧浮き防止用に `4.7kΩ` ダミー負荷抵抗を並列配置）。
  - LED 電源電圧：$V_{DD} \approx 4.25\text{V}$（WS2812B-2020 の動作範囲 3.7V〜5.3V の中央値）
  - 論理 HIGH 閾値：$V_{IH} = 0.7 \times 4.25\text{V} \approx 2.98\text{V} < 3.3\text{V}$（**データシート保証内の安定動作**）
- さらに `3V3` LDO ではなく `USB_VBUS` 側から独立給電するため、`VBUS_EN = OFF` の安全状態でも「AI's First Blink」が動作し、かつ白色全点灯時の電流スパイクが `3V3` アナログ基準（ADC）へノイズとして乗るのを防ぎます。

---

## 5. ピン配分と ADX 20P IDC アサイン (Pin Assignment)

### 5.1 ATtiny1616-MNR（QFN-20-EP）全 20 ピン割り当て（rev5 確定案）
ADX 20P IDC バス側（`P01`〜`P10`、`S00`）は [attiny1616.md](../../docs/spec/mcu_mapping/attiny1616.md)（`ADX-BUS-SPEC-v4.2`）から変更なし。オンボード専用 7 ピンのみ AI フレンドリー仕様へ最適化しています。

| QFN Pin | ポート名 | 接続先 | ADX 論理 ID / マクロ | 役割・仕様 |
| :---: | :--- | :--- | :--- | :--- |
| **1** | `PA2/RX` | `CH342K` Port 1 TX | *(オンボード)* | USART0 Alt-RX（AI-Firmata 受信 / デバッグ） |
| **2** | `PA3/EXTCLK` | `CN1` Pin 5 | **`P02`** (`ADX_CLK`) | 外部クロック / PWM (`TCB1`) / ADC0 |
| **3** | `GND` | `GND` | `GND` | システムグラウンド |
| **4** | `VDD` | `3V3` | *(オンボード)* | 内部ロジック電源 (3.3V、常時生存) |
| **5** | **`PA4`** | **電流センスアンプ OUT** | **`ADX_VBUS_ISENSE`** | **外部バス電流計測 (`ADC0 AIN4` / 1.0V/A / 0〜3.3A)** |
| **6** | **`PA5`** | **`VBUS_PROT` 1/2 分圧** | **`ADX_VBUS_VSENSE`** | **外部バス電圧計測 (`ADC0 AIN5` / 0.5V/V / 0〜6.6V)** |
| **7** | `PA6/AIN` | `CN1` Pin 3 | **`P01`** (`ADX_AIN`) | ADX バス標準アナログ入力 / DAC0 / GPIO |
| **8** | **`PA7`** | `AP22814` Pin 3 (`FLG`) | **`ADX_EFUSE_FLG`** | **eFuse 過電流・過熱フラグ（Active-LOW、10kΩ PU）** |
| **9** | **`PB5`** | `AP22814` Pin 4 (`EN`) | **`ADX_VBUS_EN`** | **外部 5V 2A バス電源 ON/OFF（Active-HIGH、100kΩ PD）** |
| **10** | **`PB4`** | `WS2812B-2020` DIN | **`ADX_AI_LED`** | **フルカラー RGB LED（330Ω 直列、旧 NeoPixel 枠継承）** |
| **11** | `PB3/RX` | `CN1` Pin 8 | **`P04`** (`ADX_RX`) | USART0 Default-RX（外部拡張 UART） |
| **12** | `PB2/TX` | `CN1` Pin 7 | **`P03`** (`ADX_TX`) | USART0 Default-TX（外部拡張 UART） |
| **13** | `PB1/SDA` | `CN1` Pin 10 | **`P06`** (`ADX_SDA`) | I2C SDA (TWI0) / ADC0 |
| **14** | `PB0/SCL` | `CN1` Pin 9 | **`P05`** (`ADX_SCL`) | I2C SCL (TWI0) / ADC0 |
| **15** | `PC0/SCK` | `CN1` Pin 15 | **`P10`** (`ADX_SCK`) | SPI SCK (SPI0 Alt) / TCD0 WOC |
| **16** | `PC1/MISO` | `CN1` Pin 14 | **`P09`** (`ADX_MISO`) | SPI MISO (SPI0 Alt) / TCD0 WOD |
| **17** | `PC2/MOSI` | `CN1` Pin 13 | **`P08`** (`ADX_MOSI`) | SPI MOSI (SPI0 Alt) |
| **18** | `PC3/CS` | `CN1` Pin 12 | **`P07`** (`ADX_CS`) | SPI CS (SPI0 Alt) |
| **19** | `PA0/UPDI` | `CN1` Pin 2 & `CH342K` | **`S00`** *(システム専用)* | UPDI 強制書込（A パターン・無人自己修復） |
| **20** | `PA1/TX` | `CH342K` Port 1 RX | *(オンボード)* | USART0 Alt-TX（AI-Firmata 送信 / デバッグ） |
| **21** | `EP` | `GND` | `GND` | サーマルパッド |

> **オンボード・ハードウェアテレメトリ総覧（AI が取得可能な 6 項目）**:
> 1. 外部バス出力状態 (`ADX_VBUS_EN` on `PB5`)
> 2. 外部バス電圧 (`ADX_VBUS_VSENSE` on `PA5`: V)
> 3. 外部バス電流 (`ADX_VBUS_ISENSE` on `PA4`: A) → 2×3 より **リアルタイム消費電力 (W)** を算出
> 4. ハードウェア過電流・過熱フラグ (`ADX_EFUSE_FLG` on `PA7`)
> 5. オンボード 3.3V レール電圧（ATtiny1616 内蔵 VDD チャンネル・ピン消費ゼロ）
> 6. MCU 内部温度（ATtiny1616 内蔵温度センサー・ピン消費ゼロ）

### 5.2 標準ヘッダ定義案（`ADX_CORE_U_rev5.h`）
```cpp
#pragma once
#include <Arduino.h>

// ADX 20-Pin Bus (v4.2) Logical Pin IDs
#define ADX_P01          PIN_PA6  // AIN / DAC
#define ADX_P02          PIN_PA3  // EXTCLK / PWM
#define ADX_P03          PIN_PB2  // UART_TX
#define ADX_P04          PIN_PB3  // UART_RX
#define ADX_P05          PIN_PB0  // I2C_SCL
#define ADX_P06          PIN_PB1  // I2C_SDA
#define ADX_P07          PIN_PC3  // SPI_CS
#define ADX_P08          PIN_PC2  // SPI_MOSI
#define ADX_P09          PIN_PC1  // SPI_MISO
#define ADX_P10          PIN_PC0  // SPI_SCK

// CORE-U rev5 Onboard AI Organs (Control / Observability / Expression)
#define ADX_VBUS_EN      PIN_PB5  // Active-HIGH (External 5V 2A Bus Enable)
#define ADX_VBUS_ISENSE  PIN_PA4  // ADC0 AIN4 (1.00 V/A, 0.0-3.3A)
#define ADX_VBUS_VSENSE  PIN_PA5  // ADC0 AIN5 (0.50 V/V, 0.0-6.6V)
#define ADX_EFUSE_FLG    PIN_PA7  // Active-LOW (eFuse Overcurrent/Thermal Fault)
#define ADX_AI_LED       PIN_PB4  // Single Addressable RGB LED (WS2812B-2020)
```

---

## 6. 主要部品構成 (BOM) ＆ 概算コスト影響

rev4 BOM をベースに、rev5 で追加・変更となる部品を **太字** で示します。

| リファレンス | カテゴリ | 型番 / メーカー | パッケージ | LCSC / JLCPCB 区分 | rev4 比 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `MCU` | 主 MCU | `ATTINY1616-MNR` (Microchip) | QFN-20-EP | `C507118` (Extended) | 継続 |
| `U1` | USB-UART (2ch) | `CH342K` (WCH) | ESSOP-10 | `C2826608` (Extended) | 継続 |
| **`U2`** | **ハイサイド電流アンプ** | **`INA180A3IDBVR` (TI) or `TP181A2-CR` (3PEAK)** | **SOT-23-5 / SC-70-6** | **`C47021223` / `C2902350` (Extended)** | **新規追加** |
| **`R_SHUNT`** | **シャント抵抗** | **`10mΩ` ±1% 0.5W〜1W** | **1206** | **選定予定 (Basic/Ext)** | **新規追加** |
| **`AI_LED`** | **フルカラー RGB LED** | **`WS2812B-2020` (Worldsemi)** | **2020 SMD** | **`C965555` (Extended)** | **LED×3 より置換** |
| **`D1`** | **LED 降圧ダイオード** | **`1N4148W`** | **SOD-123** | **Basic 部品** | **新規追加** |
| `03-eFuse` | ハイサイドスイッチ | `AP22814AW5-7` (Diodes Inc) | SOT-25 | `C2680321` (Extended) | 継続（EN を PB5 へ） |
| `04-LDO` | 3.3V LDO | `ME6211C33M5G-N` (Microne) | SOT-23-5 | `C82942` (Extended) | 継続（eFuse 手前へ） |
| `02_TVS` | 5V TVS ダイオード | `SMF5.0CA-MS` (MSKSEMI) | SOD-123FL | `C7473196` (Extended) | 継続 |
| `USB_Serge` | USB ESD 保護 | `USBLC6-2SC6` (TECH PUBLIC) | SOT-23-6 | `C2827654` (Extended) | 継続 |
| `01-USB_*` | USB Type-C 16P ×2 | `UC09-0B16F00P11` (BXCONN) | SMD Right Angle | `C36936538` (Extended) | 継続 |
| `CN1` | 拡張 IDC 20P | `321020MG0CBK00A01` (JILN) | SMD 2×10P | `C601970` (Extended) | 継続 |
| `POW_LED` | 本体電源 LED (緑) | `KT-0805G` (KENTO) | 0805 | `C2297` (Basic) | 継続（`3V3` 側へ） |
| `BUS_LED` | 外部バス通電 LED (黄) | `KT-0805Y` (KENTO) | 0805 | `C2296` (Basic) | 継続（`VBUS_EFUSE` 側へ） |

### 概算コスト試算（120 pcs PCBA ロット想定）
- **rev4 実績見積り**: `$3.83 / pc`（4 倍算売価 `$15.35`）
- **rev5 部品増減（見込み）**:
  - 電流センスアンプ（`~$0.20〜$0.30`）＋ シャント抵抗（`~$0.03`）＋ `WS2812B-2020`（`~$0.05`）＋ Basic 抵抗/ダイオード（`~$0.02`）＋ Extended フィー増分（2 種 = `$6 / 120 = $0.05`）
  - **合計増分**: **`+$0.35 〜 +$0.45 / pc`**
- **rev5 想定原価**: **約 `$4.20 〜 $4.30 / pc`**
- **想定販売価格（4 倍算）**: **約 `$16.80 〜 $17.20`**

> **コスト評価**: 原価わずか **+$0.40 前後** で、「プログラマブル 5V 2A 電源スイッチ＋電圧計＋電流計（電力計）＋フルカラー RGB LED」をすべて標準装備できます。単なるマイコンボードから **「AI 専用のパワーアナライザ内蔵・自律制御ボード」** へ格上げされる投資対効果は圧倒的です。

---

## 7. アートワーク・試作前のレビュー確認事項

1. **シャント抵抗のケルビン配線（4 端子風センス配線）**:
   - `10mΩ` は微小抵抗のため、`R_SHUNT` の両パッドの内側から専用パターンを細く引き出して `INA180A3` / `TP181A2` の `IN+` / `IN-` へ直結し、大電流パターンの銅箔抵抗（数 mΩ）による測定誤差を防ぐこと。
2. **電流センスアンプの最終型番選定（JLCPCB 在庫・単価確認）**:
   - `INA180A3IDBVR`（TI、SOT-23-5）と `TP181A2-CR`（3PEAK、SC-70-6）のどちらが調達性・在庫・コストで有利かを実発注前に LCSC で最終確認する。
3. **RGB LED の割り込みタイミング確認（B パターン運用時）**:
   - `WS2812B-2020` 1 個のデータ送出は約 `30µs`（割り込み禁止区間）。UART `115200 bps` の 1 バイト受信間隔は約 `87µs` のため、1 個駆動なら受信取りこぼしは原則起きないが、`AI-Firmata` 実装時に LED 更新直後のレスポンス送信順序を設計しておく。
4. **`VBUS_EN` 初期 OFF 仕様の UX 確認**:
   - スタンドアロン（A パターン）で外部負荷を使う際、`setup()` 先頭で `digitalWrite(ADX_VBUS_EN, HIGH)` を呼ぶルールになる。`adx-cli` のテンプレートと AI 向け Skill に標準組み込みとすることで、人間・AI ともに迷わない仕様とする。
