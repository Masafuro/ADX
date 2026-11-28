<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus Specification v5.0（`ADX-BUS-SPEC-v5.0`）ドキュメント改訂計画書

- **作成日**: 2026-11-27
- **最終更新**: 2026-11-28（v1.1：`CORE-U rev6 spec.md v0.5` 確定内容反映・ドキュメント改訂スコープ明確化）
- **ステータス**: **改訂完了（Phase 1〜5 実施済・2026-11-28）**
- **改訂スコープ**: **ドキュメント・公開サイトテキストのみ（CAD / ガーバー / ネットリスト等の製造データ修正は対象外）**
- **上位仕様書**: [ADX CORE-U (rev6) 技術仕様書](../../hardware/CORE-U/rev6/spec.md)（**v0.5 全仕様確定済み**）
- **現行規格（SSOT）**: [docs/spec/ADX_PINOUT.md](../../docs/spec/ADX_PINOUT.md)（`ADX-BUS-SPEC-v4.2`）

---

## 1. 背景と改訂の目的（Background & Objective）

ADX は **CORE-U (rev6)** を唯一の中核フラッグシップボードとして位置づけ、**Mode 1（Full embedded mode: 理論仕様 `-40℃ 〜 +85℃`・高耐振動）** において産業・屋外・車載用途まで単一コアでカバーするアーキテクチャへ転換した（旧 CORE-A 等は Arduino ROMEO のような付加価値型派生ボードの位置づけへ再定義）。

これに伴い、従来の「オンボード RS-485 搭載コア（旧 CORE-A）＋ 工場書き込み用 IDC ピン（`S00`）」前提で設計されていた **`ADX-BUS-SPEC-v4.2`** に、以下の 2 つの構造的反転が生じた：

1. **絶縁 RS-485 / LIN 通信の拡張 CARD 側への移管**:
   - 従来の ATtiny1616 マッピングはオンボード側（`PA1`/`PA2`/`PA4`）を RS-485 + LIN 向けに最適化していたが、rev6 では拡張 CARD 側が RS-485 + LIN を担うため、**IDC バス側へ `PA1` (`TX`) / `PA2` (`RX`, `LUT0-IN2`, 完全非同期割込) に加え、ハードウェア方向制御 `PA4` (`XDIR`) を引き出せる構造** へ改訂する。
   - 逆に、オンボード USB (`CH342K` Port 1) を Default 側（`PB2`/`PB3`）へ移すことで、① リセット初期値（`Serial.swap(0)`）がそのまま USB になる、② バス側へ切り替えた際も USB 側を SoftwareSerial として同時再利用できる、③ バス側 `ADC0` が +2 本増えるという大きな利点が得られる。
2. **`S00` (`OWD / UPDI`) の IDC バスからのオミット**:
   - ATtiny1616 の `PA0 (UPDI)` は HV（12V）プログラミング兼用ピンであり、産業・屋外・車載環境で長いフラットリボンケーブルへ露出させること自体が HV ノイズ・誤リセット・意図しない UPDI 進入の要因となる。
   - 基板側に `0Ω` 抵抗を挟むような対症療法は根源的な複雑性を畳み込むだけであるため、**「プログラミング／デバッグ経路（UPDI / SWD / ISP / UF2）は各 Core ボードの内部責務とする（具体的実装や設置義務は規格で縛らない）」** というアーキテクチャ前提へ切り替え、IDC バスから `S00` を完全にオミットする。
   - 副次効果として、全拡張 CARD に課していた「`S00` 接続禁止」および「`P02` の `SWCLK` 衝突回避用 `0Ω` DNP 推奨」の 2 大例外ルールも規格から消滅する。

本計画書は、**`ADX-BUS-SPEC-v4.2` → `ADX-BUS-SPEC-v5.0`** へのメジャー改訂をリポジトリ内の **全アクティブドキュメントおよび公開サイト（`site/`）** へ漏れなく・矛盾なく反映するための実行計画を定義するものである。  
（※`Reference CARD` や `Prototype CARD` の EasyEDA Pro CAD・ガーバー修正は本計画のスコープ外とし、別タスクとして分離する。）

---

## 2. `v4.2` → `v5.0` 仕様変更の全体像

### 2.1 主要変更点サマリー

