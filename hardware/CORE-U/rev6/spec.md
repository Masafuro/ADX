<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U (rev6) 技術仕様書（リビングドキュメント）

〜 **ADX フラッグシップ・3 Mode 耐環境 / 筐体フレンドリー / AI フレンドリー脊髄マイコンボード** 〜

> **One Core, From AI Bench to Harsh Field.**  
> 机上での AI 自律開発・デバッグから、絶縁 RS-485／絶縁電源拡張 CARD と組み合わせた `-40℃ 〜 +85℃` の産業・屋外・車載現場稼働までを、同一の 1 枚でシームレスに貫く ADX フラッグシップボード。

- **ドキュメント種別**: 設計仕様書（**追記型リビングドキュメント**）
- **版数**: **v0.5**（2026-11-28 更新：§6.4 省電力チューニングの `LED_SYS` 緑カット手段を大きめ `0Ω` 抵抗（`0805` 推奨・端寄り配置）へ確定）
- **ステータス**: **アーキテクチャ仕様確定（回路図・部品型番最終選定フェーズへ移行可能）**
- **関連文書**:
  - rev6 コンセプト原点: [concept.md](concept.md)
  - 産業用 PLC 耐熱性ベンチマーク: [plc_thermal_benchmark_report.md](plc_thermal_benchmark_report.md)
  - ディープスリープ可能性考察: [deep_sleep_potential.md](deep_sleep_potential.md)
  - ADX バス規格 v5.0 改訂計画書: [ADX_BUS_SPEC_V5_REVISION_PLAN.md](../../../memo/PLAN/ADX_BUS_SPEC_V5_REVISION_PLAN.md)
  - 前世代（rev5）技術仕様案: [rev5/spec_proposal.md](../rev5/spec_proposal.md)
  - フォームファクター規格: [8858 Form Factor](../../../docs/ja/8858_formfactor.md)
  - 拡張バス規格（SSOT）: [ADX_PINOUT.md](../../../docs/spec/ADX_PINOUT.md)（`ADX-BUS-SPEC-v5.0` 準拠） / [ATtiny1616 Mapping](../../../docs/spec/mcu_mapping/attiny1616.md)

---

## 0. 本仕様書の確定状況マップ（Status Dashboard）

| セクション | テーマ | ステータス | 備考 |
| :--- | :--- | :---: | :--- |
| **§1** | プロダクト位置づけ・フラッグシップ戦略（CORE-A の再定義含む） | **確定** | CORE-U rev6 を ADX 主軸に一本化 |
| **§2** | 3 Mode 運用定義 ＆ 耐熱・耐振動仕様（理論仕様） | **確定** | Mode 1: `-40〜+85℃` 理論仕様（後日試験実証） |
| **§3** | 基本電気・観測・開発 UX 仕様 | **確定** | `PA5=VSENSE`, `PA7=ISENSE`, `PB5=USB_COM_DET` で 3 Mode 完全自己認識 |
| **§4** | 回路アーキテクチャ ＆ 電源・遮断設計 | **確定（詳細定数最終化前）** | 全体ブロック図確定。回路図作成時に抵抗値・保護定数を最終検証 |
| **§5** | ADX Pinout (`v5.0` パターン B) ＆ ATtiny1616 全 20 ピン配分 | **確定** | UPDI オミット・`Pin 6 = UART_DE (PA4)`・UART 逆転確定（`ADX-BUS-SPEC-v5.0` 正式改訂済） |
| **§6** | LED ＆ `PB4`/`PB5` 設計（案 1 採用） | **確定** | `LED_SYS` (ハード 3 色) ＋ `PB4` (`LED_MCU` 1 色 PWM) ＋ `PB5` (`USB_COM_DET`) |
| **§7** | 主要部品構成（BOM）＆ コスト試算 | **仮確定** | Type-C 高耐熱 THT シェル実型番・`LED_MCU` 発光色最終選択後に確定 |
| **§8** | 検証・実証試験計画（耐熱・耐振・通信・電気） | **ドラフト** | 試作機完成後に実測・合否判定を実施 |

---

## 1. 概要とプロダクト位置づけ【確定】

### 1.1 ADX エコシステムにおけるフラッグシップへの統合
CORE-U rev5 において「3 Mode 運用定義」「`CH342K` 非接続時完全遮断」「全稼働部品 `-40〜+105℃` 化」「THT IDC Eject Header 採用」が成立したことで、**CORE-U 単体で産業用小型エントリー PLC を上回る耐環境性（理論仕様 `-40℃ 〜 +85℃`・耐振動）を獲得できる見通し**が立った。

これにより、ADX の製品アーキテクチャを以下の通り抜本的に再編する：

1. **CORE-U (rev6) を ADX の唯一の中核フラッグシップボードとする**:
   - 机上での AI 開発（`AI-Firmata` / `adx-cli` + UPDI 自動焼込）から、現場での組込稼働（`Full embedded mode`）までを、基板を載せ替えることなく同一の CORE-U で完結させる。
2. **絶縁 RS-485 通信・絶縁 24V / 48V 電源入力は拡張 CARD 側へ委譲する**:
   - 電源電圧の違い（5V / 12V / 24V / 48V）やフィールド通信方式の違い（絶縁 RS-485 / LIN / DMX / CAN 等）を CORE 側の SKU 増加に持ち込まず、拡張 CARD 側で吸収する。
   - 高電圧・サージ・落雷に晒される外界との絶縁境界を CARD 側に分離することで、沿面距離設計の自由度と現場保守性（故障時の CARD 交換）を高める。
3. **CORE-A 等の特化ボードの位置づけ再定義**:
   - 旧 CORE-A（ATtiny1616 ＋ 絶縁 RS-485 統合ボード等）は、Arduino エコシステムにおける **Arduino ROMEO 等の付加価値型派生ボード** と同様の分類とする。
   - 将来的な開発可能性自体は排除しない（例：常時 +85〜+105℃ 連続の密閉灯具、USB 開口部ゼロの完全ポッティング用途、1 枚完結の大量配備スレーブ等）が、**ADX の主たる中核製品とは位置づけない**。

### 1.2 設計上の優先順位（トレードオフ判断基準）
設計判断が競合した際は以下の優先順位を厳守する：
1. **第一優先：耐熱性・耐振動性**（`Full embedded mode` での `-40℃ 〜 +85℃` 理論仕様と十分な熱マージンの確保）
2. **第二優先：観測性**（AI 自律開発・現場診断のための高精度 V / I / W・温度テレメトリおよび **3 Mode 自己認識**）
3. **第三優先：視認性（LED 表示灯）**（耐熱要件を満たす車載グレード LED によるハード監視灯＋PWM 単色ステータス灯）

