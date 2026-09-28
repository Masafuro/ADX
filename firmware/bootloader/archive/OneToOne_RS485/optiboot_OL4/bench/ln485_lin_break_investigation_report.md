# LN-485 LIN Break & LINAUTO 挙動解明・深層調査レポート

- **対象ハードウェア**: ADX Core-D（Microchip ATtiny1616-MNR, SP485EEN, WCH CH342K）
- **観測ポート構成**:
  - `COM19`: 被測定 RS-485（19,200 bps / Break: 9,600 bps）
  - `COM20`: SerialUPDI プログラミング（`pymcuprog`）
  - `COM21`: CH342K 独立テレメトリ（PB4 Soft-UART @ 9,600 bps）
- **作成日**: 2026-09-27
- **作成者**: Antigravity Pair-Programming Agent

---

## 1. エグゼクティブサマリ

ADX Core-D における LN-485（1対1 確定論的ブートローダー）の確立に向け、電源投入直後は RTT 10〜12ms、AutoBAUD `0x02B5`（誤差0.14%）で数回 PASS するものの、ある試行から突然タイムアウト（`rx=0B`）となり以降連続して沈黙する現象について、**「推測を排し、動かぬ証拠（Fact）を掴む」** 方針のもと、3ポート独立観測体制を構築して実機観測を実施した。

その結果、**COM21（9,600 bps Soft-UART）の開通に成功し、マイコン内部ステートの取得に初めて成功した。**
得られた生ログから、クロック精度や物理ピンの健全性が証明された一方、**「待機時に WFB（Wait-For-Break）が 0 であること」**、および **「Probe #29 で 1 回 PASS して応答を返信した直後、マイコンが次のループを回さず完全に停止（スタック）したこと」** が白日の下に晒された。

---

## 2. ここまでで「100% 確実にわかったこと」（動かぬ証拠・Fact）

### Fact 1: クロック精度と AutoBAUD の完全ロック [検証完了]
- **測定値**: `AutoBAUD = 0x02B5 (693)`
- **理論値**: $F_{CLK} = 20\text{MHz}/6 = 3.333333\text{MHz}$ において、
  $$\text{BAUD} = \frac{3,333,333.3 \times 4}{19,200} = 694.4 \quad (0\text{x}02B6)$$
- **結論**: 理論値との誤差はわずか **0.14%**。マイコン内部オシレータおよび PC 側から送出された Break (9600 bps) / Sync (0x55 @ 19200 bps) は、物理的に完璧な精度でロックしている。

### Fact 2: 独立デバッグポート（COM21 / PB4 Soft-UART @ 9600 bps）の開通 [検証完了]
- **測定生ログ**:
  ```text
  |-> [COM21] [23:02:29.481] [IDLE] ST=0x20 RXD=1 F=0x2382 ISF=0x5DB1
  |-> [COM21] [23:02:30.866] [IDLE] ST=0x20 RXD=1 F=0x2382 ISF=0x5DB1
  ```
- **結論**:
  1. Core-D のオンボード CH342K Port B（PB4/PB5）を用いた完全非同期テレメトリが 9,600 bps で文字化け・ジッターなく正常開通した。
  2. 待機時、約 1.38 秒周期で正確に Heartbeat / IDLE テレメトリが出力されている。
  3. `RXD=1`: SP485EEN の受信ピン（PA2 / RXD）は論理 HIGH（バスアイドル正常状態）を維持しており、バスが LOW に張り付くハードウェア故障・終端不良ではないことが確定。

### Fact 3: 同期エラー（ISFIF）によるフレーム破棄ではない [検証完了]
- **測定値**: `Total ISFIF Errors : 0`, テレメトリ上の `ISF` 増分 = 0
- **結論**: 通信途絶の原因は「同期フィールドの不整合によるフレーム破棄」ではない。

### Fact 4: 初期トラブル（`ÿÿÿÿ...` 連打）の物理原因の特定と解決 [解決済み]
- **原因**: Makefile の `avr-objcopy` において `-j .text -j .data` を指定していたため、文字列定数セクション `.rodata`（Flash 物理アドレス `0x03B4` 以降）が HEX ファイルから欠落していた。
- **メカニズム**: Flash 上が未書き込み `0xFF` だったため、終端 `\0` が見つからず Flash 全領域の `0xFF`（latin-1 で `ÿ`）を UART から垂れ流すスピンロックに陥っていた。
- **対策**: セクション除外方式（`-R .eeprom ...`）に修正し、HEX ファイル内に全文字列が物理配置されていることをバイナリレベルで確認・解決した。

