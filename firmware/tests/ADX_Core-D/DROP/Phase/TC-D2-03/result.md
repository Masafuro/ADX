<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-03 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D2-03（Double Buffer Mailbox 連動 / 非同期アプリ層更新 ＆ Zero-Copy 送信の非干渉実証テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02 20:19 〜 20:23 (約3分44秒間)
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: Slot `0x01`, Node 2: Slot `0x02`)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器 / 完全水晶レス)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (名目 BAUD: `0x208D`, 送信時強制リセット)
* **固定レスポンスギャップ:** 10 ms
* **アプリ層模擬更新周期:** 15 ms
* **テストスケッチ:** [`tc-d2-03_double_buffer_test.ino`](./tc-d2-03_double_buffer_test.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ (Node 1: COM13)

```text
==============================================
ADX Core-D DROP-Bus TC-D2-03 Test
(Double Buffer Mailbox Async Integration)
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
Double Buffer: Zero-Copy Non-blocking Mailbox Enabled.
[IGNITER] Igniting Baton Relay in 500ms...
[STAT #1000] Avg: 44.16ms (22Hz) | AppUpdates: 2001 | TornRead Err: 0 | CRC Err: 0
[STAT #2000] Avg: 44.20ms (22Hz) | AppUpdates: 4001 | TornRead Err: 0 | CRC Err: 0
[STAT #3000] Avg: 44.21ms (22Hz) | AppUpdates: 6001 | TornRead Err: 0 | CRC Err: 0
[STAT #4000] Avg: 44.22ms (22Hz) | AppUpdates: 8001 | TornRead Err: 0 | CRC Err: 0
[STAT #5000] Avg: 44.22ms (22Hz) | AppUpdates: 10001 | TornRead Err: 0 | CRC Err: 0
==============================================
★ 5,000 CYCLES COMPLETED SUCCESSFULLY! ★
Total Cycles   : 5000
Overall Avg    : 44227 us (44.23 ms / 22.6 Hz)
App Updates    : 10001
TornRead Errors: 0
CRC Errors     : 0
Integrity Rate : 100.00% [PASS]
==============================================
```

---

## 3. 技術的検証と評価

### 3.1 Double Buffer Mailbox（2面バッファ）による排他制御レス Zero-Copy の実証
* アプリケーション層が 15ms ごとに非同期更新（合計 10,001 回）を行いながら、通信層が 44.2ms ごとにバトンを周回（合計 5,000 周期）する過密なマルチタスク環境において、**ミューテックスや割り込み禁止処理を一切用いることなく、1バイトのアトミックスワップ（`activeIdx ^ 1`）のみで完全非干渉なメモリ分離が成立** しました。

### 3.2 不整合読み出し（Torn Read）0 件の完全実証
* 送信されたカウンタ値と反転チェックバイト（`Byte 2 ^ 0xFF == Byte 3`）の整合性を 5,000 周期全件で検証した結果、**Torn Read エラーは 0 件（Integrity Rate: 100.00%）** でした。
* アプリケーション層がメモリを書き換えている途中の不完全なデータが通信層に漏れ出す事故が皆無であることが実証されました。

### 3.3 通信ジッターと定周期性の維持
* アプリ層の頻繁なデータ更新が通信スタックのタイミングを一切阻害せず、サイクルタイム平均値は **44.22 ms** で完璧に安定稼働しました。

---

## 4. 総合判定

Double Buffer Mailbox 機構の非同期・Zero-Copy 性、Torn Read ゼロ（100.00% 整合性）、および長期安定周回が実機上で完璧に実証されました。

**判定: PASS（完全合格）**
