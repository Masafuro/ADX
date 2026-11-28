<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX 20-Pin Bus `VBUS` 電圧仕様再策定 ＆ ドキュメント改訂計画書

- **作成日**: 2026-11-28
- **版数**: **v1.3（Track 1〜3 仕様整理完了反映）**
- **ステータス**: **全 3 トラック仕様整理完了（Track 1 規格文書 15 件・Track 2 CORE-U rev6 MUST 要件整理＆過電圧検討メモ分離・Track 3 Reference CARD v1 仕様策定：2026-11-28）**
- **規格バージョン方針**: **`ADX-BUS-SPEC-v5.0` へ統合（未製造・未コミット段階の `v5.0` 初版へ正式織り込み）**
- **改訂スコープ**:
  - **本計画の対象（Phase 1〜4）**: バス規格 SSOT・日英ガイド・フォームファクタ・開発ガイドライン・ポータル・公開サイト（`site/`）のドキュメント改訂のみ（CAD / ガーバー / ネットリスト等の製造データ修正は対象外）
  - **後続連動トラック（別タスクとして明確分離）**:
    1. **CORE-U rev6 電源保護・LDO 部品再選定**（`TLV75533` / `SMF5.0CA` / OVLO の `5.5V` 対応見直し）
    2. **Reference CARD v1 改訂**（`ADX-BUS v5.0` ＋ `4.5V~5.5V` シルク ＋ THT Eject Header 標準化 ＋ 既存マーカー `100010`/`100011` 継承）
- **関連計画書**: [ADX 20-Pin Bus Specification v5.0 ドキュメント改訂計画書](ADX_BUS_SPEC_V5_REVISION_PLAN.md)（ピンアウト改訂：実施済）
- **規格 SSOT**: [docs/spec/ADX_PINOUT.md](../../docs/spec/ADX_PINOUT.md)

---

## 1. 背景と改訂の目的（Background & Objective）

`Reference CARD v1` の改訂検討（`ADX-BUS-SPEC-v5.0` 対応および Eject Header 標準化）を進める過程で、ADX 共通拡張バスの主電源幹線 **`VBUS`（Pins 17–20）の電圧範囲仕様** を根本から再レビューした結果、以下の 2 つの構造的課題が顕在化した。

### 1.1 リポジトリ内の既存表記の不整合（3 パターン並存）
現行アクティブ文書間で、`VBUS` の電圧仕様が以下の 3 通りに分裂していた：

| 現行表記 | 該当ドキュメント | 発生していた問題 |
| :--- | :--- | :--- |
| **`4.5V – 5.25V`** | `docs/spec/ADX_PINOUT.md`, `mcu_mapping/*.md`, `README.md`, `site/`, `CORE-U rev6 spec.md` | 上限 `5.25V` が現代の USB Type-C `vSafe5V` 規格上限（`5.50V`）を下回る |
| **`4.75V 〜 5.25V`** | `docs/ja/8858_formfactor.md`, `docs/en/8858_formfactor.md`, `Reference CARD v0` | 旧 USB 2.0 由来。2A 給電時のケーブル・保護素子降下（`~4.5V`）も USB-C 上限（`5.50V`）もカバーできない |
| **`DC 5.0V (±10%)（4.5V〜5.0V 動作、推奨 4.75V〜5.25V）`** | `docs/ja/adx_formfactor_guidelines.md`, `docs/en/adx_formfactor_guidelines.md` | `±10%` と記しながら動作上限が `5.0V` となっており文内で自己矛盾 |

### 1.2 現実世界の 5V 電源環境と LDO 過渡耐圧リスク
1. **USB Type-C (`vSafe5V`) との整合**:
   - USB Type-C の `vSafe5V` 規格は **`4.75V 〜 5.50V`** を許容しており、市販の USB-PD 充電器やモバイルバッテリーが無負荷時に `5.1V〜5.4V`（最大 `5.50V`）を出力することは完全に規格準拠である。
   - 同様に、産業用 5V 電源や絶縁 DC-DC モジュール（`B0505S-1WR3` 等）の多くも `5V ±10%`（`4.5V 〜 5.5V`）で設計されている。
   - ADX バス仕様の上限を `5.25V` に留めると、**規格準拠の市販 USB-C 電源を `USB-POW` へ接続しただけで ADX 仕様外となる矛盾** が生じる。
