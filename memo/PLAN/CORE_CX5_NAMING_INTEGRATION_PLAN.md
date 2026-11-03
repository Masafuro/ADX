<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX CORE-CX5 名称統合・移行計画書
(ADX CORE-CX5 Product Naming Migration & Integration Plan)

- **文書ID**: PLAN-ADX-CORE-CX5-MIGRATION-001
- **ステータス**: DRAFT / 承認待ち
- **作成日**: 2026-11-03
- **対象ディレクトリ**: `hardware/CORE-CX3/` ➔ `hardware/CORE-CX5/` およびリポジトリ全域
- **目的**: かつての CH32V003 前提の仮称「CORE-CX3」から、現行採用 MCU である CH32X035 (CH32VX035) に適合した正式名称 **「ADX CORE-CX5」** へのディレクトリ名・ファイル名・ドキュメント記述の完全統一。

---

## 1. 背景と改名理由 (Background & Rationale)

### 1.1 「CORE-CX3」という名称の歴史的経緯
- ADX プロジェクトにおいて、超低価格 USB コントローラ構想（旧 CORE-U）の模索中、一時的に WCH 社の 10 セントマイコン **CH32V003**（QFN-20 / TSSOP-20, RISC-V2A, 16KB Flash / 2KB RAM）が検討対象となった経緯があります。
- 当時の検討コードネームとして、CH32V0**03** に由来する **「CORE-CX3」** という呼称が仮付けされました。

### 1.2 本日（2026-11-03）到達した真実と「CORE-CX5」への必然性
- しかし、CH32V003 はハードウェア USB PHY を搭載しておらず、スペック的にも ATtiny1616 と同等（16KB/2KB）であったため、100pcs/1lot の小ロット生産において「USB 2チップ構成の BOM 高騰」「リソース枯渇」の壁を打破できませんでした。
- 本日設計が完了した rev1 ハードウェアは、WCH 社の最新マイコン **`CH32X035G8U6`（CH32VX035）** を採用しています：
  - **内蔵 USB 2.0 Full-Speed PHY**: USB ブリッジ IC および外付けクリスタルを全廃し、1チップで USB 完結。
  - **スペックの桁違いの飛躍**: 32-bit RISC-V4C (48MHz)、Flash 62KB（約4倍）、SRAM 20KB（10倍）。
  - **ADX Bus v4.1 適合**: ポゴピンレス 2-wire SDI 書込（Pins 2 & 5）。
- したがって、ADX ファミリの命名規則（型番末尾・コアアーキテクチャの反映）に則り、**CH32X035 の「035」を冠した正式プロダクト名称は `ADX CORE-CX5` であるべき** です。

