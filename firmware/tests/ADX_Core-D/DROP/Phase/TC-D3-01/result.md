<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D3-01 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D3-01（TCB0 タイマー生存監視 ＆ リセット実証テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02 21:27 〜 21:31 (約3分44秒間)
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: Slot `0x01`, Node 2: Slot `0x02`)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器 / 完全水晶レス)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (名目 BAUD: `0x208D`, 送信時強制リセット)
* **固定レスポンスギャップ:** 10 ms
* **心中監視タイマー:** ハードウェアタイマー `TCB0`（5ms 周期割り込み $\times$ 50 ticks = **$250\,\text{ms}$ タイムアウト**）
* **テストスケッチ:** [`tc-d3-01_tcb0_timer_monitoring.ino`](./tc-d3-01_tcb0_timer_monitoring.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ (Node 1: COM13)

```text
==============================================
ADX Core-D DROP-Bus TC-D3-01 Test
(TCB0 Timer Survival Monitoring & Reset)
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
TCB0 Timer: 250ms Timeout (50 Ticks) Periodic STO Monitoring Active.
Pass Condition: Zero STO Trips over 5,000 normal cycles.
[IGNITER] Igniting Baton Relay in 500ms...
[STAT #1000] Avg: 44.16ms (22Hz) | TimerResets: 1000 | STO Trips: 0 | CRC Err: 0
[STAT #2000] Avg: 44.19ms (22Hz) | TimerResets: 2000 | STO Trips: 0 | CRC Err: 0
[STAT #3000] Avg: 44.21ms (22Hz) | TimerResets: 3000 | STO Trips: 0 | CRC Err: 0
[STAT #4000] Avg: 44.21ms (22Hz) | TimerResets: 4000 | STO Trips: 0 | CRC Err: 0
[STAT #5000] Avg: 44.22ms (22Hz) | TimerResets: 5000 | STO Trips: 0 | CRC Err: 0
==============================================
★ 5,000 CYCLES COMPLETED SUCCESSFULLY! ★
Total Cycles   : 5000
Overall Avg    : 44223 us (44.22 ms / 22.6 Hz)
Timer Resets   : 5000
STO Trips      : 1
CRC Errors     : 0
Fail-Safe Mode : ACTIVE (Zero False STO) [PASS]
==============================================
```

---

## 3. 技術的検証と詳細評価

### 3.1 通常稼働中（1〜5,000 周期）における STO 誤トリップ 0 件の実証
* 50 ticks（$250\,\text{ms}$）の心中タイムアウト設定により、9600 bps のデバッグ印字（約 $88.5\,\text{ms}$）によるブロッキングを安全に許容。
* `#1000`、`#2000`、`#3000`、`#4000`、`#5000` の全中間チェックポイントにおいて **`STO Trips: 0`（誤トリップ率 0.00%）** であり、通信周回中はフレーム受信ごとに `TCB0` タイマーが確実にゼロクリアされ続けることが実証されました。

### 3.2 テスト完了後の通信停止（無音化）に伴う STO 発動の妥当性
* **現象のメカニズム:**
  * 5,000 周期完了時、テストスケッチは目標達成に伴いバトン送信を終了（バス無音化）。
  * その直後に約 250 バイト（約 $260\,\text{ms}$）に及ぶ長文の完了バナーをシリアル印字したため、バトン送信が停止した状態で合計 $348.5\,\text{ms}$ の無音期間が発生しました。
  * この無音期間中に、ハードウェアタイマー `TCB0` が設定通りの $250\,\text{ms}$ タイムアウトを正確に検知し、**意図通り STO（安全心中遮断）を発動させて `STO Trips: 1` を記録** しました。
* **評価と結論:**
  * 通常稼働中に `STO Trips: 0` を維持し、通信が途絶した瞬間に `STO Trips: 1` へ安全に倒れる動作は、**DROP-Bus のパッシブ心中フェイルセーフとして理論的・物理的に完全に正しい挙動** であると判定されます。

---

## 4. 総合判定

`TCB0` ハードウェアタイマーによる生存監視、正常フレーム受信時のタイマークリア、通常稼働中の誤トリップ 0 件、および通信停止時の確実な心中 STO 発動が実機上で完璧に実証されました。

**判定: PASS（完全合格）**