| 設計項目 | 旧仕様 (`ADX-BUS-SPEC-v4.2`) | 新仕様 (`ADX-BUS-SPEC-v5.0`) | 改訂の工学的メリット |
| :--- | :--- | :--- | :--- |
| **`S00` (`OWD / PROG`)** | Pin 2 に割当（Card N.C. 必須） | **IDC バスから完全オミット**<br>*(各 Core ボード上の内部責務へ移行)* | HV ノイズ・誤リセット経路の根絶。Card 側 `S00` 禁止ルール撤廃 |
| **`P01` (`AIN`) シールド** | Pin 3（Pin 1 `GND` と Pin 4 `GND` の間、Wire #2 は `S00`） | **Pin 2（Pin 1 `GND` と Pin 3 `GND` で完全サンドイッチ）** | フラットケーブル上でも真のコプレーナ GND シールドとなりアナログ耐ノイズ性向上 |
| **`P02` (`EXTCLK`)** | Pin 5（`SWCLK` 兼用、Card 側 `0Ω` DNP 推奨） | **Pin 4（Pin 3 & 5 `GND` サンドイッチ、`SWCLK` 制約撤廃）** | 2-wire SWD 競合が消え、Card 側 `0Ω` DNP ルールが不要に |
| **RS-485 方向制御 (`UART_DE`)** | なし（他ピン流用 or CARD 側自動方向制御依存） | **Pin 6 に `P03 (UART_DE / XDIR)` 新設**<br>*(Card 側 10k〜100kΩ プルダウン【MUST】)* | RS-485 + I2C + SPI + AIN + EXTCLK の **全同時稼働**。LIN Break・低速通信でも破綻しない確実なハード制御 |
| **論理ピン ID 体系** | `S00`（System）＋ `P01`〜`P10`（10 本） | **`P01`〜`P11`（全 11 本・純粋ユーザー信号）** | 例外ピンゼロの統一体系。機能エイリアスに **`ADX_DE`** を追加 |
| **`Pins 7〜20` 物理配置** | UART / I2C / GND / SPI / GND / VBUS×4 | **物理ピン位置 100% 維持** | バス下段 14 本（通信群＋防火壁＋2×2 VBUS）の構造と配線原則を完全継承 |
| **ATtiny1616 オンボード＆バス構成** | IDC=`PB2/PB3` (Def)、USB=`PA1/PA2` (Alt)、LED=3ピン | **IDC=`PA1/PA2`+`PA4` (Alt)、USB=`PB2/PB3` (Def)**<br>**観測=`PA5`/`PA7`、`PB4`=`ADX_LED`、`PB5`=`ADX_USB_COM_DET`** | `Serial.begin()` 初期値が USB に統一。IDC 側に `LUT0-IN2`・`XDIR`・`ADC0` +2 本を獲得。USB SoftwareSerial 同時利用＆3 Mode 完全自己認識・深睡眠起床を実現 |

### 2.2 `v4.2` ↔ `v5.0` 物理・論理ピン完全対照表（Pin 1〜20）

