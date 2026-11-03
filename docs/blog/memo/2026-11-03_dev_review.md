<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 開発履歴・技術進捗メモ：2026年11月3日
〜 100pcs 小ロット製造の壁、電源・通信の物理分離（CORE-CX5）、および盤内配線トポロジー（ADX-BUS-SPEC-v4.1）への仕様整合 〜

- **策定日**: 2026-11-03
- **ステータス**: APPROVED / 内部公式開発レビュー ＆ Hackaday.io 執筆用 SSOT
- **対象ファイル**: [`docs/blog/memo/2026-11-03_dev_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/memo/2026-11-03_dev_review.md)
- **準拠指針**: [`docs/blog/TONE_AND_MANNER.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/TONE_AND_MANNER.md) (Honest Engineering / Lab Notebook)
- **対象期間**: 2026-11-02 22:00 UTC 〜 2026-11-03 20:30 JST
- **関連ドキュメント**:
  - 前回のHackadayログ (2026-11-02): [`docs/blog/posts/2026-11-02.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/posts/2026-11-02.md) / [HTML](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/dist/2026-11-02.html)
  - 前回振り返りメモ: [`docs/blog/memo/2026-11-02_dev_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/memo/2026-11-02_dev_review.md)
  - Hackaday.io メトリクスログ: [`docs/blog/memo/hackadayio_log.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/memo/hackadayio_log.md)
  - CORE-CX5 ハードウェア構想書: [`hardware/CORE-CX5/concept.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX5/concept.md)
  - CORE-CX5 rev1 ネットリスト評価書: [`hardware/CORE-CX5/rev1/netlist_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX5/rev1/netlist_review.md)
  - CORE-CX5 rev1 ピンアサイン検討書: [`hardware/CORE-CX5/rev1/pinout_study.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX5/rev1/pinout_study.md)
  - CORE-CX5 名称移行計画書: [`memo/PLAN/CORE_CX5_NAMING_INTEGRATION_PLAN.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/PLAN/CORE_CX5_NAMING_INTEGRATION_PLAN.md)
  - ADX 20-Pin Bus Specification v4.1 (SSOT): [`docs/spec/ADX_PINOUT.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/spec/ADX_PINOUT.md)
  - CH32X035 MCU マッピング: [`docs/spec/mcu_mapping/ch32x035.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/spec/mcu_mapping/ch32x035.md)
  - 設計ノート「USB One Cable Debug とは何なのか？」: [`memo/note/what_is_usb_one_cable_debug.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/note/what_is_usb_one_cable_debug.md)
  - 表現適正化計画書（重力フェイルセーフ是正）: [`memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md)

---

## 1. エグゼクティブ・サマリー（本日の設計焦点と主な決定事項）

昨夜（11月2日）、オフザシェルフ実機による Web MIDI (SysEx) と RP2040 PIO を用いた通信基盤の実証ログを公開しました。本日（11月3日）は、その成果を踏まえて、長年停滞していた USB 対応メインボードの設計を一気に前進させました。

本日の作業の中心は、**「小ロット製造（100pcs/lot）の経済的現実」**、**「現場運用における電源と通信の分離」**、そして**「過剰表現の排除と工学的な整合性の確保」**の3点です。

```text
┌────────────────────────────────────────────────────────────────────────┐
│                   【 2026-11-03 本日の主な決定事項 】                   │
├────────────────────────────────────────────────────────────────────────┤
│ 1. 100pcs 小ロット制約の打破と CORE-CX5 への着地                        │
│    - ATtiny1616 での USB 2チップ構成（BOM高騰）の限界を認め、           │
│      USB PHY 内蔵の WCH CH32X035 (32-bit RISC-V, 62KB Flash) を採用。 │
│    - かつての CH32V003 前提の仮称「CX3」を清算し、正式に「CORE-CX5」へ命名。│
│    - CORE 上のユニバーサルエリア等の安易な付加価値の迷走を淘汰。         │
├────────────────────────────────────────────────────────────────────────┤
│ 2. 電源と通信の物理分離アーキテクチャ (CORE-CX5 rev1 回路設計)          │
│    - 通信用 16P Type-C の VBUS を基板上で結線せず（N.C.）、            │
│      電源供給を給電専用 6P Type-C（または 20-Pin バス）に一本化。       │
│    - PC 側への電源逆流事故の防止と、「マイコンだけ動いて周辺負荷が動かない」│
│      ことによる無駄なソフトウェアデバッグの混乱を物理的に排除。         │
│    - DPDT オルタネイトスイッチ ＋ 直結警告 LED による DFU 戻し忘れ防止。│
│    - eFuse (SY6280AAC) の EN ピン接地制御によるアークレス電源スイッチ。 │
├────────────────────────────────────────────────────────────────────────┤
│ 3. ADX-BUS-SPEC-v4.1 策定と盤内配線トポロジーへのリフレーミング         │
│    - Pin 2 (OWD: SDI) ＋ Pin 5 (EXTCLK: SWCLK) により、20P IDC コネクタ│
│      単体で給電と 2-wire SDI 書き込みが完結（ポゴピン治具を不要化）。   │
│    - Pin 5 コプレーナ GND ガード環境を活用し、Card 側 0Ω DNP ルールを策定。│
│    - 「重力フェイルセーフ」という過剰表現を是正し、産業用制御盤の原則   │
│      （弱電上部・強電下部）に基づく非交差アートワークへ再定義。         │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 本日の開発タイムライン（時系列の歩み: 2026-11-03）

```mermaid
timeline
    title ADX 2026年11月3日 開発タイムライン
    07:14 (JST) : CH32X035 採用構想策定 (c21e82f) : rev0 初期ネットリスト作成
    08:08 (JST) : トーン＆マナー指針策定 (f6fa1f6) : バス仕様 v4.0 策定レポート作成
    08:55 (JST) : ADX Bus v4.0 リポジトリ全域反映 (56d3287) : CH32X035 マッピング初版作成
    17:18 (JST) : QFN-28 データシート精査 (731b96e) : 架空ピンの排除と ADX Bus v4.1 改訂 : 20P IDC 単体 SDI 書込 & 0Ω DNP ルール策定
    18:11 (JST) : CH32X035 MCU マッピング v4.1 同期 (b3461ec)
    18:42 (JST) : 電源配置表現の適正化完了 (426f453) : 盤内トポロジー適合電源配置へリフレーミング
    20:05 (JST) : rev1 ネットリスト・回路ブロック体系化 (50b1319) : netlist_review.md / what_is_usb_one_cable_debug.md 作成
    20:14 (JST) : Hackaday.io ログ計測記録 (c30fa15) : Views 393, Followers 1
    20:50 (JST) : CORE-CX5 正式名称統合計画の承認・実行 : ディレクトリおよび全ドキュメントのリネーム完了
```

### コミット履歴一覧

| コミットハッシュ | 日時 (UTC / JST) | コミットメッセージ | 主な変更内容・成果物 |
| :--- | :--- | :--- | :--- |
| [`c21e82f`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/02 22:14 UTC<br>(11/03 07:14 JST) | feat: add CORE-CX3 hardware concept and initial netlist revision 0 | `hardware/CORE-CX5/concept.md`, `rev0/Netlist_PCB1_2026-11-03 (1).enet` |
| [`f6fa1f6`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/02 23:08 UTC<br>(11/03 08:08 JST) | docs: add tone and manner guidelines and pinout revision reports | `memo/ADX_TONE_AND_MANNER.md`, `memo/REPORT/ADX_PINOUT_V4_SPECIFICATION_REPORT.md` |
| [`56d3287`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/02 23:55 UTC<br>(11/03 08:55 JST) | docs: update documentation and pinout to v4.0 with core-cx3 additions | `README.md`, `docs/spec/ADX_PINOUT.md` (v4.0), `docs/spec/mcu_mapping/ch32x035.md` |
| [`731b96e`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 08:18 UTC<br>(11/03 17:18 JST) | docs: update ADX pinout specification to v4.1 for CORE-CX3 support | `docs/datasheet/CH32X035G8U6.pdf`, `docs/spec/ADX_PINOUT.md` (v4.1), `pinout_study.md` |
| [`b3461ec`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 09:11 UTC<br>(11/03 18:11 JST) | docs: update CH32X035 MCU mapping to ADX-BUS-SPEC-v4.1 | `docs/spec/mcu_mapping/ch32x035.md` (v4.1 同期) |
| [`426f453`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 09:42 UTC<br>(11/03 18:42 JST) | docs: update pinout terminology and guidelines for panel-aligned power topology | `GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`, 各種仕様書表現の適正化 |
| [`50b1319`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 11:05 UTC<br>(11/03 20:05 JST) | feat: add CORE-CX3 rev1 hardware design files and documentation | `adx_core-cx5_rev1_WIP.png`, `adx_core-cx5_rev1-wip.enet`, `netlist_review.md`, `what_is_usb_one_cable_debug.md` |
| [`c30fa15`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 11:14 UTC<br>(11/03 20:14 JST) | docs: add hackaday.io data log memo | `docs/blog/memo/hackadayio_log.md` (Views: 393) |

---

## 3. 主要技術トピックと設計判断の深掘り

### 3.1 小ロット生産（100pcs/lot）の経済的現実と CORE-CX5 への着地

#### ① ホビーマイコンの量産規模とインディーハードウェアのギャップ
Raspberry Pi Pico（\$4〜\$5）や Arduino UNO 互換機（\$3〜\$4）は、月産数十万台規模の自動実装ライン、ウェハ・リール単位の一括調達を前提に価格が設定されています。

一方、私たちが計画しているような「100pcs / 1lot」規模の小ロット生産（JLCPCB や PCBWay 等の PCBA サービス利用）では、以下のコスト構造に直面します：
- 基本セットアップフィーやステンシル費用が、1枚あたりの単価に直接跳ね返る。
- パーツ種別数（BOM ライン数）や、Extended Component（リール交換が必要な部品）が増えるごとにフィーが加算される。
- チップ調達がカットテープ単位になり、大量購入割引が効かない。

この条件下で、初期に検討していた **ATtiny1616（Flash 16KB / SRAM 2KB）** で USB 対応ボード（旧 CORE-U 構想）を作ろうとすると、以下の構造的破綻が生じていました：
1. **部品点数の肥大化（2チップ構成）**:
   - MCU 内に USB PHY がないため、USB-UART ブリッジ IC（CH340 等）と外付け水晶発振子、周辺受動部品が必須になる。
   - これにより BOM コストと実装費用が嵩み、100pcs ロットでの製造原価だけでホビーマイコンの小売り価格を大きく上回る。
2. **スペックと価格の不整合**:
   - ボード原価が高くなるにもかかわらず、マイコンのリソースは Flash 16KB / SRAM 2KB に留まり、USB 制御やスタックにメモリを割くと実用的なユーザーコード領域が残らない。

#### ② 付加価値の迷走と淘汰
このコスト高を正当化しようとして、過去の設計では安易な「足し算の工夫」が検討されたこともありました：
- **CORE 基板へのユニバーサルエリア（蛇の目）配置**:
  - 8748 フォームファクタの空き面積に試作エリアを設ければお得感が出るのではないかという発想。
  - しかし、そこに部品をハンダ付けしてしまうと、**「ネジを外してコアだけ瞬時に交換できる」という ADX 本来のスワップ保守性が損なわれる**。
  - プロトタイピングは前面端子台に直結する専用の「Card（Prototyping Card）」が担うべきであり、CORE に試作エリアを混載することはアーキテクチャの越境（アンチパターン）であったと判断し、完全に却下しました。
- **USB 1 ポート化によるコストダウン**:
  - コストを下げるために端子を 1 つにまとめようとしたが、後述の通り「PC への電源逆流」と「中途半端な動作によるデバッグの混乱」を招くため却下。

#### ③ CH32V003（CX3）から CH32X035（CORE-CX5）へ
- 当初、10 セントマイコンとして話題になった **CH32V003**（QFN-20）を用いた構成（仮称: CORE-CX3）も検討されましたが、USB PHY が非搭載で Flash 16KB / RAM 2KB と、ATtiny1616 と同様の壁を越えられませんでした。
- 最終的に採用された **WCH CH32X035G8U6** は：
  - **USB 2.0 Full-Speed PHY 内蔵**（外付けブリッジ IC・水晶発振子が不要、1 チップで USB 完結）。
  - **Flash 62KB / SRAM 20KB、48MHz 32-bit RISC-V4C**。
- これにより、100pcs ロットでも部品点数を最小限に抑えつつ、実用的なリソースを確保できる見通しが立ちました。型番「035」に基づき、製品呼称を正式に **`ADX CORE-CX5`** と決定しました。

---

### 3.2 電源・通信の物理分離と「USB 1本デバッグ」の功罪
（参照: [`memo/note/what_is_usb_one_cable_debug.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/note/what_is_usb_one_cable_debug.md)）

#### ① Arduino UNO が DC ジャックを残した理由
Raspberry Pi Pico 等のホビーボードでは、「USB ケーブル 1 本で電源・書き込み・シリアルモニタが完結する」スタイルが主流です。しかし、Arduino UNO は 20 年間、丸型 DC ジャックを併設し続けました。

- PC の USB 2.0 ポートが供給できる電力は規格上 **$5\,\text{V} / 500\,\text{mA} = 2.5\,\text{W}$**。
- シールドを重ねてリレー、RC サーボ、モータ、各種センサを駆動すると、必要電流は容易に **$1\,\text{A} \sim 2\,\text{A}$** に達します。
- Arduino UNO の利点は、**「USB-B（四角）」と「DC ジャック（丸）」という明確な物理形状の差**により、開発者が「机上でのスケッチ書き込み」と「実負荷駆動」を自然に区別できた点にありました。

#### ② Type-C 統一によるアフォーダンスの崩壊と UX の課題
現代では AC アダプタも Type-C が主流となったため、基板上には「同じ外形のコネクタ」が複数並ぶことになります。

ここで「通信ポートからもマイコンに給電できるようにしよう」と中途半端な共通化を行うと、以下のトラブルが発生します：
1. 通信ケーブル 1 本を挿すと、ボード上の LED が点灯し、マイコンが起動し、ファームウェア書き込みも成功し、シリアルログも流れる。
2. しかし、外部のモータやセンサには電力が供給されず、一切動作しない。
3. 開発者は「初期化コードのバグか？」「I2C アドレスが違うのか？」とソフトウェア側の原因を疑い、無駄な調査時間を費やす。

> **「一部だけ中途半端に動く」状態は、開発者にとって最も原因特定が難しい失敗モードです。**  
> 電源が入っていないなら、最初からマイコンも一切起動しない方が、「電源ケーブルが挿さっていない」という事実に即座に気づけます。

#### ③ 通信ポート VBUS の非接続（N.C.）設計
CORE-CX5 では、産業用 PLC の考え方を採用し、**通信用 16P Type-C の VBUS を基板上で完全に未接続（N.C.）** としました。
- **PC 逆流破損の防止**: 電源ラインそのものが物理的に接続されていないため、外部電源からの回り込みによる PC ポート破損リスクを根本から排除。
- **明確な責任分界**: 電源ポート（6P Type-C）または 20-Pin バスから給電されない限り、ボード全体が起動しないため、電源供給の有無が一目で判断可能。
- **主電源スイッチの挙動**: オンボードの電源スイッチを切れば、PC が接続されていてもボード全体の通電が確実に遮断される。

---

### 3.3 CORE-CX5 rev1 回路設計の要点
（参照: [`hardware/CORE-CX5/rev1/netlist_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX5/rev1/netlist_review.md)）

```text
［01: 通信 Type-C］── [D+/D-] ──➔［USB_Serge (ESD)］──➔［06: MCU (PC16/PC17)］
   (16P, 5.1k Rd, VBUS N.C.)

［02: 電源 Type-C］──➔［02: TVS/Cap］──➔［03: eFuse (SY6280)］──➔ VBUS_PROT (Pins 17-20)
   (6P, 5.1k Rd, SMF5.0CA)                        │                   │
                                          [POW_SW: EN制御]      ├─➔［04: LDO (ME6211)］──➔ 3V3 ──➔［06: MCU］
                                                                └─➔ 拡張 Card / サーボ負荷
```

1. **回路ブロックの体系化**:
   - ネットリストおよび BOM を機能ブロックごと（`01-`〜`06-`）に整理。
2. **DFU_SW（自锁スイッチ ＋ ハードウェア直結赤色 LED）**:
   - モメンタリ方式（ボタンを押しながら USB 挿入）は両手が塞がるため、**オルタネイト（自锁: `HX PS5.8x5.8-6P CC`）** を採用。
   - スイッチの片極（回路2）で **高輝度赤色 LED を物理的に直結点灯** させ、通常モードへの戻し忘れを視覚的に警告。
3. **POW_SW（eFuse EN ピン接地制御）**:
   - メカニカルスイッチで 2A 級の電源ラインを直接断続すると、接点の摩耗やアーク放電の原因となる。
   - `03_eFuse` (SY6280AAC) の **Pin 4 (EN)** をプルアップし、スイッチで GND に落とす制御を採用。スイッチ電流は $0.5\,\text{mA}$ 程度に抑えられ、突入電流も eFuse のソフトスタートで抑制。
4. **MCU 直近パスコン (`06-C1`) の新設**:
   - LDO の出力コンデンサ（位相補償用）とは別に、MCU の高速動作時の過渡電流を補うため、Pin 2 (VDD) と Pin 29 (EPAD/GND) の **至近距離（1〜2mm以内）に 100nF を配置**。

---

### 3.4 ADX-BUS-SPEC-v4.1 と CH32X035 のペリフェラルマッピング
（参照: [`hardware/CORE-CX5/rev1/pinout_study.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX5/rev1/pinout_study.md) / [`docs/spec/ADX_PINOUT.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/spec/ADX_PINOUT.md)）

#### ① データシート精査によるピン配修正
旧ドラフトに混入していた他品種由来の架空ピン（PD7、PA9/10、PB13〜15）を排除し、CH32X035G8U6 (QFN-28) の実端子に基づき再マッピングを行いました。

#### ② 20P IDC コネクタ単体での自動検査・2-wire SDI 書き込み
- CH32X035 のデバッグ書き込みには `SWDIO`（PC18）と `SWCLK`（PC19）が必要です。
- ADX バスの **Pin 2 (`OWD`)** に `SWDIO`、**Pin 5 (`EXTCLK`)** に `SWCLK` を割り当て。
- Pin 5 は両隣（Pin 4, Pin 6）が GND で挟まれた **コプレーナ導波路（GND サンドイッチ）** となっており、高周波デバッグクロックをフラットケーブル経由で伝送するのに適した環境です。
- これにより、量産治具でポゴピン（ベッド・オブ・ネイルズ）を立てることなく、**20-Pin IDC ケーブルを 1 本挿すだけで給電・自己診断・ファームウェア書き込みが完結** します。

#### ③ ハードウェア I2C 制約の回避
CH32X035 のハードウェア I2C はピン重複が多く、一部ロットでシリコンエラッタが報告されているため、**48MHz RISC-V による高速 GPIO ソフトウェア I2C（Bit-banging）** を採用して安定性を優先しました。

#### ④ 拡張 Card 側の 0Ω DNP 推奨ルール
Core ボードによって Pin 5 の役割が異なる（ATtiny1616 では外部クロック入力、CORE-CX5 では SWCLK / PWM）ため、拡張 Card 側の Pin 5 には **直列 0Ω 抵抗（またはソルダーブリッジ）を配置し、未実装（DNP）で切り離せる構造を推奨** としました。これにより、1種類の Card 基板で全 Core ボードとの互換性を確保します。

---

### 3.5 盤内配線トポロジーへの回帰（「重力フェイルセーフ」表現の適正化）
（参照: [`memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md)）

仕様書内の過剰・ドラマチックな表現を見直し、工学的な整合性に基づく記述へ是正しました。

#### ① 旧記述の反省
従来のドキュメントでは、Pins 17–20 に電源を配置する理由を「端子ネジが緩んでも重力で電線が下に落ちるため、上部の通信線に触れない（重力フェイルセーフ）」と説明していました。しかし、IDC フラットケーブルのピンが重力で落ちるわけではなく、特定の都合の良いシナリオに依存した過剰な表現でした。

#### ② 制御盤の実務原則：「弱電上部・強電下部」
本質的な理由は、産業用制御盤の配線原則にあります：
- **盤内配線ダクト**: 上部ダクトに通信・計装線、下部ダクトに主電源・動力線を通す。
- **前面 I/O 端子台**: モジュール上部に通信端子、下部に電源端子を配置する。

```text
  【 盤内ダクト 】       【 8748 モジュール前面 I/O 】           【 20-Pin IDC バス (背面) 】
  ┌──────────────┐         ┌─────────────────────────┐            ┌────────────────────┐
  │ 上部ダクト   │ ──────➔ │ 上部端子台: RS-485 / 通信│ ─────────➔ │ Pins 7–15: 通信    │ (弱電)
  │ (通信・計装) │         └─────────────────────────┘            └────────────────────┘
  └──────────────┘                      │                                    │
                                  基板内アートワーク                    基板内アートワーク
                                (弱電パターン上半分)                  (強電パターン下半分)
                                        │                                    │
  ┌──────────────┐         ┌─────────────────────────┐            ┌────────────────────┐
  │ 下部ダクト   │ ──────➔ │ 下部端子台: 5V大電流受電│ ─────────➔ │ Pins 17–20: VBUS   │ (強電)
  │ (電源・動力) │         └─────────────────────────┘            └────────────────────┘
  └──────────────┘
```

- バックプレーン最下段（Pins 17–20）に `VBUS` を置くことで、基板下層を通って前面最下段の電源受電端子台へ**一直線に太いベタ銅箔パターンを引く**ことができます。
- これにより、基板内部で大電流ラインが上部の微弱なアナログ（AIN）や高周波通信（SPI/UART）を横切る必要がなくなり、**基板内クロスオーバーによるノイズ誘導や IR ドロップ干渉を原理的に防止**できます。
- この工学的必然性に基づき、用語を「重力フェイルセーフ」から **「盤内配線トポロジー適合電源配置 (Panel-Aligned Power Topology)」** へ全面改訂しました。

---

## 4. Hackaday.io 現状メトリクスと観察

### メトリクスログ (`docs/blog/memo/hackadayio_log.md`)

| 計測日時 (JST) | Views | Comments | Followers | Likes (Skulls) | 備考 |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **2026/11/03 20:11** | **393** | **0** | **1** | **0** | 前回記事（Web MIDI SysEx + PIO）公開から約24時間後 |

- **閲覧数**: 24時間で約 400 ビューと堅調に推移。
- **分析**:
  - 前回の記事は Web MIDI や PIO という特定技術に焦点を当てたディープな内容でした。
  - 次回記事では、小ロットハードウェア開発の現実、電源供給をめぐるトラブルなど、**より多くのエンジニアが実体験として共感できるテーマ**を取り上げることで、読者との対話（コメント）を促す構成が効果的と考えられます。

---

## 5. 次期 Hackaday.io 記事の企画・切り口案 (Lab Notebook 視点)

`docs/blog/TONE_AND_MANNER.md` の 4大原則（Lab Notebook, Honest Engineering, Story-Driven, Community-First）に基づき、以下の3つの切り口を提案します。

---

### 【切り口案 A: 製造の現実と電源分離の哲学（推奨）】
### タイトル: *"Why We Killed 'USB One-Cable Debug' on Our RISC-V Controller"*
- **サブタイトル**: *Balancing 100-piece production limits with industrial realities: why half-working hardware is the worst UX in embedded design.*
- **ヒーロー画像**: `docs/blog/images/adx_core-cx5_rev1_WIP.png`（CORE-CX5 rev1 基板 CAD 図、デュアル Type-C 部のクローズアップ）
- **ストーリー展開**:
  1. **The Economic Wall**: 100pcs/1lot のインディー開発者にとって、Pico（\$4）や Arduino の量産規模は高すぎる壁だった。ATtiny1616 で USB をやろうとして 2 チップ構成で BOM が跳ね上がった経緯と、CH32X035 による 1 チップ化の選択。
  2. **The Convenience Trap**: USB ケーブル 1 本で動くのは机上プロトタイプでは便利だが、アクチュエータやセンサを繋ぐ現場では PC の 500mA では電力が足りない。Arduino UNO が丸型 DC ジャックを残した理由。
  3. **The Toxic UX of 'Half-Working'**: 通信ポートからマイコンだけに中途半端に給電すると、「ソフトのバグか？ 電源が入っていないのか？」で時間を浪費する。動かないなら最初から一切動かない方が、人間は即座に電源抜けに気づける。
  4. **The Circuit Implementation**: 通信 Type-C の VBUS を基板上で未接続（N.C.）にし、給電専用 Type-C と完全分離。eFuse EN 制御スイッチや DPDT 直結警告 LED の回路工夫。
  5. **Let's Hear From You**: *「皆さんは開発ボードで『マイコンだけ動いて周辺回路が動かず無駄なデバッグをした』経験はありませんか？ 電源と通信の分離についてどう考えますか？」*

---

### 【切り口案 B: 仕様書の過剰表現の自己是正（工学的誠実さ）】
### タイトル: *"Tearing Down Our Own Hype: From 'Gravity Fail-Safe' to Honest Panel Topology"*
- **サブタイトル**: *We caught ourselves writing marketing fiction into our open bus spec. Here is why we killed 'Gravity Fail-Safe' and aligned with 50-year-old cabinet wiring rules.*
- **ヒーロー画像**: 盤内ダクト・前面端子台・基板パターン・背面 IDC の直線配線トポロジー図
- **ストーリー展開**:
  1. **The Confession**: バス仕様書に「ネジが緩んで電線が外れても重力で落ちるから安全」と書いていたが、プロのエンジニアの視点で見直すと、後付けのフィクションであることに気づき反省した。
  2. **The True Axiom**: なぜコネクタ最下段に電源を置くのか？ 制御盤の基本原則「弱電は上、強電は下」との整合。
  3. **Non-Crossing Artwork**: 盤内ダクト ➔ 前面端子台 ➔ 基板内パターン ➔ 背面 IDC コネクタを直線で一致させることで、基板内部で大電流パターンが信号線を横切る（クロスオーバーする）リスクを全廃した設計根拠。
  4. **Let's Hear From You**: *「スペックシートやドキュメントで、後から『これは言い過ぎだった』と気づいて直した経験はありますか？」*

---

### 【切り口案 C: 製造治具のハック（ポゴピンレス自動検査）】
### タイトル: *"No Pogo Pins Needed: Flashing a RISC-V Core Through Standard Ribbon Cables"*
- **サブタイトル**: *Using coplanar ground shields and 0-ohm jumper rules to turn an ordinary 20-pin IDC header into a factory programming port.*
- **ヒーロー画像**: 20P IDC ヘッダーと CH32X035 QFN-28 の結線図、コプレーナ GND サンドイッチの拡大図
- **ストーリー展開**:
  1. **The Test Fixture Problem**: 小ロット製造でポゴピン治具（ベッド・オブ・ネイルズ）を作るのはコストが高く、接触不良のメンテナンスも大変。
  2. **The Dual-Use Header**: 20-Pin 拡張コネクタの Pin 2 (`OWD`) と Pin 5 (`EXTCLK`) をデバッグ線（SWDIO/SWCLK）として定義し、フラットケーブル 1 本で給電と書き込みを完結させた。
  3. **Signal Integrity on Ribbon Cables**: Pin 5 を両隣の GND で挟むコプレーナ導波路構造と、拡張 Card 側での 0Ω DNP ルールによる異種マイコン（ATtiny と RISC-V）の共存。
  4. **Let's Hear From You**: *「小規模ロットの検査治具、皆さんはどう工夫していますか？ ポゴピン派ですか、それともコネクタ兼用派ですか？」*

---

## 6. 次のアクションアイテム

1. **Hackaday.io 記事原稿の執筆**:
   - 上記切り口案 A（推奨）をベースに、`docs/blog/posts/2026-11-03.md` を執筆。
   - `build_hackaday.py` による HTML ビルドの確認。
2. **CORE-CX5 rev1 PCB アートワーク設計**:
   - `hardware/CORE-CX5/rev1/adx_core-cx5_rev1-wip.enet` をベースに基板レイアウトを開始。
   - 底面 Pins 17-20 から前面端子台への直線ベタ銅箔パターンの配線。
   - MCU 直近パスコン `06-C1` の配置。
3. **UF2 ブートローダーの CH32X035 向け実装検討**:
   - USB Full-Speed PHY を活用した Mass Storage ドラッグ＆ドロップ書き込みの実装調査。
