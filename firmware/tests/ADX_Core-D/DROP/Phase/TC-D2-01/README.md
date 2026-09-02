<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-01: 2ノード基本バトンパス ＆ 相互購読テスト
(2-Node Autonomous Baton Relay & Pub/Sub Subscription Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 2 の最初のステップ（**Step 1: `TC-D2-01`**）です。

---

## 1. テスト目的

1. **自律分散ピンポン・バトンリレーの実証:**
   中央マスターが存在しない環境において、Node 1（Publisher: `0x01`）と Node 2（Publisher: `0x02`）が相互に発話権（バトン）をパスし合い、自律的な連続リレーが成立することを確認します。
2. **Common Subscriber（相互トピック購読）の実証:**
   各ノードが共有バス上のフレームを受信し、相手ノード（`SENDER_ID`）のペイロードデータを自機の `rxMailbox` へ正確に格納・購読できることを確認します。
3. **初期点火シーケンスの確認:**
   起動時に Node 1（Igniter）が最初のバトンをバスへ投入し、以降は双方が受動的リレーへ滑らかに移行することを確認します。

```text
【2ノード 自律ピンポンリレー シーケンス】
[Node 1 (Slot: 0x01)] ──(Target: 0x02, Sender: 0x01, Data1)──> [Node 2 (Slot: 0x02)]
                                                                    │ (Data1 購読 & バトン受領)
                                                                    ▼
[Node 1 (Slot: 0x01)] <──(Target: 0x01, Sender: 0x02, Data2)── [Node 2 (Slot: 0x02)]
      │ (Data2 購読 & バトン受領)
      ▼
[Node 1 (Slot: 0x01)] ──(Target: 0x02, Sender: 0x01, Data1')──> [Node 2 (Slot: 0x02)] ...
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d2-01_ping_pong_relay.ino`](./tc-d2-01_ping_pong_relay.ino)

### 2.2 書き込み設定
1. **Node 1 (点火役 / Slot 0x01)**:
   * スケッチ冒頭で `#define NODE_ID (1)` を設定してビルド・書き込み。
2. **Node 2 (受領役 / Slot 0x02)**:
   * スケッチ冒頭で `#define NODE_ID (2)` を設定してビルド・書き込み。

### 2.3 起動順序
1. Node 2（受領役）の電源を先に入れるか、両ノードの電源を同時に入れます。
2. Node 1 が起動後約3.5秒で初期点火フレーム（Target: `0x02`）をバスへ送出します。
3. Node 2 がそれを受信して直ちに Node 1（Target: `0x01`）へバトンを返送し、ピンポンリレーが開始されます。

---

## 3. 期待される出力ログ

### Node 1 SoftwareSerial (9600 bps)
```text
==============================================
   ADX Core-D DROP-Bus TC-D2-01 Relay Test    
   (2-Node Autonomous Baton Pass & Pub/Sub)   
==============================================
Node Configuration: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
[IGNITER] Firing Initial Baton into Bus in 500ms...
[TX #1 (IGNITION)] Passed Baton to Target=0x02
[RELAY #2] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x22 ] -> Passing Baton to Target=0x02
[RELAY #3] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x02 ] -> Passing Baton to Target=0x02
[RELAY #4] Recv from Sender=0x02 Data:[ 0x10 0x20 0x30 0x03 ] -> Passing Baton to Target=0x02
```

### Node 2 SoftwareSerial (9600 bps)
```text
==============================================
   ADX Core-D DROP-Bus TC-D2-01 Relay Test    
   (2-Node Autonomous Baton Pass & Pub/Sub)   
==============================================
Node Configuration: [NODE_ID = 2] SlotID=0x02 -> TargetID=0x01
[LISTENER] Waiting for incoming Baton from Node 1...

[RELAY #1] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x11 ] -> Passing Baton to Target=0x01
[RELAY #2] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x02 ] -> Passing Baton to Target=0x01
[RELAY #3] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x03 ] -> Passing Baton to Target=0x01
[RELAY #4] Recv from Sender=0x01 Data:[ 0x10 0x20 0x30 0x04 ] -> Passing Baton to Target=0x01
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. Node 1 の初期点火後、両ノードの `[RELAY #N]` カウンタが同期して連続カウントアップすること。
  2. 相手ノードの送信データ（`Sender=0x01` または `0x02`）が正しく受信・表示されること。
  3. 白LED（`PB3`）がバトン送受信ごとにトグル点滅し、赤LED（エラー）が点灯しないこと。