2. **「受電側の耐性（MUST）」と「LDO 絶対最大定格 6.0V の落とし穴」**:
   - MCU 自体は PoL（Point of Load）原則により 3.3V レギュレータ後段に隔離されているため、`VBUS = 5.5V` でも直接破損しない。
   - 一方、LDO については `TI TLV75533` / `MIC5504` / `XC6206` 等の **`6.0V` 絶対最大定格品** を採用すると、`5.5V` 定常時の残マージンが **わずか `0.5V`** しかなく、USB ケーブルや IDC リボンケーブルの活線挿抜・2A 負荷急変時の LC リンギング（`+0.5V〜+1.5V`）で容易に絶対最大定格を突破するリスクがある。
   - また `5.0V → 5.5V` で LDO 発熱 $P_D = (V_{IN} - 3.3\text{V}) \times I_{OUT}$ は **約 +29% 増加** するため、大電流 3.3V CARD での降圧 DC-DC 推奨や、狭レンジ 5V IC（`4.75–5.25V` 品）の直結禁止を規格として明文化する必要がある。

---

## 2. 再策定する `VBUS` 電圧仕様（`ADX-BUS-SPEC-v5.0` 正式統合版）

RFC 2119（`MUST` / `SHOULD` / `MAY`）に基づき、**「全ボードが耐えるべきバス許容範囲（MUST）」** と **「電源供給側の推奨精度（SHOULD）」**、および **「PoL レギュレータ・狭レンジ部品の設計規則」** を以下の 4 条項として確定する。

### 2.1 新 `VBUS` 電圧規格 4 条項

| 条項 | 要件レベル | 規定内容 | 工学的根拠・補足 |
| :--- | :---: | :--- | :--- |
| **条項 1：バス許容動作電圧範囲（全モジュール耐性）** | **【MUST】** | **`4.5 V 〜 5.5 V DC`**<br>（公称 `5.0 V ±10%`） | **すべての ADX Core ボードおよび拡張 CARD は、`VBUS` が `4.5 V 〜 5.5 V` の全範囲にあるとき、破損・誤動作・保護誤作動なく安全かつ正常に動作するよう設計しなければならない。** USB-C `vSafe5V`（〜`5.50 V`）と 2A 給電時の電圧降下（〜`4.5 V`）を両端カバーする。 |
| **条項 2：電源供給側の出力電圧範囲（Core / Power CARD）** | **【MUST】範囲**<br>**【SHOULD】精度** | ・**許容出力範囲【MUST】**: `4.5 V 〜 5.5 V DC`<br>・**推奨出力精度【SHOULD】**: **`4.75 V 〜 5.25 V DC`**（`5.0 V ±5%`） | 24V→5V 絶縁 Power CARD 等の自前給電モジュールは `5.0 V ±5%` を狙うことで、配線ドロップ後でも `4.5 V` 以上・軽負荷浮きでも `5.5 V` 以下を維持しやすくする。一方、USB-C 電源スルー給電（最大 `5.5 V`）も規格内として正式許容する。過電圧保護（OVLO）は `5.5 V` 定常で誤遮断せず、異常過電圧を確実に遮断する設計とする。 |
| **条項 3：PoL レギュレータ耐圧 ＆ 過渡マージン指針** | **【SHOULD】** | `VBUS` に直結する 3.3V 生成用 LDO / DC-DC および保護素子は、活線挿抜リンギング・負荷急変スパイクに備え、**絶対最大定格 `6.0 V` クラスの部品を避け、動作入力耐圧 `≥ 10 V`（推奨 `12 V 〜 30 V` クラス）を選定** する。 | `5.5 V` 定常＋活線スパイクに対する十分な安全余裕を確保し、TVS クランプ電圧（例：`SMF6.0CA` の `10.3 V`）との保護協調を成立させる。また `5.5 V` 時の LDO 発熱増（`+29%`）に留意し、3.3V 系で目安 `150〜200 mA` 超を消費する CARD は降圧 DC-DC を推奨する。 |
| **条項 4：狭レンジ 5V 部品（`4.75–5.25 V` 品）の直結禁止** | **【MUST】** | `4.75 V 〜 5.25 V` 等、`4.5–5.5 V` 全域に耐えられない 5V 駆動 IC・絶縁モジュール等を CARD に搭載する場合は、**`VBUS` へ直結せず、CARD 内で昇降圧または電圧安定化回路を挟んで `4.5–5.5 V` 入力を吸収すること**（または `3.15–5.5 V` 等の広入力対応品を選定）。 | 任意の給電元（USB-C 充電器 / Power CARD）と任意の拡張 CARD を組み合わせても破損しない相互運用契約を保証する。 |