---

## 2. 3 Mode 運用定義 ＆ 耐熱・耐振動仕様【確定】

CORE-U (rev6) の物理端子は **`USB-COM`・`USB-POW`・`IDC 20P (Eject Header)` の 3 つのみ** とする。接続状態に応じた 3 Mode に対して、**産業機器流のディレーティング（部品定格から 20℃ 以上の熱マージン確保）** を織り込んだ以下の温度仕様を規定する。

### 2.1 3 Mode 動作環境・耐環境マトリクス（理論仕様）

> [!NOTE]
> 本節の温度仕様は部品定格・自己発熱・ディレーティングに基づく **理論仕様（Design Target）** として規定し、試作機完成後に恒温槽を用いた実機試験によって実証・保証範囲を確定する（§8 参照）。

| 運用モード | `USB-COM` | `USB-POW` | `IDC 20P` | 給電経路 | `CH342K` 状態 | **動作周囲温度（理論仕様）** | 耐振動・構造プロファイル |
| :--- | :---: | :---: | :---: | :--- | :--- | :---: | :--- |
| **Mode 1:<br>Full embedded mode** | 非接続 | 非接続 | **接続**<br>*(ラッチ固定)* | **IDC 20P 経由で外部受電**<br>*(絶縁電源 CARD 等)* | **完全遮断 (0mW)**<br>保存領域 `-55〜+105℃` | **`-40℃ 〜 +85℃`**<br>*(結露・氷結なきこと)* | **◎ 最高水準**<br>THT ラッチ固定＋8858 四隅 M3 固定＋短尺/スタック配線で高耐振動を確立 |
| **Mode 2:<br>Semi embedded mode** | 非接続 | **接続 (6P)** | **接続**<br>*(ラッチ固定)* | **`USB-POW` より 5V 2A 給電**<br>*(IDC へ出力)* | **完全遮断 (0mW)**<br>保存領域 `-55〜+105℃` | **`-20℃ 〜 +70℃`**<br>*(2A 連続時はディレーティング)* | **○ 準高耐環境（耐振に構造限界）**<br>USB-POW 挿抜構造ゆえ耐振は Mode 1 に劣る。市販ケーブル耐熱 (~80℃) を考慮 |
| **Mode 3:<br>Debug mode** | **接続 (16P)** | **接続 (6P)** | 任意 / 接続 | **`USB-POW` より 5V 2A 給電**<br>*(PC 非給電)* | **自動起動 (3.3V)**<br>UPDI + UART 同時稼働 | **`0℃ 〜 +55℃`** *(推奨)*<br>*(最大目安 `-20℃ 〜 +70℃`)* | **△ 開発・保守用**<br>`CH342K` 動作上限 `+85℃` に対し自己発熱マージンを確保 |

- **保存周囲温度（非通電）**: **`-40℃ 〜 +85℃`**（結露・氷結なきこと。105℃ 定格 Type-C コネクタ採用時は `-40℃ 〜 +100℃` まで拡張余地あり）
- **使用・保存周囲湿度（目安）**: `10 〜 90% RH`（結露なきこと）

### 2.2 理論仕様の根拠と産業用 PLC に対する位置づけ
詳細は [plc_thermal_benchmark_report.md](plc_thermal_benchmark_report.md) を参照。

- **熱マージン設計（20℃ ディレーティング）**:
  - Mode 1 稼働時の全部品を **`-40℃ 〜 +105℃` 以上**（主要電源・アナログ・スイッチは `+125℃`、車載 LED は `+110℃`、`CH342K` は無通電保存 `-55〜+105℃`）で統一。
  - アルミ電解コンデンサ・バックアップ電池・メカ式リレー・液晶・無線モジュールを一切持たず、待機時自己発熱は **約 35 mW（6〜7mA @ 5V）** と極小であるため、密閉 8858 筐体・縦横設置方向を問わず基板温度上昇 $\Delta T$ を 2℃ 程度以内に抑制できる。
  - これにより、周囲温度 `+85℃` 稼働時でもすべての部品に対して **20℃ 以上の安全マージン** を確保する。
- **市場ベンチマークとの対比**:
  - 国内外の主要小型エントリー PLC（KEYENCE KV Nano / 三菱 FX5S・FX5U / OMRON CP1E・CP2E / パナソニック FP0H / IDEC FC6A Plus / Siemens LOGO! 8・S7-1200 / Allen-Bradley Micro820）の動作上限 **`+55℃ 〜 +65℃`**、および Arduino Opta / M5Stack StamPLC の **`+50℃`** を大幅に上回る。
  - 極限環境 PLC（Siemens SIPLUS S7-1200 / WAGO 750 XTR: **`-40〜+70℃`**）を超え、建機・農機・車載モバイル PLC（ifm ecomatMobile: **`-40〜+85℃`**）と同等の温度帯を狙う。
- **適用範囲の明確な線引き**:
  - **対象**: 産業用制御盤、舞台・照明・農業ハウス、屋外防水ボックス（直射日光下含む）、車載・農機・建機キャビン内（アフターマーケット／特装用途）。
  - **対象外**: 自動車エンジンルーム（+125℃〜）、純正自動車 OEM 規格（AEC-Q100 / ISO 7637 認証必須領域）、防爆・安全計装（SIL）領域。
- **屋外・車載運用時の付帯要件**:
  - 未接続となる `USB-COM` / `USB-POW` コネクタは、振動疲労防止のため **THT シェル固定（DIP 足付き）かつ高耐熱 LCP ハウジング（`-40〜+85℃` 以上、望ましくは `+105℃`）品** を選定する。
  - 粉塵・湿気・塩害環境での Mode 1 運用時は、**シリコン製 Type-C 防塵キャップ装着または 8858 筐体パネル側でのポート密閉** を推奨構成とする。
  - 拡張 CARD とスタックした**システム全体**の動作温度範囲は、CORE-U と接続される各 CARD の定格のうち低い方（および CARD 側の自己発熱ディレーティング）に従う。CORE-U が `-40〜+85℃` を担保することで、CORE 側がシステムの熱ボトルネックになることを防ぐ。

---

## 3. 基本電気・観測・開発 UX 仕様【確定】

### 3.1 基本電気仕様

