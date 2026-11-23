<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 電源配置・トポロジー表現適正化計画書
(ADX Power Topology & Gravity-Safe Documentation Revision Plan)

- **ステータス**: COMPLETED / 適用完了
- **作成日**: 2026-11-03
- **完了日**: 2026-11-03
- **対象ドキュメント**: `docs/spec/`, `docs/ja/`, `docs/en/`, `hardware/CORE-CX5/` 配下全般
- **目的**: 「重力フェイルセーフ (Gravity Fail-Safe)」の過剰・誇大表現を是正し、産業用制御盤の実務原則（弱電上・強電下）および基板前面 I/O 端子との直線的配置（非交差トポロジー）に基づく地に足のついた技術仕様表現へリフレーミングする。

---

## 1. 背景と課題意識 (Context & Problem Statement)

### 1.1 現行ドキュメントの課題：過剰・ドラマチックな表現
現在、リポジトリ内の多くのドキュメントにおいて、ADX 20-Pin バスの Pins 17–20 (`VBUS`) 配置について以下のような説明がなされています：
> *「万一ネジ端子が緩んで電線が外れても、重力によって電線が下へ落下するため、上部にある通信線やアナログ線に触れず短絡を防ぐ（重力フェイルセーフ）」*

この記述は一見キャッチーですが、産業機器・ハードウェア設計の実務観点から見ると以下の問題（無理・斑・無駄）を孕んでいます：
1. **後付けのフィクション感**: IDC フラットケーブルのピン自体が重力で滑落するわけではなく、また「ネジから外れた線が奇跡的に下だけに落ちる」という極めて局所的でドラマチックすぎるシナリオに依存しており、プロのエンジニアが読んだ際に説得力を欠く。
2. **本質の看過**: なぜコネクタ最下段に電源を置くのかという**真の工学的必然性（前面 I/O 端子配置、盤内配線ダクトとの整合、基板内部アートワークの非交差性）**が語られていない。

### 1.2 現場の真実と公理（弱電上部・強電下部）
制御盤・配電盤の設計において、**「弱電（計装・通信・アナログ信号）は上、強電（主電源・動力・大電流）は下」**は、誘導ノイズ防止と配線交差防止のための公理・鉄則です。
- **盤内ダクト**: 上部ダクトに通信・センサー線を通し、下部ダクトに電源・動力線を通す。
- **前面 I/O 端子台**: モジュールの上部に通信端子（RS-485等）を、下部に大電流受電端子（5V/24V端子台等）を配置する。
- **配線の直進性**: この配置により、盤内ダクトからモジュール端子へ電線が最短・ストレートに引き込まれ、**盤内で強電線と弱電線が立体交差するリスクが根本から全廃**されます。

---

## 2. 本質的再定義（リフレーミングの核）

### 2.1 VBUS が最下段（Pins 17–20）に配置されている真の工学的理由

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

1. **前面 I/O 端子配置との完全な直進一致 (Straight-Through I/O Alignment)**:
   - バックプレーン側の 20-Pin IDC 最下段（Pins 17–20）に `VBUS` を置くことで、基板下辺を通って前面最下段の電源受電端子台へ**一直線に太い大電流プレーンを引き通す**ことができます。
2. **基板内クロスオーバーの全廃 (Elimination of On-Board Crossings)**:
   - 大電流プレーンが基板下層に固定されるため、基板上部を走る微弱なアナログ計測ライン（AIN）や高周波通信ライン（SPI/UART）を大電流ラインが横切る（クロスする）必要が一切なく、**基板内部でのスイッチングノイズ放射や IR ドロップ干渉を根本から排除**できます。
3. **盤内配線トポロジーとの調和**:
   - 「IDCケーブルが滑落するのを防ぐ」のではなく、**「このピン配置にすることで、ボード全体の前面・背面・基板内配線すべてが、盤の『弱電上・強電下』原則と完璧に調和する」**点こそが本質です。

### 2.2 用語の適正化（誇大語彙からプロの工学表現へ）
- **「重力フェイルセーフ (Gravity Fail-Safe)」の過剰連呼を廃止**:
  - 単独で過剰にアピールするのをやめ、**「盤内トポロジー適合電源配置 (Panel-Oriented Power Topology)」** または **「非交差型・底面電源クラスタ (Bottom-Heavy Non-Crossing Power Cluster)」** を主たる工学表現とします。
  - 重力による物理的な安全性（外れた線の接触防止）については、「副次的な利点（Secondary Physical Benefit）」として簡潔に触れる程度に留めます。