### 2.2 基板シルクおよび簡略表記の統一ルール

| 文脈・媒体 | 推奨統一表記 | 廃止する旧表記 |
| :--- | :--- | :--- |
| **規格表・仕様書（フル表記）** | `4.5V – 5.5V DC (5.0V ±10% MUST; 4.75–5.25V rec. supply)` | `4.5V – 5.25V DC` / `4.5V〜5.0V` |
| **サマリー表・ポータル（簡略表記）** | `4.5–5.5V`（または `4.5V〜5.5V` / `5V ±10%`） | `4.5–5.25V` / `4.75–5.25V` |
| **PCB シルク（Reference CARD v1 等）** | **`4.5V~5.5V`**（または **`5V (4.5~5.5V)`**） | `4.75V~5.25V` |

---

## 3. 差し替えタイプ分類（Change Type Taxonomy）

今回の改訂は単純な一括置換ではなく、ファイルごとに変更の性質が異なるため、以下 **5 つの差し替えタイプ（Type A〜E）** に厳密分類して管理する。

| タイプ | 分類名 | 変更の性質と作業上の注意点 |
| :---: | :--- | :--- |
| **Type A** | **規格新設・条項改訂**<br>*(Normative Spec Rewrite)* | RFC 2119 の `MUST`/`SHOULD` 条項、LDO 過渡耐圧指針、狭レンジ 5V 部品ルール、開発者セルフチェックリスト項目を新規起草・改訂する。日英の規範レベル（`MUST`/`SHOULD`）を 100% 一致させる必要がある。 |
| **Type B** | **既存矛盾の解消・定義整理**<br>*(Contradiction Fix)* | 既存の自己矛盾（`±10%` かつ `4.5–5.0V`）や旧 USB 2.0 表記（`4.75–5.25V` のみ）を、「受電許容 `4.5–5.5V`【MUST】」と「供給推奨 `4.75–5.25V`【SHOULD】」の二層定義へ整理・修正する。 |
| **Type C** | **数値・要約テキスト同期**<br>*(Summary / Table Sync)* | マッピング表やポータル・Starlight Web サイトの要約行にある `4.5–5.25V` を `4.5–5.5V` へ同期し、必要に応じて `5V ±10%` / `MUST` 要約を補足する。 |
| **Type D** | **ハードウェア部品再選定連動**<br>*(HW Re-Selection Gated)* | **`CORE-U rev6` 固有の扱い**。文書上の `4.5–5.25V` だけ書き換えると、現行 BOM の `TLV75533`（絶対最大 `6.0V`）や `SMF5.0CA`（$V_{RWM}=5.0\text{V}$）と **仕様矛盾（虚偽仕様）** が起きる。したがって本計画では「要再選定」の連動管理にとどめ、部品再選定トラック完了後に `spec.md` / `deep_sleep_potential.md` を正式同期する。 |
| **Type E** | **履歴保全・次期ハードウェアへ引継**<br>*(Frozen Legacy / Handoff)* | 既存の `v4.2` / 旧世代文書・`v0` ガーバー・過去ブログ・アーカイブは一切改変しない。`Reference CARD v1` の仕様策定時に本規格（`4.5V~5.5V`）を引き継ぐ。 |

