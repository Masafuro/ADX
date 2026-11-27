<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-U (rev5) 技術仕様書
〜 **「筐体フレンドリー × AI フレンドリー × 組込耐熱 (-40℃〜+105℃)」3 Mode 対応・脊髄マイコンボード** 〜

> **AI's First Blink. No Arduino IDE. No BOOTSEL Button. Full Embedded -40℃〜+105℃ Ready.**

- **ステータス**: 設計仕様案（rev5 / 3 Mode 耐熱・観測再編版）
- **最新コンセプト**: [concept_20261127_1856.md](concept_20261127_1856.md)
- **AI フレンドリー構想**: [CORE-U_AI_friendly_microcontroller_board.md](../CORE-U_AI_friendly_microcontroller_board.md)
- **前世代仕様**: [spec_proposal.md](../spec_proposal.md)（rev4）
- **関連規格**: [8858 Form Factor](../../../docs/ja/8858_formfactor.md) / [ATtiny1616 Mapping](../../../docs/spec/mcu_mapping/attiny1616.md) / [CORE-A5 仕様検討書](../../archive/CORE-A5/spec_study.md)

---

## 1. 概要と設計思想（3 Mode × 優先順位）

**ADX CORE-U (rev5)** は、以下の 3 本柱を単一の `8858` 規格マイコンボードへ統合した、**第 4 世代 AI フレンドリー・組込脊髄ボード** です。

1. **筐体フレンドリー（8858 Form Factor）**：3D プリント筐体・PLC 型縦挿し・市販ケースへそのまま収まる固定外形・光学マーカー・低自己発熱設計。
2. **AI フレンドリー（独立 2 ポート ＋ 精密電流電圧観測）**：VS Code / Antigravity から物理操作ゼロで直接操作（`AI-Firmata`）と UPDI 強制焼込（`adx-cli`）を 2 秒往復しつつ、電圧・電流・電力・内部温度をリアルタイム観測。
3. **組込耐熱・耐振動（-40℃〜+105℃ ＆ Eject Header）**：USB 接続でのデバッグ時を除く組込稼働時において、CORE-A の血統を引く **-40℃〜+105℃ 定格部品構成** と **ラッチ付き IDC Eject Header** による高い耐環境性を発揮。

### 設計上の明確な優先順位
トレードオフが生じた際は、次の優先順位に従って設計判断を統一します：
1. **第一優先：耐熱性**（`Full embedded mode` で稼働する全部品を `-40℃〜+105℃` 以上で統一）
2. **第二優先：観測性**（AI のための高精度な電圧・電流・温度テレメトリを妥協なく確保）
3. **第三優先：LED 表示灯**（耐熱性を満たす車載グレード 3 色 LED を用い、System / GPIO の 2 系統へ明快に分離）

### rev4 → rev5 の主要アップデート一覧

| 項目 | rev4（旧） | **rev5（新：3 Mode・耐熱・AI 観測版）** | 設計意図 |
| :--- | :--- | :--- | :--- |
| **開発体験・ソフトウェア** | Arduino IDE 1.8.x 推奨 | **Arduino IDE 撤廃。VS Code / Antigravity Native（A/B パターン両立）** | 工場出荷 **AI-Firmata (MCP)** と **CLI＋UPDI 自動焼込** を AI が 2 秒で自在往復 |
| **稼働モード定義** | 卓上 USB 運用前提 | **3 Mode 定義（Full embedded / Semi embedded / Debug）** | 端子接続状態により耐熱・耐振動の保証範囲を明快に分離 |
| **拡張コネクタ (`CN1`)** | SMD Box Header (`C601970`) | **THT Eject Header (`C9039`, -55〜105℃, ラッチ付)** | `Full embedded mode` での IDC 経由受電・高耐振動・パッド剥離防止を実現 |
| **電源保護・トポロジ** | `AP22814` (85℃) ＋ `ME6211` ＋ `X5R` (85℃) | **ADX 標準双方向 VBUS 幹線（MCU 制御なし）＋ TI `TPS259470` (125℃) ＋ `TLV75533` (125℃) ＋ 全 `X7R` ＋ 6P `USB-POW`** | OR-ing 不要で IDC 双方向受電と AC アダプタ／ケーブル保護を両立し、電源系を完全 125℃ 化 |
| **USB-COM 隔離** | `CH342K` (85℃) 常時 3V3 通電 (~15mA) | **`USB-COM` 非接続時に電源＆信号 (`SN74LV4066A`) を完全遮断** | `CH342K` を 0mW の保存状態 (`-55〜+105℃`) へ移行しつつ待機電流・自己発熱を約 1/3 へ削減 |
| **物理観測（Observability）** | 電圧・電流は盲目（`FLG` のみ） | **`VBUS_ISENSE` (`PA4`: 10mΩ + `TP181A2` 125℃) ＋ `VBUS_VSENSE` (`PA5`) 標準内蔵** | I2C を汚さず mA 級の精密電流・電圧・電力 (W)・チップ温度を AI がリアルタイム観測 |
| **オンボード LED** | 汎用 0805 単色 LED×4 (85℃) | **車載 110℃ 3 色 LED `HVRTB-3433EPX` ×2（`LED_SYS`＋`LED_AI` 2 系統分離）** | `LED_SYS` でハード電源/異常/Debug を直感表示し、`LED_AI` は 3ch フルカラー RGB を -40〜+110℃ で動作 |

