<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 開発履歴・技術進捗メモ：2026年11月3日
〜 「USB One Cable Debug」への決別、ADX-BUS-SPEC-v4.1 策定、そして「重力フェイルセーフ」の自己是正と盤内配線トポロジーへの回帰 〜

- **策定日**: 2026-11-03
- **ステータス**: APPROVED / 内部公式開発レビュー ＆ Hackaday.io 執筆用 SSOT
- **対象ファイル**: [`docs/blog/memo/2026-11-03_dev_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/memo/2026-11-03_dev_review.md)
- **対象期間**: 2026-11-02 22:00 UTC 〜 2026-11-03 20:30 JST（本日一日集中開発スプリント）
- **関連ドキュメント**:
  - 前回のHackadayログ (2026-11-02): [`docs/blog/posts/2026-11-02.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/posts/2026-11-02.md) / [HTML](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/dist/2026-11-02.html)
  - 前回振り返りメモ: [`docs/blog/memo/2026-11-02_dev_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/memo/2026-11-02_dev_review.md)
  - Hackaday.io メトリクスログ: [`docs/blog/memo/hackadayio_log.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/memo/hackadayio_log.md)
  - CORE-CX3 ハードウェア構想書: [`hardware/CORE-CX3/concept.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX3/concept.md)
  - CORE-CX3 rev1 ネットリスト評価書: [`hardware/CORE-CX3/rev1/netlist_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX3/rev1/netlist_review.md)
  - CORE-CX3 rev1 ピンアサイン検討書: [`hardware/CORE-CX3/rev1/pinout_study.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX3/rev1/pinout_study.md)
  - ADX 20-Pin Bus Specification v4.1 (SSOT): [`docs/spec/ADX_PINOUT.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/spec/ADX_PINOUT.md)
  - CH32X035 MCU マッピング: [`docs/spec/mcu_mapping/ch32x035.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/spec/mcu_mapping/ch32x035.md)
  - 設計哲学ノート「USB One Cable Debug とは何なのか？」: [`memo/note/what_is_usb_one_cable_debug.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/note/what_is_usb_one_cable_debug.md)
  - 表現適正化計画書（重力フェイルセーフ是正）: [`memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md)
  - トーン＆マナー指針: [`memo/ADX_TONE_AND_MANNER.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/ADX_TONE_AND_MANNER.md) / [`docs/blog/TONE_AND_MANNER.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/blog/TONE_AND_MANNER.md)

---

## 1. エグゼクティブ・サマリー（今日一日の核心と3大ブレークスルー）

2026年11月2日夜、オフザシェルフ実機（Raspberry Pi Pico H ＋ Waveshare 2CH-RS485）を用いた「Web MIDI API (SysEx) ＋ PIO 自律波形制御 ＋ 8B UART フレーム」による Over-the-Wire ブートローディングの実証完了を公開してから、わずか**24時間足らず**。

本日（2026年11月3日）、ADXプロジェクトは、先行実証プローブ（RPR4）の成功を足がかりに、次世代メインライン製品である **「ADX CORE-CX3 (The Enclosure-Native USB Core)」** の具現化と、全規格の屋台骨である **「ADX 20-Pin Bus Specification v4.1」** の策定、さらには技術者としての誠実さを貫く **「誇大表現の自己是正」** を成し遂げました。

```text
┌────────────────────────────────────────────────────────────────────────┐
│             【 2026-11-03 (本日) の 3大エンジニアリング・ブレークスルー 】          │
├────────────────────────────────────────────────────────────────────────┤
│ 1. 「USB One Cable Debug」への決別と PLC 思想への回帰 (CORE-CX3 rev1)   │
│    - デュアル Type-C ポート（通信用 16P ＋ 電源用 6P）の完全物理分離。   │
│    - 通信ポートの VBUS を完全 N.C.（未接続）化。PC 逆流破損事故 0.00% と │
│      「中途半端に動いて失敗を隠蔽する最悪の UX」を原理から根絶。       │
│    - DPDT オルタネイト DFU スイッチ ＋ ハードウェア直結赤色警告 LED。   │
│    - eFuse (SY6280AAC) の EN ピン接地制御によるアークレス電源スイッチ。 │
├────────────────────────────────────────────────────────────────────────┤
│ 2. ADX-BUS-SPEC-v4.1 策定とポゴピンレス自動検査アーキテクチャ           │
│    - Pin 2 (OWD: UPDI/SWDIO) ＋ Pin 5 (EXTCLK/SWCLK: コプレーナ導波路)  │
│      により、20P IDC ケーブル 1 本で VBUS 給電 ＋ 2-wire SDI 書込が完結。 │
│    - QFN-28 (CH32X035) データシート精査で旧ドラフトの架空ピンを一掃。   │
│    - ハードウェア I2C シリコンエラッタを 48MHz ソフトウェア I2C で完全回避。│
│    - 拡張 Card 側の「Pin 5 0Ω DNP 推奨ルール」で全 Core 互換性を確立。  │
├────────────────────────────────────────────────────────────────────────┤
│ 3. 誇大表現の自己是正：重力フェイルセーフから「盤内トポロジー整合」へ     │
│    - 「外れた電線が重力で落ちる」というドラマチックな後付け表現を反省。 │
│    - 制御盤 50 年の鉄則「弱電上部・強電下部」に基づく工学的真理へ回帰。 │
│    - バックプレーン最下段 Pins 17-20 ➔ 基板下層大電流ベタ ➔ 前面最下部  │
│      端子台の完全直線配置により、基板内部の強電・弱電交差を全廃。       │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 本日の開発タイムライン（時系列の歩み: 2026-11-03）

```mermaid
timeline
    title ADX 2026年11月3日の濃密な一日開発スプリント
    07:14 (JST) : CORE-CX3 ハードウェア構想策定 (c21e82f) : rev0 初期ネットリスト作成
    08:08 (JST) : トーン＆マナー指針制定 (f6fa1f6) : ADX Bus v4.0 策定レポート作成
    08:55 (JST) : ADX Bus v4.0 リポジトリ全域反映 (56d3287) : CH32X035 初代マッピング策定
    17:18 (JST) : CH32X035 データシート徹底精査 (731b96e) : 架空ピンの排除と ADX Bus v4.1 改訂 : ポゴピンレス 2-wire SDI 書込 & 0Ω DNP 策定
    18:11 (JST) : CH32X035 MCU マッピング v4.1 正式同期 (b3461ec)
    18:42 (JST) : 「重力フェイルセーフ」表現適正化計画の完遂 (426f453) : 盤内トポロジー適合電源配置へ全面リフレーミング
    20:05 (JST) : CORE-CX3 rev1 ネットリスト完成 (50b1319) : 回路ブロック 01〜06 体系化 & レビュー書策定 : 「USB One Cable Debug とは何なのか？」哲学ノート公開
    20:14 (JST) : Hackaday.io メトリクスログ記録 (c30fa15) : Views: 393, Followers: 1
```

### コミット履歴一覧 (本日分)

| コミットハッシュ | 日時 (UTC / JST) | コミットメッセージ | 主な変更内容・成果物 |
| :--- | :--- | :--- | :--- |
| [`c21e82f`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/02 22:14 UTC<br>(11/03 07:14 JST) | feat: add CORE-CX3 hardware concept and initial netlist revision 0 | `hardware/CORE-CX3/concept.md`, `rev0/Netlist_PCB1_2026-11-03 (1).enet` |
| [`f6fa1f6`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/02 23:08 UTC<br>(11/03 08:08 JST) | docs: add tone and manner guidelines and pinout revision reports | `memo/ADX_TONE_AND_MANNER.md`, `memo/REPORT/ADX_PINOUT_V4_SPECIFICATION_REPORT.md` |
| [`56d3287`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/02 23:55 UTC<br>(11/03 08:55 JST) | docs: update documentation and pinout to v4.0 with core-cx3 additions | `README.md`, `docs/spec/ADX_PINOUT.md` (v4.0), `docs/spec/mcu_mapping/ch32x035.md` |
| [`731b96e`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 08:18 UTC<br>(11/03 17:18 JST) | docs: update ADX pinout specification to v4.1 for CORE-CX3 support | `docs/datasheet/CH32X035G8U6.pdf`, `docs/spec/ADX_PINOUT.md` (v4.1), `pinout_study.md` |
| [`b3461ec`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 09:11 UTC<br>(11/03 18:11 JST) | docs: update CH32X035 MCU mapping to ADX-BUS-SPEC-v4.1 | `docs/spec/mcu_mapping/ch32x035.md` (v4.1 正式同期) |
| [`426f453`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 09:42 UTC<br>(11/03 18:42 JST) | docs: update pinout terminology and guidelines for panel-aligned power topology | `GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`, 全ドキュメントの表現適正化 |
| [`50b1319`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 11:05 UTC<br>(11/03 20:05 JST) | feat: add CORE-CX3 rev1 hardware design files and documentation | `adx_core-cx3_rev1_WIP.png`, `adx_core-cx3_rev1-wip.enet`, `netlist_review.md`, `what_is_usb_one_cable_debug.md` |
| [`c30fa15`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/.git) | 11/03 11:14 UTC<br>(11/03 20:14 JST) | docs: add hackaday.io data log memo | `docs/blog/memo/hackadayio_log.md` (393 views) |

---

## 3. 主要技術トピックと設計決定の深掘り

### 3.1 「USB One Cable Debug」の甘い罠と PLC への回帰（なぜ通信 Type-C の VBUS を切断したのか？）
（参照: [`memo/note/what_is_usb_one_cable_debug.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/note/what_is_usb_one_cable_debug.md)）

#### ① Arduino UNO が丸型 DC ジャックを捨てなかった理由
現代のマイコンボード（Raspberry Pi Pico や ESP32 開発キット等）では、「USB ケーブル 1 本挿せば電源も書き込みもシリアルデバッグもすべて動く」のが常識とされています。

しかし、オープンソースハードウェアの元祖である **Arduino UNO は、20 年間頑なに 2.1mm 丸型 DC ジャックを廃止しませんでした**。
- PC の USB 2.0 ポートが供給できる電力は、規格上 **$5\,\text{V} / 500\,\text{mA} = 2.5\,\text{W}$**。
- 一方、シールドを重ね、ステッピングモータ、リレー、RC サーボ、無線モジュールを繋いだ瞬間、必要な電力は容易に **$1\,\text{A} \sim 2\,\text{A}$（$5\,\text{W} \sim 10\,\text{W}$）** に達する。
- Arduino UNO が優れていたのは、**「USB-B（四角）」と「DC ジャック（丸）」という明確な物理的アフォーダンス**により、開発者が「スケッチを書くときは USB」「実負荷を動かすときは DC ジャック」と無意識に使い分けられた点にありました。

#### ② USB Type-C 統一がもたらしたジレンマ
ところが現代では、AC アダプタの主役が丸型 DC ジャックから USB Type-C へ移行しました。その結果、ボード上には **「通信用 Type-C」と「電源用 Type-C」という、まったく同じ外形の端子が並ぶ** ことになります。

ここで「通信ポートからも MCU にだけ給電できるようにしよう」と中途半端な妥協をすると、**エンジニアリングにおいて最も有害な UX** が生まれます：
1. PC から通信 Type-C ケーブルを 1 本挿す。
2. ボードの LED が点灯し、マイコンが起動し、ファームウェアも書き込め、シリアルログも正常に出る。
3. **しかし、拡張 Card のモータやセンサが一切動かない。**
4. 開発者は「コードの初期化ルーチンが悪いのか？」「I2C プルアップか？」「ピン配置ミスか？」と何時間も無駄なソフトウェアデバッグを繰り返し、最後に「あ……電源ポート挿してなかっただけか……」と絶望する。

> **「一部だけ中途半端に動いて失敗を隠蔽すること」こそが最悪の UX。**  
> 動かないなら、最初から 1 ミリも動かない方が、人間は 1 秒で「電源が入っていない」と気づける。

#### ③ PLC 思想への完全回帰
産業用制御盤で稼働する PLC において、保守用 Ethernet や USB ケーブルから主電源を賄うという設計は存在しません。「まず盤の主電源を投入し、その上で保守ケーブルを挿す」という明確な責任分界点があります。
CORE-CX3 はこの PLC 思想を徹底し、**通信用 16P Type-C の VBUS を基板上で完全にオミット（N.C.）** しました。
- **PC 逆流事故 0.00%**: 電線そのものが繋がっていないため、ダイオードの故障や誤配線による PC ポート破壊リスクが原理的にゼロ。
- **直感的な主電源スイッチ**: PC が何本繋がっていようが、オンボードの `POW_SW` を切ればボード全体が 100% 完全に遮断される。

---

### 3.2 CORE-CX3 rev1 回路設計の妙技とネットリストレビュー
（参照: [`hardware/CORE-CX3/rev1/netlist_review.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX3/rev1/netlist_review.md)）

本日完成した `hardware/CORE-CX3/rev1/adx_core-cx3_rev1-wip.enet` では、以下の工学的工夫が盛り込まれました。

```text
［01: 通信 Type-C］── [D+/D-] ──➔［USB_Serge (ESD)］──➔［06: MCU (PC16/PC17)］
   (16P, 5.1k Rd, VBUS N.C.)

［02: 電源 Type-C］──➔［02: TVS/Cap］──➔［03: eFuse (SY6280)］──➔ VBUS_PROT (Pins 17-20)
   (6P, 5.1k Rd, SMF5.0CA)                        │                   │
                                          [POW_SW: EN制御]      ├─➔［04: LDO (ME6211)］──➔ 3V3 ──➔［06: MCU］
                                                                └─➔ 拡張 Card / サーボ負荷
```

1. **回路ブロック体系化 (`01-` 〜 `06-` プレフィックス)**:
   - ネットリストおよび BOM を機能ブロックごとに完全モジュール化。
2. **DFU_SW: オルタネイトスイッチ ＋ DPDT ハードウェア直結赤色警告 LED**:
   - モメンタリ方式（ボタンを押しながら挿す）は両手が塞がる。本機では **オルタネイト（自锁: `HX PS5.8x5.8-6P CC`）** を採用し、片手で DFU 待機状態を事前セット可能に。
   - スイッチが押し込まれている間は、MCU の状態に関係なく **超高輝度赤色 LED が物理的に直結点灯** し、「通常モードへの戻し忘れ」を 100% 視覚警告。
3. **POW_SW: eFuse (SY6280AAC) EN ピン制御によるアークレススイッチング**:
   - 2A 級の電源ラインをメカニカルスイッチで直接トグルすると、接点アークによる火花・劣化やチャタリングが生じる。
   - `03_eFuse` の **Pin 4 (EN)** をプルアップし、スイッチで GND に落とす制御トポロジーを採用。スイッチに流れる電流はわずか $0.5\,\text{mA}$ となり、スイッチ寿命が半永久化。eFuse 内蔵ソフトスタートにより突入電流も防止。
4. **MCU 直近パスコン (`06-C1`) の新設**:
   - LDO の出力コンデンサ（位相補償用）とは独立に、48MHz 動作に伴う過渡電流パルスを吸収するため、MCU の Pin 2 (VDD) と Pin 29 (EPAD/GND) の **1〜2mm 最至近に 100nF を配置**。

---

### 3.3 CH32X035G8U6 データシート精査と ADX Bus v4.1 ピン配格闘記
（参照: [`hardware/CORE-CX3/rev1/pinout_study.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/CORE-CX3/rev1/pinout_study.md) / [`docs/spec/ADX_PINOUT.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/docs/spec/ADX_PINOUT.md)）

#### ① 架空ピンの排除とデータシート精査
初期ドラフト仕様では、STM32 や CH32V003 の記述が混入し、「PD7」や「PA9/PA10」「PB13〜PB15」といった **CH32X035G8U6 (QFN-28) には存在しない架空ピン** が割り当てられていました。データシート V2.2 を徹底精査し、物理的に引き出されているピンのみで全ペリフェラルを再設計しました。

#### ② ポゴピン治具不要！20P IDC コネクタ単体での自動検査・2-wire SDI 書込
- CH32X035 のデバッグには `SWDIO`（PC18）と `SWCLK`（PC19）の 2 線が必要です。
- ADX バスの **Pin 2 (`OWD`)** に `SWDIO`、**Pin 5 (`EXTCLK`)** に `SWCLK` を割り当てました。
- Pin 5 は、両隣（Pin 4, Pin 6）が GND で挟まれた **コプレーナ導波路（GND サンドイッチ）** です。十数MHz に及ぶ鋭いクロック信号 `SWCLK` をフラットケーブルに通す上で、これ以上ない理想的なシールド環境です。
- これにより、**高価で壊れやすいポゴピン検査治具を一切使わず、量産工場で 20P IDC ケーブルを 1 本挿すだけで、電源投入・自己診断・ファームウェア書込が 100% 完結** します。

#### ③ ハードウェア I2C シリコンエラッタの回避
CH32X035 のハードウェア I2C はピン重複が多く、特定製造ロットでシリコンエラッタが存在します。本機では **48MHz 32-bit RISC-V の演算力を活かした高速 GPIO ソフトウェア I2C（Bit-banging）** を採用し、エラッタを根本から回避しました。

#### ④ 拡張 Card 側の 0Ω DNP アイソレーション推奨ルール (v4.1)
Core ボードによって Pin 5 の使われ方が異なる（ATtiny1616 では外部クロック入力、CH32X035 ではデバッグクロック SWCLK / PWM）ため、拡張 Card 側には **Pin 5 に 0Ω 抵抗（またはソルダーブリッジ）を配置し、電気的に切り離し可能（DNP）とすることを推奨** しました。これにより、同一の Card 基板がすべての Core で干渉なく動作します。

---

### 3.4 「重力フェイルセーフ」の自己是正と盤内配線トポロジーの真理
（参照: [`memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md)）

本日の特筆すべき進捗は、仕様書の「言葉の嘘」を自分たちで暴き、是正したことです。

#### ① キャッチーだがフィクションだった「重力フェイルセーフ」
これまでの仕様書では、Pins 17–20 に電源を置く理由を以下のように説明していました：
> *「万一端子のネジが緩んで電線が外れても、重力で電線が下へ落下するため、上部の通信線に触れず短絡を防ぐ（重力フェイルセーフ）」*

一見キャッチーですが、プロのエンジニアが見れば「IDC コネクタのピンが重力で落ちるわけがない」「ネジから外れた線が奇跡的に下だけに落ちる都合の良いシナリオに頼りすぎている」という後付けのフィクション感が拭えませんでした。

#### ② 制御盤 50 年の公理：「弱電上部・強電下部」
本質的な理由はそこにありませんでした。盤内配線の鉄則は **「弱電（通信・計装）は上、強電（主電源・動力）は下」** です。

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

1. **前面・背面の完全な直線配置 (Straight-Through Alignment)**:
   - バックプレーン最下段（Pins 17–20）に `VBUS` を置くことで、基板下辺を通って前面最下段の受電端子台へ一直線に太いベタ銅箔を引き通せる。
2. **基板内クロスオーバーの全廃**:
   - 大電流プレーンが基板下層に固定されるため、上部の微弱なアナログ（AIN）や高周波通信（SPI/UART）を大電流ラインが横切る必要が一切なく、**基板内部でのスイッチングノイズ放射や IR ドロップ干渉を根本から排除**できる。

リポジトリ内の全ドキュメントを「重力フェイルセーフ」連呼から **「盤内配線トポロジー適合電源配置 (Panel-Aligned Power Topology)」** へ全面改訂し、地に足のついた工学ドキュメントへと昇華させました。

---

## 4. Hackaday.io 現状メトリクスとコミュニティの反応

### メトリクスログ (`docs/blog/memo/hackadayio_log.md`)

| 計測日時 (JST) | Views | Comments | Followers | Likes (Skulls) | 備考 |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **2026/11/03 20:11** | **393** | **0** | **1** | **0** | 前回記事（Web MIDI SysEx + PIO）公開から約24時間後 |

- **閲覧数の伸び**: 24時間で約 400 ビュー近くまで着実に読まれている。
- **課題**: Comments や Likes (Skulls) がまだゼロ。
- **示唆**: 
  - 前回の「Web MIDI SysEx」は技術的に非常に深い内容だったが、読者が「自分ごと」として議論に参加しやすいフック（問いかけ）がもう少し必要だった可能性がある。
  - 次の記事では、**すべての開発者が一度は痛い目を見たことのある「あるあるネタ（USB One Cable Debug の罠）」** や、**「自己批判・誇大表現の是正」** といった人間味・共感を呼ぶ切り口を前面に出すことで、コメント欄での活発な議論を誘発できる。

---

## 5. 本日の Hackaday.io 向け Post 記事の企画・切り口案

本日の濃密な成果をもとに、Hackaday.io のハッカー・組込エンジニアに強烈に刺さる **3つの記事切り口案** を提案します。

---

### 【切り口案 A（激推し / コア哲学アプローチ）】
### タイトル: *"Why We Killed 'USB One-Cable Debug' for Our Industrial RISC-V Controller"*
- **サブタイトル**: *From Arduino's DC jack to dual Type-C ports: why half-working hardware is the most toxic UX in embedded engineering.*
- **ヒーロー画像**: `docs/blog/images/adx_core-cx3_rev1_WIP.png`（CORE-CX3 rev1 基板回路・デュアル Type-C の CAD 図）
- **ストーリー構成**:
  1. **The Trap**: ラズパイPicoやArduinoで当たり前の「ケーブル1本で開発も給電も完了」。机の上では天国だが、産業現場や実アクチュエータ（サーボ・モータ）を繋いだ瞬間に地獄が始まる。
  2. **The Mystery of Arduino UNO**: なぜ Arduino は 20 年間丸型 DC ジャックを捨てなかったのか？ PC の 500mA では実負荷は動かせないという不都合な真実。
  3. **The Most Toxic UX**: 通信 Type-C からマイコンだけに中途半端に給電すると、「ソフトが悪いのか？電源が入ってないのか？」で何時間も無駄なデバッグをする羽目になる。動かないなら 1 ミリも動かない方が 1 秒で気づける。
  4. **The PLC Axiom**: 通信 16P の VBUS を基板上で完全切断（N.C.）し、電源 6P と物理分離。PC 逆流事故 0% と、アークレス eFuse 電源スイッチ、DPDT 直結赤色警告 LED を備えた CORE-CX3 の回路設計。
  5. **Call to Conversation**: *「皆さんは開発ボードで『マイコンだけ動いてシールドが動かず半日溶かした』経験はありませんか？ デュアル Type-C の物理分離についてどう思いますか？」*

---

### 【切り口案 B（エンジニアの誠実さ・自己批判アプローチ）】
### タイトル: *"Tearing Down Our Own Hype: From 'Gravity Fail-Safe' to Honest Industrial Panel Topology"*
- **サブタイトル**: *We caught ourselves writing marketing fiction into our open bus spec. Here is why we killed 'Gravity Fail-Safe' and embraced 50-year-old cabinet wiring axioms.*
- **ヒーロー画像**: 盤内ダクトとモジュール前面・背面 IDC コネクタの配線フロー図（弱電上・強電下のトポロジー図）
- **ストーリー構成**:
  1. **The Confession**: 仕様書に「電線が外れても重力で下に落ちるから安全」と書いて悦に入っていたが、プロの目で見直して「なんて都合の良いフィクションを書いていたんだ」と猛省した。
  2. **The True Axiom**: 制御盤 50 年の鉄則「弱電は上、強電は下」。なぜ Pins 17–20 が最下段なのか？
  3. **Straight-Through Artwork**: 上部ダクト ➔ 前面通信端子 ➔ 背面 IDC 通信ピン、下部ダクト ➔ 前面電源端子 ➔ 背面 IDC 電源ピン。基板内部で強電プレーンが微弱信号を一切横切らない（クロスオーバー全廃）という本質。
  4. **The Revision**: 仕様書を v4.1 へ全面改訂し、言葉の誠実さを取り戻したプロセス。
  5. **Call to Conversation**: *「スペックシートのキャッチコピーと現場の実務の乖離に気づいた経験はありますか？」*

---

### 【切り口案 C（量産治具・製造ハックアプローチ）】
### タイトル: *"No Pogo Pins Allowed: Flashing a \$0.15 RISC-V Chip Through 20-Pin Ribbon Cables"*
- **サブタイトル**: *How we used coplanar ground shields and 0-ohm jumper rules to turn a standard IDC ejector header into an automated factory SDI programmer.*
- **ヒーロー画像**: 20-Pin IDC ヘッダーと CH32X035 QFN-28 の結線図、コプレーナ GND サンドイッチの拡大図
- **ストーリー構成**:
  1. **The Challenge**: 量産時のポゴピン治具（ベッド・オブ・ネイルズ）は高価で、針が曲がりやすく、接触不良の温床。
  2. **The Breakthrough**: 20-Pin IDC 拡張コネクタそのものを治具インターフェースにしてしまえば、ケーブル 1 本挿すだけで給電と書き込みが完結する。
  3. **The Physics**: Pin 5 (EXTCLK/SWCLK) の両隣を GND で挟む「コプレーナ導波路」。高周波デバッグクロックをフラットケーブルで安定伝送するノウハウ。
  4. **Cross-Core Compatibility**: 拡張 Card 側の 0Ω DNP 推奨ルールによって、ATtiny（外部クロック）と CH32X035（SWD）を同一基板で共存させるハック。
  5. **Call to Conversation**: *「皆さんは小規模量産の検査治具をどう作っていますか？ ポゴピン派ですか？ コネクタ兼用派ですか？」*

---

## 6. 次のアクションアイテム（明日以降の展望）

1. **Hackaday.io 記事の執筆と公開**:
   - 切り口案 A（または A と B の融合）に基づき、`docs/blog/posts/2026-11-03.md` を執筆。
   - `build_hackaday.py` を実行して `docs/blog/dist/2026-11-03.html` をビルド。
2. **CORE-CX3 rev1 PCB アートワーク着手**:
   - `hardware/CORE-CX3/rev1/adx_core-cx3_rev1-wip.enet` をベースに、KiCad / EasyEDA Pro で 2 層基板配線を開始。
   - 底面 Pins 17-20 から前面端子台への太いベタ銅箔プレーンの引き通し。
   - MCU 直近パスコン `06-C1` の配置。
3. **UF2 ブートローダーの CH32X035 向け移植調査**:
   - 内蔵 USB Full-Speed PHY を活用した UF2 Mass Storage ブートローダーの実装検討。

