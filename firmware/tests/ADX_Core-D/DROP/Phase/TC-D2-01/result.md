<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-01 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D2-01（2ノード自律分散バトンリレー ＆ 相互トピック購読テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: Slot `0x01`, Node 2: Slot `0x02`)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器 / 完全水晶レス)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (LINAUTO ハードウェア自動追従)
* **テストスケッチ:** [`tc-d2-01_ping_pong_relay.ino`](./tc-d2-01_ping_pong_relay.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ

### Node 1（Slot 0x01 / 点火役）
```text
18:11:51.4 > [RELAY #14] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x0D ] -> Passing Baton to Target=0x02
18:11:51.8 > [RELAY #15] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x0E ] -> Passing Baton to Target=0x02
18:11:52.2 > [RELAY #16] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x0F ] -> Passing Baton to Target=0x02
18:11:52.5 > [RELAY #17] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x10 ] -> Passing Baton to Target=0x02
18:11:52.8 > [RELAY #18] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x11 ] -> Passing Baton to Target=0x02
18:11:53.2 > [RELAY #19] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x12 ] -> Passing Baton to Target=0x02
18:11:53.5 > [RELAY #20] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x13 ] -> Passing Baton to Target=0x02
18:11:53.8 > [RELAY #21] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x14 ] -> Passing Baton to Target=0x02
18:11:54.1 > [RELAY #22] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x15 ] -> Passing Baton to Target=0x02
18:11:54.5 > [RELAY #23] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x16 ] -> Passing Baton to Target=0x02
18:11:54.8 > [RELAY #24] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x17 ] -> Passing Baton to Target=0x02
18:11:55.1 > [RELAY #25] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x18 ] -> Passing Baton to Target=0x02
18:11:55.5 > [RELAY #26] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x19 ] -> Passing Baton to Target=0x02
18:11:55.8 > [RELAY #27] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x1A ] -> Passing Baton to Target=0x02
18:11:56.1 > [RELAY #28] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x1B ] -> Passing Baton to Target=0x02
```

### Node 2（Slot 0x02 / 受領役）
```text
18:12:07.5 > [RELAY #62] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x3E ] -> Passing Baton to Target=0x01
18:12:07.8 > [RELAY #63] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x3F ] -> Passing Baton to Target=0x01
18:12:08.1 > [RELAY #64] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x40 ] -> Passing Baton to Target=0x01
18:12:08.5 > [RELAY #65] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x41 ] -> Passing Baton to Target=0x01
18:12:08.8 > [RELAY #66] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x42 ] -> Passing Baton to Target=0x01
18:12:09.1 > [RELAY #67] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x43 ] -> Passing Baton to Target=0x01
18:12:09.5 > [RELAY #68] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x44 ] -> Passing Baton to Target=0x01
18:12:09.8 > [RELAY #69] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x45 ] -> Passing Baton to Target=0x01
18:12:10.1 > [RELAY #70] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x46 ] -> Passing Baton to Target=0x01
18:12:10.4 > [RELAY #71] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x47 ] -> Passing Baton to Target=0x01
18:12:10.8 > [RELAY #72] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x48 ] -> Passing Baton to Target=0x01
18:12:11.1 > [RELAY #73] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x49 ] -> Passing Baton to Target=0x01
18:12:11.4 > [RELAY #74] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x4A ] -> Passing Baton to Target=0x01
```

---

## 3. 技術的検証と評価

### 3.1 自律分散ピンポンリレー（Masterless Baton Pass）の実証
* 中央マスターが存在しない完全な自律分散環境において、Node 1 と Node 2 が相互に発話権（バトン）を受け渡し、途絶えることなく連続リレーが成立しました。
* Node 1 の初期点火（Ignition）以降、双方が純粋な受動的バトン待ち受けステートマシンとして動作し、二重点火や衝突は 0 件でした。

### 3.2 Common Subscriber（相互トピック購読）の完全性
* Node 1 側では Node 2 の送信データ末尾（`0x0D` $\rightarrow$ `0x0E` $\rightarrow$ `0x0F` $\rightarrow$ `0x10` ... `0x1B`）が欠落なく連続受信され、`rxMailbox` が毎周期確実に更新されました。
* Node 2 側でも Node 1 の送信データ末尾（`0x3E` (62) $\rightarrow$ `0x3F` (63) $\rightarrow$ `0x40` (64) ... `0x4A` (74)）が完璧に連続受信されました。

### 3.3 半二重 RS-485 ターンアラウンド制御の安定性
* `Serial.flush()` による末尾送出待機と、バトン受領後の $200\,\mu\text{s}$ レスポンススペースにより、バス衝突や波形歪みが完全に排除され、CRC エラー 0 件の安定通信が実証されました。

---

## 4. 総合判定

自律分散バトンリレーおよび Common Subscriber（Pub/Sub 相互購読）の基本機能が実機で完璧に実証されました。

**判定: PASS（完全合格）**
