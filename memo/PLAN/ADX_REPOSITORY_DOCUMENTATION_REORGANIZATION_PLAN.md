<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX リポジトリ ドキュメント再整備計画書
(ADX Repository Documentation Reorganization Plan)
**〜 `ADX_TONE_AND_MANNER.md` および `ADX-BUS-SPEC-v4.0` に基づく包括的改修ロードマップ 〜**

- **文書ID**: PLAN-ADX-DOCS-REORG-001
- **策定日**: 2026-11-03
- **ステータス**: APPROVED / 実行計画 (Execution Plan)
- **基準規範**:
  - [`memo/ADX_TONE_AND_MANNER.md`](../ADX_TONE_AND_MANNER.md)（プロジェクト公式トーン＆マナー）
  - [`memo/REPORT/ADX_PINOUT_V4_SPECIFICATION_REPORT.md`](../REPORT/ADX_PINOUT_V4_SPECIFICATION_REPORT.md)（Pinout v4.0 策定仕様）

---

## 1. 計画の背景と目的 (Background & Objectives)

### 1.1 背景
ADX プロジェクトは、CORE-A5（高耐環境モデル）の量産準備、CORE-CX5（CH32X035 採用低価格モデル）の新規構想、および現場の物理安全性を極限まで高めた **「ADX-BUS-SPEC-v4.0（重力フェイルセーフ ＆ 1-Wire デバッグ）」** の策定という重要な節目を迎えました。

一方で、リポジトリ内のドキュメント群には以下の課題が顕在化していました：
1. **仕様ドキュメント（SSOT）と解説ガイドの密結合・二重管理（Drift）**:
   - `docs/ja/ADX_pinout.md` や `docs/en/ADX_pinout.md` にピン配置一覧表がベタ書きされており、仕様改訂時に更新が漏れ、SSOT との間に重大な乖離が発生していた。
2. **過剰・誇大表現の残存**:
   - 一部ドキュメントに「鉄壁」「完全保護」「自己発熱ゼロ」「絶対」などの誇張・非物理的表現が散見されていた。
3. **新規構想（CORE-CX5 / v4.0）の未反映**:
   - `hardware/README.md` やハードウェア構想書に、今回の決定事項がまだ反映されていない。

### 1.2 目的
[`memo/ADX_TONE_AND_MANNER.md`](../ADX_TONE_AND_MANNER.md) で定義された**「仕様と説明の疎結合」「誠実な工学主義」「明快な動線」**の規律に基づき、リポジトリ全域のドキュメントを体系的に再整備します。これにより、**「将来どんな仕様変更があっても、修正箇所が世界でたった1箇所に集約される堅牢なドキュメンテーション構造」**を確立します。

---

## 2. 再整備の基本原則 (Architecture Principles)

