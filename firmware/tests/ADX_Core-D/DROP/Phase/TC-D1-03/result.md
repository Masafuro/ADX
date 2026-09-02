<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-03 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D1-03（CRC-16 誤り検出 ＆ 不正パケット破棄・自律復帰テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: 送信機, Node 2: 受信機)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (LINAUTO ハードウェア同期)
* **テストスケッチ:** [`tc-d1-03_crc_error_test.ino`](./tc-d1-03_crc_error_test.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ

### Node 1（送信機側: 4サイクルごとに意図的 CRC 反転注入）
```text
17:32:02.9 > [TX #62] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:04.0 > [TX #63] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:05.2 > [TX #64] [TEST: CRC CORRUPT] Sending Frame with Inverted CRC (0x33D5)...
17:32:06.4 > [TX #65] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:07.6 > [TX #66] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:08.8 > [TX #67] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:10.1 > [TX #68] [TEST: CRC CORRUPT] Sending Frame with Inverted CRC (0x33D5)...
17:32:11.3 > [TX #69] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:12.5 > [TX #70] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:13.7 > [TX #71] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)...
17:32:14.9 > [TX #72] [TEST: CRC CORRUPT] Sending Frame with Inverted CRC (0x33D5)...
```

### Node 2（受信機側: 破損検知・破棄 ＆ 正常自律復帰）
```text
17:32:19.9 > [RX CRC ERROR #7] Recv=0x33D5 != Calc=0xCC2A [MISMATCH] -> FRAME DISCARDED (Safety Preserved)!
17:32:20.2 > [Node 2] Heartbeat: Listening on RS-485 bus...
17:32:21.1 > [RX PASS #21] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:32:22.4 > [RX PASS #22] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:32:23.6 > [RX PASS #23] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:32:24.8 > [RX CRC ERROR #8] Recv=0x33D5 != Calc=0xCC2A [MISMATCH] -> FRAME DISCARDED (Safety Preserved)!
17:32:25.2 > [Node 2] Heartbeat: Listening on RS-485 bus...
17:32:26.0 > [RX PASS #24] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:32:27.2 > [RX PASS #25] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:32:28.4 > [RX PASS #26] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:32:29.7 > [RX CRC ERROR #9] Recv=0x33D5 != Calc=0xCC2A [MISMATCH] -> FRAME DISCARDED (Safety Preserved)!
17:32:30.2 > [Node 2] Heartbeat: Listening on RS-485 bus...
17:32:30.9 > [RX PASS #27] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
```

---

## 3. 技術的検証と評価

### 3.1 CRC-16-CCITT 誤り検出能力の実証
* 送信機が注入した CRC 反転破損パケット（`0x33D5`）に対し、受信機が例外なく `MISMATCH`（不一致）を検知。
* 破損パケットがアプリケーション層に一切受け渡されることなく、即時安全に破棄（`FRAME DISCARDED`）される動作が確認されました。

### 3.2 パケット破棄後の完全自律復帰
* 破損パケットを破棄した後、受信機はスタックのハングアップやデッドロックを起こすことなく、直ちに `WFB=1`（Wait for Break）へ再アーム。
* 次の正常パケット（`PASS #21` や `PASS #24`, `PASS #27`）を受信した瞬間に即座に正常受信動作へ復帰することが実証されました。

---

## 4. 総合判定

CRC-16 による破損パケット検出・即時破棄、および後続正常パケットでの自律復帰機能が完璧に実証されました。

**判定: PASS（完全合格）**