---

## 2. 3 Mode 運用定義（端子構成と耐環境プロファイル）

CORE-U (rev5) に実装される物理端子は **`USB-COM`・`USB-POW`・`IDC 20P (Eject Header)` の 3 つのみ** です。接続状態に応じて以下の 3 モードとして振る舞います。

| モード | `USB-COM` | `USB-POW` | `IDC 20P` | 給電経路 | `CH342K` 状態 | 耐熱・耐振動プロファイル |
| :--- | :---: | :---: | :---: | :--- | :--- | :--- |
| **1. Full embedded mode** | 非接続 | 非接続 | **接続 (ラッチ固定)** | **IDC 20P 経由で外部受電**（POWER CARD 等） | **完全遮断 (0mW)**<br>保存領域 `-55〜+105℃` | **◎ 最高水準**<br>全稼働部品 `-40℃〜+105℃` 以上＋THT ラッチ固定で十分な耐熱・耐振動を主張可能 |
| **2. Semi embedded mode** | 非接続 | **接続 (6P)** | **接続 (ラッチ固定)** | **`USB-POW` より 5V 2A 給電**（IDC へ出力） | **完全遮断 (0mW)**<br>保存領域 `-55〜+105℃` | **○ 準高耐熱（耐振動に構造限界）**<br>`USB-POW` 挿抜構造ゆえ耐振動は Mode 1 に劣るが、コア回路は 105〜125℃ 定格で高い熱マージンを確保 |
| **3. Debug mode** | **接続 (16P)** | **接続 (6P)** | 任意 / 接続 | **`USB-POW` より 5V 2A 給電**（PC からは非給電） | **自動起動 (3.3V)**<br>UPDI + UART 同時稼働 | **△ 開発用（努力目標）**<br>`CH342K`・Type-C 制約により `-40℃〜+85℃` が目安。可能な限り耐熱・信頼性を引き上げる |

---

## 3. 開発ツールチェーン ＆ UX 仕様（A / B パターン両立）

```text
           ┌──────────────────────────────────────────────────────────┐
           │     Host PC : VS Code / Antigravity (AI エージェント)     │
           └──────────────┬──────────────────────────┬────────────────┘
                          │                          │
        [ A パターン: adx-cli / UPDI ]     [ B パターン: adx-mcp / AI-Firmata ]
        コード生成 → 1秒ビルド              箱出し 0 秒で直接操作・リアルタイム観測
        → 強制焼込 → 暴走時自己修復         (車載RGB LED / V・I・W・温度 / I2C / SPI)
                          │                          │
                          ▼                          ▼
     [ PC ] ──▶ USB-COM (16P / VBUS 非給電・検知のみ) ──▶ [ パワーゲート & SN74LV4066A ]
                                                             └─▶ [ WCH CH342K (Mode 3 のみ稼働) ]
                                                                   ├─▶ Port 0: SerialUPDI ──▶ PA0
                                                                   └─▶ Port 1: UART       ──▶ PA1/PA2

     [ 5V 3A ACアダプタ ] ──▶ USB-POW (6P) ──▶ eFuse (TPS259470) ──▶ VBUS 幹線 (5V 2A) ◀──▶ IDC 20P Eject Header
                                                                                              (Mode 1 では逆方向受電!)
```

### 3.1 B パターン：工場出荷標準（`ADX AI-Firmata` / MCP）
- 箱から出して接続した瞬間から、コンパイル不要で Antigravity が `Port 1` 経由で直接ボードを操作可能。
- オンボードの車載フルカラー RGB LED（`LED_AI`：`PB4`/`PB5`/`PA7`）により、拡張 CARD ゼロの状態でも 60 秒以内に **「AI が L チカさせた（AI's First Blink）」** を完結できる。
- 標準コマンド例：`set_rgb(r, g, b)`, `get_telemetry()`（VBUS 電圧・供給電流・消費電力・3.3V VDD・MCU 内部温度）, `gpio_rw`, `pwm_set`, `adc_read`, `i2c_scan/xfer`, `spi_xfer`。

### 3.2 A パターン：AI 自律ビルド ＆ UPDI 強制焼込（`adx-cli`）
- Arduino IDE を不要とし、VS Code / Antigravity から CLI ワンコマンドで `build → flash (Port 0 UPDI) → monitor (Port 1 UART)` を自動実行。
- スケッチ暴走やハングが発生しても、物理ボタン操作ゼロで AI が `Port 0 (UPDI)` から 2 秒で復旧・再書込、あるいは B パターン（`AI-Firmata`）への復元が可能。
- 完成後は `USB-COM` を抜けば **Semi embedded mode**（待機電流約 1/3 削減）、さらに `USB-POW` も抜いて POWER CARD 等から IDC 給電すれば **Full embedded mode（-40℃〜+105℃・耐振動稼働）** へシームレスに移行する。

