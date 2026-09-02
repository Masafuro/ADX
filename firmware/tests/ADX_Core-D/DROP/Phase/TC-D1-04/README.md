<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-04: LEN 不正値ガード ＆ バッファオーバーラン防御テスト
(LEN Guard & Buffer Overrun Prevention Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus 実機検証 Phase 1 の最終ステップ（**Step 4**）です。

---

## 1. テスト目的

1. **バッファオーバーラン防御の実証:**
   通信路上でノイズ混入等により `LEN` フィールドが破損し、最大許容データ長（64バイト）を超える不正長（例: `LEN = 0xFF` / 255バイト）となった際、受信機（Node 2）が `len > MAX_PAYLOAD_SIZE` を直ちに検知して受信を安全にアボート（中絶）し、SRAM 破壊やマイコン暴走を防止することを確認します。
2. **赤LED ガード警告 ＆ 白LED 制御:**
   不正長検知時に赤LED（`PB2`）が点灯し、白LED（`PB3`）が消灯することを確認します。
3. **正常フレーム受信時の自律復帰:**
   不正フレームを破棄・アボートした後、後続の正常フレームを受信した際に即座に正常受信（PASS）へ復帰できることを確認します。

---

## 2. 送信テストパターン スケジュール

送信機（Node 1）は 1.2 秒ごとに以下のパターンを自動送出します：

* **Cycle 1, 2, 3:** 正常フレーム（`LEN = 4B` 送信）
* **Cycle 4:** **【エラー注入】** 不正長フレーム（`LEN = 0xFF` / 255B 送信）
* **Cycle 5, 6, 7:** 正常フレーム（復帰確認）
* **Cycle 8:** **【エラー注入】** 不正長フレーム...

---

## 3. 実機テスト手順

### 3.1 スケッチ
* ファイル: [`tc-d1-04_len_guard_test.ino`](./tc-d1-04_len_guard_test.ino)

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
   ADX Core-D DROP-Bus TC-D1-04 Test          
   (LEN Guard & Buffer Overrun Defense Test)  
==============================================
Role: [TRANSMITTER (Node 1)]
Pattern Schedule:
  Cycle 1..3 : Valid Frames (LEN=4B)
  Cycle 4    : [ERROR INJECTION] Overflow Malformed Frame (LEN=0xFF)
  Cycle 5..  : Valid Frames (Recovery Confirmation)

[TX #1] [NORMAL] Sending Valid Frame (LEN=4B)...
[TX #2] [NORMAL] Sending Valid Frame (LEN=4B)...
[TX #3] [NORMAL] Sending Valid Frame (LEN=4B)...
[TX #4] ★ [TEST: LEN OVERFLOW] Sending Frame with Invalid LEN=0xFF (255B)...
[TX #5] [NORMAL] Sending Valid Frame (LEN=4B)...
```

### Node 2（受信機側） SoftwareSerial (9600 bps)
```text
==============================================
   ADX Core-D DROP-Bus TC-D1-04 Test          
   (LEN Guard & Buffer Overrun Defense Test)  
==============================================
Role: [SLAVE LINAUTO RECEIVER (Node 2)]
Status: WFB=1. Guarding against LEN > 64 Buffer Overrun...

[RX PASS #1] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
[RX PASS #2] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
[RX PASS #3] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
★ [GUARD #1] Invalid LEN=255 (0xFF > 64B). Buffer Overrun PREVENTED -> Frame Aborted (SRAM Safe)!
[RX PASS #4] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
[RX PASS #5] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
```

---

## 5. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 正常フレーム（Cycle 1..3, 5..）受信時に `[MATCH]` と判定され、白LED（`PB3`）が点灯すること。
  2. 不正長フレーム（Cycle 4）受信時に `[GUARD] Invalid LEN=255` と判定されてアボートされ、赤LED（`PB2`）が点灯すること。
  3. 不正フレーム検知時でも SRAM 破壊・マイコン暴走を起こさず、次の正常フレーム（Cycle 5）で正常受信（PASS #4）へ自律復帰すること。
