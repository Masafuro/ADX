<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-CX3 (rev1) ハードウェア配線・ピンアサイン検討書
(ADX CORE-CX3 rev1 Hardware Pinout & Architecture Study)

- **ボード名称**: ADX CORE-CX3 (The Enclosure-Native USB Core)
- **リビジョン**: rev1 (検討段階)
- **最新改訂日**: 2026-11-03
- **準拠規格**: [ADX 20-Pin Bus Specification v4.0](../../docs/spec/ADX_PINOUT.md) (SSOT)
- **対象 MCU**: WCH **CH32X035G8U6** (QFN-28, 4mm × 4mm, 0.4mm pitch)
- **参照データシート**: `docs/datasheet/CH32X035G8U6.pdf` (WCH CH32X035/X033 数据手册 V2.2)

---

## 1. 検討の背景と目的

### 1.1 プロダクト概要
ADX CORE-CX3 は、WCH 製 32-bit RISC-V マイコン **CH32X035G8U6** を採用し、産業用 8748 フォームファクタとデュアル Type-C ポート（通信用 16P ＋ 電源用 6P 完全物理分離）を組み合わせた筐体組込ネイティブのエッジコントローラです。

### 1.2 本検討書の目的
データシート（`docs/datasheet/CH32X035G8U6.pdf` V2.2）の実仕様に基づき、以下を精査・確定することを目的とします：
1. 旧ドラフト仕様（`docs/spec/mcu_mapping/ch32x035.md` 等）に存在した架空ピンや別ファミリ前提の記述を是正し、CH32X035G8U6 実チップの物理ピンと機能を正確に把握する。
2. **ADX 20-Pin Bus Specification v4.0** に対する最適なピン配線を策定する。
3. 「無理・斑・無駄」のないハードウェアアーキテクチャに基づき、**20-Pin IDC コネクタ単体での自動検査・2-wire SDI 書き込み完結アーキテクチャ** を確立する。
4. 拡張 Card 側との相互運用性を保つための「システムピン（Pins 2, 5）の 0Ω アイソレーション設計ルール」を定義する。

---

## 2. データシート精査で判明した事実とアーキテクチャ決定

### 2.1 旧ドラフト仕様との相違点（是正ポイント）

事前ドラフトでは別型番（STM32系やCH32V003/V103等）を模した仮ピンアサインが記載されていましたが、データシートの精査により以下の重大な相違点が判明しました：

| 項目 | 旧ドラフトの記述 (誤) | データシート CH32X035G8U6 実仕様 (正) | 技術的背景・是正理由 |
| :--- | :--- | :--- | :--- |
| **Pin 2 (OWD)** | **PD7** (QFN Pin 28) | **PC18** (QFN Pin 25: `SWDIO`) | CH32X035 に Port D は存在しない。SDI は PC18/PC19 の 2-wire 構成。 |
| **Pin 5 (EXTCLK)** | **PA0** (QFN Pin 6) | **PC19** (QFN Pin 24: `SWCLK / TIM2_CH1`) | 治具 20P 単体書込を完結させ、かつ GND ガードを SWCLK に活用するため。 |
| **Pin 7 (UART_TX)** | **PA9** (QFN Pin 17) | **PB10** (QFN Pin 21: `USART1_TX`) | PA9 は QFN-28 では未引き出し。PB10 が USART1_TX。 |
| **Pin 8 (UART_RX)** | **PA10** (QFN Pin 18) | **PB11** (QFN Pin 22: `USART1_RX`) | PA10 は QFN-28 では未引き出し。PB11 が USART1_RX。 |
| **Pin 9 (I2C_SCL)** | **PB10** (QFN Pin 19) | **PB8** (QFN Pin 19: GPIO) | PB10 は USART1_TX。ハード I2C の制約・エラッタ回避のため GPIO を割り当て。 |
| **Pin 10 (I2C_SDA)** | **PB11** (QFN Pin 20) | **PB9** (QFN Pin 20: GPIO) | PB11 は USART1_RX。同上。 |
| **Pin 12–15 (SPI)** | **PB12–PB15** (Pin 21–24) | **PA4–PA7** (Pin 9–12: `SPI1`) | PB13–PB15 は QFN-28 に存在しない。ハード SPI1 は PA4–PA7 に位置。 |
| **USB D- / D+** | **PA11 / PA12** (Pin 19, 20) | **PC16 / PC17** (Pin 26, 27) | 内蔵 USB PHY は PC16 (`UDM`) と PC17 (`UDP`) に直結。 |
| **Type-C CC1 / CC2** | **PC14 / PC15** (Pin 2, 3) | **外付け 5.1kΩ プルダウン (Rd)** | USB PD は不使用。ハードウェア抵抗で即時 5V 受電を確立。 |
| **BOOT ピン** | **PB2** (Pin 12) | **PC17** (Pin 27: UDP) | PB2 は QFN-28 に存在しない。上電時 PC17=HIGH で内蔵 ROM ISP 起動。 |
| **リセットピン** | **NRST** (Pin 4) | **PC3** (Pin 4) | QFN-28 に専用 NRST 端子は引き出されておらず、Pin 4 は PC3。 |

