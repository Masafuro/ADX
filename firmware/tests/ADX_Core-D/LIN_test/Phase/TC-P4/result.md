<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 Phase 4 テスト結果記録シート (`TC-P4`)
(Type B [Slave Pub → Master Sub] 実証 ＆ Master Broker MVP 完成)

## 1. テスト実施概要

* **テスト実施日:** 2026年08月26日
* **テスト実施者:** Tester
* **対象ファームウェア:** [`tc-p4_master_broker_slave_pub.ino`](./tc-p4_master_broker_slave_pub.ino)
* **テスト環境・機材構成:**
  * **Master Broker 機 (Node #1):** ADX Core-D 実機 (`#define ROLE_MASTER` 有効)
  * **Slave Node 機 (Node #2):** ADX Core-D 実機 (`ROLE_MASTER` コメントアウト)
  * **RS-485 バス:** 3P 端子台（A, B, GND: 配線長 20cm, 終端抵抗 H4 オープン）
  * **シリアルモニタ:** SoftwareSerial (PB4/PB5, 9600 bps)

---

## 2. テスト結果サマリー

| テストID | テスト項目名 | 担当・対象 | 判定 (PASS/FAIL) | 備考・確認内容 |
| :---: | :--- | :---: | :---: | :--- |
| **`TC-P4-01`** | **Slave Publisher 応答 ＆ ターンアラウンド制御** | **Slave** | **PASS (合格)** | Double Buffer Mailbox からの 4B 稼働時間 + CS 送信、および $60\,\mu\text{s}$ レスポンススペース・DE 制御の正常動作を確認 |
| **`TC-P4-02`** | **Master Broker プロミスキャス傍受 ＆ タイムアウト管理** | **Master** | **PASS (合格)** | スレーブ応答の全傍受（4B Uptime + CS 完全一致）、および未接続 ID=0x05 における 15ms タイムアウト自律復帰を確認（**★Master Broker MVP 完成**） |
| **`TC-P4-03`** | **双方向対話・統合巡回エコーバック** | **全体** | **PASS (合格)** | Slot 0 (Type A LED) $\rightarrow$ Slot 1 (Type B Uptime) $\rightarrow$ Slot 2 (タイムアウト検証) の 1.5秒周期連続巡回動作を確認 |

**総合判定: 合格 (PASS) / ★ Master Broker MVP 完成**

---

## 3. 実機テスト実行ログ

### 3.1 マスター側シリアルモニタログ (Node #1)
```txt

[Master Broker] [Slot 0] [Type A Pub] PID=0x42 (ID=0x02) CMD=0x0 (LED: OFF)
[Master Broker] [Slot 1] [Type B Poll] PID=0x03 (ID=0x03: Slave Uptime Request) -> Promiscuous RX
  --> [RECV PROMISCUOUS OK] Slave Uptime: 9288 ms (Hex: [48 24 00 00 ] CS: 0x93)
[Master Broker] [Slot 2] [Type B Poll - Unconnected Node] PID=0x85 (ID=0x05: Timeout Test)
  --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms.
[Master Broker] [Slot 0] [Type A Pub] PID=0x42 (ID=0x02) CMD=0x1 (LED: ON)
[Master Broker] [Slot 1] [Type B Poll] PID=0x03 (ID=0x03: Slave Uptime Request) -> Promiscuous RX
  --> [RECV PROMISCUOUS OK] Slave Uptime: 13861 ms (Hex: [25 36 00 00 ] CS: 0xA4)
[Master Broker] [Slot 2] [Type B Poll - Unconnected Node] PID=0x85 (ID=0x05: Timeout Test)
  --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms.
[Master Broker] [Slot 0] [Type A Pub] PID=0x42 (ID=0x02) CMD=0x0 (LED: OFF)
[Master Broker] [Slot 1] [Type B Poll] PID=0x03 (ID=0x03: Slave Uptime Request) -> Promiscuous RX
  --> [RECV PROMISCUOUS OK] Slave Uptime: 18442 ms (Hex: [0A 48 00 00 ] CS: 0xAD)
[Master Broker] [Slot 2] [Type B Poll - Unconnected Node] PID=0x85 (ID=0x05: Timeout Test)
  --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms.
[Master Broker] [Slot 0] [Type A Pub] PID=0x42 (ID=0x02) CMD=0x1 (LED: ON)
[Master Broker] [Slot 1] 


```

### 3.2 スレーブ側シリアルモニタログ (Node #2)
```txt
[Slave Sub Recv OK] Type A (ID=0x02) LED: ON
[Slave Pub Sent] Type B (ID=0x03) Uptime: 50494 ms, CS: 0xFB
[Slave Sub Recv OK] Type A (ID=0x02) LED: OFF
[Slave Pub Sent] Type B (ID=0x03) Uptime: 55073 ms, CS: 0x7
[Slave Sub Recv OK] Type A (ID=0x02) LED: ON
[Slave Pub Sent] Type B (ID=0x03) Uptime: 59650 ms, CS: 0x14
[Slave Sub Recv OK] Type A (ID=0x02) LED: OFF
[Slave Pub Sent] Type B (ID=0x03) Uptime: 64231 ms, CS: 0x1D
[Slave Sub Recv OK] Type A (ID=0x02) LED: ON
[Slave Pub Sent] Type B (ID=0x03) Uptime: 68806 ms, CS: 0x2C
[Slave Sub Recv OK] Type A (ID=0x02) LED: OFF
[Slave Pub Sent] Type B (ID=0x03) Uptime: 73386 ms, CS: 0x36
[Slave Sub Recv OK] Type A (ID=0x02) LED: ON
[Slave Pub Sent] Type B (ID=0x03) Uptime: 77962 ms, CS: 0x44
[Slave Sub Recv OK] Type A (ID=0x02) LED: OFF
[Slave Pub Sent] Type B (ID=0x03) Uptime: 82541 ms, CS: 0x4F
[Slave Sub Recv OK] Type A (ID=0x02) LED: ON



```

---

## 4. 考察・特記事項

1. **Double Buffer Mailbox（完全非同期・Zero-Copy 送信）の完璧な動作**:
   - スレーブ側アプリケーション層が 100ms 周期で裏バッファ（`payload[nextIdx]`）へ稼働時間を更新し、`activeIdx` を瞬時切り替えする設計が意図通り完全に機能した。
   - 通信層は `uptimeMailbox.activeIdx` の表バッファからデータを直接 `USART0.TXDATAL` へ送出（Zero-Copy）し、データ欠落や Torn Read（データの食い違い破損）が一切発生しないことが実証された。
   - マスター側ログの Hex バイト列と Classic Checksum 計算値が 100% 一致した（例: Uptime `9288 ms` $\rightarrow$ `[48 24 00 00]` $\rightarrow$ Sum: `0x6C` $\rightarrow$ CS: `0x93` 完全一致）。

2. **レスポンススペース（$60\,\mu\text{s}$）と DE ターンアラウンドの完全適合**:
   - マスター機が PID 送出後に `Serial.flush()` を経て受信モード（`DE=0`）へ移行し、スレーブ機が $60\,\mu\text{s}$ のレスポンススペースを置いてから送信モード（`DE=1`）へ移行するシーケンスにより、バス衝突（ショート波形・破損）がゼロであることが確認された。
   - スレーブ側も送信末尾バイト（チェックサム）送出後に `slaveTxFlush()`（`TXCIF` 待機）を経て即座に `setRxMode()`（`DE=0`）へ戻すことで、次スロットへの影響を完全に排除できた。

3. **Master Broker MVP の非ブロッキング・プロミスキャス傍受 ＆ タイムアウト管理の完成**:
   - Slot 1 において、スレーブからパブリッシュされた 4 バイト稼働時間 ＋ CS を全傍受（Promiscuous RX）し、即座に PC モニタへパススルー出力できた。
   - Slot 2（未接続ノード ID=0x05 の要求）において、マスターが永久ブロッキングすることなく、**正確に 15ms でタイムアウトを検知して自律復帰** し、次の周期（Slot 0）へとスムーズに移行した（`TC-P4-02` 合格）。
   - これにより、LN-485 における **Master Broker MVP（最小動作実用版）が完全に完成** した。