---

## 4. 全リポジトリ影響範囲マトリクス（Detailed Impact Inventory）

全ファイル grep 監査に基づき、対象ファイル・該当行・差し替えタイプ・具体的変更内容を以下に網羅する。

### Tier 1：規格 SSOT ＆ MCU マッピング（3 ファイル）

| ファイルパス | 該当箇所 | タイプ | 具体的改訂内容 |
| :--- | :--- | :---: | :--- |
| [`docs/spec/ADX_PINOUT.md`](../../docs/spec/ADX_PINOUT.md) | ・§3 ピン表 (L76–79)<br>・§4.1 電源仕様 (L91–98)<br>・§4.5 PoL 規定 (L134)<br>・§5.5 シルク指針 (L190)<br>・§8 改訂履歴 (L219) | **Type A** | ・Pins 17–20 電気規格：`4.5V – 5.25V DC` → **`4.5V – 5.5V DC (5.0V ±10%)`**<br>・§4.1 に **4 条項を正式規定**：① 全 Core/CARD の `4.5V–5.5V` 動作耐性【MUST】、② 給電側出力範囲 `4.5–5.5V`【MUST】／推奨精度 `4.75–5.25V`【SHOULD】＆ OVLO 非誤作動要件、③ `VBUS` 直結 LDO/保護素子の `≥10V` 高耐圧選定推奨【SHOULD】（`6.0V` 絶対最大品回避）、④ 狭レンジ 5V 部品（`4.75–5.25V`）の直結禁止【MUST】<br>・§4.5 PoL 原則へ `4.5–5.5V` 入力耐性および高電流 CARD 向け降圧 DC-DC 推奨を追記<br>・§5.5 シルク指針へ電源表記（`4.5V~5.5V`）を補足<br>・§8 `v5.0` 改訂履歴へ `VBUS 4.5V–5.5V (5V ±10%) MUST` 統合を明記 |
| [`docs/spec/mcu_mapping/attiny1616.md`](../../docs/spec/mcu_mapping/attiny1616.md) | ・§2 ピン表 (L46) | **Type C** | ・Pins 17–20：`4.5V–5.25V` → **`4.5V–5.5V (5V ±10%)`** |
| [`docs/spec/mcu_mapping/ch32x035.md`](../../docs/spec/mcu_mapping/ch32x035.md) | ・§2 ピン表 (L46) | **Type C** | ・Pins 17–20：`4.5V–5.25V` → **`4.5V–5.5V (5V ±10%)`** |

### Tier 2：日英ガイド・フォームファクタ・開発ガイドライン（6 ファイル）