---

### 2.2 ペリフェラル別の詳細仕様とブレークスルー決定

#### 1. システムピン・ペア（Pins 2, 5）による 20P IDC 単体書込の実現
- **クロック入力の実態**:
  - CH32X035 のシステムクロック源は **内蔵 48MHz RC 発振器（HSI）のみ** です（外部クリスタル入力ピンや外部クロック注入端子はチップ上に存在しません）。
  - そのため、Pin 5 (`EXTCLK`) を MCU システムクロック入力として使うシーンは CORE-CX3 では存在しません。
- **デバッグ仕様（2-wire SDI）**:
  - WCH-LinkE による書き込みには `SWDIO`（PC18）と `SWCLK`（PC19）の 2 線が必要です（NRST 接続は不要）。
- **Pin 2 ＋ Pin 5 への割り当ての必然性**:
  - **Pin 2 (`OWD`)** に `PC18 (SWDIO)`、**Pin 5 (`EXTCLK`)** に `PC19 (SWCLK)` を割り当てます。
  - Pin 5 は両隣（Pin 4, Pin 6）が GND で完全に挟まれた **コプレーナ導波路（GND サンドイッチ）** です。鋭い立ち上がりエッジを持つ高周波クロック `SWCLK`（数MHz〜十数MHz）を通すのに最も理想的なシールド環境です。
  - これにより、**ポゴピン治具を全廃し、20-Pin IDC ケーブルを 1 本挿すだけで「VBUS 給電 ＋ 2-wire SDI 書込」が 100% 完結** します。
- **名実ともに規格適合する多重機能**:
  - `PC19` は AFIO リマップ 5（`T2C1_5`）により **`TIM2_CH1`** として動作します。
  - 通常稼働時はファームウェア起動直後に SDI を閉じることで、Pin 5 は規格通りの **「EXTCLK / 高速タイマー入力 / PWM」** として機能します。

#### 2. ハードウェア I2C の制約とソフトウェア I2C の採用
- **物理ピンの欠落と競合**: CH32X035 のハードウェア I2C1 デフォルト（PA10/PA11）は QFN-28 に存在せず、リマップでは USB（PC16/17）やデバッグ（PC18/19）と重複します。
- **製造ロットによるシリコンエラッタ**: データシート注3に記載の通り、特定ロットではハードウェア I2C 自体が機能しない不具合が存在します。
- **結論**: CH32X035 コミュニティのデファクトスタンダードに従い、**48MHz 32-bit RISC-V の演算能力を活かした高速 GPIO ソフトウェア I2C（Bit-banging / Open-Drain）** を採用します。これにより、シリコンエラッタを完全に回避し、100kHz Standard / 400kHz Fast Mode を極めて安定して駆動できます。

#### 3. USB PD 不使用と確実な 5V 受電
- 今回の設計では USB PD は利用せず、Type-C 規格に準拠した **外付け 5.1kΩ プルダウン抵抗（Rd）** を CC1 / CC2 に配置します。
- これにより、MCU のファームウェア起動や PD 通信の確立を待たずに、PC や USB 充電器からケーブル挿入と同時に確実な 5V 給電が得られます。
- これに伴い、MCU の `PC14` (Pin 28) および `PC15` (Pin 1) は N.C. または予備 GPIO として温存されます。

#### 4. 内蔵 ROM ISP ブート（PC17 / UDP）
- データシート注8により、`PC17 (USB D+)` が BOOT 検出ピンとして機能します。
- 通信用 Type-C の D+ 信号ラインにタクトスイッチ（BOOT ボタン）を介した 3.3V プルアップ回路を設けることで、PC 接続時にボタンを押しながら挿すだけで内蔵 WCH ISP ブートローダー（ROM ブート）へ強制移行し、WCH-LinkE なしでも USB 経由でファームウェア復旧が可能です。

