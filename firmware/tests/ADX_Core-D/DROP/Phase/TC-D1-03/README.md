<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-03: CRC-16 誤り検出 ＆ 不正パケット破棄テスト
(CRC-16 Error Detection & Corrupted Packet Discard Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus 実機検証の第3ステップ（**Step 3**）です。

---

## 1. テスト目的

1. **CRC-16 誤り検出能力の実証:**
   通信線へのノイズ混入等によりパケットのビット化け（CRC破損）が発生した際、受信機（Node 2）が CRC-16 不一致（`MISMATCH`）を検出し、パケットを即時破棄（Discard）してアクチュエータやアプリ層への反映を防ぐことを確認します。
2. **赤LED エラー警告 ＆ 白LED 制御:**
   CRC エラー検知時に赤LED（`PB2`）が点灯し、白LED（`PB3`）が消灯することを確認します。
3. **正常パケット受信時の自律復帰:**
   破損パケットの後に続く正常パケットを受信した際、受信スタックがフリーズせず即座に正常受信（PASS / 白LED点灯）へ復帰できることを確認します。

---

## 2. 送信テストパターン スケジュール

送信機（Node 1）は 1.2 秒ごとに以下のパターンを自動送出します：

* **Cycle 1, 2, 3:** 正常フレーム（`CRC: 0xCC2A` 送信）
* **Cycle 4:** **【エラー注入】** CRC 全ビット反転フレーム（`CRC: 0x33D5` 送信）
* **Cycle 5, 6, 7:** 正常フレーム（復帰確認）
* **Cycle 8:** **【エラー注入】** CRC 全ビット反転フレーム...

---

## 3. 実機テスト手順

### 3.1 スケッチ
* ファイル: [`tc-d1-03_crc_error_test.ino`](./tc-d1-03_crc_error_test.ino)

### 3.2 書き込み設定
1. **Node 1 (送信機)**:
   * スケッチ冒頭の `#define ROLE_TX` を**有効化**してビルド・書き込み。
2. **Node 2 (受信機)**:
   * スケッチ冒頭の `// #define ROLE_TX` のように**コメントアウト**してビルド・書き込み。

---

## 4. 期待される出力ログ

### Node 1（送信機側） SoftwareSerial (9600 bps)
```text
==============================================
   ADX Core-D DROP-Bus TC-D1-03 Test          
   (CRC-16 Error Detection & Recovery Test)   
==============================================
Role: [TRANSMITTER (Node 1)]
Pattern Schedule:
  Cycle 1..3 : Valid Frames (LEN=4B, CRC OK)
  Cycle 4    : [ERROR INJECTION] Corrupted CRC Frame
  Cycle 5..  : Valid Frames (Recovery Confirmation)

[TX #1] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
[TX #2] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
[TX #3] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
[TX #4] ★ [TEST: CRC CORRUPT] Sending Frame with Inverted CRC (0x33D5)...
[TX #5] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
```

### Node 2（受信機側） SoftwareSerial (9600 bps)
```text
==============================================
   ADX Core-D DROP-Bus TC-D1-03 Test          
   (CRC-16 Error Detection & Recovery Test)   
==============================================
Role: [SLAVE LINAUTO RECEIVER (Node 2)]
Status: WFB=1. Monitoring for Valid and Corrupted Frames...

[RX PASS #1] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
[RX PASS #2] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
[RX PASS #3] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
★ [RX CRC ERROR #1] Recv=0x33D5 != Calc=0xCC2A [MISMATCH] -> FRAME DISCARDED (Safety Preserved)!
[RX PASS #4] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
[RX PASS #5] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
```

---

## 5. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 正常フレーム（Cycle 1..3, 5..）受信時に `[MATCH]` と判定され、白LED（`PB3`）が点灯すること。
  2. 破損フレーム（Cycle 4）受信時に `[MISMATCH] -> FRAME DISCARDED` と判定され、赤LED（`PB2`）が点灯すること。
  3. 破損パケット破棄後、次の正常パケット（Cycle 5）で受信機が何事もなく正常復帰（PASS #4）すること。
