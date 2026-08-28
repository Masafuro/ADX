<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PAL Network レポート & 仕様書インデックス

本ディレクトリ（`firmware/tests/ADX_Core-D/PAL/REPORT/`）には、**PAL (Polling Access Link) Network** のプロトコル仕様書、検証結果レポート、および技術ドキュメントを配置します。

---

## 1. 主要ドキュメント

* [**`PAL_SPECIFICATION.md`**](./PAL_SPECIFICATION.md): **PAL Network プロトコル仕様書 (Version 1.0 Draft)**
  * PAL の基本思想とエンティティ定義（Node, Topic, UPT, CS, Master, Shared Bus）
  * 基本原則（UPT 原則、1 Slot = 1 Bounded Publication、決定論的周期 $T_{cycle} = \sum T_{slot}$）
  * フレームフォーマット仕様（Poll フレーム ＆ Publication パケットのビットレイアウト）
  * 通信シーケンス（通常通信・タイムアウト・リカバリ）
  * ATtiny1616 ハードウェアドライバ（`pal_hal`）の必須制約
  * 動的サービス（Join / Registration / 不正パブリッシャー隔離）

---

## 2. 関連ディレクトリ・ドキュメント

| ドキュメント / パス | 概要 |
| :--- | :--- |
| [**`../PAL_whitepaper.md`**](../PAL_whitepaper.md) | **PAL Network 基本骨子・ホワイトペーパー（設計原則の原典）** |
| [**`../PLAN/DEVELOPMENT_PLAN.md`**](../PLAN/DEVELOPMENT_PLAN.md) | **PAL Network 開発計画書 (全体ロードマップ & フェーズ定義)** |
| [**`../../LIN_test/Phase/`**](../../LIN_test/Phase/) | **先行 LN-485 PoC 実績・ハードウェア知見アーカイブ** |