---

## 3. 修正対象ドキュメント一覧と影響範囲

リポジトリ内の該当ファイルを網羅的に特定し、修正優先度を設定します：

| 対象ファイル | 現状の課題・過剰表現箇所 | 修正方針 | 優先度 |
| :--- | :--- | :--- | :---: |
| **`docs/spec/ADX_PINOUT.md`** | 4.1 節「Gravity Fail-Safe Power Architecture」、表 3 の「Gravity Fail-Safe Cluster」表記 | セクション名を「Panel-Aligned Bottom Power Architecture」等の実務的表現へリフレーミング。盤内ダクトおよび前面 I/O との直線的一致を主理由に書き換える。 | **P0 (最優先)** |
| **`docs/spec/mcu_mapping/ch32x035.md`** | 表 2「Gravity Fail-Safe Main Power Bus」表記 | 「Main High-Current Power Bus (Pins 17–20 $2 \times 2$ Solid Plane)」等の誠実・客観的表記へ改訂。 | **P0 (最優先)** |
| **`docs/spec/mcu_mapping/attiny1616.md`** | 表 2「Gravity Fail-Safe Main Power Bus」表記 | 同上。客観的な大電流電源バス表記へ修正。 | **P1 (重要)** |
| **`docs/ja/ADX_pinout.md`** | 2.1 節「重力フェイルセーフ構造」、導入部の記述 | 盤の配線原則（弱電上・強電下）と前面端子台・基板内アートワークの非交差性（直進性）を主軸とした技術解説へ全面改訂。 | **P0 (最優先)** |
| **`docs/en/ADX_pinout.md`** | 2.1 節「Gravity Fail-Safe Power Placement」 | 上記 JA 版と完全同期させ、盤内トポロジー整合性を前面に出した記述へ改訂。 | **P0 (最優先)** |
| **`docs/ja/README.md`** | 特徴一覧「重力フェイルセーフ電源」表記 | 「盤内配線トポロジー適合電源（最下部 $2 \times 2$ 大電流バス）」等へ適正化。 | **P1 (重要)** |
| **`docs/en/README.md`** | 特徴一覧「Gravity Fail-Safe Power」表記 | 「Panel-Aligned Power Bus (Bottom $2 \times 2$ Cluster)」等へ適正化。 | **P1 (重要)** |
| **`hardware/CORE-CX5/concept.md`** | 基本仕様表「重力フェイルセーフ」表記 | 「ADX 20-Pin Bus v4.1 準拠 (Pins 17–20: VBUS 底面電源クラスタ)」へ更新。 | **P1 (重要)** |
| **`hardware/CORE-CX5/rev1/pinout_study.md`** | 表 5「重力フェイルセーフ給電クラスタ」表記 | 「底面大電流給電クラスタ ($2 \times 2$ ベタ銅箔プレーン)」へ更新。 | **P1 (重要)** |
| **`hardware/CORE-CX5/rev1/README.md`** | 概要「重力フェイルセーフ」表記 | 「底面電源クラスタ」へ適正化。 | **P1 (重要)** |

---

## 4. 具体的な修正案（Before / After サンプル）

### 4.1 `docs/spec/ADX_PINOUT.md` (SSOT)

#### Before (現行)
```markdown
### 4.1 Gravity Fail-Safe Power Architecture (`VBUS`: Pins 17–20)
- **Bottom-Heavy Physical Placement**:  
  Pins 17, 18, 19, and 20 are positioned at the lowest physical row of the connector. When modules are slotted vertically (e.g., in DIN-rail enclosures or vertical chassis), high-current and external power wiring remains at the bottom of the assembly. If an external supply wire disconnects due to mechanical vibration or screw-terminal loosening, gravity pulls the conductor downward toward the chassis floor or wiring duct, physically preventing unintended contact with delicate communication lines (RS-485, I2C, SPI) or analog inputs located higher on the assembly.
```

