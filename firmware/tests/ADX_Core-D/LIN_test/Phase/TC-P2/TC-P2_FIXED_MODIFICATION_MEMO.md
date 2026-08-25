<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# TC-P2 ハードウェア LINAUTO 実機動作修正技術メモ
(TC-P2 Hardware LINAUTO Fix & Technical Analysis Memo)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616）における Phase 2 テスト（ハードウェア LIN スレーブエンジン `LINAUTO` の動作検証）において、初版スケッチ（[`tc-p2_slave_linauto_test.ino`](./tc-p2_slave_linauto_test.ino)）から [`fixed.ino`](./fixed.ino) への修正点、動かなかった技術的原因、および AVR マイコン特有のレジスタ挙動に関する知見をまとめた技術メモです。

---

## 1. 修正点一覧サマリー (Summary of Changes)

| # | 修正項目 | 初版コード (`tc-p2_slave_linauto_test.ino`) | 修正後コード (`fixed.ino`) | 修正の目的・理由 |
| :---: | :--- | :--- | :--- | :--- |
| **1** | **スレーブ初期化シーケンス** | `Serial.swap(1)` のみ実行し、直接レジスタを設定 | `Serial.swap(1)` $\rightarrow$ `Serial.begin(BAUDRATE)` $\rightarrow$ 一時ディスエーブル後に LINAUTO レジスタ設定 | megaTinyCore のペリフェラル・ピン初期化を確実に完了させてから LINAUTO モードへ移行するため |
| **2** | **STATUS レジスタ設定・クリア** | `USART0.STATUS |= USART_WFB_bm;`（論理和 `\|=`） | `USART0.STATUS = USART_WFB_bm \| USART_ISFIF_bm \| USART_BDF_bm;`（直接代入） | AVR の「1書き込みでフラグクリア」仕様による誤動作を防ぎ、残留フラグを確実にクリアしつつ WFB をセットするため |
| **3** | **待機復帰シーケンス** | 個別にビット操作 | WFB セット ＆ BDF/ISFIF クリアを一括代入 | フレーム終了時およびエラー復帰時に、次フレームへ確実にクリーンな状態で移行するため |
| **4** | **受信エラー分岐ロジック** | `DATA` ビット（bit 0）のみで分岐 | `PERR`（パリティエラー）および `FERR`（フレーミングエラー）を最優先で多段判定 | 異常フレームやノイズによる誤判定を完全に排除し、安全に破棄するため |

---

## 2. 動かなかった技術的原因の深掘り解析

### 原因 1: megaTinyCore のペリフェラル初期化との競合
* **現象**: `Serial.swap(1)` だけを呼び出して `Serial.begin()` を呼ばずに `USART0` のレジスタを直叩きした場合、PORTMUX のルーティング、ボーレートジェネレータのクロック供給、またはペリフェラル有効化の内部シーケンスが未完了のままとなり、受信割り込みやフラグが立たない事象が発生しました。
* **解決策**:
  ```c
  // ① 一度 begin を呼んで USART0 を安全に完全初期化
  Serial.swap(1);
  Serial.begin(BAUDRATE);
  Serial.flush();

  // ② レジスタ上書きのため一時ディスエーブル
  USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);

  // ③ LINAUTO モードで安全に再設定
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
  ```

---

### 原因 2: AVR の「1 書き込みフラグクリア仕様」と `|=`（論理和）の落とし穴
* **現象**: 
  AVR マイコンのステータスレジスタ（`USART0.STATUS` など）のフラグビット（`ISFIF`, `BDF`, `RXCIF` 等）は、**「該当ビットに '1' を書き込むことでフラグがクリアされる（Write 1 to Clear）」** というハードウェア仕様になっています。
* **落とし穴**:
  `USART0.STATUS |= USART_WFB_bm;` というコードは、C言語の展開上 **`USART0.STATUS = USART0.STATUS | USART_WFB_bm;`（リード・モディファイ・ライト）** になります。
  もし STATUS レジスタ内で既に他のフラグ（例: `BDF` や `ISFIF`）が立っていた場合、その値が読み出され、そのまま書き戻されるため、**「意図せず勝手にフラグがクリアされてしまう」** または **「クリアしたくないタイミングでフラグが消える」** という競合バグを引き起こします。
* **解決策**:
  `|=` 演算子を使わず、明示的に **「セットしたい制御ビット（`WFB`）と、クリアしたいフラグビット（`ISFIF`, `BDF`）を 1 まとめで直接代入（`=`）」** します。
  ```c
  // 正しい復帰シーケンス (WFB をセットし、同時に BDF と ISFIF を完全クリア)
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
  ```

---

### 原因 3: 受信フラグおよびエラー判定の優先順位
* **現象**: `USART0.RXDATAH` の最下位ビット `DATA` だけに依存して判定していると、パリティエラー時やフレーミングエラー時にもデータが流れてきてしまい、不正フレームを正常と誤判定するリスクがありました。
* **解決策**:
  `PERR`（パリティエラー）と `FERR`（フレーミングエラー）を最優先でチェックし、エラー発生時は即座にフレームを破棄して `USART0.STATUS = ...` で `WFB=1` に戻す構造に修正されました。

---

## 3. 実機ログの検証結果 (`result.txt` より)

修正後のスケッチ（[`fixed.ino`](./fixed.ino)）によって、以下の通り完璧なハードウェア動作が実証されました。

```txt
[Slave Event #19] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E4 | BDF: 0 -> [ PASS / OK ]
[Slave Event #20] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
[Slave Event #21] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E1 | BDF: 0 -> [ PASS / OK ]
[Slave Event #22] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E4 | BDF: 0 -> [ PASS / OK ]
[Slave Event #23] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
```

1. **AutoBAUD 自動更新の成功**:
   * マスターの送信に対し、`USART0.BAUD` が **`0x20E0` 〜 `0x20E4`** の範囲で毎フレーム動的に自動更新され、Auto-baud エンジンが完全に機能していることが確認されました。
2. **ISFIF エラー検知と自律リカバリの成功**:
   * マスターが送信した不正 Sync（`0xAA`）を瞬時に `STATUS.ISFIF` として検知し、安全にフラグをクリアして `WFB=1` に復帰、次フレームの正常受信を維持できています。

---

## 4. Phase 3 以降のスケッチ開発への教訓・適用ルール

本修正で得られた以下の 3 大原則は、今後の **Phase 3（Master-Publish）**、**Phase 4（Slave-Publish）**、および **LN-485 ブートローダ** の実装において必須の設計規約として全スケッチに適用します。

1. **`Serial.begin()` $\rightarrow$ レジスタ上書きの標準化**:
   * すべてのスケッチで、`Serial.swap(1); Serial.begin();` による安全初期化を必須とする。
2. **STATUS レジスタ操作での `|=` 禁止**:
   * `USART0.STATUS` に対して `|=` は使わず、`USART0.STATUS = USART_WFB_bm | ...` による直接代入を徹底する。
3. **エラーリカバリの決定論的実装**:
   * ISFIF やパリティエラー検知時は、必ず 1 行でクリーンに `WFB=1` 状態へ復帰させる。
