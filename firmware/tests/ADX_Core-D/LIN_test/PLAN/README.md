<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト計画ディレクトリ (PLAN)

本ディレクトリ（`firmware/tests/ADX_Core-D/LIN_test/PLAN/`）には、**ADX Core-D**（MCU: ATtiny1616, RS-485 トランシーバー: SP485EEN）における **LN-485 (LIN-based RS-485)** 通信機能の詳細テスト仕様書および実施計画をまとめています。

---

## 1. 収録ドキュメント

| ドキュメント | 内容概要 |
| :--- | :--- |
| [**`LN-485_TEST_SPECIFICATION.md`**](./LN-485_TEST_SPECIFICATION.md) | **LN-485 詳細テスト仕様書兼実施計画書**<br>・テスト環境および共通前提条件<br>・Phase 1 〜 Phase 6（全19テストケース）のテストマトリクス<br>・各テストケースの **【実行手順 (Action)】** と **【合否判定基準 (OK / NG)】**<br>・実機テスト実施記録（Phase 1〜3 PASS反映済み） |
| [**`PHASE4_DEVELOPMENT_PLAN.md`**](./PHASE4_DEVELOPMENT_PLAN.md) | **LN-485 Phase 4 開発計画書**<br>・Type B (Slave-Pub → Master-Sub) 実証 ＆ Master Broker MVP 開発計画<br>・スレーブ側レジスタ直接送信、ターンアラウンド、非ブロッキング・タイムアウト設計方針 |

---

## 2. 関連ドキュメントへのリンク

* [**技術調査レポート (`REPORT/LN-485_TECHNICAL_INVESTIGATION_REPORT.md`)**](../REPORT/LN-485_TECHNICAL_INVESTIGATION_REPORT.md)
* [**段階的テスト計画ロードマップ (`REPORT/LN-485_STEP_BY_STEP_TEST_PLAN.md`)**](../REPORT/LN-485_STEP_BY_STEP_TEST_PLAN.md)
* [**ATtiny1616 LINスレーブ技術リファレンス (`reference.md`)**](../reference.md)
* [**RS-485 先行検証結果 (`RS-485_test/README.md`)**](../../RS-485_test/README.md)