#### After (改訂案)
```markdown
### 4.1 Panel-Aligned Power Bus Architecture (`VBUS`: Pins 17–20)
- **Topology & Enclosure Alignment (Weak-Signal Top, Heavy-Power Bottom)**:  
  In industrial control panels and vertical DIN-rail enclosures, routing standards mandate physical segregation between high-current power wiring (bottom ducts) and sensitive instrumentation/communication signals (top ducts). Pins 17, 18, 19, and 20 are dedicated to `VBUS` at the lowest physical row of the connector to mirror this architecture:
  1. **Direct Straight-Through Routing**: Allows high-current power planes to run straight along the bottom edge of the PCB directly into front-panel power terminals, without traversing or coupling noise into low-voltage signals above.
  2. **Zero Signal Crossings**: Eliminates magnetic and capacitive crosstalk into sensitive analog (`AIN`) and high-speed communication pairs (`UART`, `SPI`) across both the backplane and module boards.
  3. **Secondary Gravity Protection**: As an inherent physical side-effect of this bottom placement, should an external terminal connection come loose in vertical mounts, the loose conductor hangs or drops downward into the lower wiring duct, preventing accidental contact with communication lines routed above.
```

---

### 4.2 `docs/ja/ADX_pinout.md` (解説ガイド)

#### Before (現行)
```markdown
### 2.1 重力フェイルセーフ構造 (`VBUS`: Pins 17〜20)
Pins 17〜20 に大電流幹線（`VBUS`）を配置したのは、現場での物理的な安全性を極限まで高めるためです。
制御盤や縦型スロットにモジュールを挿入した際、電源端子は常にアセンブリの最下部に位置します。万一、振動やネジの緩みによって強電側の給電線が端子台から脱落しても、重力によって電線は下方（ダクトや盤底面）へ落下します。上方に配置された微弱なアナログ信号線や通信線（RS-485, I2C, SPI）に接触することは構造上あり得ません。
```

#### After (改訂案)
```markdown
### 2.1 産業盤トポロジー適合電源配置 (`VBUS`: Pins 17〜20)
Pins 17〜20 に大電流幹線（`VBUS`）を集中配置したのは、制御盤・産業機器の鉄則である**「弱電は上、強電は下」**という配線公理に、基板内部および前面端子配置を完全に一致させるためです。

1. **前面 I/O 端子台との直進ルーティング**:
   盤内配線ダクトは「上部＝通信・計装」「下部＝電源・動力」で分離されています。20-Pin IDC の最下段に `VBUS` を置くことで、基板下辺のベタ銅箔パターンを通じて、前面最下部の電源端子台へ一直線に最短配線できます。
2. **基板内クロスオーバー（交差）の全廃**:
   大電流ラインが基板の下半分で完結するため、基板上部を走る微弱なアナログ計測（AIN）や高速通信（SPI/UART）のパターンを大電流プレーンが横切る必要が一切ありません。大電流スイッチングノイズや IR ドロップによる通信妨害を根本から防ぎます。
3. **副次的な重力安全性の確保**:
   この底面配置の副次的効果として、縦型スロット配置時に万一ネジ端子台から給電線が緩んで外れた場合でも、電線は自然と下部の配線ダクト側へ垂れ下がるため、上方の通信端子台へ接触・混触する事故が構造的に防止されます。
```

---

## 5. 実行ステップとスケジュール (Action Plan)

本改訂は以下の 3 フェーズで順次実施します：

### フェーズ 1: SSOT および MCU 実装仕様の改訂 (P0)
- `docs/spec/ADX_PINOUT.md` (SSOT) の 4.1 節およびテーブル表記の修正。
- `docs/spec/mcu_mapping/ch32x035.md` および `attiny1616.md` のテーブル表記修正。

### フェーズ 2: ユーザー向け解説ガイドおよびポータルの改訂 (P0〜P1)
- `docs/ja/ADX_pinout.md` および `docs/en/ADX_pinout.md` の 2.1 節の全面書き換え。
- `docs/ja/README.md` および `docs/en/README.md` の概要・特徴リストの文言適正化。

### フェーズ 3: ハードウェア開発検討書の改訂 (P1)
- `hardware/CORE-CX5/concept.md`、`hardware/CORE-CX5/rev1/pinout_study.md`、`README.md` の表記更新。

---

## 6. 完了条件 (Definition of Done)

- [x] リポジトリ内の主要仕様書・解説書から、誇大・ドラマチックな「重力フェイルセーフ」単独連呼が排除されていること。
- [x] すべてのドキュメントにおいて、「盤内配線公理（弱電上・強電下）」「前面 I/O 端子台との直進的一致」「基板内クロスオーバー全廃」が主たる工学的理由として明記されていること。
- [x] 英語 SSOT と日本語解説ガイドの間で技術的論理のブレがないこと。