| ファイルパス | 該当箇所 | タイプ | 具体的改訂内容 |
| :--- | :--- | :---: | :--- |
| [`docs/ja/adx_formfactor_guidelines.md`](../../docs/ja/adx_formfactor_guidelines.md)<br>[`docs/en/adx_formfactor_guidelines.md`](../../docs/en/adx_formfactor_guidelines.md) | ・§3.1 電圧仕様 (JA L40–43 / EN L39–42)<br>・§3.2 Power CARD 規則<br>・§7 チェックリスト (JA L135–141 / EN L134–140) | **Type A**<br>＋<br>**Type B** | ・§3.1 の矛盾文（`4.5V〜5.0V 動作範囲、推奨 4.75V〜5.25V`）を解消し、**バス許容動作範囲【MUST】: `4.5V 〜 5.5V DC`（`5.0V ±10%`）** と **給電側推奨精度【SHOULD】: `4.75V 〜 5.25V DC`（上限 `5.5V` 厳守）** に刷新<br>・§3.1 PoL 原則へ **高耐圧 LDO（動作耐圧 `≥ 10V` 推奨・`6.0V` 絶対最大品回避）**・大電流時 DC-DC 推奨・**狭レンジ 5V 部品の直結禁止【MUST】** を追記<br>・§7 セルフチェックリストへ **`【MUST】VBUS 4.5V〜5.5V（5V ±10%）全域での動作耐性を確保したか（狭レンジ 5V 部品の非直結、および LDO 入力耐圧・過渡マージン確認）`** を新設 |
| [`docs/ja/8858_formfactor.md`](../../docs/ja/8858_formfactor.md)<br>[`docs/en/8858_formfactor.md`](../../docs/en/8858_formfactor.md) | ・§4.2 電源供給仕様 (JA L105–110 / EN L105–110) | **Type B** | ・`供給電圧: 4.75 V 〜 5.25 V` を二層定義へ整理：<br>　- **バス許容動作電圧（全カード耐性【MUST】）**: **`4.5 V 〜 5.5 V DC`**（`5.0 V ±10%`）<br>　- **電源供給側の推奨精度【SHOULD】**: `4.75 V 〜 5.25 V DC`（上限 `5.5 V` 厳守）<br>　- 既存の保護要件（過電流 `≈ 2A`・短絡・逆流防止）および OVLO 整合を明記 |
| [`docs/ja/ADX_pinout.md`](../../docs/ja/ADX_pinout.md)<br>[`docs/en/ADX_pinout.md`](../../docs/en/ADX_pinout.md) | ・§3.1 電源配置解説 (JA L92–93 / EN L95)<br>・§4 Card 設計ガイドライン 項目 1 (JA L126–128 / EN L119–120) | **Type A** | ・§3.1 に `4.5V〜5.5V (5V ±10%)`（USB-C `vSafe5V` および配線降下両対応）の設計意図を簡潔に追記<br>・§4 項目 1（PoL レギュレーション）へ **全 CARD の `4.5V〜5.5V` 耐性【MUST】**、**6.0V 絶対最大 LDO 回避（動作耐圧 `≥10V` 推奨）**、**狭レンジ 5V 部品の直結禁止** を追記 |

### Tier 3：ポータル・README ＆ Starlight Web サイト（6 ファイル）

| ファイルパス | 該当箇所 | タイプ | 具体的改訂内容 |
| :--- | :--- | :---: | :--- |
| [`README.md`](../../README.md) | ・§2.3 `VBUS` 説明 (L92) | **Type C** | ・`$4.5\text{V} \sim 5.25\text{V}$` → **`$4.5\text{V} \sim 5.5\text{V}$ ($5\text{V} \pm 10\%$)`** |
| [`docs/ja/README.md`](../../docs/ja/README.md)<br>[`docs/en/README.md`](../../docs/en/README.md) | ・§2.3 `VBUS` 説明 (JA L92 / EN L87) | **Type C** | ・電流表記（`2A〜3A`）に加えて **`4.5V〜5.5V` (`5V ±10%`)** を明記し、ルート `README.md` と情報粒度を統一 |
| [`site/src/content/docs/adx/standards.mdx`](../../site/src/content/docs/adx/standards.mdx) | ・Quick Reference `Power Bus` (L26) | **Type C** | ・`4.5V – 5.25V DC` → **`4.5V – 5.5V DC (5V ±10% MUST tolerance across all boards; 4.75–5.25V recommended supply)`** |
| [`site/src/content/docs/adx/index.mdx`](../../site/src/content/docs/adx/index.mdx)<br>[`site/src/content/docs/ja/adx/index.mdx`](../../site/src/content/docs/ja/adx/index.mdx) | ・規格表 `VBUS` (EN L21 / JA L23) | **Type C** | ・`4.5–5.25V` → **`4.5–5.5V`**（Starlight ビルド検証 `cd site && npm run build` 実施） |

### Tier 4：後続ハードウェア連動トラック（本計画の Phase 1〜4 とは分離管理）