| 項目 | 仕様 | 温度定格・備考 |
| :--- | :--- | :--- |
| **主 MCU** | **Microchip `ATtiny1616-MNR`** | `-40〜+105℃`、QFN-20-EP (3×3mm)、3.3V / 10MHz 内蔵発振（水晶なし）、Flash 16KB / SRAM 2KB / EEPROM 256B |
| **主電源入力 (`USB-POW`)** | **5.0V DC（定格 2.0A / 10W）** | 6P Power-Only Type-C（THT シェル固定・LCP、CC 5.1kΩ×2）、TVS (`SMF5.0CA-MS`: `-55〜+150℃`) |
| **通信ポート (`USB-COM`)** | **USB 2.0（データ専用・PC 非給電）** | 16P Type-C（THT シェル固定・LCP）、`USBLC6-2SC6` (`-55〜+125℃`) ESD 保護。VBUS は 25µA 接続検知のみ |
| **電源保護 eFuse** | **TI `TPS259470A`** *(入手性次第で `0L`)* | `-40〜+125℃`、$R_{ON}=28.3\text{m}\Omega$、**真の逆流阻止**、OVLO (~6.0V)、電流制限 (~3.0A)、**EN 固定有効（MCU 制御なし）** |
| **システム幹線 (`VBUS_PROT`)** | **4.5V 〜 5.25V DC（双方向）** | Mode 2/3 で最大 2A 給電出力、Mode 1 で外部 CARD より受電 |
| **内部ロジック電源 (`3V3`)** | **3.3V DC（最大 500mA）** | **TI `TLV75533PDBVR`** (`-40〜+125℃`、SOT-23-5、`VBUS_PROT` より生成) |
| **幹線電圧計測 (`VBUS_VSENSE`)** | **0.00V 〜 6.60V（分圧比 1/2）** | `10kΩ + 10kΩ` ±1% (`-55〜+155℃`) ＋ `0.1µF X7R` → **`PA5` (`ADC0 AIN5`)** |
| **供給電流計測 (`VBUS_ISENSE`)** | **0.00A 〜 3.30A（1.00 V/A）** | `10mΩ` ±1% 1206 シャント (`-55〜+155℃`) ＋ **3PEAK `TP181A2-CR`** (`-40〜+125℃`, 100V/V) → **`PA7` (`ADC0 AIN7`)** |
| **USB 接続検知 (`USB_COM_DET`)** | **3.3V デジタル入力（割込＆深睡眠復帰対応）** | `USB_COM_EN` 信号を `10kΩ` 保護抵抗経由で **`PB5`** へ入力（Mode 3 検知・3 Mode 完全自己認識） |
| **USB ブリッジ IC** | **WCH `CH342K`** | 動作 `-40〜+85℃` / **保存 `-55〜+105℃`**、Dual CDC（Port 0: SerialUPDI → `PA0` / Port 1: UART → `PB2`/`PB3`、Mode 3 のみ通電＆信号接続） |
| **USB 信号隔離スイッチ** | **TI `SN74LV4066A`** | `-40〜+125℃`、4ch 双方向アナログスイッチ（Mode 1/2 で `PA0`/`PB2`/`PB3` を完全 Hi-Z 隔離） |
| **オンボード LED（2 個構成）** | **車載 110℃ LED `HVRTB-3433EPX` ×2** | `-40〜+110℃` (`C5246356`)、5V 駆動＋ローサイド `2N7002`。<br>① **`LED_SYS`**（ハード直結 3 色：緑/シアン/赤）＋ ② **`LED_MCU`**（**`PB4`** `TCA0 WO1*` ハード PWM 単色ステータス灯、§6 参照） |
| **受動部品** | **MLCC 全 `X7R` / 抵抗 `0603WAF`** | MLCC `-55〜+125℃`（`X5R`・電解コンデンサ全廃）、抵抗 `-55〜+155℃` |
| **拡張コネクタ (`CN1`)** | **20P THT Eject Header (`C9039`)** | `-55〜+105℃`、2.54mm 2×10P ラッチ付きバーティカル牛角、**`ADX-BUS-SPEC-v5.0` 準拠** |
| **基板材・外形** | **`FR4-TG155` / `8858` 規格** | `88.0 × 58.0 mm`、四隅 M3 (`76.0 × 46.0 mm`)、キラル光学マーカー M1〜M4 対応 |
| **コスト・価格目標** | **製造原価上限 `$6.50` / 許容売価上限 `$26.00`** | 120 pcs PCBA ロット想定、4 倍算ベース |

### 3.2 開発ツールチェーン ＆ UX（A / B パターン両立 ＆ 3 Mode 自律適応）
- **B パターン（工場出荷 `AI-Firmata` / MCP）**: 箱出し 0 秒で Antigravity / VS Code から `Port 1` 経由の直接操作（`LED_MCU` PWM 点滅・V/I/W/温度/現在モードのリアルタイム観測・GPIO/PWM/ADC/I2C/SPI）。リセット初期値 `PORTMUX.CTRLB.USART0 = 0` がそのまま USB (`PB2`/`PB3`) であるため、`Serial.begin()` のみで即通信できる。
- **A パターン（`adx-cli` による自律ビルド ＆ UPDI 強制焼込）**: Arduino IDE 不要。`Port 0 (SerialUPDI)` がオンボード専用で `PA0` へ直結（Mode 3 時）されているため、スケッチ書込だけでなく **ブートローダー書込・ヒューズ管理・メモリダンプ・暴走時の 2 秒強制復旧** まで物理操作ゼロで完結する。
- **フィールド通信 × USB デバッグの自動切替**: `PB5 (USB_COM_DET)` により、平時（Mode 1/2）は `Serial.swap(1)` で RS-485 / LIN フィールド制御（またはディープスリープ）に専念し、保守員が PC / Android GOT を `USB-COM` へ挿した瞬間だけ `PB5` 割込で自動検知して USB SoftwareSerial（`PB2`/`PB3`）コンソールを起動するスマート運用が可能。
- **Android GOT 親和性**: `USB-POW` / `USB-COM` 電源分離と Dual CDC により、余剰 Android 端末を操作盤（GOT）として安全に接続可能（※`CH342K` ホットプラグ安定性は §4 で最終検証）。

---

## 4. 回路アーキテクチャ ＆ 電源・隔離設計【確定】

