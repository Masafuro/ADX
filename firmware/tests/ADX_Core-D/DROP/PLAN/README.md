<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus 開発・テスト計画書ディレクトリ (PLAN)

本ディレクトリには、**ADX Core-D** における DROP-Bus プロトコルの段階的実機開発ロードマップ、詳細テストケース仕様書、および各フェーズの詳細開発計画書をまとめています。

---

## ドキュメント一覧

| ファイル | 概要 |
| :--- | :--- |
| 📄 **[`DROP_DEVELOPMENT_PLAN.md`](./DROP_DEVELOPMENT_PLAN.md)** | **段階的実機開発計画書（ロードマップ）**<br>Phase 1（単体フレーム・CRC-16）から Phase 5（XDIR・高ボーレート）までの開発哲学、スコープ、機材構成、完了基準を規定。 |
| 📄 **[`DROP_TEST_SPECIFICATION.md`](./DROP_TEST_SPECIFICATION.md)** | **詳細テストケース仕様書兼実施記録**<br>全15テストケース（`TC-D1-01` 〜 `TC-D5-02`）の詳細手順（Action）、合否判定基準（Pass Criteria）、および実機検証進捗管理表。 |
| 📄 **[`PHASE1_DEVELOPMENT_PLAN.md`](./PHASE1_DEVELOPMENT_PLAN.md)** | **Phase 1 開発詳細計画書**<br>一体型フレーム送受信、LINAUTO自動同期、CRC-16検証、およびバッファオーバーラン防御の実機開発・検証詳細計画。 |
| 📄 **[`PHASE2_DEVELOPMENT_PLAN.md`](./PHASE2_DEVELOPMENT_PLAN.md)** | **Phase 2 開発詳細計画書**<br>自律分散バトンリレー、Pub/Sub相互購読、ジッター計測、Double Buffer連動、Multi-rateスロットの実装詳細計画。 |
| 📄 **[`PHASE3_DEVELOPMENT_PLAN.md`](./PHASE3_DEVELOPMENT_PLAN.md)** | **Phase 3 開発詳細計画書**<br>パッシブ心中フェイルセーフ、TCB0タイマー連動STO、意図的断線、CRC破損時安全停止の実装詳細計画。 |

---

## 開発フェーズ概要

```mermaid
flowchart LR
    P1["Phase 1: 一体型フレーム & CRC-16"] --> P2["Phase 2: 自律Pub/Sub & Multi-rate"]
    P2 --> P3["Phase 3: パッシブSTO (心中)"]
    P3 --> P4["Phase 4: SYSTEM 0xFF & 再送"]
    P4 --> P5["Phase 5: XDIR & 高ボーレート"]
```
