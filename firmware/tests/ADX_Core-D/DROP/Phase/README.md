<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus フェーズ別テスト実装ディレクトリ (Phase)

本ディレクトリには、**ADX Core-D** における DROP-Bus の各テストフェーズ（Phase 1 〜 Phase 5）のスケッチ開発ガイドライン、テストファームウェア（`.ino`）、および実機検証レポートをまとめていきます。

---

## 開発ガイドライン ＆ 完了報告書

* 📄 **[`SKETCH_DEVELOPMENT_GUIDELINE.md`](./SKETCH_DEVELOPMENT_GUIDELINE.md)**
  ATtiny1616 / RS-485 制御における **10大鉄則**、CRC-16 高速計算ルーチン、`TCB0` タイマー連動 STO ロジック、および単一SKU対応のスケッチ基本テンプレート。
* 📄 **[`PHASE1_COMPLETION_REVIEW_REPORT.md`](./PHASE1_COMPLETION_REVIEW_REPORT.md)**
  **Phase 1 完了レビューレポート**（一体型フレーム送受信・LINAUTO同期・CRC-16検証・バッファ防御 実機検証総括）。全4テストケース ALL PASS 達成記録。
* 📄 **[`PHASE2_COMPLETION_REVIEW_REPORT.md`](./PHASE2_COMPLETION_REVIEW_REPORT.md)**
  **Phase 2 完了レビューレポート**（自律分散バトンリレー・Pub/Sub相互購読・ジッター計測・Double Buffer・Multi-rate スロット 実機検証総括）。全4テストケース ALL PASS 達成記録。

---

## フェーズ別テスト構成

| ディレクトリ | フェーズ / 対象テスト | 概要 | 状態 |
| :--- | :--- | :--- | :---: |
| 📁 **[`TC-D1-01/`](./TC-D1-01/)** | **Phase 1 (Step 1):** 生フレームダンプ | `TC-D1-01`: Break + `0x55` + ヘッダ + データ + CRC-16 の通常UART生バイト列受信検証 | **PASS** |
| 📁 **[`TC-D1-02/`](./TC-D1-02/)** | **Phase 1 (Step 2):** LINAUTO 同期 ＆ 可変長 | `TC-D1-02`: LINAUTO 自動校正、可変長ペイロード (4B/8B/16B)、CRC-16 照合 | **PASS** |
| 📁 **[`TC-D1-03/`](./TC-D1-03/)** | **Phase 1 (Step 3):** CRC 誤り検出 ＆ 破棄 | `TC-D1-03`: 意図的 CRC 反転破損パケットの検知・破棄、赤LED警告、自律復帰 | **PASS** |
| 📁 **[`TC-D1-04/`](./TC-D1-04/)** | **Phase 1 (Step 4):** オーバーラン防御 | `TC-D1-04`: `LEN > 64` 不正フレーム受信時の即時アボート・SRAM 保護 | **PASS** |
| 📁 **[`TC-D2-01/`](./TC-D2-01/)** | **Phase 2 (Step 1):** 2ノード自律ピンポンリレー | `TC-D2-01`: 2ノード基本バトンパス ＆ 相互トピック購読（Common Subscriber）実証 | **PASS** |
| 📁 **[`TC-D2-02/`](./TC-D2-02/)** | **Phase 2 (Step 2):** 10,000周期周回 ＆ ジッター | `TC-D2-02`: 連続周回安定性 ＆ サイクルタイム $T_{\text{cycle}}$ ジッター計測 | **PASS** |
| 📁 **[`TC-D2-03/`](./TC-D2-03/)** | **Phase 2 (Step 3):** Double Buffer 連動 | `TC-D2-03`: アプリ層非同期更新 ＆ Zero-Copy 送信の非干渉実証 | **PASS** |
| 📁 **[`TC-D2-04/`](./TC-D2-04/)** | **Phase 2 (Step 4):** Multi-rate スロット | `TC-D2-04`: 1ノード複数スロット所有による不等周期リレー実証 | **PASS** |
| 📁 **[`TC-D3-01/`](./TC-D3-01/)** | **Phase 3 (Step 1):** TCB0 生存監視 ＆ リセット | `TC-D3-01`: 正常フレーム受信ごとの `TCB0` タイマーリセット実証 (STO非発動) | **PASS** |
| 📁 **[`TC-D3-02/`](./TC-D3-02/)** | **Phase 3 (Step 2):** 意図的断線 ＆ 即時安全心中 | `TC-D3-02`: 通信途絶時における全ノード一斉タイマー満了・安全遮断 (STO) | **PASS** |
| `TC-D3-03/` | **Phase 3 (Step 3):** CRC 破損時タイマー非リセット | `TC-D3-03`: 不正パケット受信時にタイマーを更新せず安全停止へ倒す動作 | 準備中 |
| `TC-D4/` | **Phase 4:** SYSTEM `0xFF` ＆ プラグイン | `TC-D4-01` 〜 `04`: Break 衝突停止、再点火、Ping点呼、局所リトライ実証 | 準備中 |
| `TC-D5/` | **Phase 5:** XDIR ＆ 高ボーレート | `TC-D5-01` 〜 `02`: ハードウェア自動 DE 制御、115.2 kbps 評価 | 準備中 |