---

## 3. 拡張 Card 側の相互運用性設計ルール (Card Design Rules)

Core ボード側の MCU 多様化（ATtiny1616、RP2040、CH32X035 等）に伴い、拡張 Card 側の設計ガイドラインとして以下のルールを規定します：

> ### 【ADX 拡張 Card 推奨規定】システム制御ピン（Pins 2, 5）の 0Ω アイソレーション
> 1. **Pin 2 (`OWD`)**:  
>    原則として N.C.（未接続）。治具や特殊カードで接続する場合でも、通常運用時は遮断可能な構造とすること。
> 2. **Pin 5 (`EXTCLK`)**:  
>    Card 側からクロックを発振・供給する場合、または Pin 5 の信号を受信・監視する場合、**直列に 0Ω 抵抗（例: `R_EXTCLK`）またはソルダーブリッジジャンパを配置し、電気的に完全切断（DNP）可能な構造とすることを「推奨（RECOMMENDED / SHOULD）」とする。**
>    - **ATtiny1616（CORE-A5）マスター構成**: 0Ω 抵抗を実装し、Card 上の発振器から Pin 5 へシステムクロックを供給。
>    - **CH32X035（CORE-CX3）構成**: 0Ω 抵抗を未実装（DNP）とすることで、Card 側のクロックと Core 側のデバッグピン（SWCLK）の衝突を未然に防止。同一の Card 基板で全 Core ボードへの完全対応が可能。

---

## 4. CH32X035G8U6 (QFN-28) 全ピン配置一覧

データシートより抽出した QFN-28 の全 28 ピンおよび EPAD (GND) の完全マップです：

```text
                        【 CH32X035G8U6 (QFN-28) Top View 】
                                ┌───────────────┐
                     PC14 (N.C.)│ 28         21 │ PB10 (UART1_TX)
               PC17 (UDP/BOOT)  │ 27         20 │ PB9  (I2C_SDA / MCO)
                 PC16 (UDM/D-)  │ 26         19 │ PB8  (I2C_SCL)
                PC18 (OWD/SDI)  │ 25   [ 0]  18 │ PB7  (LED_BLUE / TIM1_CH2N)
               PC19 (EXTCLK/CK) │ 24   GND   17 │ PB6  (LED_RED / TIM1_CH1N)
                    PB12 (TIM)  │ 23   EPAD  16 │ PB1/PB5 (Short)
                PB11 (UART1_RX) │ 22         15 │ PB4  (USART3_RX)
                                └───────────────┘
                                  1 2 3 4 5 6 7
                                  │ │ │ │ │ │ │
                     PC15 (N.C.)──┘ │ │ │ │ │ └── PA2 (USART2_TX / ADC2)
                       VDD (3.3V) ──┘ │ │ │ └──── PA1 (ADX Pin 3: AIN)
                  PC0 (UF2_BOOT) ─────┘ │ └────── PA0 (予備 GPIO / ADC0)
                    PC3 (RESET?) ───────┘
```

