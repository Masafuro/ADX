<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-01: 一体型フレーム生バイト列ダンプテスト
(Unified Frame Raw Byte Dump Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus 実機検証の最初のステップ（**Step 1**）です。

---

## 1. テスト目的

送信機（Node 1）が送出する「14 Tbit Break ＋ 2 Tbit Delimiter ＋ `0x55` ＋ `LEN(0x04)` ＋ `TARGET(0x02)` ＋ `SENDER(0x01)` ＋ `DATA(11 22 33 44)` ＋ `CRC-16`」の一体型フレームが、受信機（Node 2: 通常 9600 bps UART）において物理的に欠落なく正確に届いているかを**生バイト列のダンプ表示**によって確認します。

```text
【送信フレーム構成 (固定 4 バイトデータ)】
+---------+------+-------+-----+-----------+-----------+-------------------+---------+
|  BREAK  | DEL  | SYNC  | LEN | TARGET_ID | SENDER_ID |      PAYLOAD      | CRC-16  |
+---------+------+-------+-----+-----------+-----------+-------------------+---------+
  14 Tbit   2Tbit  0x55   0x04    0x02        0x01      0x11 0x22 0x33 0x44  0x30 0x82
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d1-01_raw_frame_dump.ino`](./tc-d1-01_raw_frame_dump.ino)

### 2.2 書き込み設定
1. **Node 1 (送信機)**:
   * スケッチ冒頭の `#define ROLE_TX` を**有効**にしてビルド・書き込み。
2. **Node 2 (受信機)**:
   * スケッチ冒頭の `// #define ROLE_TX` のように**コメントアウト**してビルド・書き込み。

---

## 3. 期待される出力ログ

### Node 1（送信機側） SoftwareSerial (9600 bps)
```text
==========================================
   ADX Core-D DROP-Bus TC-D1-01 Test      
==========================================
Role: [TRANSMITTER (Node 1)]
Sending Frame every 1000ms:
  [Break] 0x55 | LEN:0x04 | TGT:0x02 | SND:0x01 | DATA:11 22 33 44 | CRC

[TX] Frame Sent.
[TX] Frame Sent.
```

### Node 2（受信機側） SoftwareSerial (9600 bps)
1秒ごとに送信されたパケットの全バイト列が順次ダンプされます。

```text
==========================================
   ADX Core-D DROP-Bus TC-D1-01 Test      
==========================================
Role: [RAW DUMP RECEIVER (Node 2)]
Listening on RS-485 bus (9600 bps)...

[RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0x30 0x82 
[RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0x30 0x82 
[RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0x30 0x82 
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 先頭に `0x00(BREAK)`（または Framing Error）が検出されること。
  2. 続くバイトが `0x55`, `0x04` (LEN), `0x02` (TARGET), `0x01` (SENDER) であること。
  3. ペイロード `0x11 0x22 0x33 0x44` が欠落なく並んでいること。
  4. 末尾に 2 バイトの CRC-16 が届いていること。
