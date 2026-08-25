<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テストファームウェア・スケッチ (Phase)

本ディレクトリ（`firmware/tests/ADX_Core-D/LIN_test/Phase/`）には、**ADX Core-D**（MCU: ATtiny1616, RS-485 トランシーバー: SP485EEN）における **LN-485 (LIN-based RS-485)** 各検証フェーズ（Phase 1 〜 Phase 6）のテストファームウェア（`.ino`）およびスケッチ作成ガイドラインをまとめています。

---

## 1. ディレクトリ構成

| ファイル / ディレクトリ | 内容概要 |
| :--- | :--- |
| [**`SKETCH_DEVELOPMENT_GUIDELINE.md`**](./SKETCH_DEVELOPMENT_GUIDELINE.md) | **スケッチ開発ガイドライン & 重要注意事項**<br>・`RS-485_test` の実績とアーキテクチャおさらい<br>・レジスタ読み出し順序（`RXDATAH` $\rightarrow$ `RXDATAL`）等の絶対ルール<br>・`WFB` 再設定、`ISFIF` クリア、GPIO Break、レスポンススペースの制約<br>・各フェーズ共通のコードテンプレート |
| [**`TC-P1-01/`**](./TC-P1-01/) | **【Phase 1】 GPIO Break 信号波形検証テスト**<br>・[`tc-p1-01_break_test.ino`](./TC-P1-01/tc-p1-01_break_test.ino)<br>・[`README.md`](./TC-P1-01/README.md)（実施手順・OK/NG判定基準）<br>・[`result.md`](./TC-P1-01/result.md)（テスト結果記録シート） |

---

## 2. フェーズ別テストスケッチ構成

| フェーズ | テストID | ディレクトリ / スケッチ | 検証対象 | 状態 |
| :--- | :---: | :--- | :--- | :---: |
| **Phase 1** | `TC-P1-01` | [`TC-P1-01/tc-p1-01_break_test.ino`](./TC-P1-01/tc-p1-01_break_test.ino) | マスター GPIO Break 送出（14 Tbit LOW + 1 Tbit HIGH） | **PASS (合格)** |
| | `TC-P1-02` | [`TC-P1-02/tc-p1-02_header_test.ino`](./TC-P1-02/tc-p1-02_header_test.ino) | Sync (`0x55`) & PID 送出・受信検証 | **PASS (合格)** |
| | `TC-P1-03` | [`TC-P1-02/tc-p1-02_header_test.ino`](./TC-P1-02/tc-p1-02_header_test.ino) | スレーブ標準 UART 受信確認 | **PASS (合格)** |
| **Phase 2** | `TC-P2-01`〜`05` | [`TC-P2/tc-p2_slave_linauto_test.ino`](./TC-P2/tc-p2_slave_linauto_test.ino) | スレーブ LINAUTO ハードウェア自動同期 ＆ PID 検証 | **PASS (合格)** |
| **Phase 3** | `TC-P3-01`〜`03` | `TC-P3-xx/` | マスター送信型データ通信 ＆ スレーブ LED コマンド制御 | 準備中 |
| **Phase 4** | `TC-P4-01`〜`02` | `TC-P4-xx/` | スレーブ応答型双方向通信 ＆ ターンアラウンド検証 | 準備中 |
| **Phase 5** | `TC-P5-01`〜`04` | `TC-P5-xx/` | ハードウェア XDIR 自動制御 ＆ 多重ボーレート・クロック比較 | 準備中 |
| **Phase 6** | `TC-P6-01`〜`02` | `TC-P6-xx/` | マルチノード ID フィルタリング ＆ ブートローダ診断疎通 | 準備中 |