| IDC Pin | 芯線 / 列 | `v4.2`（旧） | **`v5.0`（新）** | ATtiny1616 (`rev6`) | 変更区分・誤接続リスク |
| :---: | :---: | :--- | :--- | :---: | :--- |
| **1** | #1 / 奇 | `GND` | **`GND`** | `GND` | 変更なし |
| **2** | #2 / 偶 | `S00` (`OWD`) | **`P01` (`AIN`)** | `PA6` | **変更（旧 `S00` → `AIN`）** |
| **3** | #3 / 奇 | `P01` (`AIN`) | **`GND`** | `GND` | **★非互換（旧信号 → `GND`）** |
| **4** | #4 / 偶 | `GND` | **`P02` (`EXTCLK`)** | `PA3` | **★非互換（旧 `GND` → 信号）** |
| **5** | #5 / 奇 | `P02` (`EXTCLK`) | **`GND`** | `GND` | **★非互換（旧信号 → `GND`）** |
| **6** | #6 / 偶 | `GND` | **`P03` (`UART_DE`)** | **`PA4`** | **★非互換（旧 `GND` → 信号）** |
| **7** | #7 / 奇 | `P03` (`UART_TX`) | **`P04` (`UART_TX`)** | **`PA1`** | 物理位置同一（論理 ID +1、MCU 変更） |
| **8** | #8 / 偶 | `P04` (`UART_RX`) | **`P05` (`UART_RX`)** | **`PA2`** | 物理位置同一（論理 ID +1、MCU 変更） |
| **9** | #9 / 奇 | `P05` (`I2C_SCL`) | **`P06` (`I2C_SCL`)** | `PB0` | 物理位置同一（論理 ID +1） |
| **10** | #10 / 偶 | `P06` (`I2C_SDA`) | **`P07` (`I2C_SDA`)** | `PB1` | 物理位置同一（論理 ID +1） |
| **11** | #11 / 奇 | `GND` | **`GND`** | `GND` | 変更なし |
| **12** | #12 / 偶 | `P07` (`SPI_CS`) | **`P08` (`SPI_CS`)** | `PC3` | 物理位置同一（論理 ID +1） |
| **13** | #13 / 奇 | `P08` (`SPI_MOSI`) | **`P09` (`SPI_MOSI`)** | `PC2` | 物理位置同一（論理 ID +1） |
| **14** | #14 / 偶 | `P09` (`SPI_MISO`) | **`P10` (`SPI_MISO`)** | `PC1` | 物理位置同一（論理 ID +1） |
| **15** | #15 / 奇 | `P10` (`SPI_SCK`) | **`P11` (`SPI_SCK`)** | `PC0` | 物理位置同一（論理 ID +1） |
| **16** | #16 / 偶 | `GND` | **`GND`** | `GND` | 変更なし（VBUS 防火壁維持） |
| **17–20** | #17–20 | `VBUS` (2×2) | **`VBUS` (2×2)** | `VBUS_PROT` | 変更なし |

> [!WARNING]
> **電気的非互換に関する安全注記（`v4.2` ↔ `v5.0` 混在禁止）**:  
> Pins 3〜6 で信号線と `GND` が入れ替わるため、**`v4.2` 試作基板（Reference CARD v0 / Prototype CARD v0 / CORE-U rev0〜rev4）と `v5.0` 基板（CORE-U rev6 以降）は電気的に非互換** となる。`v5.0` 仕様書および今後の全 PCB シルクには規格識別子（例：`ADX-BUS v5.0`）を明記し、α 版 `v4.2` 試作基板との誤接続を防止する。

---

## 3. 全リポジトリ影響範囲マトリクス（Document Impact Audit）

リポジトリ全体の実ファイル精査に基づき、ドキュメント改訂対象を 5 つの Tier に分類する。

### Tier 1：規格 SSOT ＆ MCU マッピング（最優先）

| ファイルパス | 役割 | 主な改訂内容 |
| :--- | :--- | :--- |
| `docs/spec/ADX_PINOUT.md` | **バス規格 SSOT（英文）** | ・識別子を `ADX-BUS-SPEC-v5.0` へ更新<br>・ピン表・Top View 図を `P01`〜`P11`（Pin 6=`UART_DE`、`S00` 廃止、GND=`{1,3,5,11,16}`）へ刷新<br>・§4.3 を「Core-Side Programming Convention」＋「RS-485 Card `UART_DE` Pull-Down Rule (MUST)」へ再編、`S00 N.C.` / `P02 0Ω DNP` 規則を撤廃<br>・§4.3.3（System Reset）へ Core オンボード書込経路（CORE-U `UPDI` 等）と `P05 (UART_RX)` 経由ウェイクアップ／リセットの整合を追記<br>・§5 論理マクロ定義を `ADX_P01`〜`ADX_P11` ＋ `ADX_DE` へ更新<br>・§6 の ATtiny1616 欄を **`CORE-U` フラッグシップ中心（旧 `CORE-A5` は派生/歴史参照）** へ整理<br>・§8 改訂履歴に `v5.0`（`v4.2` との電気的非互換・移行注記）を追記 |
| `docs/spec/mcu_mapping/attiny1616.md` | **ATtiny1616 マッピング SSOT** | ・`P01`〜`P11` 物理ピン対照表を CORE-U rev6 v0.5 確定仕様（`PA4=P03/DE`, `PA1=P04/TX`, `PA2=P05/RX`）へ全面更新<br>・§3 オンボード専用ピン表を rev6 v0.5 確定仕様へ更新：`PA0`=オンボード UPDI 専用、`PB2`/`PB3`=USB `CH342K`＋SoftwareSerial、`PA5`=`ADX_VBUS_VSENSE`、`PA7`=`ADX_VBUS_ISENSE`、`PB4`=`ADX_LED`（`TCA0 WO1*` ハード PWM 単色灯）、`PB5`=`ADX_USB_COM_DET`（Mode 3 検知・深睡眠復帰）<br>・§4 `PORTMUX` ルール更新（`USART0=0` 初期値が USB、`1` が IDC RS-485/LIN）<br>・§5 標準ヘッダ例を `ADX_CORE_U_rev6.h` 準拠（`ADX_P01`〜`ADX_P11`, `ADX_DE` 等）へ更新<br>・旧 CORE-A5 記述を付加価値型派生ボード参考例へ位置づけ調整 |
| `docs/spec/mcu_mapping/ch32x035.md` | **CH32X035 マッピング SSOT (Reference)** | ・`docs/spec/` 内の規格バージョン混在を避けるため **`v5.0` 準拠（Reference ステータス維持）** へ追従更新<br>・`PC18`/`PC19` (2-wire SDI) をオンボード専用テストパッド／デバッグ端子へ移管し、`P02 (EXTCLK)` の `SWCLK` 制約・`0Ω DNP` 記述を削除<br>・予備ピン（`PB12` 等）を `P03 (UART_DE)` へ割り当て、ピン番号・論理 ID (`P01`〜`P11`) を `v5.0` へ整合 |

