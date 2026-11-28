<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Track 2 エビデンス調査：中間表現（IR）・マッス階層設計 ＆ KiCad 圏エッセンスの EasyEDA Pro 橋渡し

- **調査日**: 2026-11-28
- **位置づけ**: `UNOMODOKI R3` 課題 2（3 階層「マッス」設計・修正可能性）および盲点 ③（KiCad 中心の研究資産をどう EasyEDA Pro へ輸入するか）の精査

---

## 1. リサーチクエスチョン（RQ）

1. なぜ生 CAD 座標・一括スクリプト生成は修正可能性（Modifiability）で破綻するのか？
2. `SchGen`・`atopile`・`JITX`・`tscircuit` は、階層（マッス）・制約・部分修正（増分編集）・ロックをどう設計しているか？
3. `PCBWorld` や KiCad MCP サーバー群で確立された **エージェント EDA 設計パターン** のうち、EasyEDA Pro へそのまま輸入できるエッセンスは何か？

---

## 2. 先行研究・ツールから得られた一次エビデンス

### 2.1 `SchGen`（2026）：生座標を捨て「相対配置＋ピン名結線＋編集プリミティブ」へ

- **`SchGen`（Luo et al., 2026, [arXiv:2605.30345](https://arxiv.org/abs/2605.30345)）**
  - **問題意識**: KiCad 等のネイティブファイルは冗長で絶対座標に強く依存するため、LLM のトークン化・空間推論と相性が最悪。
  - **解決策（Semantic-Grounded Code Representation）**:
    1. **相対配置（Relative Placement）**: 絶対 `(x, y)` ではなく、アンカー部品・機能ブロックからの相対オフセットで配置（例：「IC 電源ピンのすぐ隣にデカップリングコンデンサ」）。
    2. **ピン名ベース結線（Pin-Name-Based Wiring）**: 座標計算を LLM にさせず、`get_pin_location(symbol_ref, pin_name)` のようなセマンティック解決に委譲。
    3. **最小編集プリミティブ API**: `add_schematic_symbol`, `add_label`, `get_pin_location` のような疎結合なプリミティブ列へ落とす。
  - **ADX への示唆**: `UNOMODOKI R3` の U2（回路図）が `5/5` で成功した理由は、まさに無意識に「機能ブロック分割＋実測ピン座標取得＋ネットラベル結線」という `SchGen` と同型の構造を採っていたからである。これを **PCB 配置・マッス設計にも横展開** する価値が大きい。

### 2.2 Code-as-CAD 3 大ツール（`tscircuit` / `atopile` / `JITX`）の「修正可能性・ロック」設計比較

| 設計課題 | `tscircuit` (TS/React) | `atopile` (`.ato` / KiCad) | `JITX` (Python) | **ADX へ抽出すべきエッセンス** |
| :--- | :--- | :--- | :--- | :--- |
| **階層・マッス表現** | `<group>` + CSS Flexbox/Grid 的相対レイアウト | モジュールがモジュールをインスタンス化する木構造 | Python 関数による幾何学的配置ライブラリ（例：IC 周囲リング配置） | **「機能マッス JSON/YAML」＋「相対アンカー規則」** を宣言的に定義し、絶対座標はソルバー／スクリプトが展開する。 |
| **増分編集（作り直し回避）** | **`manual-edits.json`（上書きパッチ層）** を分離管理 | ネットリスト差分を KiCad 側へコンパイル反映 | コード側パラメータ＋決定的再コンパイル | **`tscircuit` の `manual-edits.json` 構造が秀逸！**「自動生成ベース配置」と「確定済み・ロック済みマッス座標（`locked_overrides.json`）」を分離すれば、一部修正しても確定マッスが壊れない。 |
| **ロック・人間介入** | GUI ドラッグが自動で `manual-edits.json` へ記録される | KiCad ネイティブのフットプリントロックへ委譲 | コード内アンカー固定＋反復プレビュー | EasyEDA Pro の `locked: true` 属性とローカルの `locked_state.json` を双方向同期させるプロトコルが最適。 |
| **現場の批判（罠）** | 配線レベルの微修正をコードに書き戻すのが苦痛 | 物理配置の細かい制約がコードと KiCad で二重管理になりやすい | 学習コスト・独自環境依存が高い | **「コードで全部書く」のではなく「マッス定義・制約・ロック状態＝コード（SSOT）、物理細部実行＝EasyEDA Pro＋外部ソルバー」と役割分担すべき。** |

### 2.3 KiCad MCP ＆ `PCBWorld`（2026）から抽出するツール非依存アーキテクチャ

- **`PCBWorld`（Song et al., LG AI Research, 2026, [arXiv:2607.05915](https://arxiv.org/abs/2607.05915)）＆ KiCad MCP サーバー群**
  - KiCad の知識がなくても、以下の **4 つの設計パターン** はそのまま EasyEDA Pro MCP へ移植可能：
    1. **Engine-Grounded ループ**：モック空間ではなく、必ず実 EDA エンジンの厳格 DRC（8 指標級：クリアランス・未配線・ビア数・配線長等）を唯一のオラクルにする。
    2. **Analysis（読取）と Mutation（書換）の責務分離**：ネットリスト監査・DRC 読取・座標抽出ツールと、配置・配線変更ツールを明確に分離し、事故（`UNOMODOKI R3` の U3 リードオンリー調査のような場面）を防ぐ。
    3. **3 層 MCP 構造（Resources / Tools / Prompts）**：ボード状態・BOM・ネットリストを軽量サマリー（Resource）として随時参照し、長大な生 JSON でコンテキストを圧迫しない。
    4. **Reflection（DRC 座標ピンポイント修復）＋ ステージング承認ゲート**：全削除・全再生成ではなく、DRC が返した違反座標・ネット周辺だけを局所的にリップアップ＆再配線する。

### 2.4 EasyEDA Pro 4.1.105 側の橋渡し能力（実測・ドキュメント確認済み）

EasyEDA Pro には KiCad 圏・外部ソルバーと接続するための強力な標準インターフェースが備わっている：
- **Specctra DSN / SES 相互運用 API**:
  - `eda.pcb_ManufactureData.getDsnFile()`：外形・パッド・ネットリスト・ルール・ロック済み配線（`fix`/`protect`）を標準 `.dsn` へ書き出し可能。
  - `eda.pcb_Document.importAutoRouteSesFile()`：外部ルーター（FreeRouting / FastRoute / 自作ソルバー）が出力した `.ses` を EasyEDA Pro へ書き戻し可能。
- **内蔵自動配線 API**:
  - `eda.pcb_Document.startAutoRouting()`：人間 GUI 操作（`UNOMODOKI R3` U4-b の最大ストレス源）を API 置換可能。
- **ロック機構**:
  - EasyEDA Pro 上の `locked: true`（コンポーネント／トラック／ビア）は、内蔵オートルーティングおよび FreeRouting 連携時に **保護対象（削除・上書き禁止）** として扱われる（M5 着手前の 15 分ミニ PoC で `getDsnFile()` の `type fix/protect` 出力仕様を実測確認する価値大）。

---

## 3. Track 2 判定：Adopt / Adapt / Build（ADX 向け）

| 要素 | 判定 | ADX（EasyEDA Pro / `CORE-U rev6`）での具体方針 |
| :--- | :---: | :--- |
| **KiCad へのツール全面移行** | **Do Not Adopt** | 開発者の実績と LCSC/JLCPCB 一貫の強みを持つ EasyEDA Pro を維持し、KiCad 圏の設計パターンと DSN/SES 互換ツールだけを輸入する。 |
| **`tscircuit` 型 `overrides / lock` 分離層 ＋ `SchGen` 型相対配置 IR** | **Build（軽量 JSON IR）** | ADX 独自の軽量中間表現 **`adx_board_ir.json`**（①マッス定義・②相対配置＆制約・③`locked_overrides.json`）を導入。全工程一括スクリプトから **「マッス単位の増分ビルド＆ロック反復」** へ脱皮する。 |
| **KiCad MCP / `PCBWorld` の 4 原則（読取/書換分離・Engine-Grounded DRC・局所修復）** | **Adapt** | 既存の EasyEDA MCP スクリプト群を「状態読取（監査）」「マッス単位ミューテーション」「DRC 局所修復」の 3 モジュールに整理。 |
| **Specctra DSN / SES 橋渡しパイプライン** | **Adopt（PoC 実証後）** | `getDsnFile()` → 外部ヘッドレスルーター → `importAutoRouteSesFile()` の標準パイプラインを確立し、EasyEDA Pro 内に閉じずに外部アルゴリズム資産を活用可能にする。 |