```text
┌────────────────────────────────────────────────────────────────────────┐
│                   【 MCU とプロダクト名称の対応関係 】                  │
├────────────────────────────────────────────────────────────────────────┤
│ 旧構想 (破棄) : CH32V003  (USBなし / 16KB Flash)  ➔ 仮称: CORE-CX3     │
│ 現行設計 (確定): CH32X035 (USB内蔵 / 62KB Flash) ➔ 正式: ADX CORE-CX5   │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 移行対象とスコープ (Scope of Changes)

### 2.1 ディレクトリおよびファイル名の変更 (Git Rename)

| 現行パス (Before) | 変更後パス (After) | 備考 |
| :--- | :--- | :--- |
| `hardware/CORE-CX3/` | **`hardware/CORE-CX5/`** | ディレクトリ名称変更 |
| `hardware/CORE-CX3/rev1/adx_core-cx3_rev1-wip.enet` | **`hardware/CORE-CX5/rev1/adx_core-cx5_rev1-wip.enet`** | EasyEDA Pro ネットリスト |
| `docs/blog/images/adx_core-cx3_rev1_WIP.png` | **`docs/blog/images/adx_core-cx5_rev1_WIP.png`** | ブログ用 CAD 画像 |

### 2.2 ドキュメント内テキストおよびリンクの更新

以下のファイル群において、`CORE-CX3` / `core-cx3` 表記を `CORE-CX5` / `core-cx5` へ更新します：

1. **ハードウェア設計ドキュメント (`hardware/CORE-CX5/`)**:
   - `hardware/CORE-CX5/concept.md`
   - `hardware/CORE-CX5/rev1/README.md`
   - `hardware/CORE-CX5/rev1/netlist_review.md`
   - `hardware/CORE-CX5/rev1/pinout_study.md`
2. **規格およびMCUマッピング仕様 (`docs/spec/`)**:
   - `docs/spec/ADX_PINOUT.md` (Table 4 など)
   - `docs/spec/mcu_mapping/ch32x035.md`
3. **多言語ドキュメント・README (`docs/` / ルート)**:
   - `README.md`
   - `docs/ja/README.md`
   - `docs/en/README.md`
   - `docs/ja/ADX_pinout.md`
   - `docs/en/ADX_pinout.md`
   - `hardware/README.md`
4. **設計ノート・レビューメモ (`memo/` / `docs/blog/memo/`)**:
   - `memo/note/what_is_usb_one_cable_debug.md`
   - `memo/ADX_TONE_AND_MANNER.md`
   - `docs/blog/memo/2026-11-03_dev_review.md`
   - `memo/PLAN/GRAVITY_FAILSAFE_EXPRESSION_REVISION_PLAN.md`（履歴メモ）
   - `memo/PLAN/ADX_REPOSITORY_DOCUMENTATION_REORGANIZATION_PLAN.md`（履歴メモ）
   - `memo/REPORT/ADX_PINOUT_V4_SPECIFICATION_REPORT.md`（履歴メモ）

---

## 3. 段階的移行ステップ (Step-by-Step Execution Plan)

### ステップ 1: ディレクトリおよびファイルのリネーム (`git mv`)
```bash
# 1. ハードウェアディレクトリの移動
git mv hardware/CORE-CX3 hardware/CORE-CX5

# 2. ネットリストファイルのリネーム
git mv hardware/CORE-CX5/rev1/adx_core-cx3_rev1-wip.enet \
       hardware/CORE-CX5/rev1/adx_core-cx5_rev1-wip.enet

# 3. 画像ファイルのリネーム
git mv docs/blog/images/adx_core-cx3_rev1_WIP.png \
       docs/blog/images/adx_core-cx5_rev1_WIP.png
```

### ステップ 2: `hardware/CORE-CX5/` 配下ドキュメントのテキスト置換
- `concept.md`, `README.md`, `netlist_review.md`, `pinout_study.md` 内の：
  - `CORE-CX3` ➔ `CORE-CX5`
  - `core-cx3` ➔ `core-cx5`
  - 内部リンクパス（`hardware/CORE-CX3/` ➔ `hardware/CORE-CX5/`）の修正。
  - ネットリストファイル内の Design Title / Board Name の整合。

### ステップ 3: 共通仕様書および MCU マッピングの置換
- `docs/spec/ADX_PINOUT.md`: コントローラ対応表（Table 4）の `CORE-CX3` ➔ `CORE-CX5`
- `docs/spec/mcu_mapping/ch32x035.md`: ドキュメントタイトル、対象ボード表記の更新。

### ステップ 4: ルートおよび多言語ガイドの更新
- `README.md`, `docs/en/README.md`, `docs/ja/README.md`, `hardware/README.md`
- `docs/en/ADX_pinout.md`, `docs/ja/ADX_pinout.md`
- リンクパスと表内表記の同期。

### ステップ 5: メモ・開発レビューの更新
- `memo/note/what_is_usb_one_cable_debug.md`
- `docs/blog/memo/2026-11-03_dev_review.md`

### ステップ 6: 整合性検証 (Verification)
- `git grep -i "core-cx3"` を実行し、過去の改名履歴説明（本計画書等）を除き、意図しない古いリンクや呼称が残っていないことを確認。
- マークダウンリンク切れがないことを確認。

---

## 4. 完了基準 (Definition of Done: DoD)

- [ ] `hardware/CORE-CX5/` ディレクトリが存在し、旧 `hardware/CORE-CX3/` が削除されていること。
- [ ] ネットリストおよび画像ファイルが `adx_core-cx5_rev1-wip` にリネームされていること。
- [ ] 全ドキュメント上のボード呼称が **「ADX CORE-CX5」** で統一されていること。
- [ ] リンク切れおよび git ステータスに不整合がないこと。