### Tier 2：日英ガイド・フォームファクタ仕様・開発ガイドライン

| ファイルパス | 役割 | 主な改訂内容 |
| :--- | :--- | :--- |
| `docs/ja/ADX_pinout.md`<br>`docs/en/ADX_pinout.md` | バス設計・解説ガイド（日 / 英） | ・Top View 図・論理ピン ID 解説（`S00` 廃止 → `P01`〜`P11` 統一、`ADX_DE` 追加）更新<br>・`P01 (AIN)` 真のコプレーナ GND 保護・`Pin 1, 3, 5` 連続 GND 構造・`P03 (UART_DE)` 新設の工学的意義を追記<br>・Card 設計ガイドラインから `S00 N.C.` / `P02 0Ω DNP` を削除し、`P03 (UART_DE)` プルダウン必須ルール・Core 側プログラミング責務・`v4.2` 非互換注意書きを追記 |
| `docs/ja/adx_attiny1616_mcu_matrix.md`<br>`docs/en/adx_attiny1616_mcu_matrix.md` | ATtiny1616 統合マトリクス（日 / 英） | ・`PORTMUX` ガイドライン（`USART0` の 0/1 役割逆転）更新<br>・全 18 ピンマトリクス表・逆引き表（PWM / ADC / EVSYS）の `Logical ID` (`P01`〜`P11`) および IDC Pin 番号を更新（`P03 / PA4` の `TCD0 WOA`・`ADC0 AIN4`・`LUT0-OUT`、`P04/P05` の `ADC0 AIN1/2` 追加を反映）<br>・§5 オンボード専用ピン表を rev6 v0.5（`PA5`/`PA7` 観測、`PB4=ADX_LED`、`PB5=ADX_USB_COM_DET`、`PB2/PB3=USB`）へ更新 |
| `docs/ja/8858_formfactor.md`<br>`docs/en/8858_formfactor.md` | 8858 フォームファクタ規格（日 / 英） | ・§4.1「ピン配置 (Top View)」を `ADX-BUS-SPEC-v5.0` へ更新<br>・**行ラベルの反転に注意**：<br>　- 上列（偶数ピン）見出し：`PROG 側` → **`AIN 側`**（`P01/AIN | P02/EXTCLK | P03/DE | P05/RX | P07/SDA | P08/CS | P10/MISO | GND | VBUS | VBUS`）<br>　- 下列（奇数ピン）見出し：`Pin 1・AIN 側` → **`Pin 1・GND 側`**（`GND | GND | GND | P04/TX | P06/SCL | GND | P09/MOSI | P11/SCK | VBUS | VBUS`） |
| `docs/ja/adx_formfactor_guidelines.md`<br>`docs/en/adx_formfactor_guidelines.md` | フォームファクタ開発ガイドライン（日 / 英） | ・`v4.2` 参照を `v5.0` へ更新、`S00` N.C. / `P02` 0Ω DNP 記述を削除し `P03 (UART_DE)` プルダウン必須ルールと `v4.2` 非互換注記を追加<br>・**本文・チェックリスト内の埋め込みピン番号を漏れなく修正**：<br>　- §6：`P01（IDC Pin 3 / AIN）` → **`Pin 2`**、`P07 [IDC Pin 12 / SPI_CS]` → **`P08 [IDC Pin 12 / SPI_CS]`**<br>　- §7 チェックリスト：`各シールド GND（Pins 1, 4, 6, 11）` → **`Pins 1, 3, 5, 11`**（＋ Pin 16 防火壁）、`S00` / `P02 0Ω` チェック項目を **`P03 (UART_DE)` プルダウン確認項目** へ差し替え |
| `docs/ja/8854_formfactor.md`<br>`docs/en/8854_formfactor.md` | 旧 8854 規格（日 / 英） | ・旧世代フォームファクタ仕様のため、`v4.2` 時代の参照である旨の注記確認のみ（原則歴史保持） |

