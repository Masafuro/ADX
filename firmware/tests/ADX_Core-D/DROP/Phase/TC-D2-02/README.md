<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-02: 10,000周期連続周回 ＆ ジッター計測テスト
(10,000-Cycle Real-Time Relay Stability & Jitter Measurement Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 2 の第2ステップ（**Step 2: `TC-D2-02`**）です。

---

## 1. 主要機能とタイミング設計

1. **固定レスポンスギャップ (10ms):**
   * 受信・購読完了後、`delay(10)`（10ms）の安全ガード時間を確保してから次ノードへバトンを送出します。
   * トランシーバー（SP485EEN）の確実な切り替え解放、およびアプリ層のセンサ取得・制御計算時間を保証します。
   * 1周のサイクルタイムは約 $40 \sim 42\,\text{ms}$（約 $24 \sim 25\,\text{Hz}$）となります。
2. **正確なフレーム長パース (LEN + 5):**
   * ヘッダの `LEN` に基づき、末尾 CRC_L 到達と同時にフレーム受信完了を即座に判定します。
3. **再点火 Mediator（Re-Ignition Watchdog）:**
   * 起動時および通信途絶時（500ms 無音検知）、Node 1 が自動的に再点火パケット（Re-Ignition Frame）をバスへ投入し、バトンドロップから自律復帰します。
4. **白色 LED 点滅 (25 周期 = 約 1 秒ごと):**
   * 25 周期ごとに白色 LED（`PB3`）がチカチカとトグル点滅し、通信の健全稼働を示します。

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d2-02_jitter_measurement.ino`](./tc-d2-02_jitter_measurement.ino)

### 2.2 書き込み設定
1. **Node 1 (点火役 ＆ 再点火 Mediator / Slot 0x01)**:
   * スケッチ冒頭で `#define NODE_ID (1)` を設定してビルド・書き込み。
2. **Node 2 (受領役 / Slot 0x02)**:
   * スケッチ冒頭で `#define NODE_ID (2)` を設定してビルド・書き込み。

### 2.3 起動順序
1. Node 2 の電源を先に入れるか、両ノードの電源を同時に入れます。
2. Node 1 が起動後約3.5秒で初期点火を行い、定周期自律周回がスタートします。
3. Node 1 の SoftwareSerial モニタに 1,000 周期ごとに統計サマリーが表示されます。
4. 約 6〜7 分で 10,000 周期を完走します。

---

## 3. 期待される出力ログ（Node 1: COMポート）

```text
==============================================
   ADX Core-D DROP-Bus TC-D2-02 Jitter Test   
   (10,000-Cycle High-Speed Relay Stability)  
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
Timing: Fixed Response Gap = 10 ms | Bus Timeout = 500 ms.

[IGNITER] Igniting Baton Relay in 500ms...

[STAT #1000] Avg: 41.25ms (24.24Hz) | Min: 41180us, Max: 41320us | Jitter: +/-70us | Re-ignites: 0 | CRC Err: 0
[STAT #2000] Avg: 41.25ms (24.24Hz) | Min: 41180us, Max: 41320us | Jitter: +/-70us | Re-ignites: 0 | CRC Err: 0
[STAT #3000] Avg: 41.24ms (24.25Hz) | Min: 41175us, Max: 41325us | Jitter: +/-75us | Re-ignites: 0 | CRC Err: 0
...
[STAT #10000] Avg: 41.24ms (24.25Hz) | Min: 41170us, Max: 41330us | Jitter: +/-80us | Re-ignites: 0 | CRC Err: 0

==============================================
 ★ 10,000 CYCLES COMPLETED SUCCESSFULLY! ★ 
 Total Cycles : 10000
 Overall Avg  : 41245 us
 Jitter Range : +/-80 us
 Re-ignites   : 0
 CRC Errors   : 0
 Error Rate   : 0.00% [PASS]
==============================================
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 10,000 周期を途中でフリーズすることなく完走すること。
  2. 平均サイクルタイムが約 $40 \sim 43\,\text{ms}$（~24Hz）で安定していること。
  3. CRC エラー数（`CRC Err`）が 0 件（エラー率 0.00%）であること。
  4. ジッターが $\pm 100\,\mu\text{s}$ 以内に収まること。
  5. 白色 LED（`PB3`）が約 1 秒ごとにテンポよく点滅すること。
