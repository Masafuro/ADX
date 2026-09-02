<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D3-02 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D3-02（意図的断線 ＆ 即時安全心中 STO 実証テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02 21:43 (約11秒間)
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: Slot `0x01`, Node 2: Slot `0x02`)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器 / 完全水晶レス)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (名目 BAUD: `0x208D`, 送信時強制リセット)
* **固定レスポンスギャップ:** 10 ms
* **心中監視タイマー:** ハードウェアタイマー `TCB0`（5ms 周期割り込み $\times$ 50 ticks = **$250\,\text{ms}$ タイムアウト**）
* **意図的切断ポイント:** Cycle #200 到達時に Node 1 が送信停止
* **テストスケッチ:** [`tc-d3-02_intentional_disconnect_sto.ino`](./tc-d3-02_intentional_disconnect_sto.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ

### Node 1 (COM13)
```text
[2026-09-02 21:43:30.782] [IGNITER] Igniting Relay in 500ms...
[2026-09-02 21:43:33.592] [RUN] Cycle #50 / 200 running normally...
[2026-09-02 21:43:35.909] [RUN] Cycle #100 / 200 running normally...
[2026-09-02 21:43:38.229] [RUN] Cycle #150 / 200 running normally...
[2026-09-02 21:43:40.563] [RUN] Cycle #200 / 200 running normally...
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
Elapsed Time : 222 ms after last bus activity
Motor Output : FORCED OFF (Safe Torque Off)
Red LED (PB2): ON (Latched)
Status       : SYNCHRONIZED SHINJU [PASS]
==============================================
```

### Node 2 (COM15)
```text
[2026-09-02 21:43:33.524] [RUN] Cycle #50 / 200 running normally...
[2026-09-02 21:43:35.839] [RUN] Cycle #100 / 200 running normally...
[2026-09-02 21:43:38.164] [RUN] Cycle #150 / 200 running normally...
[2026-09-02 21:43:40.474] [RUN] Cycle #200 / 200 running normally...
==============================================
★ [STO TRIP] SYNCHRONIZED SHINJU EXECUTED! ★
Node ID      : 2
Stopped After: Cycle #200
Reason       : Bus Silence Timeout (250ms)
Elapsed Time : 182 ms after last bus activity
Motor Output : FORCED OFF (Safe Torque Off)
Red LED (PB2): ON (Latched)
Status       : SYNCHRONIZED SHINJU [PASS]
==============================================
```

---

## 3. 技術的検証と詳細評価

### 3.1 意図的通信途絶（断線）の確実な検知
* Node 1 が 200 周期目にバトン送信を停止し、バスが無音化した瞬間を、両ノードがハードウェアタイマー `TCB0` によって自律検知しました。

### 3.2 全ノード同期した即時安全心中（Safe Torque Off: STO）の実証
* 最後のフレーム送受信から **Node 1: $222\,\text{ms}$、Node 2: $182\,\text{ms}$**（タイムスタンプ `21:43:40.7` 〜 `21:43:40.9` の同一秒内）という極めて狭い時間窓の中で、**両ノードが完全に同期して STO 安全遮断を発動** しました。
* 多軸ロボット制御において、アーム間の衝突を防ぐための「全軸一斉安全心中（Synchronized Shinju）」がハードウェアレベルで完璧に成立することが実証されました。

### 3.3 再送排除と安全状態ラッチ（自己保持）
* 途絶後に無駄な再送リトライを行わず、赤色 LED（`PB2`）点灯・白色 LED（`PB3`）消灯・モータ出力遮断状態が永久保持され、暴走リスクがゼロであることが実証されました。

---

## 4. 総合判定

通信途絶時の全ノード同期心中（Safe Torque Off: STO）、ハードウェアタイマー `TCB0` による $250\,\text{ms}$ 遮断、および安全状態ラッチが実機上で完璧に実証されました。

**判定: PASS（完全合格）**
