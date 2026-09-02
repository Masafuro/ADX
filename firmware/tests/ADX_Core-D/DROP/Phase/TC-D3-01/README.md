<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D3-01: TCB0 タイマー生存監視 ＆ リセットテスト
(TCB0 Timer Survival Monitoring & Reset Verification Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 3 の第1ステップ（**Step 1: `TC-D3-01`**）です。

---

## 1. テスト目的

1. **TCB0 ハードウェアタイマーによる心中監視機構の実装:**
   * ATtiny1616 の 16-bit タイマー `TCB0` を周期割り込みモード（5ms 周期）で稼働させ、50 回カウント（$250\,\text{ms}$）で STO（安全遮断）を発動するハードウェア心中機構を構築します。
2. **正常周回時の生存タイマーリセット実証:**
   * 共有バス上に正常フレームが流れるたび（自宛て・他宛て問わず）、`stoTickCounter = 0` および `TCB0.CNT = 0` をクリア。
   * 5,000 周期の正常周回中において、**STO（赤LED点灯 / 出力遮断）の誤トリップが 0 回（STO Trips: 0）** で安定稼働し続けることを実証します。

```text
【TCB0 タイマー生存監視 ＆ リセット構造】
           [ 共有バス上の正常フレーム受信 (CRC MATCH) ]
                               │
                               ▼ (タイマーゼロリセット)
                 [ stoTickCounter = 0, TCB0.CNT = 0 ]
                               ▲
                               │ 5ms 周期割り込みでカウントアップ (+1)
                      [ TCB0 ハードウェアタイマー ]
                               │
                               ▼ (250ms 無音タイムアウト満了時)
                  【 ISR 内で即時 STO 安全遮断 (心中) 】
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d3-01_tcb0_timer_monitoring.ino`](./tc-d3-01_tcb0_timer_monitoring.ino)

### 2.2 書き込み設定
1. **Node 1 (点火役 / Slot 0x01)**:
   * スケッチ冒頭で `#define NODE_ID (1)` を設定してビルド・書き込み。
2. **Node 2 (受領役 / Slot 0x02)**:
   * スケッチ冒頭で `#define NODE_ID (2)` を設定してビルド・書き込み。

### 2.3 起動順序
1. Node 2 の電源を先に入れるか、両ノードの電源を同時に入れます。
2. Node 1 が起動後約3.5秒で初期点火を行い、自律ピンポンリレーがスタートします。
3. Node 1 の SoftwareSerial モニタに 1,000 周期ごとに統計サマリーが表示されます。
4. 約 3〜4 分で 5,000 周期を完走します。

---

## 3. 期待される出力ログ（Node 1: COMポート）

```text
==============================================
   ADX Core-D DROP-Bus TC-D3-01 Test          
   (TCB0 Timer Survival Monitoring & Reset)   
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
TCB0 Timer: 250ms Timeout (50 Ticks) Periodic STO Monitoring Active.
Pass Condition: Zero STO Trips over 5,000 normal cycles.

[IGNITER] Igniting Baton Relay in 500ms...

[STAT #1000] Avg: 44.18ms (22Hz) | TimerResets: 1000 | STO Trips: 0 | CRC Err: 0
[STAT #2000] Avg: 44.20ms (22Hz) | TimerResets: 2000 | STO Trips: 0 | CRC Err: 0
[STAT #3000] Avg: 44.22ms (22Hz) | TimerResets: 3000 | STO Trips: 0 | CRC Err: 0
[STAT #4000] Avg: 44.23ms (22Hz) | TimerResets: 4000 | STO Trips: 0 | CRC Err: 0
[STAT #5000] Avg: 44.23ms (22Hz) | TimerResets: 5000 | STO Trips: 0 | CRC Err: 0

==============================================
 ★ 5,000 CYCLES COMPLETED SUCCESSFULLY! ★ 
 Total Cycles   : 5000
 Overall Avg    : 44230 us
 Timer Resets   : 5000
 STO Trips      : 0
 CRC Errors     : 0
 Fail-Safe Mode : ACTIVE (Zero False STO) [PASS]
==============================================
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 5,000 周期を途中で止まることなく連続完走すること。
  2. STO 誤発動回数（`STO Trips`）が 0 回（誤トリップ率 0.00%）であること。
  3. タイマーリセット回数（`Timer Resets`）が正常にインクリメントされていること。
  4. CRC エラー数（`CRC Errors`）が 0 件であること。
  5. 白色 LED（`PB3`）が約 1 秒ごとにテンポよく点滅し、赤色 LED（`PB2`）が点灯しないこと。