| ファイルパス | タイプ | 扱い・連動方針 |
| :--- | :---: | :--- |
| [`hardware/CORE-U/rev6/spec.md`](../../hardware/CORE-U/rev6/spec.md)<br>[`hardware/CORE-U/rev6/deep_sleep_potential.md`](../../hardware/CORE-U/rev6/deep_sleep_potential.md) | **Type D** | **【部品再選定トラック完了後に反映】**<br>・現行 BOM の `TLV75533PDBVR`（推奨 `5.5V` / 絶対最大 `6.0V`）および `SMF5.0CA-MS`（$V_{RWM}=5.0\text{V}$）は `VBUS 4.5–5.5V` MUST 仕様に対し過渡マージン・リーク面で不足。<br>・別ステップにて **LDO（最有力候補：`TI TPS70933DBVR` / 動作 30V・絶対最大 32V・1µA $I_Q$・`-40〜+125℃`）**、**TVS（最有力候補：`SMF6.0CA` または単方向 `SMF6.0A` / $V_{RWM}=6.0\text{V}$）**、**eFuse (`TPS259470`) OVLO 閾値（`~5.8V–6.0V`）・`FLT` 保護**、および **LED 電流上限（`@ 5.5V`）** を再選定・検算したうえで、`spec.md`（v0.6）および `deep_sleep_potential.md`（LDO $I_Q$ 変更反映）を更新する。<br>・※本計画（Phase 1〜4）実行時に一時的な齟齬を防ぐため、`spec.md` §0・§3.1 へ「`VBUS = 4.5–5.5V` 改訂に伴い LDO/TVS 再選定中」の最小注記のみ先行付与することも可。 |
| [`hardware/CARD/Reference/concept.md`](../../hardware/CARD/Reference/concept.md)<br>[`hardware/CARD/Prototype/proposal.md`](../../hardware/CARD/Prototype/proposal.md) | **Type E** | **【履歴保全・改変しない】**<br>・既存 `v0` は旧 `v4.2`（`4.75–5.25V`）実績としてそのまま保持する。<br>・次期 **`Reference CARD v1`** 仕様策定タスクにて、① `ADX-BUS-SPEC-v5.0` ピン表・GND 接続 `{1,3,5,11,16}`、② シルク電圧表記 **`4.5V~5.5V`**、③ **THT Eject Header** BOM 差替、④ 既存 M4 マーカー（`100010` / `100011`）継承をまとめて設計する。 |

### Tier 5：完全対象外（歴史的記録・アーカイブ）
以下は一切編集しない：
- すべての CAD / ガーバー / ネットリスト / 画像ファイル（`*.zip`, `*.net`, `*.enet`, `*.png`, `*.jpg`）
- `docs/ja/8854_formfactor.md`, `docs/en/8854_formfactor.md`, `docs/ja/8748_formfactor.md`, `docs/en/8748_formfactor.md`
- `docs/blog/posts/*`, `docs/blog/memo/*`
- `docs/memo/adx_pinout_v2.md`, `memo/architecture/*`
- `hardware/archive/*`, `hardware/OLD/*`, `hardware/CORE-U/rev0`〜`rev5`
- `memo/PLAN/archive/*`, `memo/REPORT/archive/*`

---

## 5. 段階的実行ロードマップ（Phased Execution Plan）

SSOT（英語マスター）から下流文書へ一方向に展開し、ドキュメント改訂とハードウェア再選定を明確に切り分ける。