---

## 4. 基本電気・環境仕様 (Electrical & Environmental Specifications)

| 項目 | 定格 / 仕様 | 温度定格・備考 |
| :--- | :--- | :--- |
| **動作温度（Full embedded mode）** | **`-40℃ 〜 +105℃`** | 全稼働部品 `-40℃〜+105℃` 以上（`CH342K` は完全無通電・保存 `-55〜+105℃`） |
| **動作温度（Debug mode）** | `-40℃ 〜 +85℃`（目安） | `CH342K` および民生 Type-C ケーブル・コネクタの上限に準拠 |
| **主電源入力 (`USB-POW`)** | **5.0V DC（定格 2.0A / 10W）** | **6P Power-Only Type-C**（CC 5.1kΩ×2）、TVS (`SMF5.0CA-MS`: -55〜150℃) |
| **通信ポート (`USB-COM`)** | **USB 2.0（データ専用・PC 非給電）** | 16P Type-C、`USBLC6-2SC6` (-55〜125℃) ESD 保護。VBUS は 25µA 接続検知のみ |
| **電源保護 eFuse** | **TI `TPS259470A`** *(入手性次第で `0L`)* | **`-40℃〜+125℃`**、$R_{ON}=28.3\text{m}\Omega$、**真の逆流阻止**、OVLO（~6.0V）、電流制限（~3.0A）、**EN 固定有効（MCU 制御なし）** |
| **システム幹線電源 (`VBUS_PROT`)** | **4.5V 〜 5.25V DC（双方向）** | ADX 標準幹線：`Semi/Debug` で 2A 給電出力、`Full embedded` で外部 CARD より受電 |
| **内部ロジック電源 (`3V3`)** | **3.3V DC（最大 500mA）** | **TI `TLV75533PDBVR`**（**`-40℃〜+125℃`**、SOT-23-5、`VBUS_PROT` より生成） |
| **供給電流計測 (`VBUS_ISENSE`)** | **0.00A 〜 3.30A（1.00 V/A）** | `10mΩ` ±1% 1206 シャント (-55〜155℃) ＋ **3PEAK `TP181A2-CR`** (**`-40℃〜+125℃`**, 100V/V) → `PA4` |
| **幹線電圧計測 (`VBUS_VSENSE`)** | **0.00V 〜 6.60V（分圧比 1/2）** | `10kΩ + 10kΩ` ±1% (-55〜155℃) ＋ `0.1µF X7R` → `PA5` |
| **主 MCU** | **Microchip `ATtiny1616-MNR`** | **`-40℃〜+105℃`**、QFN-20-EP (3×3mm)、3.3V / 10MHz 内蔵発振（水晶なし）、Flash 16KB / SRAM 2KB |
| **USB ブリッジ IC** | **WCH `CH342K`** | 動作 `-40〜+85℃` / **保存 `-55〜+105℃`**、Dual CDC（Mode 3 のみ通電＆信号接続） |
| **USB 信号隔離スイッチ** | **TI `SN74LV4066A`** | **`-40℃〜+125℃`**、4ch 双方向アナログスイッチ（Mode 1/2 で `PA0/PA1/PA2` を完全 Hi-Z 隔離） |
| **オンボード LED（2 系統）** | **車載 3 色 LED `HVRTB-3433EPX` ×2** | **`-40℃〜+110℃`**（`C5246356`）。`LED_SYS`（ハード監視）＋ `LED_AI`（MCU 3ch RGB 制御） |
| **受動部品（MLCC / 抵抗）** | **MLCC 全 `X7R` / 抵抗 `0603WAF`** | MLCC `-55〜+125℃`（`X5R` 全廃）、抵抗 `-55〜+155℃` |
| **拡張インターフェース (`CN1`)** | **20P THT Eject Header (`C9039`)** | **`-55℃〜+105℃`**、2.54mm 2×10P ラッチ付きバーティカル牛角、`ADX-BUS-SPEC-v4.2` 準拠 |
| **PCB 基板材・外形** | **`FR4-TG155`（高耐熱） / `8858` 規格** | `88.0 × 58.0 mm`、M3 取付穴 `76.0 × 46.0 mm`、光学マーカー M1〜M4 対応 |

---

## 5. 回路アーキテクチャ (Circuit Architecture)

