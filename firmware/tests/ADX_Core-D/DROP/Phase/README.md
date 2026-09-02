<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus フェーズ別テスト実装ディレクトリ (Phase)

本ディレクトリには、**ADX Core-D** における DROP-Bus の各テストフェーズ（Phase 1 〜 Phase 5）のスケッチ開発ガイドライン、テストファームウェア（`.ino`）、および実機検証レポートをまとめていきます。

---

## 開発ガイドライン

* 📄 **[`SKETCH_DEVELOPMENT_GUIDELINE.md`](./SKETCH_DEVELOPMENT_GUIDELINE.md)**
  ATtiny1616 / RS-485 制御における **10大鉄則**、CRC-16 高速計算ルーチン、`TCB0` タイマー連動 STO ロジック、および単一SKU対応のスケッチ基本テンプレート。**実装前に必ず参照してください。**

---

## フェーズ別テスト構成（予定）

| ディレクトリ | フェーズ / 対象テスト | 概要 |
| :--- | :--- | :--- |
| `TC-D1/` | **Phase 1:** 一体型フレーム ＆ CRC-16 | `TC-D1-01` 〜 `04`: 単体フレーム送受信、可変長 `LEN`、CRC-16-CCITT 検証 |
| `TC-D2/` | **Phase 2:** 自律バトンリレー | `TC-D2-01` 〜 `03`: 2ノードピンポンリレー、ジッター計測、Mailbox 連動 |
| `TC-D3/` | **Phase 3:** パッシブ・フェイルセーフ | `TC-D3-01` 〜 `03`: `TCB0` タイマー生存監視、意図的断線時の安全心中 (STO) |
| `TC-D4/` | **Phase 4:** Mediator ＆ プラグイン | `TC-D4-01` 〜 `03`: Break 衝突停止、再点火フレーム、局所リトライ実証 |
| `TC-D5/` | **Phase 5:** XDIR ＆ 高ボーレート | `TC-D5-01` 〜 `02`: ハードウェア自動 DE 制御、115.2 kbps 評価 |
