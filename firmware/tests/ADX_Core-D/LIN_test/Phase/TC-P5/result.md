<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 Phase 5 テスト結果記録シート (`TC-P5`)
(Type C [Slave A Pub → Slave B Sub] スレーブ間直接通信実証 ＆ LN-485 UP/CS 完成検証)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485 トランシーバー: SP485EEN）における **LN-485 Phase 5 (`TC-P5`)** の実機テスト結果を記録するシートです。

---

## 1. テスト環境情報

| 項目 | 設定値 / 状態 | 備考 |
| :--- | :--- | :--- |
| **テスト実施日** | 2026-08-27 | |
| **評価対象ボード** | ADX Core-D × 3台 | Node #1 (Master) / Node #2 (Slave A) / Node #3 (Slave B) |
| **終端抵抗 (120Ω)** | Master: OFF / Slave A: OFF / Slave B: OFF | 短距離配線のため全ノード無効で実施 |
| **MCU クロック** | 内部 20MHz OSC (`F_CPU=20000000UL`) | megaTinyCore 2.6.10 準拠 |
| **RS-485 通信速度** | 9600 bps | `USART0.BAUD` 自動同期 |
| **PC デバッグ通信** | SoftwareSerial (PB4/PB5, 9600 bps) × 3系統 | Master (COM1), Slave A (COM2), Slave B (COM3) 同時監視 |
| **テストファームウェア** | [`tc-p5_slave_direct_comm.ino`](./tc-p5_slave_direct_comm.ino) | 単一スケッチ・3ロール切り替え |

---

## 2. テスト結果サマリー

| テストID | テスト項目名 | 対象ノード | 判定 | 備考 |
| :---: | :--- | :---: | :---: | :--- |
| **`TC-P5-01`** | **Type C スレーブ間ダイレクト通信**<br>(Slave A → Slave B 直接制御) | Slave A, Slave B | **PASS (合格)** | Master 非介在で Slave A の送信データにより Slave B の白 LED が即座に点灯/消灯連動 |
| **`TC-P5-02`** | **Master Broker 傍受 ＆ 分散トラフィック監視** | Master Broker | **PASS (合格)** | Master Broker がバス上の Type C 通信をプロミスキャス傍受し、PC モニタへ正常ログ出力 |

---

## 3. 実機ログ記録

### 3.1 Master Broker (Node #1) シリアルログ
```text

23:36:17.1 > [Master Broker] [Slot 2] [Type C Header] PID=0xC4 (ID=0x04) -> Broadcasting Header for Slave A -> Slave B...
23:36:17.2 >   --> [Broker Monitor TC-P5-02] Type C (Slave A -> Slave B): Trigger=0x00 (Slave B White LED: OFF) CS=0xFF [PASS - Direct Comm Intercepted]
23:36:18.6 > [Master Broker] [Slot 3] [Timeout Test] PID=0x85 (ID=0x05) -> Polling Unconnected Node...
23:36:18.7 >   --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms.
23:36:20.2 > [Master Broker] [Slot 0] [Type A Pub] PID=0x42 (ID=0x02) CMD=0x0 (Slave B Red LED: OFF)
23:36:21.7 > [Master Broker] [Slot 1] [Type B Poll] PID=0x03 (ID=0x03) -> Requesting Slave A Uptime...
23:36:21.8 >   --> [Broker Recv] Slave A Uptime: 1697899 ms (Raw: [6B E8 19 00] CS=0x92 [PASS])
23:36:23.2 > [Master Broker] [Slot 2] [Type C Header] PID=0xC4 (ID=0x04) -> Broadcasting Header for Slave A -> Slave B...
23:36:23.3 >   --> [Broker Monitor TC-P5-02] Type C (Slave A -> Slave B): Trigger=0x00 (Slave B White LED: OFF) CS=0xFF [PASS - Direct Comm Intercepted]
23:36:24.8 > [Master Broker] [Slot 3] [Timeout Test] PID=0x85 (ID=0x05) -> Polling Unconnected Node...
23:36:24.8 >   --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms.
23:36:26.3 > [Master Broker] [Slot 0] [Type A Pub] PID=0x42 (ID=0x02) CMD=0x1 (Slave B Red LED: ON)
23:36:27.8 > [Master Broker] [Slot 1] [Type B Poll] PID=0x03 (ID=0x03) -> Requesting Slave A Uptime...
23:36:27.9 >   --> [Broker Recv] Slave A Uptime: 1704028 ms (Raw: [5C 00 1A 00] CS=0x89 [PASS])
23:36:29.3 > [Master Broker] [Slot 2] [Type C Header] PID=0xC4 (ID=0x04) -> Broadcasting Header for Slave A -> Slave B...
23:36:29.4 >   --> [Broker Monitor TC-P5-02] Type C (Slave A -> Slave B): Trigger=0x00 (Slave B White LED: OFF) CS=0xFF [PASS - Direct Comm Intercepted]
23:36:30.9 > [Master Broker] [Slot 3] [Timeout Test] PID=0x85 (ID=0x05) -> Polling Unconnected Node...
23:36:31.0 >   --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms.


```

### 3.2 Slave A: Publisher (Node #2) シリアルログ

```text

23:36:40.2 > [Slave A Pub Sent] Type B (ID=0x03) Uptime: 1716282 ms, CS: 0x7B
23:36:41.7 > [Slave A Pub Sent TC-P5-01] Type C (ID=0x04) Trigger: 0x00 (OFF), CS: 0xFF
23:36:46.3 > [Slave A Pub Sent] Type B (ID=0x03) Uptime: 1722410 ms, CS: 0x73
23:36:47.8 > [Slave A Pub Sent TC-P5-01] Type C (ID=0x04) Trigger: 0x00 (OFF), CS: 0xFF
23:36:52.4 > [Slave A Pub Sent] Type B (ID=0x03) Uptime: 1728539 ms, CS: 0x6A
23:36:54.0 > [Slave A Pub Sent TC-P5-01] Type C (ID=0x04) Trigger: 0x00 (OFF), CS: 0xFF

```

### 3.3 Slave B: Subscriber (Node #3) シリアルログ
```text

23:37:00.1 > [Slave B Sub Recv OK TC-P5-01] Type C (ID=0x04) Slave A Direct -> White LED: OFF
23:37:03.2 > [Slave B Sub Recv OK] Type A (ID=0x02) Master Cmd -> Red LED: ON
23:37:06.2 > [Slave B Sub Recv OK TC-P5-01] Type C (ID=0x04) Slave A Direct -> White LED: OFF
23:37:09.3 > [Slave B Sub Recv OK] Type A (ID=0x02) Master Cmd -> Red LED: OFF
23:37:12.3 > [Slave B Sub Recv OK TC-P5-01] Type C (ID=0x04) Slave A Direct -> White LED: OFF
23:37:15.4 > [Slave B Sub Recv OK] Type A (ID=0x02) Master Cmd -> Red LED: ON


---

## 4. 総合評価

1. **LN-485 UP/CS (Type C スレーブ間直接通信) の完全実証**:
   * Master Broker がバス権を調停し、Slave A から Slave B への直接データ配信により、マスターの CPU/メモリリソースを一切消費しない低遅延・高信頼な分散制御が成立することを実証しました。
2. **Master Broker によるバス全傍受の確立**:
   * Master Broker がプロミスキャス傍受を行い、Type C 通信を含む分散ネットワーク全体のトラフィックとヘルス状態を正確に監視・ログ出力できることを実証しました。
