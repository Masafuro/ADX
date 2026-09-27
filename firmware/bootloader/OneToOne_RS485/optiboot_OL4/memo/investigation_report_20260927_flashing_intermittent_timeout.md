<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Optiboot_OL4 間欠的タイムアウト現象の観察と要因仮説レポート

**作成日**: 2026-09-27  
**対象**: ADX Core-D (Microchip ATtiny1616-MNR, RS-485: SP485EEN)  
**対象試験ログ**: `ol4_flasher.py --hex test_ol4_app_0400.hex` 実行結果  

---

## 1. エグゼクティブサマリ & 観察された客観的事実

ユーザー様のご指示に基づき、安易な解決策（パッチ）を急ぐのを一旦止め、**今回の実行ログで観測された客観的事実の精密な観察** と、**考えられる可能性の網羅的な洗い出し** を行いました。

### 1.1 観測された実行ログの精密トレース
```text
[STAGE 1: PASS] Power-on detected in 4.22s (probe #50)!
  [DEVICE INFO] Signature: 0x1E 0x94 0x21 | Optiboot_OL4 Version: 1.0
[HEX] Loaded 704 bytes from test_ol4_app_0400.hex
[PLAN] Flashing 11 pages (0x0400 ~ 0x06C0)...
  Flashing Page 1/11 @ 0x0400...    [ADDR RETRY #1/2] set_address 0x0400 failed (TIMEOUT), retrying...
    [RETRY OK] set_address 0x0400 succeeded on attempt #2
    [WRITE RETRY #1/2] write_page 0x0400 failed (TIMEOUT), retrying slot...
    [ADDR RETRY #1/2] set_address 0x0400 failed (TIMEOUT), retrying...
  [ERROR] Failed to set address 0x0400 (TIMEOUT)
  [ERROR] Write page failed at 0x0400 (TIMEOUT)
 [WRITE FAIL]
```

### 1.2 ログから判明した決定的な事実（ファクト）
1. **Core-D は死んでいない（ハードウェア健全）**:
   * `STAGE 1`（PING）は正常通過。
   * `DEVICE INFO`（Signature: `1E 94 21`, Version: `1.0`）は **リトライなしの一撃で完全成功**。
   * `set_address 0x0400` の 1 回目は TIMEOUT したが、**2 回目の試行では `[RETRY OK]` で正常に応答が返っている**。
2. **すべてのコマンドに「成功実績」がある**:
   * `PID_PING`: 100% 成功。
   * `PID_GET_INFO`: 100% 成功。
   * `PID_SET_ADDR`: 成功実績あり（今回も 2 回目で成功）。
   * `PID_WRITE_PAGE`: 前回実行時、Page 1（0x0400）の書き込みが一発成功している。
   * `PID_READ_PAGE`: 単体テストおよび前回実行時、64 バイト完全一致ベリファイ（VERIFY PASS）が一発成功している。
3. **現象の本質は「エラー（CRC不一致等）」ではなく「沈黙（TIMEOUT）」である**:
   * スレーブから `STATUS_ERR_CRC` や `STATUS_ERR_ADDR` 等のエラーコードが返ってくるのではなく、**Core-D が一切応答を返さず沈黙する（TIMEOUT）** という症状が一貫している。

---

## 2. 考えられる可能性（仮説群）の網羅的リスト

なぜすべてのコマンドに成功実績があるにもかかわらず、間欠的に「沈黙（TIMEOUT）」が発生し、一度沈黙すると後続が連鎖的に崩れるのか？考えられる物理的・論理的要因をリスト化しました。

---

### 【仮説 1】スレーブ側 `getch_payload()` のタイムアウト欠如による「1バイト欠損デッドロック」

* **メカニズム**:
  * 現在の `optiboot_ol4.c` における `getch_payload()` の実装：
    ```c
    uint8_t getch_payload(void) {
      while (!(USART0.STATUS & USART_RXCIF_bm))
        ; // ★データが来るまで無限ループ！
      (void)USART0.RXDATAH;
      return USART0.RXDATAL;
    }
    ```
  * `PID_SET_ADDR` は 5 バイト（Len, AddrL, AddrH, CRCH, CRCL）を要求します。
  * `PID_WRITE_PAGE` は 67 バイト（Len, 64B Data, CRCH, CRCL）を要求します。
  * もし、ノイズや USB シリアル変換の過渡現象により、ホストから送られたバイト列のうち **「たった 1 バイト」** でもスレーブが取りこぼした場合：
    - スレーブは「最後の 1 バイト」を待って `while (!(USART0.STATUS & USART_RXCIF_bm))` で **永久停止（デッドロック）** します。
    - ホスト側はタイムアウトし、次のスロットで新しいヘッダ（Break + 0x55 + PID）を送出します。
    - しかし、スレーブはまだ前のパケットの `getch_payload()` の中で止まっているため、ホストが送った新しい Break や `0x55` を **「前のパケットの最後の 1 バイト」として吸い取ってしまいます**。
    - その結果、スレーブの受信バイト列が恒久的に 1 バイトズレてしまい、二度とコマンドループに復帰できず沈黙（連鎖 TIMEOUT）に至ります。
