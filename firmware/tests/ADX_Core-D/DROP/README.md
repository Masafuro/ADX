<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus: 自律分散Pub/Sub ＆ パッシブフェイルセーフ フィールドネットワーク

**DROP-Bus** (**D**eterministic **R**elay with **O**ptional-mediator & **P**assive-failsafe Bus) は、極限の低コスト（単一SKU・完全水晶レス）と高度な決定論性（ジッター拘束）、そして産業用機能安全（受動的フェイルセーフ：沈黙＝安全）を両立させるために設計された、RS-485 半二重通信向けの**自律分散型Pub/Subフィールドネットワーク規格**（Unique ID / Common Subscriber モデル）です。

---

## 1. ドキュメント構成

```text
DROP/
├── README.md                                  # 本書: DROP-Bus 概要およびドキュメント案内
├── DROP_whitepaper.md                         # DROP-Bus 仕様書・ホワイトペーパー（決定版）
├── PLAN/                                      # 開発・テスト計画書
│   ├── README.md                              # 計画書ディレクトリ案内
│   ├── DROP_DEVELOPMENT_PLAN.md               # 段階的実機開発計画書（Phase 1〜5 ロードマップ）
│   └── DROP_TEST_SPECIFICATION.md             # 詳細テストケース仕様書兼実施記録（全15ケース）
├── Phase/                                     # フェーズ別テスト実装・ガイドライン
│   ├── README.md                              # 実装ディレクトリ案内
│   └── SKETCH_DEVELOPMENT_GUIDELINE.md        # スケッチ開発規約＆10大鉄則（Pub/Sub対応）
└── memo/                                      # プロトコル策定過程の設計検討メモ群
    ├── 01_vision.md                           # コア思想・初期ビジョン
    ├── 02_basic_design.md                     # 用語定義・基本設計
    ├── 03_design_rule_concept.md              # Mediator概念と設計裁量
    ├── 04_data_frame.md                       # 大容量・可変長フレーム検討
    └── 05_retry_plugin.md                     # 再送プラグイン仕様
```

| ファイル / ディレクトリ | 概要 |
| :--- | :--- |
| 📄 **[`DROP_whitepaper.md`](./DROP_whitepaper.md)** | **DROP-Bus 仕様書・ホワイトペーパー（決定版）**<br>Decentralized Pub/Sub（Unique ID / Common Subscriber）、Multi-rate スケジューリング、物理層・LINAUTO同期、データフレーム仕様、決定論的バトンリレー、Mediator論理ロール、再送プラグインAPI、機能安全規格適合性を網羅した総合文書。 |
| 📁 **[`PLAN/`](./PLAN/)** | **開発・テスト計画書ディレクトリ**<br>・[`DROP_DEVELOPMENT_PLAN.md`](./PLAN/DROP_DEVELOPMENT_PLAN.md)（段階的ロードマップ）<br>・[`DROP_TEST_SPECIFICATION.md`](./PLAN/DROP_TEST_SPECIFICATION.md)（全15テストケース仕様書） |
| 📁 **[`Phase/`](./Phase/)** | **フェーズ別テスト実装・ガイドライン**<br>・[`SKETCH_DEVELOPMENT_GUIDELINE.md`](./Phase/SKETCH_DEVELOPMENT_GUIDELINE.md)（10大鉄則・Pub/Subコードテンプレート） |
| 📁 **[`memo/`](./memo/)** | **プロトコル策定過程の設計検討メモ群（初期ドラフト 01〜05）** |

---

## 2. コア特徴サマリー

```text
+-----------------------------------------------------------------------------------+
|                     DROP-Bus: Decentralized Baton Pub/Sub                         |
+-----------------------------------------------------------------------------------+
|  [Silence is Safety]        バトンが落ちた瞬間に全ノード一斉に安全停止 (STO)        |
|  [Unique ID / Common Sub]   論理バトンIDによる一意送信 ＆ 共有バスでの全員自由購読  |
|  [Multi-rate TDMA]          1ノード複数スロット所有による非対称周波数スケジューリング|
|  [Crystal-less Single SKU]  ATtiny1616内蔵RC発振器 + LINAUTO (0x55) で毎周自動校正  |
|  [Decoupled Mediator]       平常時マスター不要。調停・再送は外付けプラグインで統制   |
+-----------------------------------------------------------------------------------+
```

1. **沈黙は安全（Silence is Safety）**
   通信途絶やパケット破損時に勝手な再送やスキップを行わず、タイマー満了で全員が同期してハードウェア安全遮断（Safe Torque Off）します。
2. **自律分散 Pub/Sub（Unique ID / Common Subscriber）**
   IDは「論理バトン（スロット/トピック）」を指し、各バトンは唯一の物理ノードが送信（Unique Publisher）。送出されたデータは全ノードが自由に傍受・購読（Common Subscriber）します。
3. **不等周期サンプリング（Multi-rate Scheduling）**
   1つの物理ノードが複数のバトンIDを所有することで、マスターレスのまま高速ノード（1kHz）と低速ノード（100Hz）の時間スロットを柔軟に配分可能です。
4. **単一SKU・完全水晶レス**
   全ノードが内蔵RC発振器で駆動可能。フレーム先頭の LINAUTO（Break + `0x55`）で受信ごとにボーレートを自動校正します。

---

## 3. 実機検証・実装ロードマップ (ADX Core-D)

```mermaid
flowchart LR
    P1["Phase 1: 一体型フレーム & CRC-16"] --> P2["Phase 2: 自律Pub/Sub & Multi-rate"]
    P2 --> P3["Phase 3: パッシブSTO (心中)"]
    P3 --> P4["Phase 4: Mediator & 再送"]
    P4 --> P5["Phase 5: XDIR & 高ボーレート"]
```

- [ ] **Phase 1:** 一体型フレーム送受信 ＆ CRC-16 検証（`TC-D1-01` 〜 `04`）
- [ ] **Phase 2:** 自律分散バトンリレー ＆ Pub/Sub 購読・Multi-rate計測（`TC-D2-01` 〜 `04`）
- [ ] **Phase 3:** パッシブ・フェイルセーフ ＆ 自律心中 STO（`TC-D3-01` 〜 `03`）
- [ ] **Phase 4:** Mediator 介入（Break衝突・再点火）＆ 再送プラグイン（`TC-D4-01` 〜 `03`）
- [ ] **Phase 5:** ハードウェア XDIR 自動方向制御 ＆ 高速ボーレート評価（`TC-D5-01` 〜 `02`）