```text
                        【 ADX CORE-U rev6 回路ブロック図（3 Mode・v5.0 準拠版） 】

  [ USB-POW (6P Type-C) ] ──▶ [ TVS: SMF5.0CA-MS ] ──▶ (USB_VBUS: 上流 5V)
  (THTシェル / CC 5.1kΩ×2)                                │
                                 ┌────────────────────────┴──────────────────────┐
                                 │                                               ▼
                                 │                        [ 03-eFuse: TI TPS259470A (-40~125℃) ]
                                 ▼                         ・EN 固定有効 / OVLO≈6.0V / ILIM≈3.0A
                      [ LED_SYS 赤 (R): 異常灯 ] ◀─────────・Pin FLT (Active-LOW, 4.7V保護付)
                      ※上流給電のため遮断中も赤点灯!       ・真の逆流阻止 (Mode 1 で上流へ逆流ゼロ)
                                                           │ Pin OUT (VBUS_EFUSE)
                                                           ▼
                                                [ R_SHUNT: 10mΩ 1% 1206 ]
                                                  ├──▶ [ U2: TP181A2-CR (100V/V) ] ──▶ PA7 (VBUS_ISENSE)
                                                  ▼
                                        ═══════════════════════════════════
                                           (VBUS_PROT : ADX 双方向 5V 幹線)
                                        ═══════════════════════════════════
                                          │         │          │          │
       ┌──────────────────────────────────┘         │          │          └──────────────────────────┐
       ▼                                            ▼          ▼                                     ▼
  [ CN1: 20P THT Eject Header ]     [ 10kΩ/10kΩ 分圧 ]   [ 04-LDO: TI TLV75533 ]       [ 車載110℃ LED ×2 (5V駆動) ]
  (C9039: -55〜+105℃ / v5.0)           ──▶ PA5          (3.3V / 500mA / 125℃)          ├─ LED_SYS (HVRTB #1 / 0ピン):
   ・Mode 2/3: 外部CARDへ 5V 2A給電      (VSENSE)           │ (3V3)                     │   G = VBUS_PROT (正常緑)
   ・Mode 1  : 外部CARDから 5V受電!                         ├──▶ MCU VDD (ATtiny1616)   │   R = FLT (上流給電・異常赤)
   ・Pins 6,7,8: PA4(DE), PA1(TX), PA2(RX/LUT0-IN2)         ├──▶ U2 (TP181A2-CR)        │   B = USB_COM_EN (Debug青→シアン)
   ・※PA0(UPDI) は IDC 非露出!                              ├──▶ U_SW (SN74LV4066A)     └─ LED_MCU (HVRTB #2 / 1ピン):
                                                            │                               単色 (推奨: R+G アンバー等)
  [ USB-COM (16P Type-C) ] ──▶ [ USBLC6-2SC6 ] ──▶ D+/D-    │                               ◀── 2N7002 ◀── PB4 (TCA0 WO1* PWM)
   (THTシェル / VBUS非給電)                                 ▼
         │ VBUS (100k/100k 分圧: 25µA検知のみ)       [ Q2: P-MOS (AO3401A) ]
         ▼                                            │ (3V3_USB: Mode 3 のみ通電!)
  [ Q1: 2N7002 ] ──▶ (USB_COM_EN_N: 3V3 PU) ──────────┘
         │                                            ▼
         └──▶ [ Q3: 2N7002 ] ──▶ (USB_COM_EN: 3V3 PU) ──▶ [ U1: WCH CH342K (Mode 1/2 は 0mW保存状態) ]
                    │                                          ▲
                    ├──▶ LED_SYS 青 (Q_DBG)                    │ (Port 0: UPDI / Port 1: UART)
                    ├──▶ [ 10kΩ ] ──▶ PB5 (USB_COM_DET) ★      │
                    └──▶ [ U_SW: SN74LV4066A (3ch) ] ◀─────────┴──▶ PA0 (UPDI: 47kΩ PU), PB2 (TX), PB3 (RX: 100kΩ PU)
```

### 回路設計・レイアウト時の必須チェックポイント
1. **`PA0 (UPDI)` のオンボード完全隔離 ＆ プルアップ**:
   - `PA0` は IDC 20P コネクタへ一切引き出さない（HV ノイズ・誤リセット経路の物理遮断）。
   - Mode 1/2 では `SN74LV4066A` が Hi-Z 開放となり、`PA0` は基板上数 mm の極短配線に閉じ込められる。EMI 耐性をさらに高めるため、MCU 側へ `47kΩ` の弱プルアップ（UPDI 規格適合）を配置する。
2. **`PB5 (USB_COM_DET)` の非回り込み保証**:
   - `USB_COM_EN` は `Q3` のドレインを本体側 **`3V3`** へプルアップする構造であるため、`VBUS_PROT = 0V`（本体無通電）の状態で `USB-COM` だけ先挿ししても `USB_COM_EN = 0V` となり、`PB5` の ESD 保護ダイオードへの回り込み電流は物理的にゼロとなる。念のため `10kΩ` 直列抵抗を挟んで `PB5` へ接続する。
3. **`CH342K` 遮断回路と Android OTG ホットプラグ整合**:
   - `USB-COM` 接続時のみ `3V3_USB` を立ち上げるパワーゲート方式において、PC だけでなく Android OTG 接続時も `UD+` プルアップタイミング・認識エラーが起きないことを試作で検証する。
4. **`TPS259470` の `FLT` ピン保護 ＆ サーマル設計**:
   - OVLO（9V〜12V 誤印加）時に `FLT` ピンが定格 5V を超えない保護回路（4.7V ツェナー等）と、2×2mm QFN 底面サーマルビア群のレイアウトを確定する。
5. **シャント抵抗のケルビン配線**:
   - `10mΩ` (1206) の両パッド内側から専用センス配線を `TP181A2-CR` へ直結し、`PA7` へ入力する。

---

## 5. ADX Pinout (`v5.0` パターン B) ＆ ATtiny1616 全 20 ピン配分【確定】

> [!IMPORTANT]
> 本節の 20P バス配列は **`ADX-BUS-SPEC-v5.0`**（[docs/spec/ADX_PINOUT.md](../../../docs/spec/ADX_PINOUT.md)）として正式改訂済みである（改訂履歴: [ADX_BUS_SPEC_V5_REVISION_PLAN.md](../../../memo/PLAN/ADX_BUS_SPEC_V5_REVISION_PLAN.md)）。

### 5.1 設計判断の要点（パターン B 採用の根拠）
1. **`UPDI (S00)` を ADX 20P バスから完全オミット**:
   - HV（12V）プログラミング兼用ピンである `PA0 (UPDI)` を長いリボンケーブルへ露出させるリスクを排除し、`0Ω` 抵抗のような妥協構造も排する。
   - 他の MCU による Core ボードにおいても、**OWD / PROG（UPDI / SWD / ISP / UF2）は各 Core ボード上に設ける**ことを仕様上のアーキテクチャ前提とし（具体的なコネクタ形態や設置義務までは強制しない）、20P IDC バスは **「純粋な拡張 I/O ＋ 給電専用」** とする。
   - これにより、v4.2 で全 CARD に課していた「`S00` 接続禁止」および「`P02` の `SWCLK` 競合回避用 `0Ω` DNP 推奨」の 2 大例外ルールが規格から完全に消滅する。
