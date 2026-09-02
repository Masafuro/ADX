<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus 開発・テスト計画書ディレクトリ (PLAN)

本ディレクトリには、**ADX Core-D** における DROP-Bus プロトコルの段階的実機開発ロードマップ、および詳細テストケース仕様書をまとめています。

---

## ドキュメント一覧

| ファイル | 概要 |
| :--- | :--- |
| 📄 **[`DROP_DEVELOPMENT_PLAN.md`](./DROP_DEVELOPMENT_PLAN.md)** | **段階的実機開発計画書（ロードマップ）**<br>Phase 1（単体フレーム・CRC-16）から Phase 5（XDIR・高ボーレート）までの開発哲学、スコープ、機材構成、完了基準を規定。 |
| 📄 **[`DROP_TEST_SPECIFICATION.md`](./DROP_TEST_SPECIFICATION.md)** | **詳細テストケース仕様書兼実施記録**<br>全14テストケース（`TC-D1-01` 〜 `TC-D5-02`）の詳細手順（Action）、合否判定基準（Pass Criteria）、および実機検証進捗管理表。 |

---

## 開発フェーズ概要

```mermaid
flowchart LR
    P1["Phase 1: 一体型フレーム & CRC-16"] --> P2["Phase 2: 自律バトンリレー"]
    P2 --> P3["Phase 3: パッシブSTO (心中)"]
    P3 --> P4["Phase 4: Mediator & 再送"]
    P4 --> P5["Phase 5: XDIR & 高ボーレート"]
```