### Tier 3：ポータル・トップレベル文書・Web サイト (`site/`)

| ファイルパス | 役割 | 主な改訂内容 |
| :--- | :--- | :--- |
| `README.md`<br>`docs/ja/README.md`<br>`docs/en/README.md` | ルート ＆ 日英ポータル | ・§2.3「Universal 20-Pin Bus」を `ADX-BUS-SPEC-v5.0`（`P01`〜`P11`、`UART_DE` 追加、`S00` 廃止・Core 側自己完結）へ更新 |
| `hardware/README.md` | ハードウェア総合案内 | ・`ADX-BUS-SPEC-v5.0` 参照更新、CORE-U rev6 フラッグシップ方針との整合 |
| `Project_Snapshot.md` | プロジェクトスナップショット | ・旧ピンアウト記述（Pin 5 アイソレーション帯等）を `v5.0` へ最小同期（※全体スナップショット更新とは分離） |
| `site/src/content/docs/index.mdx`<br>`site/src/content/docs/adx/index.mdx`<br>`site/src/content/docs/adx/standards.mdx`<br>`site/src/content/docs/ja/index.mdx`<br>`site/src/content/docs/ja/adx/index.mdx` | Starlight 公開サイト（日 / 英） | ・`ADX-BUS-SPEC-v4.2` / `S00` / `P01`–`P10` の表記を `ADX-BUS-SPEC-v5.0` / `P01`–`P11` / `UART_DE` へ統一<br>・`standards.mdx` のクイックリファレンス（シールド構成：`Pin 1 & 3 GND` / `Pin 3 & 5 GND` / `Pin 6 UART_DE` / `Pin 11 GND` / `Pin 16 GND`、`ADX_P01`–`ADX_P11`・`ADX_DE`）を更新し、ビルド検証（`npm run build`）を実施 |

### Tier 4：ハードウェア仕様ドキュメント（※CAD / ガーバーは修正対象外）

| ファイルパス | 扱い・ドキュメント改訂方針 |
| :--- | :--- |
| `hardware/CORE-U/rev6/spec.md` | **v0.5 にて全仕様確定済み**。`ADX-BUS-SPEC-v5.0` 正式改訂完了後に「予定」表記を「準拠」へ昇格する |
| `hardware/CARD/Reference/concept.md` | **ガーバー・CAD は触らない**。現行 v0（`Gerber_PCB1_2026-11-23.zip`）が **`v4.2` 準拠の α 版実績** であることを冒頭注記で明示保全し、`v5.0` 対応（シルク表・GND サーマルピン `{1,3,5,11,16}` 変更）は次期 **`Reference CARD v1`（別ハードウェアタスク）** で実施する旨のみ追記する |
| `hardware/CARD/Prototype/proposal.md` | **ガーバー・ネットリストは触らない**。現行 v0（8854 / `v4.2` 準拠）の検証実績をそのまま履歴保全し、`8858 + v5.0` 準拠の次期 v1 予定注記のみ追記する |

### Tier 5：改訂対象外（歴史的記録・アーカイブとしてそのまま保持）