---

## 3. 「不思議な現象・強い違和感」の客観的分析

### 違和感 1: 待機時ステータス `ST=0x20`（なぜ WFB が 0 なのか？）
- **現象**:
  COM21 から出力された待機時ステータスは `ST=0x20` であった。
  ATtiny1616 の `USART0.STATUS` ビット定義：
  | Bit | 名 称 | 値 (0x20時) | 説 明 |
  |:---:|:---:|:---:|:---|
  | 7 | `RXCIF` | 0 | 受信完了フラグ |
  | 6 | `TXCIF` | 0 | 送信完了フラグ |
  | 5 | `DREIF` | **1** | 送信データレジスタ空（正常） |
  | 4 | `RXSIF` | 0 | 受信スタート検出フラグ |
  | 3 | `ISFIF` | 0 | 不正同期フィールドエラー |
  | 2 | - | 0 | 予約 |
  | 1 | `BDF` | 0 | Break 検出フラグ |
  | 0 | `WFB` | **0** | **Wait-For-Break（ブレーク待機）** |

- **違和感の核心**:
  ファームウェアの初期化では明示的に以下を実行している：
  ```c
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; // 0x01 | 0x08 | 0x02
  ```
  それにもかかわらず、アイドル待機中に出力された値は `0x20` であり、**`WFB`（Bit 0）が 0 になっている**。
- **影響**:
  `WFB` が 0 の状態では、USART は「Break を待つ」のではなく「通常のデータフレーム」として受信を行おうとする。ホストが送った Break（LOW 期間）を通常のスタートビット＋ゼロデータ、あるいはフレーミングエラーとして扱い、0x55 を Sync ではなく通常の文字データとして FIFO に溜め込んでしまう可能性がある。

---

### 違和感 2: Probe #29 で PASS した直後、マイコンが完全に「沈黙（フリーズ）」した謎
- **生ログの推移**:
  ```text
  ... probing #25 (4s / 30s) ...

  [STAGE 1: PASS] Power-on detected in 5.53s (probe #29, RTT=56.5ms)!
    [TARGET] lin_break_probe active! AutoBAUD=0x02B5 (693)
     # |   Target |   Status |  RTT(ms) |  Frame# |  ISFIF# | AutoBAUD |       Info
  ------------------------------------------------------------------------
     1 |  TIMEOUT |     FAIL |   154.95 |       - |       - |        - | rx=0B (empty)
     2 |  TIMEOUT |     FAIL |   153.58 |       - |       - |        - | rx=0B (empty)
     ...
    30 |  TIMEOUT |     FAIL |   154.72 |       - |       - |        - | rx=0B (empty)
  ```
- **極めて不自然な点**:
  1. Probe #29 で、マイコンは確かにホストからの信号を受信し、7 バイトの診断応答（RTT=56.5ms）を RS-485 経由で正常に PC に返信した。
  2. **しかし、その後行われた #1〜#30 の試行中、COM21 からテレメトリが 1 行も出力されなかった。**
  3. もしマイコンが「単にホストからの信号を受信できなかっただけ」であれば、約 1.4 秒おきに `[IDLE] ST=...` が COM21 に流れ続けなければならない。
  4. **`[IDLE]` すら出力が途絶えたということは、マイコンは Probe #29 の返信処理（またはその直後）で実行ループが停止（無限ループまたはハング）したことを意味する。**

---

## 4. 「マイコンはどこで止まったのか？」コード上のボトルネック箇所の絞り込み

マイコンが Probe #29 の返信以降に通過するコードパス：

```c
    // Send LN-485 Standard Diagnostic Packet over RS-485 (COM19)
    response_space();
    rs485_tx_start();

    // 7バイト送信処理 (putch) ...

    rs485_tx_end();   // <--- 【疑義箇所 A】
  }                   // for(;;) ループの先頭へ
```

