<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# PAL Network 開発計画 & ドキュメントインデックス

本ディレクトリ（`firmware/tests/ADX_Core-D/PAL/PLAN/`）には、**PAL (Polling Access Link) Network** の開発計画、フェーズ定義、および設計ドキュメントを配置します。

---

## 1. 主要ドキュメント

* [**`DEVELOPMENT_PLAN.md`**](./DEVELOPMENT_PLAN.md): **PAL Network 開発計画書 (全体ロードマップ & 過去資産参照一覧)**
  * LN-485 から PAL への進化の背景
  * 参照すべき過去ドキュメント・レジスタ知見一覧
  * 階層化アーキテクチャ方針 (`pal_hal` / `pal_core` / `app`)
  * Phase 1 〜 Phase 5 の詳細検証項目と PASS 基準

---

## 2. 関連する上位・外部ドキュメント

| ドキュメント | 概要 |
| :--- | :--- |
| [**`../PAL_whitepaper.md`**](../PAL_whitepaper.md) | **PAL Network 基本骨子・ホワイトペーパー（基本仕様）** |
| [**`../../LIN_test/Phase/SKETCH_DEVELOPMENT_GUIDELINE.md`**](../../LIN_test/Phase/SKETCH_DEVELOPMENT_GUIDELINE.md) | **ハードウェア開発ガイドライン & ATtiny1616 レジスタ必須制約** |
| [**`../../LIN_test/Phase/README.md`**](../../LIN_test/Phase/README.md) | **先行 LN-485 実機検証実績 (Phase 1 〜 Phase 5 PASS 記録)** |
| [**`../../LIN_test/Attiny1616_reference.md`**](../../LIN_test/Attiny1616_reference.md) | **ATtiny1616 USART0 / LINAUTO / XDIR ハードウェア仕様** |
| [**`../../RS-485_test/README.md`**](../../RS-485_test/README.md) | **SP485EEN トランシーバ・物理層回路実績** |
