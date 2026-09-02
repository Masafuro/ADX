<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus Phase 1: 一体型フレーム送受信テスト (TC-D1)

本ディレクトリには、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における DROP-Bus Phase 1（一体型フレーム送受信・LINAUTO同期・CRC-16検証・バッファ防御）の実機検証用スケッチおよび手順書をまとめています。

---

## 1. テスト概要と構成

* **テストスケッチ:** [`tc-d1_unified_frame_test.ino`](./tc-d1_unified_frame_test.ino)
* **対象テストケース:**
  * `TC-D1-01`: 一体型フレーム生成 ＆ 通常UART受信
  * `TC-D1-02`: スレーブ LINAUTO 同期 ＆ 可変長（1〜64B）受信
  * `TC-D1-03`: CRC-16 誤り検出 ＆ 不正パケット破棄
  * `TC-D1-04`: LEN 不正値ガード（オーバーラン防御）

---

## 2. ビルド設定切り替え方法

[`tc-d1_unified_frame_test.ino`](./tc-d1_unified_frame_test.ino) の冒頭にあるマクロ定義（`#define`）を有効化/コメントアウトすることで、送信機・受信機・各テストモードを切り替えます。

```cpp
// =============================================================================
// テスト動作ロールの選択 (いずれか1つを有効化)
// =============================================================================
#define ROLE_TX_TEST          // 送信テスト機 (1秒周期で可変長フレーム送出)
// #define ROLE_RX_DUMP       // 通常UART受信ダンプ機 (TC-D1-01 検証用)
// #define ROLE_RX_LINAUTO    // LINAUTO & CRC-16検証機 (TC-D1-02〜04 検証用)

// --- 送信機用エラー注入オプション (ROLE_TX_TEST 有効時のみ) ---
// #define TX_CORRUPT_CRC     // TC-D1-03: CRC 反転破損テスト
// #define TX_OVERFLOW_LEN    // TC-D1-04: LEN=0xFF オーバーフローテスト
```

---

## 3. 実機検証手順 (Step-by-Step)

### Step 1: 生バイト列のダンプ確認 (`TC-D1-01`)
1. **Node 1 (送信機):** `#define ROLE_TX_TEST` を有効にして書き込み。
2. **Node 2 (受信機):** `#define ROLE_RX_DUMP` を有効にして書き込み。
3. Node 2 の SoftwareSerial モニタ（9600 bps）を開き、1秒ごとに以下のようなバイト列が受信されることを確認：
   ```text
   0x00(BREAK/FERR) 0x55 0x08 0x02 0x01 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 0xXX 0xYY
   ```
4. **判定:** Break 検出（`0x00`）に続き、`0x55`, `LEN`, `TARGET`, `SENDER`, データ, CRC が完全一致でダンプできれば **PASS**。

---

### Step 2: LINAUTO 同期 ＆ 可変長受信確認 (`TC-D1-02`)
1. **Node 2 (受信機):** `#define ROLE_RX_LINAUTO` に変更して再書き込み。
2. Node 2 のモニタを開き、1B, 8B, 16B, 32B, 64B の可変長パケットが CRC `[MATCH]` で連続受信されることを確認：
   ```text
   [RX PASS #1] LEN=1B, Target=0x2, Sender=0x1, CRC=0x38F1 [MATCH] Data: [ 0x10 ]
   [RX PASS #2] LEN=8B, Target=0x2, Sender=0x1, CRC=0xA452 [MATCH] Data: [ 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 ]
   [RX PASS #3] LEN=16B, Target=0x2, Sender=0x1, CRC=0x90B3 [MATCH] Data: [ 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 ... ]
   [RX PASS #4] LEN=32B, Target=0x2, Sender=0x1, CRC=0xE204 [MATCH] Data: [ 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 ... ]
   [RX PASS #5] LEN=64B, Target=0x2, Sender=0x1, CRC=0x6B15 [MATCH] Data: [ 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 ... ]
   ```
3. **判定:** 全長において CRC-16 が一致し、白LEDが点灯すれば **PASS**。

---

### Step 3: CRC 誤り検出 ＆ 不正パケット破棄 (`TC-D1-03`)
1. **Node 1 (送信機):** `#define TX_CORRUPT_CRC` を追加有効化して書き込み。
2. Node 2 のモニタに以下が表示されることを確認：
   ```text
   [CRC ERROR #1] Recv=0xC70E, Calc=0x38F1 -> Frame Discarded!
   ```
3. **判定:** 赤LED（エラー）が点灯し、パケットが破棄されれば **PASS**。

---

### Step 4: バッファオーバーラン防御 (`TC-D1-04`)
1. **Node 1 (送信機):** `#define TX_OVERFLOW_LEN` を有効化して書き込み。
2. Node 2 のモニタに以下が表示されることを確認：
   ```text
   [GUARD] Invalid LEN=255 (>64). Buffer overrun prevented! Aborting frame...
   ```
3. **判定:** マイコンが暴走せず即時アボートして待機状態へ戻れば **PASS**。