```text
                        【 ADX CORE-U rev5 回路ブロック図（3 Mode・ADX標準電源版） 】

  [ USB-POW (6P Type-C) ] ──▶ [ TVS: SMF5.0CA-MS ] ──▶ (USB_VBUS: 上流 5V)
  (CC 5.1kΩ×2 / 電源専用)                                 │
                                 ┌────────────────────────┴──────────────────────┐
                                 │                                               ▼
                                 │                        [ 03-eFuse: TI TPS259470A (-40~125℃) ]
                                 ▼                         ・EN 固定有効 / OVLO≈6.0V / ILIM≈3.0A
                      [ LED_SYS 赤 (R): 異常灯 ] ◀─────────・Pin FLT (Active-LOW, 4.7V保護付)
                      ※上流給電のため遮断中も赤点灯!       ・真の逆流阻止 (Mode 1 で上流へ逆流ゼロ)
                                                           │ Pin OUT (VBUS_EFUSE)
                                                           ▼
                                                [ R_SHUNT: 10mΩ 1% 1206 ]
                                                  ├──▶ [ U2: TP181A2-CR (100V/V) ] ──▶ PA4 (VBUS_ISENSE)
                                                  ▼
                                        ═══════════════════════════════════
                                           (VBUS_PROT : ADX 双方向 5V 幹線)
                                        ═══════════════════════════════════
                                          │         │          │          │
       ┌──────────────────────────────────┘         │          │          └──────────────────────────┐
       ▼                                            ▼          ▼                                     ▼
  [ CN1: 20P THT Eject Header ]     [ 10kΩ/10kΩ 分圧 ]   [ 04-LDO: TI TLV75533 ]       [ 車載110℃ LED ×2 (5V駆動) ]
  (C9039: -55〜+105℃ / ラッチ付)        ──▶ PA5          (3.3V / 500mA / 125℃)          ├─ LED_SYS (HVRTB #1):
   ・Mode 2/3: 外部CARDへ 5V 2A給電      (VSENSE)           │ (3V3)                     │   G = VBUS_PROT (正常緑)
   ・Mode 1  : 外部CARDから 5V受電!                         ├──▶ MCU VDD (ATtiny1616)   │   R = FLT (上流給電・異常赤)
                                                            ├──▶ U2 (TP181A2-CR)        │   B = USB_COM_EN (Debug青→シアン)
                                                            ├──▶ U_SW (SN74LV4066A)     └─ LED_AI (HVRTB #2 / 2N7002駆動):
                                                            │                               R ◀── PB4 (TCA0 WO1* PWM)
  [ USB-COM (16P Type-C) ] ──▶ [ USBLC6-2SC6 ] ──▶ D+/D-    │                               B ◀── PB5 (TCA0 WO2* PWM)
   (VBUS非給電 / CC 5.1kΩ×2)                                ▼                               G ◀── PA7 (GPIO / ソフト・CCL PWM)
         │ VBUS (100k/100k 分圧: 25µA検知のみ)       [ Q2: P-MOS (AO3401A) ]
         ▼                                            │ (3V3_USB: Mode 3 のみ通電!)
  [ Q1: 2N7002 ] ──▶ (USB_COM_EN_N) ──────────────────┘
         │                                            ▼
         └──▶ [ Q3: 2N7002 ] ──▶ (USB_COM_EN) ──▶ [ U1: WCH CH342K (Mode 1/2 は 0mW保存状態) ]
                    │                                   ▲
                    ├──▶ LED_SYS 青 (Q_DBG)             │ (UPDI / TX / RX)
                    └──▶ [ U_SW: SN74LV4066A (3ch) ] ◀──┴──▶ PA0 (UPDI), PA1 (TX), PA2 (RX, 100kΩ PU)
```

### 5.1 ADX 標準双方向 VBUS 幹線と eFuse の責務分離
- **eFuse を MCU 制御から切り離し ADX 標準へ統一**：`TPS259470` の `EN` は固定有効とし、AC アダプタ保護・IDC リボンケーブル焼損防止・過電圧遮断（OVLO）・逆流阻止に専念させます。
- **OR-ing Controller 不要で `Full embedded mode` 起動**：`04-LDO (TLV75533)` を `VBUS_PROT` 幹線へ直結することで、`Mode 1 (Full embedded)` では IDC 20P からの 4.5〜5.25V 受電で MCU が自動起動し、`TPS259470` のバック・トゥ・バック FET が未接続の `USB-POW` 側への逆流を完全阻止します（逆流阻止は正常動作扱いであり `FLT` も誤発火しません）。
- **復帰方式（`TPS259470A` 優先）**：組込現場での過渡過電流からの自動復帰性を重視し **Auto-Retry 型 (`TPS259470A`)** を第一候補としつつ、実発注時の LCSC 入手性・在庫状況に応じて Latch-Off 型 (`TPS259470L`) も許容します。
- **OVLO と TVS の協調設計**：入力側 TVS (`SMF5.0CA-MS`) のブレークダウン電圧 $V_{BR} \approx 6.4\text{V}$ より低い **`6.0V` 前後** に `TPS259470` の OVLO 閾値を設定することで、誤って 9V/12V アダプタを接続した際に TVS 導通より先に高速遮断（~1.2µs）＆赤 LED 警告を行えます。