| Pin | ピン名 | 種別 | 主機能 / デフォルト | 主な多重機能 (AF) | 本ボードでの用途 |
| :---: | :--- | :---: | :--- | :--- | :--- |
| **0** | **GND** | P | EPAD (Exposed Pad) | 内部グランド | **GND プレーン直結 (サーマルビア群)** |
| **1** | **PC15** | I/O/A | PC15 | CC2 / TIM2_CH3 / TIM1_ETR | 未使用 (N.C. / 予備 GPIO) |
| **2** | **VDD** | P | VDD (2.0V–5.5V) | 主電源入力 | **3.3V LDO 出力直結 (0.1µF+4.7µF)** |
| **3** | **PC0** | I/O/A | PC0 | ADC_IN10 / USART2_TX / TIM2_CH4 | **UF2 BOOT スイッチ (タクトSW)** |
| **4** | **PC3** | I/O/A | PC3 | ADC_IN13 / CMP / TIM1_CH4 | **RESET スイッチ (タクトSW)** |
| **5** | **PA0** | I/O/A | PA0 | TIM2_CH1 / ADC_IN0 / CMP1_IN | 未使用 (予備 GPIO / 高精度 ADC0 入力) |
| **6** | **PA1** | I/O/A | PA1 | TIM2_CH2 / ADC_IN1 / OPA / CMP | **ADX Pin 3: AIN (12-bit ADC1_IN1)** |
| **7** | **PA2** | I/O/A | PA2 | USART2_TX / TIM2_CH3 / ADC_IN2 | 未使用 (予備 GPIO / テストパッド) |
| **8** | **PA3** | I/O/A | PA3 | USART2_RX / TIM2_CH4 / ADC_IN3 | 未使用 (予備 GPIO / テストパッド) |
| **9** | **PA4** | I/O/A | PA4 | SPI1_NSS / ADC_IN4 / OPA2_OUT | **ADX Pin 12: SPI_CS** |
| **10**| **PA5** | I/O/A | PA5 | SPI1_SCK / ADC_IN5 / USART4_TX | **ADX Pin 15: SPI_SCK** |
| **11**| **PA6** | I/O/A | PA6 | SPI1_MISO / ADC_IN6 / TIM3_CH1 | **ADX Pin 14: SPI_MISO** |
| **12**| **PA7** | I/O/A | PA7 | SPI1_MOSI / ADC_IN7 / TIM1_CH1N| **ADX Pin 13: SPI_MOSI** |
| **13**| **PB0** | I/O/A | PB0 | USART4_TX / ADC_IN8 / OPA1_P0 | 未使用 (予備 GPIO) |
| **14**| **PB3** | I/O/A | PB3 | USART3_TX / CMP3_OUT / TIM2_CH3| 未使用 (予備 GPIO) |
| **15**| **PB4** | I/O/A | PB4 | USART3_RX / TIM2_CH4 / TIM3_CH1| 未使用 (予備 GPIO) |
| **16**| **PB1/PB5**| I/O/A | PB1/PB5 (合封短絡)| ADC_IN9 / USART4_RX / OPA | 未使用 (内部短絡のため N.C. 推奨) |
| **17**| **PB6** | I/O/A | PB6 | TIM1_CH1N / USART3_CTS / OPA | **Onboard User LED 1 (Red / PWM)** |
| **18**| **PB7** | I/O/A | PB7 | TIM1_CH2N / USART3_RTS / OPA | **Onboard User LED 2 (Blue / PWM)** |
| **19**| **PB8** | I/O/A | PB8 | TIM1_CH3N / OPA1_P1 | **ADX Pin 9: I2C_SCL (GPIO Open-Drain)** |
| **20**| **PB9** | I/O | PB9 | TIM1_CH1 / MCO / USART4_TX | **ADX Pin 10: I2C_SDA (GPIO Open-Drain)** |
| **21**| **PB10**| I/O | PB10 | USART1_TX / TIM1_CH2 | **ADX Pin 7: UART_TX** |
| **22**| **PB11**| I/O | PB11 | USART1_RX / TIM1_CH3 | **ADX Pin 8: UART_RX** |
| **23**| **PB12**| I/O | PB12 | TIM1_CH4 / TIM2_CH2N | 未使用 (予備 GPIO) |
| **24**| **PC19**| I/O/A | PC19 (DCK) | SWCLK / TIM2_CH1(AF5) / PIOC_IO1 | **ADX Pin 5: EXTCLK (SWCLK / TIM2_CH1)** |
| **25**| **PC18**| I/O | PC18 (DIO) | SWDIO / PIOC_IO0 / USART3_TX | **ADX Pin 2: OWD (SWDIO)** |
| **26**| **PC16**| I/O/A | PC16 (UDM) | USB Full-Speed D- (PC11 合封短絡)| **通信用 Type-C D-** |
| **27**| **PC17**| I/O/A | PC17 (UDP) | USB Full-Speed D+ (BOOT 検出) | **通信用 Type-C D+ (BOOT回路接続)** |
| **28**| **PC14**| I/O/A | PC14 | CC1 / TIM1_CH3 / TIM2_CH2 | 未使用 (N.C. / 予備 GPIO) |

---

## 5. ADX 20-Pin 拡張バス配線仕様 (rev1 確定案)

[ADX-BUS-SPEC-v4.0](../../docs/spec/ADX_PINOUT.md) に完全準拠する最終マッピングです：