2. **`Pin 6` に RS-485 方向制御専用ピン `P03 (UART_DE / XDIR)` を新設**:
   - `Pins 1, 3, 5` を連続 `GND`、`Pin 2 = P01 (AIN)`（真の両側 GND コプレーナシールド）、`Pin 4 = P02 (EXTCLK)`（両側 GND 維持）、`Pin 6 = P03 (UART_DE)` とし、`Pins 7〜20` は v4.2 と物理位置を完全一致させる。
   - **`RS-485 (ハード XDIR)` ＋ `I2C` ＋ `SPI` ＋ `AIN` ＋ `EXTCLK` が 1 ピンの衝突もなく全同時稼働**でき、RS-485 を使わない CARD では `P03` を高機能 3.3V GPIO（`ADC0` / `TCD0 48MHz 12-bit PWM` / `CCL LUT0-OUT`）として自由に活用できる。
3. **RS-485 拡張 CARD 側のフェイルセーフ規約【MUST】**:
   - MCU リセット中・未書込時の Hi-Z 状態でフィールドバスを占有・ロックしないよう、RS-485 拡張 CARD は **`P03 (UART_DE)` に `10kΩ〜100kΩ` のプルダウン抵抗を必ず実装** し、デフォルトを受信状態（DE=LOW）に固定することを規格要件とする。

### 5.2 ADX 20-Pin Bus 配列（`ADX-BUS-SPEC-v5.0` 準拠）

```text
                  【 20-Pin Shrouded / Eject Box Header (Top View) — v5.0 】

                             ┌────────[ Center Key ]────────┐
                GND     [ 1] │  ●   ●  │ [ 2]  P01  (AIN)
                GND     [ 3] │  ●   ●  │ [ 4]  P02  (EXTCLK)
                GND     [ 5] │  ●   ●  │ [ 6]  P03  (UART_DE / XDIR)
    (UART_TX)   P04     [ 7] │  ●   ●  │ [ 8]  P05  (UART_RX)
    (I2C_SCL)   P06     [ 9] │  ●   ●  │ [10]  P07  (I2C_SDA)
                GND     [11] │  ●   ●  │ [12]  P08  (SPI_CS)
   (SPI_MOSI)   P09     [13] │  ●   ●  │ [14]  P10  (SPI_MISO)
    (SPI_SCK)   P11     [15] │  ●   ●  │ [16]  GND
                VBUS    [17] │  ●   ●  │ [18]  VBUS
                VBUS    [19] │  ●   ●  │ [20]  VBUS
                             └──────────────────────────────┘
```

### 5.3 ATtiny1616-MNR（QFN-20-EP）全 20 ピン確定配分（1 本の過不足なし）

| QFN | ポート | rev6 接続先 | ADX 論理 ID / マクロ | 役割・ペリフェラル機能 |
| :---: | :--- | :--- | :--- | :--- |
| **19** | **`PA0`** | `SN74LV4066A` → `CH342K` Port 0 | *(オンボード専用)* | **UPDI（IDC 非露出・Mode 1/2 完全隔離・47kΩ PU）** |
| **12** | **`PB2`** | `SN74LV4066A` → `CH342K` Port 1 RX | `ADX_USB_TX` | `USART0 TxD (Def)` / SoftwareSerial TX（`Serial.begin()` 初期値） |
| **11** | **`PB3`** | `SN74LV4066A` → `CH342K` Port 1 TX | `ADX_USB_RX` | `USART0 RxD (Def)` / SoftwareSerial RX（MCU 側 100kΩ PU） |
| **7** | `PA6` | `CN1` Pin 2 | **`P01`** (`ADX_AIN`) | `ADC0 AIN6` / `DAC0 OUT` / Async Int（Pin 1 & 3 GND 保護） |
| **2** | `PA3` | `CN1` Pin 4 | **`P02`** (`ADX_CLK`) | `EXTCLK` / `TCB1 WO` / `ADC0 AIN3`（Pin 3 & 5 GND 保護） |
| **5** | **`PA4`** | **`CN1` Pin 6** | **`P03`** (`ADX_DE`) | **`USART0 XDIR (Alt)`** / `TCD0 WOA` / `LUT0-OUT` / `ADC0 AIN4` |
| **20** | **`PA1`** | **`CN1` Pin 7** | **`P04`** (`ADX_TX`) | **`USART0 TxD (Alt)`** / `LUT0-IN1` / `ADC0 AIN1` |
| **1** | **`PA2`** | **`CN1` Pin 8** | **`P05`** (`ADX_RX`) | **`USART0 RxD (Alt)`** / **`LUT0-IN2`** / Async Int / `ADC0 AIN2` |
| **14** | `PB0` | `CN1` Pin 9 | **`P06`** (`ADX_SCL`) | `TWI0 SCL (Def)` / `ADC0 AIN11` |
| **13** | `PB1` | `CN1` Pin 10 | **`P07`** (`ADX_SDA`) | `TWI0 SDA (Def)` / `ADC0 AIN10` |
| **18** | `PC3` | `CN1` Pin 12 | **`P08`** (`ADX_CS`) | `SPI0 SS (Alt)` / `TCA0 WO3*` / `ADC1 AIN9` |
| **17** | `PC2` | `CN1` Pin 13 | **`P09`** (`ADX_MOSI`) | `SPI0 MOSI (Alt)` / `EVOUT2` / `ADC1 AIN8` |
| **16** | `PC1` | `CN1` Pin 14 | **`P10`** (`ADX_MISO`) | `SPI0 MISO (Alt)` / `TCD0 WOD` / `ADC1 AIN7` |
| **15** | `PC0` | `CN1` Pin 15 | **`P11`** (`ADX_SCK`) | `SPI0 SCK (Alt)` / `TCD0 WOC` / `TCB0 WO*` / `ADC1 AIN6` |
| **6** | **`PA5`** | `VBUS_PROT` 1/2 分圧 | `ADX_VBUS_VSENSE` | 幹線電圧計測 (`ADC0 AIN5` / 0〜6.60V) |
| **8** | **`PA7`** | `TP181A2-CR` OUT | `ADX_VBUS_ISENSE` | 供給電流計測 (`ADC0 AIN7` / 1.00 V/A / 0〜3.30A) |
| **10** | **`PB4`** | **`LED_MCU` (`2N7002`)** | **`ADX_LED`** | **ユーザー/AI 制御ステータス LED（Active-HIGH、`TCA0 WO1*` ハード PWM、100kΩ PD）** |
| **9** | **`PB5`** | **`USB_COM_EN` (`10kΩ` 経由)** | **`ADX_USB_COM_DET`** | **Mode 3 (`USB-COM`) 接続検知入力（Active-HIGH、`BOTHEDGES`/`LEVEL` 深睡眠復帰対応）** |
| **4 / 3 / 21** | `VDD` / `GND` / `EP` | `3V3` / `GND` | — | 電源・グラウンド・サーマルパッド |

