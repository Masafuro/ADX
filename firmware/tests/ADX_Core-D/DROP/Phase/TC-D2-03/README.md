<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D2-03: Double Buffer Mailbox 連動テスト
(Double Buffer Mailbox Async Integration & Zero-Copy Relay Test)

本テストは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 2 の第3ステップ（**Step 3: `TC-D2-03`**）です。

---

## 1. テスト目的

1. **Double Buffer Mailbox（2面バッファ）による排他制御レス Zero-Copy 通信の実証:**
   * アプリケーション層（15ms 非同期センサ更新模擬）と通信層（DROP-Bus バトンリレー）が、ミューテックスや割り込み禁止を使わずに 1 バイトのアトミックスワップ（`activeIdx ^ 1`）のみで完全非同期・安全にデータを交換できることを実証します。
2. **不整合読み出し（Torn Read）の完全防止検証:**
   * アプリ層が書き込むデータに「カウンタ値」と「反転チェックバイト（`counter ^ 0xFF`）」を同封。
   * 受信側で `(Byte 2 ^ 0xFF) == Byte 3` を全周期で検証し、更新途中のデータ破損（Torn Read）が 0 件（エラー率 0.00%）であることを確認します。
3. **長期稼働における完全性確認 (5,000 周期):**
   * 5,000 周期連続周回において、Torn Read エラー 0 件、CRC エラー 0 件で完走することを確認します。

```text
【Double Buffer Mailbox 構造】
[ アプリケーション層 (15ms 周期) ] ──(最新データを書き込み)──> [ 非アクティブ面 (writeIdx) ]
                                                                      │
                                                             (アトミックスワップ: activeIdx = writeIdx)
                                                                      ▼
[ 通信層 (DROP-Bus 送信) ] <──(Zero-Copy で直接参照・送信)── [ アクティブ面 (activeIdx) ]
```

---

## 2. 実機テスト手順

### 2.1 スケッチ
* ファイル: [`tc-d2-03_double_buffer_test.ino`](./tc-d2-03_double_buffer_test.ino)

### 2.2 書き込み設定
1. **Node 1 (点火役 ＆ 再点火 Mediator / Slot 0x01)**:
   * スケッチ冒頭で `#define NODE_ID (1)` を設定してビルド・書き込み。
2. **Node 2 (受領役 / Slot 0x02)**:
   * スケッチ冒頭で `#define NODE_ID (2)` を設定してビルド・書き込み。

### 2.3 起動順序
1. Node 2 の電源を先に入れるか、両ノードの電源を同時に入れます。
2. Node 1 が起動後約3.5秒で初期点火を行い、自律周回がスタートします。
3. Node 1 の SoftwareSerial モニタに 1,000 周期ごとに統計サマリーが表示されます。
4. 約 3〜4 分で 5,000 周期を完走します。

---

## 3. 期待される出力ログ（Node 1: COMポート）

```text
==============================================
   ADX Core-D DROP-Bus TC-D2-03 Test          
   (Double Buffer Mailbox Async Integration)  
==============================================
Node Config: [NODE_ID = 1] SlotID=0x01 -> TargetID=0x02
Double Buffer: Zero-Copy Non-blocking Mailbox Enabled.

[IGNITER] Igniting Baton Relay in 500ms...

[STAT #1000] Avg: 44.20ms (22Hz) | AppUpdates: 2945 | TornRead Err: 0 | CRC Err: 0
[STAT #2000] Avg: 44.22ms (22Hz) | AppUpdates: 5892 | TornRead Err: 0 | CRC Err: 0
[STAT #3000] Avg: 44.23ms (22Hz) | AppUpdates: 8840 | TornRead Err: 0 | CRC Err: 0
[STAT #4000] Avg: 44.24ms (22Hz) | AppUpdates: 11788 | TornRead Err: 0 | CRC Err: 0
[STAT #5000] Avg: 44.24ms (22Hz) | AppUpdates: 14735 | TornRead Err: 0 | CRC Err: 0

==============================================
 ★ 5,000 CYCLES COMPLETED SUCCESSFULLY! ★ 
 Total Cycles   : 5000
 Overall Avg    : 44245 us
 App Updates    : 14735
 TornRead Errors: 0
 CRC Errors     : 0
 Integrity Rate : 100.00% [PASS]
==============================================
```

---

## 4. 合否判定基準 (Pass Criteria)

* **PASS 条件:**
  1. 5,000 周期を途中で止まることなく連続完走すること。
  2. 不整合読み出しエラー（`TornRead Errors`）が 0 件（整合率 100.00%）であること。
  3. CRC エラー数（`CRC Errors`）が 0 件であること。
  4. アプリケーション更新回数（`App Updates`）が正常にインクリメントされていること。
  5. 白色 LED（`PB3`）が約 1 秒ごとにテンポよく点滅すること。
