<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-02: スレーブ LINAUTO 自動同期 ＆ CRC-16 検証テスト
(Slave LINAUTO Auto-Baudrate Synchronization & CRC-16 Verification Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus 実機検証の第2ステップ（**Step 2**）です。

---

## 1. テスト目的

1. **LINAUTO 自動ボーレート校正の実証:**
   受信機（Node 2）の `USART0` を `USART_RXMODE_LINAUTO_gc` に設定し、フレーム先頭の Break ＋ `0x55`（Sync Byte）によってハードウェアが自動的にボーレートレジスタ（`USART0.BAUD`）を校正し、内蔵RCドリフトを吸収できることを確認します。
2. **可変長ペイロード ＆ CRC-16-CCITT 照合:**
   4バイト、8バイト、16バイトの各可変長フレームに対し、受信機が `LEN` に応じたバイト数を正しく受信し、末尾の `CRC-16`（計算値 vs 受信値）が 100% 一致（`[MATCH]`）することを確認します。

```text
【送出フレーム (可変長 4B / 8B / 16B サイクル)】
+---------+------+-------+-----+-----------+-----------+-----------------------+---------+
|  BREAK  | DEL  | SYNC  | LEN | TARGET_ID | SENDER_ID | PAYLOAD (4, 8, 16 B)  | CRC-16  |
+---------+------+-------+-----+-----------+-----------+-----------------------+---------+
  14 Tbit   2Tbit  0x55   可変      0x02        0x01      0x11.. / 0xA0.. / 0x20..   2B
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d1-02_slave_linauto_test.ino`](./tc-d1-02_slave_linauto_test.ino)

### 2.2 書き込み設定
1. **Node 1 (送信機)**:
   * スケッチ冒頭の `#define ROLE_TX` を**有効化**してビルド・書き込み。
2. **Node 2 (受信機)**:
   * スケッチ冒頭の `// #define ROLE_TX` のように**コメントアウト**してビルド・書き込み。

---

## 3. 期待される出力ログ

### Node 1（送信機側） SoftwareSerial (9600 bps)
```text
==============================================
   ADX Core-D DROP-Bus TC-D1-02 Test          
   (Slave LINAUTO Auto-Baud & CRC-16 Verify)  
==============================================
Role: [TRANSMITTER (Node 1)]
Transmitting Frames (LEN=4, 8, 16B) every 1000ms...

[TX #1] Sent Frame: LEN=4B, Target=0x02, Sender=0x01
[TX #2] Sent Frame: LEN=8B, Target=0x02, Sender=0x01
[TX #3] Sent Frame: LEN=16B, Target=0x02, Sender=0x01
[TX #4] Sent Frame: LEN=4B, Target=0x02, Sender=0x01
```

### Node 2（受信機側） SoftwareSerial (9600 bps)
LINAUTO により `0x55` が自動消費され、後続の `LEN` から CRC までのバイト列が正確にパース・照合されます。

```text
==============================================
   ADX Core-D DROP-Bus TC-D1-02 Test          
   (Slave LINAUTO Auto-Baud & CRC-16 Verify)  
==============================================
Role: [SLAVE LINAUTO RECEIVER (Node 2)]
[Config] Initial USART0.BAUD = 0x208D
[Status] WFB=1. Listening for Break + 0x55 on RS-485 bus...

[RX PASS #1] Calibrated BAUD=0x208D | LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] Data: [ 0x11 0x22 0x33 0x44 ]
[RX PASS #2] Calibrated BAUD=0x208D | LEN=8B, Target=0x02, Sender=0x01 | CRC=0xXXXX [MATCH] Data: [ 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 0xA6 0xA7 ]
[RX PASS #3] Calibrated BAUD=0x208D | LEN=16B, Target=0x02, Sender=0x01 | CRC=0xYYYY [MATCH] Data: [ 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27 ... ]
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. `[RX PASS]` が 1 秒間隔で安定してカウントアップすること。
  2. `Calibrated BAUD` がハードウェア自動校正され、ボーレートの狂いがないこと。
  3. `LEN = 4B, 8B, 16B` の各サイズにおいて、データ列と CRC-16 が `[MATCH]` すること。
  4. 受信時に白色 LED（`PB3`）が正常点灯すること。