```text
┌────────────────────────────────────────────────────────────────────────┐
│                      【 ドキュメント再整備 3大原則 】                  │
│                                                                        │
│ 1. 【 仕様 (SSOT) の英語統一 ＆ 1箇所集約 】                           │
│    規格マスター（docs/spec/）は英語で統一。ピン配置・定格・数値の変更  │
│    が発生した場合、修正するファイルは「該当する英語SSOT 1つ」のみ。    │
│                                                                        │
│ 2. 【 説明ガイドの疎結合化 (No Redundant Tables) 】                    │
│    解説ガイド（docs/ja/, docs/en/）からピン配置表や定格数値を全廃。    │
│    「なぜその設計なのか（Rationale）」の思想解説と SSOT へのリンクに特化│
│                                                                        │
│ 3. 【 誠実な工学表現への全面改訂 (DO/DON'T 遵守) 】                    │
│    「絶対」「完全」「鉄壁」等の誇張・威圧表現を排除し、物理限界と      │
│    トレードオフを誠実に語る言葉へ全面的にリファクタリング。            │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 3. ドキュメント監査マトリクス (Current State Audit)

| 対象ファイル | 現状の課題 | 再整備アクション | 優先度 |
| :--- | :--- | :--- | :---: |
| **`docs/spec/ADX_PINOUT.md`** | v3.0（旧配置）のまま | **v4.0（Gravity-Safe, OWD, AINコプレーナ）へ全面刷新**。英語SSOTとして完成させる。 | **P0 (最優先)** |
| **`docs/spec/mcu_mapping/attiny1616.md`** | 旧ピン配置マッピング | v4.0 に合わせた PORTMUX・ピンアサインへ更新（Pin 2=PA0/UPDI）。 | **P1** |
| **`docs/spec/mcu_mapping/ch32x035.md`** | 未作成 | **新規策定**。CORE-CX5（CH32X035）の v4.0 マッピングを英語で記述。 | **P1** |
| **`docs/ja/ADX_pinout.md`** | 旧v2表のベタ書き、SSOTと乖離、誇大語彙 | **ピン配置表を全廃（SSOTリンクへ委譲）**。v4.0の重力安全・コプレーナ保護の設計思想解説に特化。 | **P0 (最優先)** |
| **`docs/en/ADX_pinout.md`** | 上記日本語版と同様の課題 | 日本語版と 1:1 で同期。表を全廃し、設計思想解説と SSOT リンクに特化。 | **P0 (最優先)** |
| **`hardware/CORE-CX5/concept.md`** | 初期メモ状態 | 通信16P/電源6PのデュアルType-C、UF2ブートローダー、Pin 2=SDI、v4.0バス準拠を反映。 | **P1** |
| **`hardware/CORE-A5/concept.md`**<br>`hardware/CORE-A5/spec_study.md` | 一部旧ピンアサイン記述 | v4.0 バス準拠（Pin 2=PA0/UPDI, Pins 17〜20=VBUS）の整合性確認・注記。 | **P2** |
| **`hardware/README.md`** | ラインナップ表に CORE-CX5 未反映 | Coreシリーズ一覧に CORE-CX5 を追加。v4.0 バス規格へのリンクを最新化。 | **P2** |
| **`README.md` / `docs/ja/README.md`** | 全体ポータル概要 | ADX Pinout v4.0 の概要、3大イノベーションの要約、クイックナビゲーション更新。 | **P2** |
| **`docs/memo/adx_pinout_v2.md`** 等 | 過去の古いドラフトが放置 | `archive/` へ退避または削除し、読者の混乱を防止。 | **P3** |

---

## 4. フェーズ別 実行計画 (Phased Execution Plan)

### フェーズ 1: 規格マスター（SSOT）の v4.0 策定 ＆ 英語統一 【P0】
- **作業内容**:
  1. [`docs/spec/ADX_PINOUT.md`](../../docs/spec/ADX_PINOUT.md) を `ADX-BUS-SPEC-v4.0` として全面改訂。
     - Pins 17〜20: `VBUS`（重力フェイルセーフ）
     - Pin 1: `GND`（最外端ガード）
     - Pin 2: `OWD`（One-Wire Debug / Factory Programming）
     - Pin 3: `AIN`（コプレーナ・シールド保護）
     - Section 4.1〜4.5 の電気定格・設計ルール（PoL、短絡防止壁 Pin 16）を最新化。
  2. [`docs/spec/mcu_mapping/attiny1616.md`](../../docs/spec/mcu_mapping/attiny1616.md) の v4.0 対応更新。
  3. `docs/spec/mcu_mapping/ch32x035.md` を英語で新規作成（CORE-CX5 用マッピング）。

### フェーズ 2: 仕様解説ガイドの疎結合化とトンマナ適合 【P0】
- **作業内容**:
  1. [`docs/ja/ADX_pinout.md`](../../docs/ja/ADX_pinout.md) のリファクタリング:
     - 冒頭に SSOT（[`docs/spec/ADX_PINOUT.md`](../../docs/spec/ADX_PINOUT.md)）への誘導バナーを配置。
     - **重複していたピン配置表を完全撤去**。
     - 「なぜ Pins 17〜20 が VBUS なのか（現場の重力落線事故の防止）」「なぜ Pin 1 が GND で Pin 3 が AIN なのか（外周ガードとコプレーナ保護）」「なぜ Pin 2 が OWD なのか（検査治具と現場長期保守）」という**設計意図（Rationale）の解説に全集中**。
     - 「鉄壁」「完全」等の誇張語彙を「信頼性の高い境界」「物理的遮断」等へ置換。
  2. [`docs/en/ADX_pinout.md`](../../docs/en/ADX_pinout.md) を日本語版と完全同期改訂。

### フェーズ 3: ハードウェア構想書および総合 README の同期 【P1】
- **作業内容**:
  1. [`hardware/CORE-CX5/concept.md`](../../hardware/CORE-CX5/concept.md) の正式アップデート:
     - MCU: CH32X035 (Flash 62KB / SRAM 20KB / 48MHz RISC-V)
     - デュアル Type-C: 通信専用 16P ＋ 電源専用 6P（逆流防止SBD ＋ PTC保護）
     - 保守思想: 出荷検査治具（IDC ヘッダー Pin 2: SDI）での UF2 自動書き込み ➔ 現場は OS 非依存のドラッグ＆ドロップ保守
     - ADX 20P Bus v4.0 完全準拠
     - 荒見積もり（100pcs: $2.69 ➔ 想定売価 $10.76）の明記
  2. [`hardware/README.md`](../../hardware/README.md) の更新:
     - Core シリーズの表に **ADX CORE-CX5**（Entry / Flagship USB、CH32X035）を正式追加。
     - バス規格への参照を v4.0 に更新。
  3. [`hardware/CORE-A5/concept.md`](../../hardware/CORE-A5/concept.md) / `spec_study.md` のピン整合注記。

### フェーズ 4: ルートポータル・ナビゲーションの更新 【P2】
- **作業内容**:
  1. ルート [`README.md`](../../README.md)、[`docs/ja/README.md`](../../docs/ja/README.md)、[`docs/en/README.md`](../../docs/en/README.md) の概要文を更新。
  2. v4.0 の 3大革新（重力安全、コプレーナ保護、OWD）のハイライトを簡潔に紹介し、SSOT および解説ガイドへの導線を整備。

### フェーズ 5: 旧作業メモの整理・アーカイブ 【P3】
- **作業内容**:
  1. `docs/memo/adx_pinout_v2.md` や `memo/architecture/ADX_pinout.md` など、更新が停止している旧ドラフトを `archive/` ディレクトリへ移動または整理。

---

## 5. 完了基準 (Definition of Done: DoD)

本再整備計画の完了判定は、以下のチェックリストがすべて満たされた時点とします。

- [ ] **DoD-1: 修正箇所1箇所の達成**: ピンアサインの定義表が存在するのは [`docs/spec/ADX_PINOUT.md`](../../docs/spec/ADX_PINOUT.md)（英語）のみであり、他の全ドキュメントはそこへのリンクとなっていること。
- [ ] **DoD-2: v4.0 仕様の完全反映**: Pins 17〜20 (VBUS), Pin 1 (GND), Pin 2 (OWD), Pin 3 (AIN) が SSOT に明記され、電気的要件が整合していること。
- [ ] **DoD-3: 疎結合解説ガイドの完成**: `docs/ja/ADX_pinout.md` および `docs/en/ADX_pinout.md` が、冗長な表を持たず、重力フェイルセーフ等の設計思想を深く解説していること。
- [ ] **DoD-4: トーン＆マナーの遵守**: `memo/ADX_TONE_AND_MANNER.md` の DO/DON'T 表に違反する表現（鉄壁、完全、絶対、自己発熱ゼロ等）がリポジトリ主要ドキュメントから一掃されていること。
- [ ] **DoD-5: CORE-CX5 構想の FIX**: `hardware/CORE-CX5/concept.md` にデュアルType-C、UF2、SDI、v4.0バス、見積もりが漏れなく文書化されていること。
- [ ] **DoD-6: ナビゲーション整合性**: リンク切れがなく、初学者が README から目的の情報へ迷わず到達できる動線が確保されていること。