| IDC Pin | 信号名 | 電流/電圧規格 | MCU 接続ピン | パッケージ Pin | 配線・設計要件 |
| :---: | :--- | :--- | :---: | :---: | :--- |
| **1** | **GND** | 0V Return | **GND (EPAD)** | Pin 0 | ケーブル最外周グラウンドガード。PCB ベタ GND 結合。 |
| **2** | **OWD** | 3.3V LVCMOS | **PC18** | Pin 25 | **SWDIO (DIO)**。弱プルアップ ($10\,\text{k}\Omega$) 実装。<br>※治具検査時に IDC Pin 5 (SWCLK) とペアで WCH-LinkE に接続。 |
| **3** | **AIN** | 0V–3.3V Analog | **PA1** | Pin 6 | **ADC1_IN1**。12-bit 高精度アナログ入力。Pin 1/4 (GND) で完全コプレーナシールド。 |
| **4** | **GND** | 0V Return | **GND (EPAD)** | Pin 0 | アナログ・クロック分離シールド。 |
| **5** | **EXTCLK** | 3.3V LVCMOS | **PC19** | Pin 24 | **SWCLK (DCK) / TIM2_CH1**。<br>・治具検査時: WCH-LinkE デバッグクロック（GNDサンドイッチ伝送）<br>・通常稼働時: SDI 無効化により `TIM2_CH1`（タイマー入力/PWM） |
| **6** | **GND** | 0V Return | **GND (EPAD)** | Pin 0 | クロック・通信分離シールド。 |
| **7** | **UART_TX** | 3.3V LVCMOS | **PB10** | Pin 21 | **USART1_TX**。ハードウェア非同期シリアル送信。 |
| **8** | **UART_RX** | 3.3V LVCMOS | **PB11** | Pin 22 | **USART1_RX**。ハードウェア非同期シリアル受信。 |
| **9** | **I2C_SCL** | 3.3V Open-Drain| **PB8** | Pin 19 | 高速 GPIO ソフトウェア I2C クロック。 |
| **10**| **I2C_SDA** | 3.3V Open-Drain| **PB9** | Pin 20 | 高速 GPIO ソフトウェア I2C データ。 |
| **11**| **GND** | 0V Return | **GND (EPAD)** | Pin 0 | I2C オープンドレインと高速 SPI プッシュプルの境界シールド壁。 |
| **12**| **SPI_CS** | 3.3V LVCMOS | **PA4** | Pin 9 | **SPI1_NSS**。ハードウェア SPI チップセレクト (Active-LOW)。 |
| **13**| **SPI_MOSI**| 3.3V LVCMOS | **PA7** | Pin 12 | **SPI1_MOSI**。ハードウェア SPI マスター出力。 |
| **14**| **SPI_MISO**| 3.3V LVCMOS | **PA6** | Pin 11 | **SPI1_MISO**。ハードウェア SPI マスター入力。 |
| **15**| **SPI_SCK** | 3.3V LVCMOS | **PA5** | Pin 10 | **SPI1_SCK**。ハードウェア SPI クロック出力。 |
| **16**| **GND** | 0V Return | **GND (EPAD)** | Pin 0 | VBUS 短絡保護壁。リボンケーブル圧着ずれ時の短絡安全バリア。 |
| **17**| **VBUS** | 4.5V–5.0V (2–3A)| **VBUS** | - | $2 \times 2$ ベタ銅箔プレーン。底面大電流給電クラスタ（盤トポロジー適合・直進配線）。 |
| **18**| **VBUS** | 4.5V–5.0V (2–3A)| **VBUS** | - | 同上。 |
| **19**| **VBUS** | 4.5V–5.0V (2–3A)| **VBUS** | - | 同上。 |
| **20**| **VBUS** | 4.5V–5.0V (2–3A)| **VBUS** | - | 同上。 |

---

## 6. オンボード機能と周辺回路仕様

### 6.1 デュアル Type-C ポート電源トポロジー
現場での PC 逆流破壊事故を物理的に防止する CORE-CX3 独自の電源構成です：

1. **通信専用 16P Type-C ポート**:
   - データ線: `PC16 (UDM)`、`PC17 (UDP)` に ESD 保護ダイオード（USBLC6-2SC6 等）を介して直結。
   - CC 線: コネクタ側で `CC1` と `CC2` をそれぞれ外付け $5.1\,\text{k}\Omega$ 抵抗（Rd）で GND へプルダウン。ホストから即座に 5V 給電を確立。
   - 電源線: 500mA PTC リセッタブルヒューズ ＋ 低 $V_F$ ショットキーバリアダイオード (SBD) を介して MCU 電源系統（3.3V LDO 入力）へ接続。外部 `VBUS` から PC ポートへの逆流をダイオードで完全遮断。
