<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D3-02: 意図的断線 ＆ 即時安全心中 (STO) テスト
(Intentional Disconnection & Synchronized STO Verification Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 3 の第2ステップ（**Step 2: `TC-D3-02`**）です。

---

## 1. テスト目的

1. **意図的通信途絶によるバトンドロップの誘発:**
   * Node 1 と Node 2 が通常周回を行い、**200 周期目に達した瞬間に Node 1 が次バトンの送信を意図的に停止** して共有バスを完全無音化します。
   * （※ 周回中に物理的に RS-485 ケーブルを抜去した場合でも同様に実証可能）
2. **全ノード同期・即時安全心中（STO）の実証:**
   * 通信が途絶した瞬間から **$250\,\text{ms}$（50 ticks）以内に、Node 1 および Node 2 の双方がハードウェアタイマー `TCB0` の満了によってミリ秒単位で完全に同期して STO 安全遮断を発動** することを確認します。
3. **安全状態ラッチ（自己保持）の確認:**
   * STO 発動後、再送を行わず、赤色 LED（`PB2`）点灯・白色 LED（`PB3`）消灯・モータ出力強制遮断状態を永久保持することを確認します。

```text
【意図的断線 ＆ 即時安全心中 (STO) シーケンス】
[Node 1] ───(Cycle #199: 44ms)───> [Node 2]
[Node 1] <───(Cycle #200: 44ms)─── [Node 2]
   │
   ▼ ★ Node 1 が Cycle #200 で送信を完全停止！（バス無音化）
===================================================================
      │ (無音時間カウント中: 0ms -> 50ms -> 100ms -> 200ms)
      ▼ (250ms 満了)
【Node 1: TCB0 満了 -> 赤LED 点灯 & STO 遮断】
【Node 2: TCB0 満了 -> 赤LED 点灯 & STO 遮断】 ★ ほぼ同時に完全同期心中！
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d3-02_intentional_disconnect_sto.ino`](./tc-d3-02_intentional_disconnect_sto.ino)

### 2.2 書き込み設定
1. **Node 1 (点火役 / Slot 0x01 / 200周で停止)**:
   * スケッチ冒頭で `#define NODE_ID (1)` を設定してビルド・書き込み。
2. **Node 2 (受領役 / Slot 0x02 / 同期心中受領)**:
   * スケッチ冒頭で `#define NODE_ID (2)` を設定してビルド・書き込み。

### 2.3 起動順序
1. Node 2 の電源を先に入れるか、両ノードの電源を同時に入れます。
2. Node 1 が起動後約3.5秒で初期点火を行い、自律ピンポンリレー（200周）がスタートします。
3. 約 8〜9 秒後に 200 周期に達し、Node 1 が送信を停止します。
4. 送信停止から約 $250\,\text{ms}$ 後に、両ノードの赤色 LED（`PB2`）が点灯し、白色 LED（`PB3`）が消灯します。

---

## 3. 期待される出力ログ

### Node 1 (COMポート)
```text
==============================================
   ADX Core-D DROP-Bus TC-D3-02 Test          
   (Intentional Disconnect & Synchronized STO)
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
TCB0 Shinju Timer: 250ms Timeout Active.
Test Scenario: Node 1 will INTENTIONALLY STOP transmitting at Cycle #200.
Pass Criteria: Both nodes trip STO (Red LED ON) within 250ms.

[IGNITER] Igniting Relay in 500ms...
[RUN] Cycle #50 / 200 running normally...
[RUN] Cycle #100 / 200 running normally...
[RUN] Cycle #150 / 200 running normally...
[RUN] Cycle #200 / 200 running normally...

==============================================
 ★ [TRIGGER] DISCONNECT INDUCED AT CYCLE #200! ★
 Node 1 is ceasing transmission to drop baton.
 Waiting for 250ms silence timeout STO trip...
==============================================

==============================================
 ★ [STO TRIP] SYNCHRONIZED SHINJU EXECUTED! ★ 
 Node ID      : 1
 Stopped After: Cycle #200
 Reason       : Bus Silence Timeout (250ms)
 Elapsed Time : 250 ms after last bus activity
 Motor Output : FORCED OFF (Safe Torque Off)
 Red LED (PB2): ON (Latched)
 Status       : SYNCHRONIZED SHINJU [PASS]
==============================================
```

### Node 2 (COMポート)
```text
==============================================
   ADX Core-D DROP-Bus TC-D3-02 Test          
   (Intentional Disconnect & Synchronized STO)
==============================================
Node Config: [NODE_ID = 2] SlotID=0x02 -> TargetID=0x01
TCB0 Shinju Timer: 250ms Timeout Active.
Test Scenario: Node 2 will detect sudden silence and trip STO synchronously.
Pass Criteria: Both nodes trip STO (Red LED ON) within 250ms.

[LISTENER] Ready. Waiting for Initial Baton from Node 1...

[RUN] Cycle #50 / 200 running normally...
[RUN] Cycle #100 / 200 running normally...
[RUN] Cycle #150 / 200 running normally...
[RUN] Cycle #200 / 200 running normally...

==============================================
 ★ [STO TRIP] SYNCHRONIZED SHINJU EXECUTED! ★ 
 Node ID      : 2
 Stopped After: Cycle #200
 Reason       : Bus Silence Timeout (250ms)
 Elapsed Time : 250 ms after last bus activity
 Motor Output : FORCED OFF (Safe Torque Off)
 Red LED (PB2): ON (Latched)
 Status       : SYNCHRONIZED SHINJU [PASS]
==============================================
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 200 周期まで正常に周回し、Node 1 の送信停止に伴いバスが無音化すること。
  2. 無音化から **$250\,\text{ms} \pm 10\,\text{ms}$** 以内に、Node 1 / Node 2 の双方がハードウェアタイマー割り込みにより STO 遮断を発動すること。
  3. STO 発動と同時に、赤色 LED（`PB2`）が点灯し、白色 LED（`PB3`）が消灯すること。
  4. 遮断後に再送リトライを行わず、安全停止状態（ラッチ）を永久維持すること。
