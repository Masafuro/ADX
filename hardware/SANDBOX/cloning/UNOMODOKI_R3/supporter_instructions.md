<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# UNOMODOKI R3 サポーター指示書（読む専用）

- **役割**: プロジェクトリーダー（PL＝AI）からサポーター（人間）への作業依頼・確認観点・実験ルールをまとめた**読む専用**ドキュメントです。
- **書き込む場所**: 結果・所要時間・感想はすべて **[supporter_log.md](supporter_log.md)** に記入してください。
- **親計画書**: [development_plan.md](development_plan.md)

---

## 1. 体制と進め方（共通ルール）

- **PL（AI）**: 仕様・座標 SSOT 作成、EasyEDA MCP 実行、自動検証（DRC / ネット対照 / 座標アサート）、出力物整備、次工程へのバトン渡し。
- **サポーター（人間）**: 意思決定、Windows ブリッジ起動・停止、目視・物理レビュー、U4 での GUI 仕上げ介入、[supporter_log.md](supporter_log.md) への記録。
- **進め方**:
  1. PL が事前作業を終え、`outputs/` に確認物を揃えてから合図を出します。
  2. サポーターはこの指示書の該当マイルストーンを見て確認・作業します。
  3. 結果を **[supporter_log.md](supporter_log.md)** に書き、チャットで一言（例：「U0 書いたよ」「U2 要修正あり」）伝えてください。
- **安全・タイムボックス**:
  - 本番クラウドプロジェクトへ触れそうな異常や GUI フリーズ時は即停止し、`start_easyeda_bridge.bat` を `Ctrl+C` で閉じて連絡してください（[agent_rules.md](../../../../agent/easyeda/agent_rules.md)）。
  - U4 で AI 配線が 2〜3 回試して収束しない場合、PL は粘らずサポーターへ GUI 引き継ぎを依頼します。

---

## 2. 本実験で確かめたい 4 つの仮説

| ID | 仮説 |
| :---: | :--- |
| **H1** | **回路図**: 30 部品級でもブロック分割＋ネットラベル中心なら 90% 以上 AI 自律で実用になる。 |
| **H2** | **メカ・配置・シルク**: 外形・穴・160mil ズレ・大量ピン名シルクは AI（SSOT 駆動）が人間手作業より圧倒的に速く正確。 |
| **H3** | **2 層 PCB 配線**: 重要ネット・ベタ・DRC は AI、一般信号配線は人間が GUI で仕上げるハイブリッドが最適解になる。 |
| **H4** | **レビュー負荷**: PL が事前検品エビデンスを揃えれば、サポーターの負荷は 1 工程 5〜15 分に収まる。 |

---

## 3. マイルストーン別 指示一覧（U0 〜 U6）

### U0：仕様凍結 ＆ 座標 SSOT 確定（PL 仕様決定済み・SSOT 確認待ち）

- **PL 完了済み作業**:
  - サポーターからの委任を受け、仕様 3 点（USB-C 5V 専任 / `ATmega328P-AU` / EasyEDA デジタル検証ゴール）を正式決定。
  - お手本図面 [Arduino_uno_r3_size_mill.png](data/Arduino_uno_r3_size_mill.png) から全頂点・4 穴・全 38 ピン座標を読み取り、[spec_geometry.json](spec_geometry.json) を作成。
- **サポーターへのお願い**:
  1. [spec_geometry.json](spec_geometry.json)（または PL の要約）に大きな違和感がないかざっくり確認し、[supporter_log.md](supporter_log.md) の `U0` 判定を `[x] PASS` にする（そのまま `U1` のブリッジ起動へ同時進行しても OK）。
- **OK 基準**: 座標 SSOT に対して異論がないこと。

---

### U1：EasyEDA ブリッジ起動 ＆ LCSC 実部品確定

- **PL が先にやること**: LCSC 検証スクリプト（`scripts/step0_verify_parts.js`）を用意し、起動の合図を出す。
- **サポーターへのお願い**:
  1. Windows で EasyEDA Pro GUI（半オフラインモード）を起動し、本番プロジェクトが閉じた状態を確認してから [start_easyeda_bridge.bat](../../../../agent/easyeda/win/start_easyeda_bridge.bat) を起動する。
  2. PL に「起動した」と伝える（PL が `doctor` → `Sandbox_UNOMODOKI_R3` 作成 → LCSC 実測を実行）。
  3. PL が出力した `bom_verified.json` を確認し、Extended 部品の種類数・USB-C 形状・スイッチの大きさ・LED 色分け・THT ヘッダ（手はんだ前提）に異論がないかチェックして記録する。
- **OK 基準**: 廃番 0 件・全ピン番号実測済みの BOM を承認できること。

---

### U2：回路図自動構築 ＆ 目視レビュー（H1 検証）

- **PL が先にやること**: ブロック別に回路図を自動生成、`SCH_Drc` エラー 0・ネット自動対照を完了し、`outputs/unomodoki_sch.png` を提出する。
- **サポーターへのお願い**:
  1. `outputs/unomodoki_sch.png`（または Windows GUI 実画面）を見て、以下をチェックする：
     - 電気的チェック：`CC1`/`CC2` 独立 `5.1kΩ`、`CH340C.V3` に 5V 直結していないか、`TX`/`RX` クロス＋`1kΩ` と LED の接続位置、`DTR# → 100nF → RESET`、全電源/GND 接続、`A4/A5` ⇄ `SDA/SCL`。
     - 可読性チェック：文字被り・交差の気持ち悪さ・ブロックの見やすさを 1〜5 で採点。
  2. 判定と指摘事項（あれば部品番号）を [supporter_log.md](supporter_log.md) の `U2` に書く。
