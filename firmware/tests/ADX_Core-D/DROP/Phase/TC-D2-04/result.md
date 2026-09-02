<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-04 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D2-04（Multi-rate スロット / 1ノード複数スロット所有による不等周期リレー実証テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02 20:32 〜 20:35 (約3分18秒間)
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台
  * **Node 1:** Slot `0x01`（制御 4B） ＆ Slot `0x03`（ログ 8B）の 2 スロット所有
  * **Node 2:** Slot `0x02`（センサ 4B）の 1 スロット所有
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器 / 完全水晶レス)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (名目 BAUD: `0x208D`, 送信時強制リセット)
* **固定レスポンスギャップ:** 10 ms
* **テストスケッチ:** [`tc-d2-04_multirate_slot_test.ino`](./tc-d2-04_multirate_slot_test.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ (Node 1: COM13)

```text
==============================================
ADX Core-D DROP-Bus TC-D2-04 Test
(Multi-rate Asymmetric Relay & Pub/Sub)
==============================================
Node Configuration: [NODE_ID = 1] Owns Slots: 0x01 (4B Control) & 0x03 (8B Log)
Relay Sequence: [0x01 (4B)] -> [0x02] -> [0x03 (8B)] -> [0x02]
Timing: Fixed Gap = 10ms | Bus Timeout = 500ms.
[IGNITER] Igniting Multi-rate Baton Relay with Slot 0x01 in 500ms...
[ROUND #500] Avg: 92.41ms (10Hz) | Rx02: 1000 | CRC Err: 0
[ROUND #1000] Avg: 92.47ms (10Hz) | Rx02: 2000 | CRC Err: 0
[ROUND #1500] Avg: 92.49ms (10Hz) | Rx02: 3000 | CRC Err: 0
[ROUND #2000] Avg: 92.50ms (10Hz) | Rx02: 4000 | CRC Err: 0
==============================================
★ 2,000 ROUNDS (8,000 SLOTS) COMPLETED! ★
Total Rounds : 2000
Round Avg    : 92502 us (92.50 ms / 10.8 Hz)
Rx Slot 0x02 : 4000 (数学的に正確な 2倍受信)
Re-ignites   : 0 (バトンドロップ 0 回)
CRC Errors   : 0 (ビット誤り 0 件)
Status       : ALL MATCH [PASS]
==============================================
```

---

## 3. 技術的検証と評価

### 3.1 Multi-rate スロット（1ノード複数スロット所有）の実証
* Node 1 が 2 つのスロット（`0x01`: 高速制御 4B, `0x03`: 低速ログ 8B）を所有し、Node 2（`0x02`: センサ 4B）との間で 1 大周期中に 2 回発話する非対称リレーが自律成立しました。
* `[0x01] -> [0x02] -> [0x03] -> [0x02]` の 4 スロット周回が 2,000 大周期（計 8,000 スロット）にわたり、バトンドロップ 0 回（Re-ignites: 0）で連続完走しました。

### 3.2 可変長フレーム（4B / 8B 混在）の完全整合性
* 1周の中に 4B フレームと 8B フレームが交互に混在する動的な伝送環境において、全 8,000 スロットで **CRC エラーは 0 件（Error Rate: 0.00%）** でした。

### 3.3 非対称スケジューリングの正確な周回比率
* 2,000 大周期に対して、Node 2 からの受信回数（`Rx Slot 0x02`）が **正確に 2 倍の 4,000 回** 受信され、非対称スケジューリングが数学的・物理的に完璧に同期していることが実証されました。

---

## 4. 総合判定

Multi-rate スロット所有、非対称リレー周回、可変長フレーム混在通信、およびトピック別購読が実機上で完璧に実証され、Phase 2 の全 4 テストケースがすべて合格（ALL PASS）となりました。

**判定: PASS（完全合格）**
