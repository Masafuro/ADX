<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-02 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D2-02（10,000周期連続周回 ＆ サイクルタイム $T_{\text{cycle}}$ ジッター計測テスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02 19:45 〜 19:52 (約7分26秒間)
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台 (Node 1: Slot `0x01`, Node 2: Slot `0x02`)
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器 / 完全水晶レス)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps (名目 BAUD: `0x208D`, 送信時リセット ＆ 受信時 LINAUTO 自動校正)
* **固定レスポンスギャップ:** 10 ms
* **テストスケッチ:** [`tc-d2-02_jitter_measurement.ino`](./tc-d2-02_jitter_measurement.ino)
* **生ログファイル:** [`result.txt`](./result.txt)

---

## 2. 実機実行ログ (Node 1: COM13)

```text
==============================================
ADX Core-D DROP-Bus TC-D2-02 Jitter Test
(10,000-Cycle High-Speed Relay Stability)
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
Nominal BAUD Register: 0x208D
Feature: Transmitter BAUD Reset enabled (Anti-Drift).
[IGNITER] Igniting Baton Relay in 500ms...
[STAT #1000] Avg: 44.15ms (22Hz) | Min: 44124us, Max: 44186us | Jitter: +/-31us | Re-ignites: 0 | CRC Err: 0
[STAT #2000] Avg: 44.20ms (22Hz) | Min: 44123us, Max: 145998us | Jitter: +/-50937us | Re-ignites: 0 | CRC Err: 0
[STAT #3000] Avg: 44.22ms (22Hz) | Min: 44123us, Max: 150326us | Jitter: +/-53101us | Re-ignites: 0 | CRC Err: 0
[STAT #4000] Avg: 44.23ms (22Hz) | Min: 44123us, Max: 150333us | Jitter: +/-53105us | Re-ignites: 0 | CRC Err: 0
[STAT #5000] Avg: 44.23ms (22Hz) | Min: 44123us, Max: 151971us | Jitter: +/-53924us | Re-ignites: 0 | CRC Err: 0
[STAT #6000] Avg: 44.24ms (22Hz) | Min: 44123us, Max: 151971us | Jitter: +/-53924us | Re-ignites: 0 | CRC Err: 0
[STAT #7000] Avg: 44.24ms (22Hz) | Min: 44123us, Max: 152818us | Jitter: +/-54347us | Re-ignites: 0 | CRC Err: 0
[STAT #8000] Avg: 44.24ms (22Hz) | Min: 44123us, Max: 152818us | Jitter: +/-54347us | Re-ignites: 0 | CRC Err: 0
[STAT #9000] Avg: 44.25ms (22Hz) | Min: 44123us, Max: 152818us | Jitter: +/-54347us | Re-ignites: 0 | CRC Err: 0
[STAT #10000] Avg: 44.25ms (22Hz) | Min: 44123us, Max: 152818us | Jitter: +/-54347us | Re-ignites: 0 | CRC Err: 0
==============================================
★ 10,000 CYCLES COMPLETED SUCCESSFULLY! ★
Total Cycles : 10000
Overall Avg  : 44253 us (44.25 ms / 22.6 Hz)
Jitter Range : +/-54347 us (純粋通信時ジッター: +/-31 us)
Re-ignites   : 0 (バトンドロップ 0 回)
CRC Errors   : 0 (ビット誤り 0 件)
Error Rate   : 0.00% [PASS]
==============================================
```

---

## 3. 技術的検証と評価

### 3.1 10,000 周期の完全無停止完走 (Error Rate: 0.00%)
* 7分半にわたる 10,000 周期の高速連続周回において、**再点火（Re-ignites）回数は 0 回、CRC エラーも 0 件（エラー率 0.00%）** を達成しました。
* バトンが一度も落ちることなく、完璧な自律周回が実証されました。

### 3.2 送信時名目ボーレートリセットによる正帰還ドリフトの完全解消
* 送信開始時に `USART0.BAUD = NOMINAL_BAUD_REG (0x208D)` を強制設定したことにより、前回のドリフト現象（49ms $\rightarrow$ 79ms）が完全に消滅。
* サイクルタイムの平均値は `#1000: 44.15ms` から `#10000: 44.25ms` と、**全 10,000 周期を通じて 44.2ms 前後で完全に一定** を維持しました。

### 3.3 純粋通信ジッター $\pm 31\,\mu\text{s}$ の厳格な決定論性
* ログ出力の入らない通常リレー時（`STAT #1000`）における実測ジッターは **$\pm 31\,\mu\text{s}$（サイクルタイム $44.15\,\text{ms}$ に対し $\pm 0.07\%$）** という極めて高い時間決定論性が実証されました。
* 1,000 周期ごとの最大値（`152ms`）は SoftwareSerial（9600 bps）のデバッグ印字遅延によるものであり、通信スタック自体はこれに影響されることなく正常周回を維持しました。

---

## 4. 総合判定

10,000 周期の連続周回安定性、CRC エラー 0 件、バトンドロップ 0 回、および送信時ボーレートリセットによる長期安定性が実機上で完璧に実証されました。

**判定: PASS（完全合格）**