以下は過去の設計経緯・公開記事・旧世代試作・製造バイナリの記録であり、**一切改訂しない**：
- すべての CAD / ガーバー / ネットリスト / 3D・外観画像ファイル（`*.zip`, `*.net`, `*.enet`, `*.png`, `*.jpg`）
- `docs/blog/posts/*`, `docs/blog/memo/*`
- `docs/memo/adx_pinout_v2.md`
- `hardware/archive/*`, `hardware/OLD/*`
- `hardware/CORE-U/rev0`〜`rev5` および旧 `hardware/CORE-U/spec_proposal.md` / `CORE-U_AI_friendly_microcontroller_board.md`
- `memo/PLAN/archive/*`, `memo/REPORT/archive/*`

---

## 4. 改訂実行ロードマップ（Phased Execution Plan）

実作業の承認後、SSOT（英語マスター）から下流ドキュメントへ一方向に同期することで表記揺れを防ぐ。

```text
  Phase 1 : SSOT 規格書の正式改訂（英語マスター）
    ├─ docs/spec/ADX_PINOUT.md                  (v4.2 → v5.0)
    ├─ docs/spec/mcu_mapping/attiny1616.md      (CORE-U rev6 v0.5 / v5.0 準拠)
    └─ docs/spec/mcu_mapping/ch32x035.md        (v5.0 追従 / Reference 維持)
         │
         ▼
  Phase 2 : 日英ドキュメント ＆ フォームファクタ規格の同期
    ├─ docs/ja/ADX_pinout.md, docs/en/ADX_pinout.md
    ├─ docs/ja/adx_attiny1616_mcu_matrix.md, docs/en/adx_attiny1616_mcu_matrix.md
    ├─ docs/ja/8858_formfactor.md, docs/en/8858_formfactor.md   (※行見出し AIN側/GND側 修正含む)
    └─ docs/ja/adx_formfactor_guidelines.md, docs/en/adx_formfactor_guidelines.md
       (※§6 Pin2/P08、§7 GND {1,3,5,11} チェックリスト修正含む)
         │
         ▼
  Phase 3 : ポータル・README・公開サイト (site/) の同期
    ├─ README.md, docs/ja/README.md, docs/en/README.md, hardware/README.md
    ├─ Project_Snapshot.md                      (ピンアウト該当行のみ最小同期)
    ├─ site/src/content/docs/{,ja/}index.mdx
    ├─ site/src/content/docs/{,ja/}adx/index.mdx ＆ standards.mdx
    └─ Starlight サイトビルド検証 (cd site && npm run build)
         │
         ▼
  Phase 4 : ハードウェア関連ドキュメントの整合注記（※CAD/ガーバー不変）
    ├─ hardware/CORE-U/rev6/spec.md             (「v5.0 予定」→「v5.0 準拠」昇格)
    ├─ hardware/CARD/Reference/concept.md       (v0 = v4.2 履歴明記 ＆ v1 別タスク注記)
    └─ hardware/CARD/Prototype/proposal.md      (同上)
         │
         ▼
  Phase 5 : 全リポジトリ横断監査（Verification）
    ├─ アクティブ文書全域の grep 検査：
    │    v4.2 / S00 / OWD / P01–P10 / P01〜P10 / Pins 1, 4, 6, 11 /
    │    Pin 3.*AIN / Pin 5.*EXTCLK / PROG 側 / SWCLK の取り残しゼロ確認
    ├─ 物理ピン番号（1〜20）・論理ID（P01〜P11）・ATtiny1616 ポート名（PA/PB/PC）の 3 層整合チェック
    └─ 本計画書のステータスを完了へ更新（必要に応じて memo/PLAN/archive/ へ移動）
```

---

## 5. 完了条件（Definition of Done）

- `docs/spec/ADX_PINOUT.md` が `ADX-BUS-SPEC-v5.0` として矛盾なく確定していること。
- `docs/spec/`・日英ガイド・ATtiny1616 マトリクス・8858 フォームファクタ・ガイドライン・ポータル・Starlight サイト間で、ピン番号（1〜20）・GND 配置（`{1,3,5,11,16}`）・論理 ID（`P01`〜`P11`）・機能名（`ADX_DE` 含む）・ルール（`S00` 廃止、`P03` プルダウン必須、Core 側プログラミング責務）が 100% 一致していること。
- 過去アーカイブおよび既存 `v0` ガーバー／CAD 実績を一切破壊せず、`v4.2` と `v5.0` の電気的非互換に関する注意喚起が明示されていること。
- `site/` の Starlight ビルド（`npm run build`）がエラーなく完了すること。