2. **電源専用 6P Type-C ポート**:
   - 大電流受電専用。2.0A〜3.0A PTC ヒューズを経由して ADX 20-Pin の `VBUS`（Pins 17–20）に直結。

### 6.2 オンボード LED 構成
CORE-A5 の UI 設計思想（電源常時点灯 ＋ 2色ステータス制御）を踏襲します：
- **PWR LED (Green)**: 3.3V LDO 出力から直接点灯（通電確認用）。
- **User LED 1 (Red / Amber)**: `PB6` (Pin 17) に接続。Active HIGH。タイマー `TIM1_CH1N` のハードウェア PWM による調光・エラー点滅制御が可能。
- **User LED 2 (Blue)**: `PB7` (Pin 18) に接続。Active HIGH。タイマー `TIM1_CH2N` のハードウェア PWM による通信アクティビティ・呼吸点滅（ブレス）制御が可能。

### 6.3 スイッチ・ブート・保守構成
- **UF2 BOOT ボタン**: `PC0` (Pin 3) にタクトスイッチを接続（押下時 LOW、外付けプルアップ $10\,\text{k}\Omega$）。ユーザーがスケッチのドラッグ＆ドロップ書込を行う際に使用。
- **ROM BOOT 回路**: `PC17 (UDP)` にタクトスイッチと直列抵抗（$1\,\text{k}\Omega$）を介して 3.3V プルアップ回路を設ける（PC 接続時にボタンを押しながら挿入することで、内蔵 ROM ISP モードへ強制移行）。
- **RESET ボタン**: `PC3` (Pin 4) にタクトスイッチを配置。
- **予備テストパッド**: 治具検査は 20-Pin IDC 単体で完結しますが、手動デバッグ用に基板裏面に `TP_3V3`, `TP_GND`, `TP_SWDIO`, `TP_SWCLK` を配置可能。

---

## 7. PCB レイアウト・配線トポロジーの優位性

QFN-28 パッケージのピン配置と本配線案は、PCB アートワーク設計において極めて理想的なグループ化を実現しています：

1. **Side 2 (Pins 9–12: PA4–PA7) ➔ SPI バス**:
   - SPI1 の 4 本（CS, SCK, MISO, MOSI）が同一辺に連続して並んでおり、20-Pin IDC コネクタの Pins 12–15 へビア交差なしでストレートに引き出し可能。
2. **Side 3 (Pins 17–22: PB6–PB11) ➔ UART / I2C / LED**:
   - PB10/PB11（UART_TX/RX）および PB8/PB9（I2C_SCL/SDA）が連続しており、バスの中央〜上位ピンへ極小の配線長でルーティング可能。
   - 同一辺の PB6/PB7 はそのまま近傍のオンボード LED へ配線可能。
3. **Side 4 (Pins 24–27: PC16–PC19) ➔ USB / デバッグ**:
   - 通信用 Type-C（PC16/17）と、IDC コネクタのシステムピン Pin 2 (PC18) / Pin 5 (PC19) へ直線的に配線可能。
4. **Side 1 (Pins 3–6: PC0, PC3, PA0, PA1) ➔ AIN / スイッチ**:
   - AIN (PA1) はそのまま IDC Pin 3 へ最短配線。PC0/PC3 は手前のタクトスイッチへ配線。

---

## 8. 今後の作業ステップ (Next Steps)

1. **ドキュメント反映**:
   - 本検討書の内容を反映し、SSOT 仕様書 [`docs/spec/mcu_mapping/ch32x035.md`](../../docs/spec/mcu_mapping/ch32x035.md) を正式改訂する。
   - [`docs/spec/ADX_PINOUT.md`](../../docs/spec/ADX_PINOUT.md) の拡張 Card 設計推奨事項に「システムピン（Pins 2, 5）の 0Ω アイソレーション推奨」を追記する。
   - [`hardware/CORE-CX3/concept.md`](../concept.md) のハードウェア記述を整合させる。
2. **回路図設計 (rev1 回路図・ネットリスト着手)**:
   - KiCad または EDA ツールにて、本ピンアサインに基づきシンボル・回路図を作成。
   - デュアル Type-C ポート、LDO、PTC ヒューズ、SBD、および 20-Pin IDC Eject Header の回路設計を確定する。
