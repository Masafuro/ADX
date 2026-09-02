<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-04: Multi-rate スロット（不等周期リレー）テスト
(Multi-rate Asymmetric Relay & Pub/Sub Subscription Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 2 の最終ステップ（**Step 4: `TC-D2-04`**）です。

---

## 1. テスト目的

1. **Multi-rate スロット（1ノード複数スロット所有）の実証:**
   * Node 1 が 2 つの異なる論理バトンID（`0x01`: 高速制御 4B, `0x03`: 低速ログ 8B）を所有し、Node 2（`0x02`: センサ 4B）との間で 1 大周期中に複数回発話する非対称リレーが自律成立することを実証します。
2. **非対称周回シーケンスの確認:**
   * 1 大周期（4 スロット）のシーケンス：
     `[0x01 (Node 1: 4B)] ──> [0x02 (Node 2: 4B)] ──> [0x03 (Node 1: 8B)] ──> [0x02 (Node 2: 4B)] ──> [0x01 ...]`
   * 可変長データ（4B と 8B）が混在するバス上で、バトンパスとトピック配信が完全同期して周回することを確認します。
3. **Common Subscriber による複数トピックの個別購読:**
   * Node 2 が受信したフレームの発信元（`SENDER_ID`）を識別し、`0x01`（制御データ）と `0x03`（ログデータ）をそれぞれ別のメールボックス（`rxMailbox_01`, `rxMailbox_03`）へ正確に分離格納できることを確認します。

```text
【Multi-rate 4スロット 非対称リレーモデル】
Slot 1: [Node 1 (0x01)] ──(Target: 0x02, Sender: 0x01, 4B)──> [Node 2 (0x02)]
                                                                   │ (0x01 購読)
Slot 2: [Node 1 (0x03)] <──(Target: 0x03, Sender: 0x02, 4B)── [Node 2 (0x02)]
             │
Slot 3: [Node 1 (0x03)] ──(Target: 0x02, Sender: 0x03, 8B)──> [Node 2 (0x02)]
                                                                   │ (0x03 購読)
Slot 4: [Node 1 (0x01)] <──(Target: 0x01, Sender: 0x02, 4B)── [Node 2 (0x02)]
             │
       (1大周期 完了 -> 次の周回へ)
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d2-04_multirate_slot_test.ino`](./tc-d2-04_multirate_slot_test.ino)

### 2.2 書き込み設定
1. **Node 1 (点火役 / Slot 0x01 ＆ 0x03 所有)**:
   * スケッチ冒頭で `#define NODE_ID (1)` を設定してビルド・書き込み。
2. **Node 2 (受領役 / Slot 0x02 所有)**:
   * スケッチ冒頭で `#define NODE_ID (2)` を設定してビルド・書き込み。

### 2.3 起動順序
1. Node 2 の電源を先に入れるか、両ノードの電源を同時に入れます。
2. Node 1 が起動後約3.5秒で初期点火（スロット 0x01 を Target: 0x02 へ送出）を行い、4スロット非対称周回がスタートします。
3. Node 1 の SoftwareSerial モニタに 500 大周期（2,000 スロット）ごとに統計サマリーが表示されます。
4. 約 3〜4 分で 2,000 大周期（計 8,000 スロット）を完走します。

---

## 3. 期待される出力ログ（Node 1: COMポート）

```text
==============================================
   ADX Core-D DROP-Bus TC-D2-04 Test          
   (Multi-rate Asymmetric Relay & Pub/Sub)    
==============================================
Node Configuration: [NODE_ID = 1] Owns Slots: 0x01 (4B Control) & 0x03 (8B Log)
Relay Sequence: [0x01 (4B)] -> [0x02] -> [0x03 (8B)] -> [0x02]
Timing: Fixed Gap = 10ms | Bus Timeout = 500ms.

[IGNITER] Igniting Multi-rate Baton Relay with Slot 0x01 in 500ms...

[ROUND #500] Avg: 88.50ms (11Hz) | Rx02: 1000 | CRC Err: 0
[ROUND #1000] Avg: 88.52ms (11Hz) | Rx02: 2000 | CRC Err: 0
[ROUND #1500] Avg: 88.53ms (11Hz) | Rx02: 3000 | CRC Err: 0
[ROUND #2000] Avg: 88.54ms (11Hz) | Rx02: 4000 | CRC Err: 0

==============================================
 ★ 2,000 ROUNDS (8,000 SLOTS) COMPLETED! ★ 
 Total Rounds : 2000
 Round Avg    : 88540 us
 Rx Slot 0x02 : 4000
 Re-ignites   : 0
 CRC Errors   : 0
 Status       : ALL MATCH [PASS]
==============================================
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 2,000 大周期（8,000 スロット）を途中で止まることなく連続完走すること。
  2. 1 大周期（4 スロット: 0x01 $\rightarrow$ 0x02 $\rightarrow$ 0x03 $\rightarrow$ 0x02）の平均時間（`Round Avg`）が約 $88\,\text{ms}$（4スロット $\times$ 約22ms）で安定していること。
  3. Node 2 からの受信回数（`Rx Slot 0x02`）が、大周回数（2,000 周）の正確に 2 倍（4,000 回）受信されていること。
  4. CRC エラー数（`CRC Errors`）が 0 件であること。
  5. 白色 LED（`PB3`）が約 2 秒ごとにテンポよく点滅すること。
