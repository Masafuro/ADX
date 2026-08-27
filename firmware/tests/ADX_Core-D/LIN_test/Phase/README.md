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
| [**`TC-P2/`**](./TC-P2/) | **【Phase 2】 スレーブ LINAUTO ハードウェア自動同期 & PID検証**<br>・[`tc-p2_slave_linauto_test.ino`](./TC-P2/tc-p2_slave_linauto_test.ino)<br>・[`TC-P2_FIXED_MODIFICATION_MEMO.md`](./TC-P2/TC-P2_FIXED_MODIFICATION_MEMO.md)<br>・[`README.md`](./TC-P2/README.md), [`result.md`](./TC-P2/result.md) |
| [**`TC-P3/`**](./TC-P3/) | **【Phase 3】 Type A 実証 ＆ Slave Subscriber 実装**<br>・[`fixed.ino`](./TC-P3/fixed.ino), [`tc-p3_master_pub_slave_sub.ino`](./TC-P3/tc-p3_master_pub_slave_sub.ino)<br>・[`fixed_report.md`](./TC-P3/fixed_report.md)<br>・[`README.md`](./TC-P3/README.md), [`result.md`](./TC-P3/result.md) |
| [**`TC-P4/`**](./TC-P4/) | **【Phase 4】 Type B 実証 ＆ Master Broker MVP 完成**<br>・[`tc-p4_master_broker_slave_pub.ino`](./TC-P4/tc-p4_master_broker_slave_pub.ino)<br>・[`README.md`](./TC-P4/README.md)（実施手順・OK/NG判定基準）<br>・[`result.md`](./TC-P4/result.md)（テスト結果記録シート） |
| [**`TC-P5/`**](./TC-P5/) | **【Phase 5】 Type C 実証 ＆ LN-485 UP/CS 完成**<br>・[`tc-p5_slave_direct_comm.ino`](./TC-P5/tc-p5_slave_direct_comm.ino)<br>・[`README.md`](./TC-P5/README.md)（実施手順・OK/NG判定基準）<br>・[`result.md`](./TC-P5/result.md)（テスト結果記録シート） |

---

## 2. フェーズ別テストスケッチ構成

| フェーズ | テストID | ディレクトリ / スケッチ | 開発対象 (Master / Slave) ＆ 検証内容 | 状態 |
| :--- | :---: | :--- | :--- | :---: |
| **Phase 1** | `TC-P1-01`〜`03` | [`TC-P1-01/`](./TC-P1-01/), [`TC-P1-02/`](./TC-P1-02/) | **【基本ヘッダ送出 ＆ 物理層・UART受信】**<br>・Master: GPIO Break (14 Tbit LOW + 1 Tbit HIGH) + Sync + PID<br>・Slave: 標準 UART 受信確認 | **PASS (合格)** |
| **Phase 2** | `TC-P2-01`〜`05` | [`TC-P2/`](./TC-P2/) | **【スレーブ LINAUTO ハードウェア自動同期 ＆ PID 検証】**<br>・Master: ヘッダ送信（Phase 1 準拠）<br>・Slave: LINAUTO モード、Auto-baud 自動補正、PID パリティ検証 | **PASS (合格)** |
| **Phase 3** | `TC-P3-01`〜`03` | [`TC-P3/`](./TC-P3/) | **【Type A (Master Pub → Slave Sub) 実証 ＆ Slave Subscriber 実装】**<br>・**Master**: ミニマム・スケジューラ（PID 配列定期送出）＋ Type A ペイロード送信<br>・**Slave**: Subscriber 機能（`DATA==1` 受信、チェックサム検証、LED コマンド制御） | **PASS (合格)** |
| **Phase 4** | `TC-P4-01`〜`03` | [`TC-P4/`](./TC-P4/) | **【Type B (Slave Pub → Master Sub) 実証 ＆ Master Broker MVP 完成】**<br>・**Master**: プロミスキャス傍受（RX 移行・PC パススルー）＋ タイムアウト管理（**★Master Broker MVP 完成**）<br>・**Slave**: Double Buffer Mailbox ＋ Publisher 応答（50µs レスポンススペース・DE 制御・Zero-Copy 送信） | **PASS (合格)** |
| **Phase 5** | `TC-P5-01`〜`02` | [`TC-P5/`](./TC-P5/) | **【Type C (Slave A Pub → Slave B Sub) スレーブ間直接通信実証】**<br>・**Master**: Broker として場作り巡回 ＆ バス全傍受ログ出力<br>・**Slave A (Pub)**: センサ / SW 入力をトリガーにパブリッシュ<br>・**Slave B (Sub)**: Slave A の PID を直接受信してアクチュエータ制御（**★LN-485 UP/CS 完成**） | **PASS (合格)** |
| **Phase 6** | `TC-P6-01`〜`03` | `TC-P6/` | **【ハードウェア XDIR 自動方向制御 ＆ ロバストネス・最適化】**<br>・**Master & Slave**: `USART0.CTRLA.RS485`（XDIR）によるハードウェア DE 自動制御<br>・多重ボーレートスイープ（9600 〜 115200 bps）、強制バスクリア、活線挿抜耐性 | 準備中 |