### 5.2 `CH342K` 非接続時完全遮断回路（Mode 1/2 での 105℃ 生存＆省電力）
- `USB-COM` の VBUS は `100kΩ + 100kΩ` 分圧（25µA）で `Q1 (2N7002)` の絶縁ゲートを叩くだけに使い、PC からの負荷電流引き抜き・`3V3 = 0V` 時のバックパワーを物理的にゼロに保ちます。
- **`USB-COM` 抜去時（Mode 1 / Mode 2）**：
  1. `Q2 (P-MOS)` が OFF になり、`CH342K` の電源 (`3V3_USB`) が完全に 0V へ遮断されます（消費電流 ~15mA 削減 → ボード単体待機電流は **約 6〜7mA** まで低下し、密閉 8858 筐体での自己発熱を極小化）。
  2. `U_SW (SN74LV4066A, -40〜125℃)` の 3 チャンネルが全オープン（Hi-Z、リーク <0.1µA）になり、`ATtiny1616` の `PA0/PA1/PA2` から無通電 `CH342K` への回り込み電流を完全遮断します（`PA2/RX` は MCU 側 100kΩ プルアップで偽 BREAK 防止）。
  3. 結果として `CH342K` は完全無通電の **保存温度領域（`-55℃〜+105℃`）** へ移行し、`Full embedded mode` の 105℃ 環境を無傷で生き抜きます。

### 5.3 ハイサイド電流計測（`PA4`）＆ 幹線電圧計測（`PA5`）
- **電流計測 (`VBUS_ISENSE`)**：`TPS259470 OUT` と `VBUS_PROT` 幹線の間に `10mΩ` ±1% 1206 シャントを配置し、3PEAK `TP181A2-CR`（100 V/V, `-40〜+125℃`）で増幅（**`1.00 V/A`**）。
  - 入力オフセット Typ **`±10µV`（±1mA 相当）**、ゲイン誤差 Max **`±1%`**、オフセット温度ドリフト **`0.5µV/°C`**（+80℃ 変化でも 4mA 相当）という計測器級の精度を `-40℃〜+125℃` 全域で維持。
  - ATtiny1616 内部基準電圧の動的切替により、`3.3V` 基準（0〜3.30A / `3.22 mA/LSB`）、`1.1V` 基準（0〜1.10A / `1.07 mA/LSB`）、`0.55V` 基準（0〜0.55A / `0.54 mA/LSB`）のオートレンジ観測が可能。
  - `Mode 2/3` ではシステム総消費電流（本体＋外部 CARD）を計測し、`Mode 1 (Full embedded)` ではシャントに逆電流が流れない（0A）ため `TP181A2` へ負差動が掛からない安全構造です。
- **電圧計測 (`VBUS_VSENSE`)**：`VBUS_PROT` を `10kΩ + 10kΩ`（±1%, -55〜155℃）で 1/2 分圧し、`0.1µF X7R` 経由で `PA5 (AIN5)` へ入力（`0.00V〜6.60V`、分解能約 `6.45 mV/LSB`）。全 3 Mode で実際の 5V 幹線電圧を常時監視できます。

### 5.4 2 系統・車載 110℃ フルカラー LED 構成（同一型番 `HVRTB-3433EPX` ×2）
民生 85℃ 上限の `WS2812B` を廃し、CORE-A5 実績の車載 `-40℃〜+110℃` 3 色 LED **`HVRTB-3433EPX` (`C5246356`)** を 2 個並列配置して、**ハードウェア監視 (`LED_SYS`)** と **MCU 制御 (`LED_AI`)** を明快に分離します。

- **5V 駆動＋ローサイド `2N7002` の採用理由**：InGaN 青・緑 LED は `-40℃` 低温時に順電圧 $V_f$ が ~3.2V 付近まで上昇するため、`3.3V` 直結ではなく `5V`（`VBUS_PROT` / `USB_VBUS`）から各色 `2〜3mA` 程度で駆動します。これにより全温度帯での安定輝度・105℃ での低発熱・`3V3` アナログ基準レールの無ノイズ化を同時に達成します。