```text
  ═══════════════════════════════════════════════════════════════════════
   【 Track 1：バス電圧規格ドキュメント改訂（本計画のスコープ）】
  ═══════════════════════════════════════════════════════════════════════
  Phase 1 : SSOT 規格書 ＆ MCU マッピング改訂（Type A / C）
    ├─ docs/spec/ADX_PINOUT.md                  (4 条項新設・Pins 17-20 更新・v5.0 履歴統合)
    ├─ docs/spec/mcu_mapping/attiny1616.md      (4.5V–5.5V 同期)
    └─ docs/spec/mcu_mapping/ch32x035.md        (4.5V–5.5V 同期)
         │
         ▼
  Phase 2 : 日英ガイド・8858 規格・開発ガイドライン同期（Type A / B）
    ├─ docs/ja/adx_formfactor_guidelines.md & docs/en/adx_formfactor_guidelines.md
    │    (矛盾解消・4 条項反映・§7 チェックリスト MUST 項目追加)
    ├─ docs/ja/8858_formfactor.md & docs/en/8858_formfactor.md
    │    (§4.2 二層定義: 許容 4.5–5.5V MUST / 供給推奨 4.75–5.25V SHOULD)
    └─ docs/ja/ADX_pinout.md & docs/en/ADX_pinout.md
         (§3.1 設計意図補足・§4 PoL 耐圧＆狭レンジ部品規則追記)
         │
         ▼
  Phase 3 : ポータル・README・Starlight 公開サイト同期（Type C）
    ├─ README.md, docs/ja/README.md, docs/en/README.md
    ├─ site/src/content/docs/adx/standards.mdx
    ├─ site/src/content/docs/adx/index.mdx & site/src/content/docs/ja/adx/index.mdx
    └─ Starlight ビルド検証 (cd site && npm run build)
         │
         ▼
  Phase 4 : 横断監査 ＆ 完了確認（Verification）
    ├─ アクティブ文書全域の grep 検査：
    │    「4.75」「5.25」「4.5V 〜 5.0V」「4.5V to 5.0V」「4.5–5.25」「4.5V–5.25V」
    │    が「供給側推奨精度 (SHOULD)」および「Type D/E 歴史保全」以外に残存していないこと
    ├─ 日英間の RFC 2119 キーワード（MUST / SHOULD）一致チェック
    └─ 本計画書ステータスを「Track 1 完了」へ更新

  ═══════════════════════════════════════════════════════════════════════
   【 後続トラック（本計画完了後に順次着手）】
  ═══════════════════════════════════════════════════════════════════════
  Track 2 : CORE-U rev6 電源保護・LDO 再選定（Type D）
    ├─ LDO 再選定検証（TPS70933DBVR 等のドロップアウト・熱・ピン配置・LCSC 在庫）
    ├─ TVS（SMF6.0CA / SMF6.0A）・TPS259470 OVLO（~5.8–6.0V）・LED 電流 (@ 5.5V) 検算
    └─ hardware/CORE-U/rev6/spec.md (v0.6) ＆ deep_sleep_potential.md 更新
         │
         ▼
  Track 3 : Reference CARD v1 改訂仕様策定（Type E 引継）
    └─ ADX-BUS v5.0 ＋ 4.5V~5.5V シルク ＋ THT Eject Header ＋ マーカー 100010/100011 継承
```

---

## 6. 完了条件（Definition of Done：Track 1）

1. [`docs/spec/ADX_PINOUT.md`](../../docs/spec/ADX_PINOUT.md) に `VBUS` 新規格 4 条項（`4.5V–5.5V` 全モジュール耐性 **MUST**、供給推奨 `4.75–5.25V` **SHOULD**、PoL 高耐圧 LDO `≥10V` 推奨 **SHOULD**、狭レンジ 5V 部品の非直結 **MUST**）が `ADX-BUS-SPEC-v5.0` として矛盾なく明文化されていること。
2. Tier 1〜3 の計 15 ファイル（SSOT・MCU マッピング・日英ガイドライン・8858 規格・日英ピンアウト解説・日英/ルート README・Starlight サイト）で、電圧表記と RFC 2119 要件レベルが 100% 一致していること。
3. アクティブ文書全域の grep 監査により、旧 `4.5–5.25V` や自己矛盾していた `4.5–5.0V` の取り残しがゼロであること（`4.75–5.25V` は「供給側推奨精度（SHOULD）」または「狭レンジ部品の注意例」および履歴保全対象のみに限定）。
4. `cd site && npm run build` がエラーなく完了すること。
5. `CORE-U rev6`（部品再選定前）や `Reference CARD v0`（`v4.2` 履歴）に実態と矛盾する書き換えを行っていないこと。