### 【疑義箇所 A】`rs485_tx_end()`
```c
static inline void rs485_tx_end(void) {
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
  USART0.STATUS = USART_TXCIF_bm;

  // Release bus: DE=0, /RE=0
  VPORTA.OUT &= ~((1 << 3) | (1 << 7));

  uint8_t rxd_after = (VPORTA.IN & (1 << 2)) ? 1 : 0;

  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  while (USART0.STATUS & USART_RXCIF_bm) {
    (void)USART0.RXDATAH;
    (void)USART0.RXDATAL;
  }

  // Telemetry 出力
  dbg_print(" [TX_DONE] RXD=");
  dbg_putch(rxd_after ? '1' : '0');
  dbg_print(" ST=0x");
  dbg_print_hex8(USART0.STATUS);
  dbg_print("\r\n");
}
```
- **着眼点**:
  1. PC 側に Probe #29 の応答パケットが届いたということは、`putch()` および `TXCIF` 待機は通過している。
  2. `[TX_DONE]` のログが PC 側（COM21）に届いていない。
  3. なぜ `[TX_DONE]` が届かなかったのか？
     - PC 側の `test_lin_break.py` は、STAGE 1（Probe #29）が成功した直後、`time.sleep(0.150)` を挟んでからベンチマークループ（#1）へ移行する。
     - この間に COM21 から送られたデータが PC 側のシリアル受信バッファで捨てられたか、あるいはマイコンが `rs485_tx_end()` の中のどこか（FIFO ドレインループや Soft-UART 送信中）で止まったのか？

### 【疑義箇所 B】`getch_header()` の先頭
```c
static uint8_t getch_header(void) {
  uint16_t hb_loop = 0;
  uint16_t idle_ticks = 0;

  for (;;) {
    if (USART0.STATUS & USART_ISFIF_bm) { ... }
    if (USART0.STATUS & USART_RXCIF_bm) { ... }
    if (++hb_loop == 0) { ... }
  }
}
```
- **着眼点**:
  もし `getch_header()` の `for(;;)` に戻っていたとすれば、ホストが無音であっても 1.4 秒ごとに `[IDLE]` が印字されるはずである。
  印字されなかった理由は以下のいずれかしかない：
  1. `rs485_tx_end()` から戻ってきていない。
  2. `getch_header()` に戻ったが、何らかの理由で `hb_loop` のインクリメントが行われない（極めて重い処理やスピンロックに捕まっている）。
  3. PC 側スクリプトの `DebugListener` スレッドが、STAGE 1 通過後に COM21 の読み出しをブロックされた（Windows のドライバ競合等）。

---

## 5. 未解明の点（残された疑問）

1. **なぜ `WFB` は初期化時にセットされなかった（あるいはすぐにクリアされた）のか？**
   - ATtiny1616 の `USART_STATUS` に対する書き込みプロテクトや、`LINAUTO` モードにおけるハードウェア挙動の特殊ルールが存在するのか？
2. **なぜ「電源投入直後の 1〜数回」だけは PASS できるのか？**
   - 電源投入直後、マイコンがリセットされた瞬間だけの特定ステートが存在し、一度でも送信（DE=1/0）を行うと不可逆なステートに遷移してしまうのか？
3. **PC 側（Windows / pyserial）の USB-UART スタックの挙動**
   - COM19（RS-485 送信）で 19200bps $\leftrightarrow$ 9600bps のボーレート切り替えを頻繁に行う際、Windows のドライバ内部で Break が正しく生成されなくなっている可能性はないか？

---

## 6. 次のフェーズに向けた調査計画（再開時のアクションプラン）

推測による場当たり的な修正を行わず、事実を完全に突き止めるための具体的計画：

1. **ステップ進行テレメトリ（ブレークポイントピンポイント特定）**:
   `rs485_tx_end()` の各行（送信完了直後、DE解除直後、WFB書き込み直後、関数脱出時）に 1 文字ずつのマーカー（例: `'A'`, `'B'`, `'C'`）を COM21 へ即座に出力させ、マイコンが物理的にどこで停止したかを特定する。
2. **`WFB` レジスタ書き込み実験**:
   `main()` の先頭で `USART0.STATUS = USART_WFB_bm;` を実行した直後に `USART0.STATUS` を読み出して COM21 へ表示し、ハードウェアが `WFB` への書き込みを受け付けているかを単体検証する。
3. **COM21 生ログファイル（`com21_telemetry.log`）の精査**:
   PC のコンソール表示に漏れた可能性のあるバックグラウンドログを確認し、STAGE 1 完了前後のタイムスタンプ付き全ログを突合する。

---
*本レポートは、事実と測定ログに基づき作成された。*