### 5.4 `PORTMUX` 運用ルール（rev6 確定版）

| レジスタ / ビット | 設定値 | Arduino (`megaTinyCore`) | 動作・用途 |
| :--- | :---: | :--- | :--- |
| `PORTMUX.CTRLB.USART0` | **`0` (Default / リセット初期値)** | `Serial.swap(0)` *(既定)* | **オンボード USB (`PB2`/`PB3` → `CH342K` Port 1)**。`Serial.begin()` のみで即動作 |
| `PORTMUX.CTRLB.USART0` | **`1` (Alternate)** | `Serial.swap(1)` | **IDC 拡張バス (`PA1`/`PA2` + `PA4 XDIR` + `PA2 LUT0-IN2`)**。RS-485 / LIN 稼働時。同時に `PB2`/`PB3` を **SoftwareSerial** として再利用可能 |
| `PORTMUX.CTRLB.TWI0` | `0` (Default 必須) | `Wire.swap(0)` *(既定)* | IDC バス I2C (`P06`/`P07`: `PB0`/`PB1`) |
| `PORTMUX.CTRLB.SPI0` | `1` (Alternate 必須) | `SPI.swap(1)` | IDC バス SPI (`P08`〜`P11`: `PC3`〜`PC0`) |
| `PORTMUX.CTRLC.TCA01` | `1` (Alternate / シングルモード) | `analogWrite(ADX_LED, v)` | `PB4` (`ADX_LED`) の 16-bit ハード PWM 出力（I2C / UART と無干渉） |

### 5.5 標準ヘッダ定義（`ADX_CORE_U_rev6.h`）

```cpp
#pragma once
#include <Arduino.h>

// ADX 20-Pin Bus (v5.0) Logical Pin IDs (P01-P11, S00 omitted)
#define ADX_P01          PIN_PA6  // AIN / DAC0
#define ADX_P02          PIN_PA3  // EXTCLK / TCB1 PWM
#define ADX_P03          PIN_PA4  // UART_DE (USART0 Alt XDIR) / TCD0 WOA / LUT0-OUT
#define ADX_P04          PIN_PA1  // UART_TX (USART0 Alt TxD)
#define ADX_P05          PIN_PA2  // UART_RX (USART0 Alt RxD / LUT0-IN2)
#define ADX_P06          PIN_PB0  // I2C_SCL
#define ADX_P07          PIN_PB1  // I2C_SDA
#define ADX_P08          PIN_PC3  // SPI_CS
#define ADX_P09          PIN_PC2  // SPI_MOSI
#define ADX_P10          PIN_PC1  // SPI_MISO
#define ADX_P11          PIN_PC0  // SPI_SCK

// Standard Bus Function Aliases
#define ADX_AIN          ADX_P01
#define ADX_CLK          ADX_P02
#define ADX_DE           ADX_P03
#define ADX_TX           ADX_P04
#define ADX_RX           ADX_P05
#define ADX_SCL          ADX_P06
#define ADX_SDA          ADX_P07
#define ADX_CS           ADX_P08
#define ADX_MOSI         ADX_P09
#define ADX_MISO         ADX_P10
#define ADX_SCK          ADX_P11

// CORE-U rev6 Onboard USB Serial (SoftwareSerial pins when Serial.swap(1) is active)
#define ADX_USB_TX       PIN_PB2  // USART0 Def TxD
#define ADX_USB_RX       PIN_PB3  // USART0 Def RxD

// CORE-U rev6 Onboard AI Observability, 3-Mode Detection & Status LED
#define ADX_VBUS_VSENSE  PIN_PA5  // ADC0 AIN5 (0.50 V/V, 0.0-6.6V)
#define ADX_VBUS_ISENSE  PIN_PA7  // ADC0 AIN7 (1.00 V/A, 0.0-3.3A, TP181A2-CR)
#define ADX_LED          PIN_PB4  // Active-HIGH (TCA0 WO1* Hardware PWM, 100k PD)
#define ADX_USB_COM_DET  PIN_PB5  // Active-HIGH (USB-COM Connected / Mode 3 Detect, Sleep Wake)
```

---

## 6. LED ＆ 3 Mode 自己認識設計（案 1 採用）【確定】

設計優先順位（**① 耐熱性 → ② 観測性 → ③ 視認性**）に基づき、余剰 2 ピンを **`PB4 = LED_MCU`（単色ハード PWM）** と **`PB5 = USB_COM_DET`（Mode 3 検知）** へ割り当てる「案 1」を採用する。

### 6.1 3 Mode 完全自己認識ロジック（`PB5` × `PA7`）
`PB5 (ADX_USB_COM_DET)` の導入により、ファームウェアは追加コストほぼゼロ（`10kΩ` 抵抗 1 本）で **現在の運用モードを 3 状態すべて 100% 自己認識** できる。

| `ADX_USB_COM_DET` (`PB5`) | `ADX_VBUS_ISENSE` (`PA7`) | 判定される運用モード | 推奨ファームウェア振る舞い例 |
| :---: | :---: | :--- | :--- |
| **`LOW`** | **`≈ 0 mA`** *(逆流阻止中)* | **Mode 1: Full embedded mode** | IDC 外部受電中。USB 関連処理・SoftwareSerial を完全停止し、RS-485/LIN 制御や省電力へ専念 |
| **`LOW`** | **`> 0 mA`** *(本体消費 ≥ 6mA)* | **Mode 2: Semi embedded mode** | `USB-POW` 受電中。V/I/W 監視・負荷保護を行いつつスタンドアロン稼働 |
| **`HIGH`** | 任意 *(通常 > 0 mA)* | **Mode 3: Debug mode** | PC / AI / Android GOT 接続中。`PB5` 割込（Power-Down 深睡眠からも `BOTHEDGES`/`LEVEL` で直接復帰可）を契機に USB コンソール／テレメトリ配信を自動起動 |

### 6.2 2 系統オンボード LED 仕様（同一パッケージ `HVRTB-3433EPX` ×2）
3D プリント筐体（8858 ライトパイプ・覗き穴）の光学位置・高さを揃え、かつ Extended 部品種を増やさないため、`LED_SYS`・`LED_MCU` ともに車載 `-40℃〜+110℃` 品 **`HVRTB-3433EPX` (`C5246356`)** を 5V 駆動＋ローサイド `2N7002` で用いる。