* **蓋然性**: **極めて高い**（今回、1回目の set_address 失敗 $\rightarrow$ 2回目成功 $\rightarrow$ write_page 失敗 $\rightarrow$ 以後全滅、という推移と完全に符号する）。

---

### 【仮説 2】Baud-Rate Trick（57600 $\leftrightarrow$ 115200 切り替え）時の OS/ドライバ過渡パルスによる `ISFIF` 自爆

* **メカニズム**:
  * ホスト（Python / `pyserial`）は、Break を送るために毎スロット以下を実行しています：
    1. `ser.baudrate = 57600`（Windows API: `SetCommState` 呼び出し）
    2. `ser.write(b'\x00')`
    3. `ser.flush()`
    4. `ser.baudrate = 115200`（Windows API: `SetCommState` 呼び出し）
    5. `ser.write(bytes([0x55, pid]) + payload)`
  * Windows のシリアルドライバ（CH340 / FTDI 等）において、ボーレートを動的変更する瞬間、**TX ラインのレベルが一瞬乱れる（微小なグリッチパルスが出る）** ことがあります。
  * Core-D のハードウェア `LINAUTO` は、Break（LOW）の後の Delimiter（HIGH）から `0x55` の立ち下がりエッジまでの時間を厳密に計測しています。
  * もしボーレート復帰時（4 $\rightarrow$ 5）に TX ラインに微小なノイズが乗ると、`LINAUTO` はそれを不正なエッジとみなし、**`ISFIF`（Inconsistent Sync Field Flag）をセットして同期を強制破棄** します。
  * その結果、スレーブはそのフレームを完全に無視して沈黙します。
* **蓋然性**: **中〜高**（PC 側の USB シリアル変換ドングルのハードウェア・ドライバ特性に依存する）。

---

### 【仮説 3】一体型フレーム送信における USB-RS485 ドングルの Auto-DE バッファ分割

* **メカニズム**:
  * `SET_ADDR` は 7 バイト、`WRITE_PAGE` は **69 バイト** のフレームを一括送信しています。
  * 市販の安価な USB-RS485 ドングル（ハードウェア Auto-DE 回路搭載）の中には、**送信データが長い（64 バイト以上）場合、内部の USB FIFO の都合で DE 制御が一瞬チャタリングする** 回路が存在します。
  * もし 69 バイト送信の途中で DE が一瞬でも LOW に落ちると、スレーブ側の受信データが欠落し、上記【仮説 1】の永久待ちに直結します。
* **蓋然性**: **中**（ドングルの回路設計に依存）。

---

### 【仮説 4】`LINAUTO` の「毎フレーム動的ボーレート再計算」による累積ドリフト・演算ブレ

* **メカニズム**:
  * ATtiny1616 の `LINAUTO` エンジンは、受信した `0x55` のエッジ間隔をタイマーで計測し、マイコンの `USART0.BAUD` レジスタを **毎フレーム自動的に書き換えます**。
  * もしホスト側の USB-RS485 の出力波形にわずかなジッター（揺らぎ）があり、1 回でも異常なボーレート値に校正されてしまうと、その直後のペイロード受信でフレーミングエラーが発生し、以降の通信が狂う可能性があります。
* **蓋然性**: **低〜中**。

---

### 【仮説 5】Flash 消去・書き込み（NVMCTRL）直後のマイコン内部クロック・電源の過渡変動

* **メカニズム**:
  * ATtiny1616 の Flash 消去・書き込み（Page Erase & Write）には約 23〜28ms 要し、内部チャージポンプの動作により最も電流を消費します。
  * 書き込み完了直後、マイコン内部の電圧・クロックが過渡状態から復帰する瞬間に次のパケットを受信しようとすると、受信エラーが発生する可能性があります。
* **蓋然性**: **低〜中**（ただし今回のログでは、Page 1 の書き込み前（`set_address`）ですでにタイムアウトの兆候が出ているため、主因ではない可能性が高い）。

---

## 3. 観察から得られる重要な示唆

1. **「リトライで何とかする」アプローチの限界**:
   ユーザー様が直感された通り、もし【仮説 1】（受信側が 1 バイト欠損で無限待ちに入る）が起きている場合、ホストが外側からいくらリトライを送っても、スレーブは「前のパケットの残りを待っている状態」のままであり、リトライすら誤解釈してしまいます。
2. **スレーブの「完全な自律復帰（Timeout リカバリ）」の重要性**:
   スレーブ側が「次のバイトが一定時間来なければ、諦めて WFB=1（アイドル）に戻る」というタイムアウト復帰機構を持たない限り、通信路上の 1 バイトのノイズに対して極めて脆弱になります。

---

## 4. 今後の調査・検証方針の提案（真因の切り分け）

解決策を焦って実装する前に、どの仮説が真因かを特定するための検証ステップ案です：

* **切り分け観点 1（スレーブはどこで止まっているのか？）**:
  タイムアウト発生時、Core-D の赤色 LED（PB2）の状態（点滅している＝`getch_header` で待機中、消灯したまま＝`getch_payload` の無限ループでスタック）を観察する。
* **切り分け観点 2（Baud Trick のノイズか、データ長か？）**:
  短いパケット（SET_ADDR: 7B）でも空振りが起きているため、パケット長（69B）特有の問題ではなく、「Break 送出〜ヘッダ受信の境界」に原因がある可能性が高い。
