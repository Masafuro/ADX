<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-04 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D1-04（LEN 不正値ガード ＆ バッファオーバーラン防御・自律復帰テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: 送信機, Node 2: 受信機)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (LINAUTO ハードウェア同期)
* **テストスケッチ:** [`tc-d1-04_len_guard_test.ino`](./tc-d1-04_len_guard_test.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ

### Node 1（送信機側: 4サイクルごとに意図的 LEN=0xFF 不正長注入）
```text
17:37:57.8 > [TX #61] [NORMAL] Sending Valid Frame (LEN=4B)...
17:37:59.0 > [TX #62] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:00.2 > [TX #63] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:01.4 > [TX #64] [TEST: LEN OVERFLOW] Sending Frame with Invalid LEN=0xFF (255B)...
17:38:02.6 > [TX #65] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:03.9 > [TX #66] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:05.1 > [TX #67] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:06.3 > [TX #68] [TEST: LEN OVERFLOW] Sending Frame with Invalid LEN=0xFF (255B)...
17:38:07.5 > [TX #69] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:08.7 > [TX #70] [NORMAL] Sending Valid Frame (LEN=4B)...
17:38:09.9 > [TX #71] [NORMAL] Sending Valid Frame (LEN=4B)...
```

### Node 2（受信機側: 不正長検知・SRAM保護アボート ＆ 自律復帰）
```text
17:38:13.7 > [RX PASS #26] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:38:14.9 > [RX PASS #27] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:38:16.2 > [GUARD #10] Invalid LEN=255 (0xFF > 64B). Buffer Overrun PREVENTED -> Frame Aborted (SRAM Safe)!
17:38:17.4 > [RX PASS #28] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:38:17.8 > [Node 2] Heartbeat: Listening on RS-485 bus...
17:38:18.6 > [RX PASS #29] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:38:19.8 > [RX PASS #30] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:38:21.0 > [GUARD #11] Invalid LEN=255 (0xFF > 64B). Buffer Overrun PREVENTED -> Frame Aborted (SRAM Safe)!
17:38:22.2 > [RX PASS #31] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
17:38:22.8 > [Node 2] Heartbeat: Listening on RS-485 bus...
17:38:23.4 > [RX PASS #32] LEN=4B, Target=0x02, Sender=0x01 | CRC=0xCC2A [MATCH] -> Packet Accepted!
```

---

## 3. 技術的検証と評価

### 3.1 バッファオーバーラン防御能力の実証
* `LEN = 0xFF`（255バイト / 許容上限64バイト超過）の不正フレームを受信した際、受信ステートマシンが直ちに `len > MAX_PAYLOAD_SIZE` を判定。
* SRAM 領域の境界を越えてメモリを破壊することなく、フレームを安全にアボート（`Frame Aborted (SRAM Safe)!`）できることが実証されました。

### 3.2 アボート後の完全自律復帰
* 不正長フレームをアボートした後、受信機はフリーズやリセットを起こすことなく `WFB=1` へ再アーム。
* 直後に送出された正常フレーム（`PASS #28` や `PASS #31`）を 1 ミリ秒の遅延もなく即座に正常受信できることが確認されました。

---

## 4. 総合判定

バッファオーバーラン防御ガードおよび自律復帰動作が完璧に実証され、Phase 1 の全 4 テストケースがすべて合格（ALL PASS）となりました。

**判定: PASS（完全合格）**