- **OK 基準**: SCH DRC 0 かつサポーターが「PCB へ進んで OK」と判断できること。

---

### U3：PCB 外形・メカ・配置・シルク自動化レビュー（H2 検証）

- **PL が先にやること**: PCB 同期後、`spec_geometry.json` から外形（右端カット）・M3 穴×4・4 ヘッダ（160mil ズレ）・ICSP・USB-C・全ピン名シルクを自動生成し、全 SMD を初期配置して `outputs/unomodoki_pcb_placed.png` を提出する。
- **サポーターへのお願い**:
  1. PNG または Windows GUI を見て、人間目線でチェックする：
     - USB-C の基板端突き出し量は適切か（ケーブルが挿しやすいか）
     - リセット SW は指で押しやすい位置か
     - 4 種 LED（`PWR`/`L`/`TX`/`RX`）は見やすいか
     - 左上 M3 穴（`Hole 4`）と `SCL` ヘッダや部品が干渉していないか
     - 水晶とパスコンが MCU 電源/XTAL ピン直近にあるか
     - 全ピン名シルクがパッド・穴に被らず読めるか
  2. 実用度（1〜5）と「ここは GUI で手で動かした方が早い」と感じた箇所を `U3` に記録する。
- **OK 基準**: ヘッダ/穴の数値誤差 0.01mm 以内かつ、配線に入ってよい配置であること。

---

### U4：2 層配線・ビア・GND ベタ — ハイブリッド分業実験（H3 検証）

- **Phase U4-a（PL 試行）**:
  - PL がクリティカルネット＋可能範囲の配線を API で試行し、タイムボックス到達または引き継ぎ好機で「配線済み／未配線リスト」と共にバトンを渡します。
  - **サポーターへのお願い**: GUI で AI の配線を見て、上手い点・イマイチな点を軽く観察する。
- **Phase U4-b（サポーター GUI 仕上げ → PL 最終検品）**:
  - **サポーターへのお願い**:
    1. ストップウォッチ等で時間を計りつつ、Windows EasyEDA Pro GUI で残り未配線の解消・ビア整理・見た目の仕上げを行う。
    2. `Ctrl+S` で保存して PL に「GUI 仕上げ完了」と伝える。
    3. PL が「表裏 GND ベタ再構築 → `pcb_Drc` 0 確認 → 未配線 0 確認 → 表裏 PNG 出力」を実行するので、最終 PNG を確認する。
    4. 作業時間・満足度・ベストな引き継ぎタイミングを [supporter_log.md](supporter_log.md) の `U4` に記録する。
- **OK 基準**: PCB DRC 0、未配線 0、表裏ベタ健全、U4 タイムログ記入済み。

---

### U5：製造データ出力 ＆ EasyEDA 測定ツール検品

- **PL が先にやること**: Gerber ZIP / BOM CSV / CPL（`.xlsx`）/ プレビュー PNG を `outputs/` に一括出力し、[spec_geometry.json](spec_geometry.json) 内の `easyedaMeasurementCheckpoints`（7 項目）を案内する。
- **サポーターへのお願い**:
  1. **EasyEDA Pro 測定ツール検品**: Windows の EasyEDA Pro 画面上で測定ツール（Measure）を使い、[Arduino_uno_r3_size_mill.png](data/Arduino_uno_r3_size_mill.png) と以下 7 項目が一致しているか確認する：
     - 全体幅 `2700 mil` (`68.58 mm`) / 全体高さ `2100 mil` (`53.34 mm`)
     - 上下ヘッダ列の Y 間隔 `1900 mil` (`48.26 mm`)
     - D8–D7 間ギャップ **`160 mil`** (`4.064 mm`)
     - 左上穴 `MH4` ↔ `SCL` ピン間 `140 mil` (`3.556 mm`)
     - 右側穴 `MH2` ↔ `MH3` 縦間隔 `1100 mil` (`27.94 mm`)
     - `ICSP Pin 1 (MISO)` 位置（原点から `(2505, 1200) mil`）
  2. **EasyEDA 3D ビュー確認**: 3D ビューで部品の向き・浮き・干渉（特に USB-C・ATmega328P 1番ピン・CH340C・4 種 LED・リセット SW）を目視確認する（任意で JLCPCB アップロード確認も可）。
  3. **終了クリーンアップ**: PL の `session.close` 後、Windows 側の `start_easyeda_bridge.bat` を `Ctrl+C` で閉じてポート閉鎖を確認する。
  4. 結果を `U5` に記録する。
- **OK 基準**: 製造データ 3 点完備＋EasyEDA 測定ツール 7 項目一致＋3D 目視 OK。

---

### U6：振り返り ＆ ADX 本丸（M5）分業マトリクス確定

- **PL が先にやること**: U0〜U5 の記録を集計し、`lessons_learned.md` の下書きを作成する。
- **サポーターへのお願い**:
  1. [supporter_log.md](supporter_log.md) の `U6` で、仮説 H1〜H4 の判定と **「ADX 本丸（M5）推奨分業マトリクス」** を確定する。
  2. `lessons_learned.md` を最終確認し、「AI＝PL / 人間＝サポーター」体制への感想をひとこと残す。
- **OK 基準**: 分業マトリクスと `lessons_learned.md` が確定していること。