| 系統 | リファレンス | 制御元 | 点灯仕様・役割 |
| :--- | :--- | :--- | :--- |
| **System LED**<br>*(ハード直結 / 0 ピン)* | **`LED_SYS`**<br>(`HVRTB` #1) | ・G: `VBUS_PROT`<br>・B: `USB_COM_EN`<br>・R: 上流 `USB_VBUS` + `FLT` | ・**緑**：通常通電（Mode 1 / Mode 2 正常）<br>・**シアン（G+B）**：Mode 3 Debug / PC・GOT 接続中<br>・**赤単独**：eFuse 遮断異常（短絡・過熱・OVLO） |
| **User / AI LED**<br>*(MCU 制御 / 1 ピン)* | **`LED_MCU`**<br>(`HVRTB` #2) | **`PB4` (`ADX_LED`)**<br>→ `2N7002` (1 個)<br>`TCA0 WO1*` ハード PWM | **Arduino UNO `D13` の思想を継承した単色・ハード PWM ステータス灯**。<br>発光色は `LED_SYS`（緑/シアン/赤）と混同しない **アンバー（R+G 並列・2 抵抗調整、~3〜4mA）** を第一候補（次点：白 R+G+B または単色青。同一フットプリント上の抵抗実装で最終決定） |

### 6.3 `LED_MCU`（`ADX_LED`）標準ステータス表現規約（推奨パターン）
ハードウェア `TCA0` PWM により CPU 負荷ゼロで輝度を自在制御できるため、標準ライブラリ／`AI-Firmata` では以下の 5 段階セベリティ表現を推奨規約とする：

| レベル | 発光パターン | 意味・ユースケース |
| :--- | :--- | :--- |
| **`OFF`** | 消灯（Duty `0%`） | 待機 / 省電力 / 暗所運用 |
| **`INFO`** | **ぼんやり点滅（PWM 呼吸点滅 / Breathing）** | 正常稼働（ハートビート）・アイドル・*AI's First Blink* |
| **`WARNING`** | **間欠点滅（例：2 秒周期の短点滅）** | 注意状態・軽微なリトライ・温度/電流しきい値接近 |
| **`ERROR`** | **はやい点滅（例：5〜10 Hz 高速点滅）** | 通信断・センサー異常・要確認エラー |
| **`CRITICAL`** | **全灯（Duty `100%` 連続点灯）** | 重大異常・緊急停止・フェイルセーフロック |

### 6.4 ディープスリープ向け省電力チューニング【OPTION / MAY（MUST ではない）】
Mode 1（`Full embedded mode`）における Power-Down / Standby ディープスリープ運用（詳細は [deep_sleep_potential.md](deep_sleep_potential.md) 参照）のポテンシャルをさらに高めるため、回路図・アートワーク設計時に以下の **コスト `$0` 省電力チューニングを任意オプション（OPTION / MAY）** として検討・敷設してよい。

> [!NOTE]
> **適用原則（非必須・優先順位の厳守）**:  
> 本項のチューニングは **MUST 要件ではない**。§1.2 の設計優先順位（**① 耐熱性・耐振動性 → ② 観測性（ADC 精度・ノイズ耐性） → ③ 視認性**）や、8858 基板レイアウト制約・量産性・低温（`-40℃`）時特性と**最終的に矛盾が生じる場合は、無理に採用せず標準定数（`10kΩ` 分圧 / `0Ω` 抵抗なし・標準 LED 電流）を優先する**。

| オプション項目 | 標準チューニング案（OPTION） | 期待効果（Mode 1 睡眠時） | 矛盾・見送り判断基準（非 MUST 条件） |
| :--- | :--- | :--- | :--- |
| **① `VBUS_VSENSE` 分圧の高抵抗化** | `10kΩ + 10kΩ` → **`100kΩ + 100kΩ` ＋ `0.1µF X7R`** | 待機電流 `250µA` → **`25µA`**（約 `225µA` 削減） | `+85℃` 高温時の `PA5` リーク電流・ADC サンプリング誤差・外来ノイズ耐性への悪影響が観測・懸念される場合は `10kΩ` を維持 |
| **② `LED_SYS` 緑の低電流化 ＆ 脱着式 `0Ω` 抵抗ブリッジ** | 緑 LED 電流を **`0.5〜1.0 mA`** 級へ適正化し、制限抵抗 `R_LED_G` と直列に **大きめの `0Ω` 抵抗（`0805` Basic 推奨、最小 `0603` / デフォルト実装）** を配置。<br>・**配置指針**：周辺部品と干渉しない基板側面・端寄りの作業しやすい位置に配置し、`JP_PWR_LED` 等の明瞭なシルクを印字 | ・通常時：約 `1.5〜2.0 mA` 削減<br>・`0Ω` 取り外し（または工場 DNP）時：ボード全体待機 **`~200〜430 µA`（`1〜2 mW`）級** へ到達<br>・カッター傷による防湿劣化・OSP 酸化ゼロ。再有効化時はんだブリッジしても `R_LED_G` が直列に残るため LED 焼損リスクゼロ | ・`-40℃` 低温〜`+85℃` 高温での視認性が不足する場合は標準 `2〜3 mA` を優先<br>・アートワーク余白や 8858 キープアウト制約と競合する場合は `0Ω` 抵抗を不採用とする |

---

## 7. 主要部品構成（BOM）＆ コスト試算【仮確定】

ベースラインは [rev5/spec_proposal.md §7](../rev5/spec_proposal.md#7-主要部品構成-bom--コスト試算)（120 pcs ロット想定原価 **約 `$5.60 〜 $5.75`**、上限 `$6.50` 以内）を継承する。rev6 では `LED_MCU` の `2N7002` が 3 個 → 1 個へ 2 個削減されるため、部品点数・原価ともに rev5 比でわずかに改善する見込み（**想定原価 約 `$5.55 〜 $5.70` / 想定売価 `$22.20 〜 $22.80`**）。

### 回路図作成前の最終選定タスク（残り 3 点）
1. **`01-USB_Pow` (6P Type-C) / `01-USB_Com` (16P Type-C)**：THT シェル固定（耐振動）＋ LCP 高耐熱ハウジング ＋ 動作/保存温度 `-40℃ 〜 +85℃`（望ましくは `+105℃` 公称品）の LCSC 実型番を確定。
2. **`03-eFuse` (`TPS259470A` / `0L`)**：発注時の LCSC 在庫・単価確認（Auto-Retry 型 `A` 優先）。
3. **`LED_MCU` (`HVRTB-3433EPX` #2) の最終発光色 ＆ 制限抵抗値**：アンバー（`R+G`）・白（`R+G+B`）・単色青（`B`）から最終決定し、`-40℃` 低温時の $V_f$ 上昇を織り込んだ制限抵抗値を確定。

---

## 8. 検証・実証試験計画【ドラフト】

§2 で定めた 3 Mode 温度仕様（特に **Mode 1: `-40℃ 〜 +85℃` 理論仕様**）、§5 の `ADX-BUS-SPEC-v5.0` 通信、§6 の 3 Mode 自己認識を実測データで裏付け、正式仕様値へ昇格させるための試験項目を以下に定義する。

1. **恒温槽による温度実証試験（Mode 1: `-40℃` ＆ `+85℃` 連続稼働）**:
   - IDC 経由 5.0V 受電・`USB-COM`/`USB-POW` 非接続状態でのコールドスタート（`-40℃`）および高温連続稼働（`+85℃`、連続 24〜72 時間目安）。
   - ATtiny1616 内蔵温度センサ、`VBUS_VSENSE`、`LED_MCU` 輝度、および拡張バス経由通信（UART / LIN / I2C / SPI）の異常・リセット・エラー率を計測。
   - **回復試験**: `+85℃`（および保存ストレス `+100℃` 付近）曝露後に常温へ戻し、`USB-COM` を接続して `CH342K` の UPDI / UART 2 ポートが正常復帰することを確認。
2. **Mode 2 / Mode 3 熱・負荷ディレーティング試験**:
   - `USB-POW` から 5V 2.0A 連続出力時の `TPS259470`・`10mΩ` シャント・基板表面温度上昇をサーモグラフィ／熱電対で実測し、Mode 2 (`-20〜+70℃`) の電流ディレーティング曲線を策定。
3. **電流・電圧計測（`VBUS_ISENSE` / `VBUS_VSENSE`）温度ドリフト検証**:
   - 常温・低温・高温での `TP181A2-CR`（`PA7`）+ `10mΩ` および `PA5` 分圧の精度検証。
4. **`CH342K` 非接続遮断・`PB5` 3 Mode 自己認識 ＆ ホットプラグ検証**:
   - Mode 1/2 での待機電流（目標 `6〜7mA`）および `USB-COM` 単独先挿し時の `PB5` / `PA0` / `PB2` / `PB3` 回り込み電流ゼロを実測。
   - `PB5 (ADX_USB_COM_DET)` による Power-Down 深睡眠からの `USB-COM` 挿入復帰、および `PA7 (ISENSE)` 併用での Mode 1 / Mode 2 / Mode 3 自動判定を実機検証。
   - PC（Windows / Linux / macOS）および Android OTG でのホットプラグ安定性を確認。
5. **拡張バス RS-485 (`XDIR` + `LINAUTO`) ＆ USB SoftwareSerial 同時運用検証**:
   - `Serial.swap(1)` でハード `USART0`（`PA1`/`PA2` + `PA4 XDIR` + `PA2 LUT0-IN2`）を絶縁 RS-485 CARD へ接続した状態で、① `-40〜+85℃` での `LINAUTO` 自動ボーレート通信、② `PA4 (XDIR)` ガードタイム波形と未起動時プルダウンによるバス非占有、③ `PB2`/`PB3` SoftwareSerial による USB デバッグ出力同時稼働のエラー率を検証。

---

## 9. 改訂・決定履歴（Decision & Revision Log）

| 版 | 日付 | 主な決定・追記内容 |
| :---: | :---: | :--- |
| **v0.1** | 2026-11-27 | 初版作成。CORE-U rev6 の ADX フラッグシップ化・CORE-A の付加価値型派生ボードへの位置づけ変更を確定。PLC 耐熱性ベンチマーク（[plc_thermal_benchmark_report.md](plc_thermal_benchmark_report.md)）に基づき 3 Mode 耐熱仕様（Mode 1: `-40〜+85℃` 理論仕様＋後日試験実証）を確定。 |
| **v0.2** | 2026-11-27 | **§5 を確定化**：`ADX-BUS-SPEC-v5.0`（パターン B）を採用し、`UPDI (S00)` を IDC バスから完全オミット（HV ノイズ対策・プログラミング責務の Core 側集約）、`Pins 1, 3, 5 = GND`、`Pin 6 = P03 (UART_DE / PA4 XDIR)` 新設、論理ピン `P01`〜`P11` 連番化、RS-485 CARD 側 `UART_DE` プルダウン必須規約を策定。ATtiny1616 の UART 逆転（USB=`PB2`/`PB3` 初期値＆SoftwareSerial、IDC=`PA1`/`PA2`+`PA4`）と観測系（`PA5`/`PA7`）を確定。規格改訂計画を [ADX_BUS_SPEC_V5_REVISION_PLAN.md](../../../memo/PLAN/ADX_BUS_SPEC_V5_REVISION_PLAN.md) として立案。 |
| **v0.3** | 2026-11-28 | **§6 を確定化（案 1 採用・アーキテクチャ仕様完成）**：`PB4 = ADX_LED`（`LED_MCU`、車載 110℃ `HVRTB-3433EPX`・`TCA0 WO1*` ハード PWM 単色ステータス灯、`INFO`/`WARNING`/`ERROR`/`CRITICAL` 表現規約定義）、`PB5 = ADX_USB_COM_DET`（`USB_COM_EN` 読込・深睡眠復帰対応・`PA7` 併用による 3 Mode 完全自己認識）を確定。§4 回路ブロック図・§5 全 20 ピン表・`ADX_CORE_U_rev6.h`・§7 BOM・§8 試験計画をすべて整合更新。 |
| **v0.4** | 2026-11-28 | **§6.4 を追記**：[deep_sleep_potential.md](deep_sleep_potential.md) に基づき、Mode 1 睡眠時の `$0` 省電力チューニング（`VBUS_VSENSE` `100k/100k` 化、`LED_SYS` 緑の低電流化＋カット可能ソルダージャンパ）を **OPTION / MAY（上位優先順位やレイアウト・精度と矛盾する場合は見送り可、MUST ではない）** として明記。 |
| **v0.5** | 2026-11-28 | **§6.4 を精査更新**：`LED_SYS` 緑の省電力オフ手段をカット可能ソルダージャンパから **大きめパッケージの脱着式 `0Ω` 抵抗ブリッジ（`0805` Basic 推奨・基板端寄りの作業しやすい位置へ明示配置）** へ変更。パターンカットによる防湿劣化・OSP 酸化を回避しつつ、工場 PCBA DNP 量産および安全な再ブリッジ復帰を両立（引き続き OPTION / 非 MUST 扱い）。 |