| 系統 | リファレンス | 色 | 接続・制御元 | 点灯状態の意味 |
| :--- | :--- | :---: | :--- | :--- |
| **System LED**<br>*(ハード直結)* | **`LED_SYS`**<br>(`HVRTB` #1) | **G (緑)** | `VBUS_PROT`（5V 幹線） | **通常通電（Embedded 稼働＝純粋な緑）** |
| | | **B (青)** | `VBUS_PROT` ＋ `Q_DBG (USB_COM_EN)` | **Debug 接続（`G+B`＝シアンで PC/AI リンク表示）** |
| | | **R (赤)** | **上流 `USB_VBUS`** ＋ `TPS259470 FLT` | **eFuse 遮断異常（短絡・過熱・OVLO 時は緑/青が消え赤単独点灯！）** |
| **GPIO LED**<br>*(MCU 制御)* | **`LED_AI`**<br>(`HVRTB` #2) | **R (赤)** | **`PB4`** → `2N7002` | `TCA0 WO1*` ハード PWM 対応（**CORE-A5 Red LED とピン完全一致**） |
| | | **B (青)** | **`PB5`** → `2N7002` | `TCA0 WO2*` ハード PWM 対応（**CORE-A5 Blue LED とピン完全一致**） |
| | | **G (緑)** | **`PA7`** → `2N7002` | デジタル 8 色 / ソフト PWM / `TCD0→CCL LUT1-OUT` PWM 対応 |

---

## 6. ピン配分と ADX 20P IDC アサイン (Pin Assignment)

### 6.1 ATtiny1616-MNR（QFN-20-EP）全 20 ピン割り当て（rev5 確定案）
ADX 20P IDC バス側（`P01`〜`P10`、`S00`）は [attiny1616.md](../../../docs/spec/mcu_mapping/attiny1616.md)（`ADX-BUS-SPEC-v4.2`）と完全同一。オンボード専用 5 ピンは **観測 2 本 (`PA4`/`PA5`) ＋ 車載フルカラー RGB 3 本 (`PB4`/`PB5`/`PA7`)** として 1 本の過不足もなく使い切ります。

| QFN Pin | ポート名 | 接続先 | ADX 論理 ID / マクロ | 役割・仕様 |
| :---: | :--- | :--- | :--- | :--- |
| **1** | `PA2/RX` | `SN74LV4066A` → `CH342K` Port 1 TX | *(オンボード)* | USART0 Alt-RX（Mode 3 のみ接続・MCU 側 100kΩ PU） |
| **2** | `PA3/EXTCLK` | `CN1` Pin 5 | **`P02`** (`ADX_CLK`) | 外部クロック / PWM (`TCB1`) / ADC0 |
| **3** | `GND` | `GND` | `GND` | システムグラウンド |
| **4** | `VDD` | `3V3` | *(オンボード)* | 内部ロジック電源 (`TLV75533` 3.3V) |
| **5** | **`PA4`** | **`TP181A2-CR` OUT** | **`ADX_VBUS_ISENSE`** | **供給電流計測 (`ADC0 AIN4` / 1.00 V/A / 0〜3.3A)** |
| **6** | **`PA5`** | **`VBUS_PROT` 1/2 分圧** | **`ADX_VBUS_VSENSE`** | **幹線電圧計測 (`ADC0 AIN5` / 0.50 V/V / 0〜6.6V)** |
| **7** | `PA6/AIN` | `CN1` Pin 3 | **`P01`** (`ADX_AIN`) | ADX バス標準アナログ入力 / DAC0 / GPIO |
| **8** | **`PA7`** | **`LED_AI` Green (`2N7002`)** | **`ADX_LED_G`** | **車載 RGB LED 緑（Active-HIGH、100kΩ PD、CCL LUT1-OUT）** |
| **9** | **`PB5`** | **`LED_AI` Blue (`2N7002`)** | **`ADX_LED_B`** | **車載 RGB LED 青（Active-HIGH、`TCA0 WO2*` PWM、CORE-A5 互換）** |
| **10** | **`PB4`** | **`LED_AI` Red (`2N7002`)** | **`ADX_LED_R`** | **車載 RGB LED 赤（Active-HIGH、`TCA0 WO1*` PWM、CORE-A5 互換）** |
| **11** | `PB3/RX` | `CN1` Pin 8 | **`P04`** (`ADX_RX`) | USART0 Default-RX（外部拡張 UART） |
| **12** | `PB2/TX` | `CN1` Pin 7 | **`P03`** (`ADX_TX`) | USART0 Default-TX（外部拡張 UART） |
| **13** | `PB1/SDA` | `CN1` Pin 10 | **`P06`** (`ADX_SDA`) | I2C SDA (TWI0) / ADC0 |
| **14** | `PB0/SCL` | `CN1` Pin 9 | **`P05`** (`ADX_SCL`) | I2C SCL (TWI0) / ADC0 |
| **15** | `PC0/SCK` | `CN1` Pin 15 | **`P10`** (`ADX_SCK`) | SPI SCK (SPI0 Alt) / TCD0 WOC |
| **16** | `PC1/MISO` | `CN1` Pin 14 | **`P09`** (`ADX_MISO`) | SPI MISO (SPI0 Alt) / TCD0 WOD |
| **17** | `PC2/MOSI` | `CN1` Pin 13 | **`P08`** (`ADX_MOSI`) | SPI MOSI (SPI0 Alt) |
| **18** | `PC3/CS` | `CN1` Pin 12 | **`P07`** (`ADX_CS`) | SPI CS (SPI0 Alt) |
| **19** | `PA0/UPDI` | `CN1` Pin 2 & `SN74LV4066A` | **`S00`** *(システム専用)* | UPDI 強制書込（Mode 1/2 は `CH342K` 切り離し） |
| **20** | `PA1/TX` | `SN74LV4066A` → `CH342K` Port 1 RX | *(オンボード)* | USART0 Alt-TX（Mode 3 のみ接続） |
| **21** | `EP` | `GND` | `GND` | サーマルパッド |

> **オンボード・ハードウェアテレメトリ（AI が取得可能な 4 計測＋算出電力）**:
> 1. 幹線電圧 (`ADX_VBUS_VSENSE` on `PA5`: V) — 全 3 Mode 対応
> 2. 供給電流 (`ADX_VBUS_ISENSE` on `PA4`: A) → 1×2 より **リアルタイム消費電力 (W)** を算出（`Mode 2/3`）
> 3. オンボード 3.3V レール電圧（ATtiny1616 内蔵 VDD チャンネル・ピン消費ゼロ）
> 4. MCU 内部温度（ATtiny1616 内蔵温度センサー・ピン消費ゼロ・105℃ 熱監視対応）

### 6.2 標準ヘッダ定義案（`ADX_CORE_U_rev5.h`）
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

// CORE-U rev5 Onboard AI Observability & Automotive RGB LED (-40~+105C Ready)
#define ADX_VBUS_ISENSE  PIN_PA4  // ADC0 AIN4 (1.00 V/A, 0.0-3.3A, TP181A2-CR)
#define ADX_VBUS_VSENSE  PIN_PA5  // ADC0 AIN5 (0.50 V/V, 0.0-6.6V)
#define ADX_LED_R        PIN_PB4  // Active-HIGH (TCA0 WO1* PWM, CORE-A5 Compatible)
#define ADX_LED_B        PIN_PB5  // Active-HIGH (TCA0 WO2* PWM, CORE-A5 Compatible)
#define ADX_LED_G        PIN_PA7  // Active-HIGH (GPIO / Soft or CCL PWM)
```

---

## 7. 主要部品構成 (BOM) ＆ コスト試算

すべての組込稼働部品（Mode 1 で通電する全素子）を **`-40℃〜+105℃` 以上** で統一しています。

| リファレンス | カテゴリ | 型番 / メーカー | パッケージ | 動作温度定格 | LCSC / JLCPCB 区分 |
| :--- | :--- | :--- | :--- | :---: | :--- |
| `MCU` | 主 MCU | `ATTINY1616-MNR` (Microchip) | QFN-20-EP | **-40〜+105℃** | `C507118` (Extended) |
| `03-eFuse` | スマート eFuse | **`TPS259470A`** *(or `0L`)* (TI) | QFN-10 (2×2) | **-40〜+125℃** | Extended (在庫確認要) |
| `04-LDO` | 3.3V LDO | **`TLV75533PDBVR`** (TI) | SOT-23-5 | **-40〜+125℃** | Extended |
| `U2` | ハイサイド電流アンプ | **`TP181A2-CR`** (3PEAK) *(互換: `INA180A3`)* | SC-70-6 | **-40〜+125℃** | `C2902350` (Extended) |
| `R_SHUNT` | シャント抵抗 | **`10mΩ` ±1% 0.5W〜1W** | 1206 | **-55〜+155℃** | 選定予定 (Basic/Ext) |
| `CN1` | 拡張 IDC 20P (ラッチ付) | **`2.54-2*10P直牛角`** (BOOMELE 等) | **THT 2×10P** | **-55〜+105℃** | **`C9039`** (THT 手実装) |
| `LED_SYS` / `LED_AI` | 車載 3 色 LED ×2 | **`HVRTB-3433EPX`** (Honglitronic) | SMD-6P (3433) | **-40〜+110℃** | **`C5246356`** (Extended) |
| `U_SW` | 4ch アナログスイッチ | **`SN74LV4066A`** (TI) | TSSOP-14 / VQFN | **-40〜+125℃** | Extended |
| `Q1, Q3, Q_DBG, Q_LED×3` | 小信号 N-MOS ×6 | **`2N7002`** | SOT-23 | **-55〜+150℃** | Basic 部品 |
| `Q2` | パワーゲート P-MOS | **`AO3401A`** 等 | SOT-23 | **-55〜+150℃** | Basic 部品 |
| `02_TVS` | 5V TVS ダイオード | `SMF5.0CA-MS` (MSKSEMI) | SOD-123FL | **-55〜+150℃** | `C7473196` (Extended) |
| `USB_Serge` | USB ESD 保護 | `USBLC6-2SC6` (TECH PUBLIC) | SOT-23-6 | **-55〜+125℃** | `C2827654` (Extended) |
| `C_BULK` | バルク MLCC (10µF 相当) | **`X7R` 4.7µF 50V 1206×2 並列** or **10µF X7R** | 1206 / 0805 | **-55〜+125℃** | Basic (`C29823`) / Ext |
| `C_DEC` / `R_*` | 0.1µF MLCC / 抵抗類 | `CL05B104` (X7R) / `0603WAF` (±1%) | 0402 / 0603 | **-55〜+125/155℃** | Basic 部品 |
| `U1` | USB-UART (2ch) | `CH342K` (WCH) *(Mode 1/2 完全遮断)* | ESSOP-10 | 動作 -40〜+85℃<br>**保存 -55〜+105℃** | `C2826608` (Extended) |
| `01-USB_Pow` | 電源用 Type-C 6P | **6P Power-Only Type-C (3A/5A, LCP)** | SMD Right Angle | -40〜+85℃ *(105℃品調査可)* | 選定予定 |
| `01-USB_Com` | 通信用 Type-C 16P | `UC09-0B16F00P11` (BXCONN) 等 | SMD Right Angle | -45〜+85℃ | `C36936538` (Extended) |
| `PCB` | 基板材 | **FR-4 `TG155`**（2 層・`8858` 規格） | 88×58 mm | Mid-Tg 高耐熱 | JLCPCB オプション |

### 概算コスト試算（120 pcs PCBA ロット想定・許容原価上限 `$6.50` / 許容売価上限 `$26.00`）
- **ベース原価（rev4 実績 `$3.83` ＋ rev5 観測系 `+$0.42`）**: 約 `$4.25 / pc`
- **耐熱・3 Mode・Eject Header・2 系統車載 LED 増分（見込み）**:
  - `TPS259470`（125℃ eFuse 置換）: `+$0.48`
  - `TLV75533PDBVR`（125℃ LDO 置換）: `+$0.04`
  - 10µF MLCC 全 `X7R` 化: `+$0.12`
  - 車載 LED `HVRTB-3433EPX` ×2 ＋ `2N7002`×6（`WS2812B`・0805 LED×2 廃止相殺）: `+$0.06`
  - `CH342K` 非接続遮断回路（`SN74LV4066A` ＋ `AO3401A`）: `+$0.22`
  - THT Eject Header `C9039` ＋ JLCPCB THT 実装費（20 ピン）: `+$0.37`
  - `USB-POW` 6P Type-C 化: `-$0.05`
  - 高耐熱基板材 `FR4-TG155`: `+$0.15`
  - **合計増分**: **`+$1.39 / pc`**
- **rev5 想定原価**: **約 `$5.60 〜 $5.75 / pc`**（上限 `$6.50` に対し約 `$0.75〜$0.90` の安全バッファあり）
- **想定販売価格（4 倍算）**: **約 `$22.40 〜 $23.00`**（最大許容売価 **`$26.00` 以内**）

---

## 8. アートワーク・試作前のレビュー確認事項

1. **THT Eject Header (`C9039`) のラッチ全開クリアランス確認**：
   - `8858` 規格の M3 座面キープアウト内側有効幅 `66.0 mm` に対し、採用する Eject Header 実型番の **レバー全開時の最大幅・全高（~18〜22 mm）** をデータシート図面で最終確認する。
2. **シャント抵抗のケルビン配線 ＆ `TPS259470` サーマル設計**：
   - `10mΩ`（1206）の両パッド内側から専用センス配線を細く引き出して `TP181A2-CR` の `IN+` / `IN-` へ直結し、銅箔抵抗による誤差を排除する。
   - `TPS259470`（2×2mm QFN）の底面サーマルパッド直下にサーマルビア群＋広面積 GND プレーンを配置し、105℃ 環境での 2A 連続通電熱マージンを最大化する。
3. **`TPS259470` の最終型番選定 ＆ `FLT` 過電圧保護**：
   - LCSC 発注時点で `TPS259470A`（Auto-Retry）と `TPS259470L`（Latch-Off）の在庫・単価を確認する（基本は `A` 優先）。
   - `USB-POW` へ 9V〜12V が誤印加され OVLO が動作した際にも `FLT` ピン電圧が推奨上限 `5V` を超えないよう、赤 LED 経路と合わせて `FLT` 保護（4.7V ツェナー等）を回路図で最終検証する。
4. **`CH342K` パワーゲート起動シーケンス ＆ `USB-POW` 6P コネクタ選定**：
   - `USB-COM` のみ先挿し（`VBUS_PROT = 0V`）状態での回り込み電流がゼロであることを試作機で実測検証する。
   - `USB-POW` 用 6P Type-C は定格 3A〜5A・LCP 耐熱ハウジング品を選定し、LCSC 上で `-40℃〜+105℃` 公称品が許容原価（`$6.50` 上限内）で入手可能か発注前に再調査する。
5. **PWM 設定の確認（`PB4`/`PB5` vs `PA7`）**：
   - `PB4` (`TCA0 WO1*`)・`PB5` (`TCA0 WO2*`) は `PORTMUX.CTRLC` の個別ビット設定により、通常シングルモードで I2C (`PB0/PB1`)・UART (`PB2/PB3`) と無干渉にハード PWM 出力できる（Split Mode 時のシリコンエラッタ回避のため `TCA0` はシングルモード運用を標準とする）。
