<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D RS-485 ブートローダー開発 ナレッジベース & 設定値集約書

**策定日**: 2026-09-28  
**対象ハードウェア**: ADX Core-D (Microchip ATtiny1616-MNR, RS-485: SP485EEN)  
**過去の試作アーカイブ**: [`archive/OneToOne_RS485/`](./archive/OneToOne_RS485/)

---

## 1. エグゼクティブサマリ

2026年9月27日に実施された「ADX Core-D 向け 1-to-1 RS-485 ブートローダー（Over-The-Wire: OTW 書き込みシステム）」の開発試行において、3つの方式（`optiboot_x`, `optiboot_O4`, `optiboot_OL4`）を実装・検証しました。  
結果として、全機能が安定稼働するブートローダーシステムの完成には至りませんでしたが、**ハードウェアの物理的特性、トランシーバー制御の重要ルール、ATtiny1616 のペリフェラル設定、通信障害の真因** に関する極めて貴重な知見・設定値が完全に特定されました。

本ドキュメントは、過去の試作コードを [`archive/OneToOne_RS485/`](./archive/OneToOne_RS485/) にバックナンバーとして隔離した上で、将来の開発者が同じ失敗を繰り返さず、最短で堅牢なブートローダーを構築できるように **確定設定値・物理層ノウハウ・失敗メカニズム・今後の推奨設計指針** を集約・体系化したものです。

---

## 2. ハードウェア仕様・確定ピンアサイン・初期化設定値 (Ground Truth)

ADX Core-D の回路図・ネットリスト精査および実機検証によって確定した、変更不可能な物理ピンアサインとマイコンレジスタ設定です。

### 2.1 ピンアサイン一覧

| 信号名 | MCU ピン番号 | ポート | 接続先デバイス / 機能 | 必須レジスタ初期化・動作モード |
| :--- | :---: | :---: | :--- | :--- |
| **TXD** | 20 | `PA1` | SP485EEN Pin 4 (DI) | `PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;`<br>`VPORTA.DIR \|= PIN1_bm;` (Output) |
| **RXD** | 1 | `PA2` | SP485EEN Pin 1 (RO) | `VPORTA.DIR &= ~PIN2_bm;` (Input) |
| **DE** | 5 | `PA4` | SP485EEN Pin 3 (DE: Driver Enable) | `VPORTA.DIR \|= PIN4_bm;`<br>`VPORTA.OUT &= ~PIN4_bm;` (Active HIGH, 通常 LOW) |
| **/RE** | 8 | `PA7` | SP485EEN Pin 2 (/RE: Receiver Enable) | `VPORTA.DIR \|= PIN7_bm;`<br>`VPORTA.OUT &= ~PIN7_bm;` (Active LOW, 通常 LOW / 送信時 HIGH) |
| **RED LED** | 7 | `PB2` | オンボード赤色 LED | `VPORTB.DIR \|= PIN2_bm;` (待機時点滅 / ステータス表示) |
| **WHITE LED**| 6 | `PB3` | オンボード白色 LED | `VPORTB.DIR \|= PIN3_bm;` (ユーザーアプリ稼働インジケータ) |
| **DEBUG TX** | 10 | `PB4` | CH342K Port B RX (SoftwareSerial) | PC デバッグシリアル送信 (9600 bps) |
| **DEBUG RX** | 11 | `PB5` | CH342K Port B TX (SoftwareSerial) | PC デバッグシリアル受信 (9600 bps) |

> [!CAUTION]
> #### 重大トラップ 1: DE ピンの誤認（PA3 ではなく PA4！）
> 過去の開発初期において、DE ピンを `PA3` と誤認して操作する致命的なバグがありました。  
> **`PA3` は外部水晶振動子（EXTCLK）入力ピン** であり、トランシーバーの DE は **`PA4`** に接続されています。

> [!CAUTION]
> #### 重大トラップ 2: DE (PA4) / /RE (PA7) の初期化漏れ（浮遊電荷・Hi-Z 現象）
> マイコン起動時、未初期化の PA4 / PA7 は Hi-Z（高インピーダンス入力）となっています。  
> このとき **基板配線の寄生容量や浮遊電荷によって、電源投入直後のみ PA4 が数秒間 HIGH（送信可能）を維持し、放電して LOW に落ちた瞬間に二度と返信できなくなる** という不可解な間欠動作を引き起こしました。  
> **起動直後（main 関数先頭）で必ず PA4=Output LOW, PA7=Output LOW を明示的に設定することが必須** です。

> [!IMPORTANT]
> #### 重大トラップ 3: PORTMUX の USART0 Alternate 設定
> ATtiny1616 の USART0 デフォルトピンは `PB2` (TXD) / `PB3` (RXD) です。  
> これは Core-D のオンボード赤色 LED (`PB2`) および白色 LED (`PB3`) と直結しています。  
> `PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;` を設定しないと、LED ピンが UART アイドル（HIGH）で点灯し続け、通信ピン PA1/PA2 には信号が一切流れません。

> [!NOTE]
> #### PB4 / PB5: オンボード USB-UART デバッグポート (SoftwareSerial)
> ADX Core-D にはデュアル USB-UART ブリッジ IC（**WCH CH342K**）がオンボード搭載されています：
> - **CH342K Port A (COM20 等)**: ATtiny1616 `PA0`（UPDI ピン）に接続 $\rightarrow$ **SerialUPDI ファームウェア書き込み専用**
> - **CH342K Port B (COM21 等)**: ATtiny1616 **`PB4` (TX) / `PB5` (RX)** に接続 $\rightarrow$ **PC デバッグシリアル（SoftwareSerial 9600 bps）専用**
> 
> ※なお、ADX 共通仕様書（CORE-S / CORE-B / CORE-U 等）では `PB4` がオンボード赤色 LED として規定されている場合がありますが、**ADX Core-D 初版基板では LED は PB2/PB3 に割り当てられ、PB4/PB5 は PC デバッグ用 SoftwareSerial（CH342K Port B）として配線されています**。昨日の実験でも COM21 のテレメトリモニタとして活用されました。

### 2.2 推奨ハードウェア初期化コード（C言語）

```c
#include <avr/io.h>

static inline void hardware_init(void) {
    // 1. USART0 ピンをオルタネートピン (PA1:TXD, PA2:RXD) へリダイレクト
    PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;

    // 2. RS-485 トランシーバー制御ピン初期化 (PA4: DE, PA7: /RE)
    //    PA4(DE)=LOW (受信モード), PA7(/RE)=LOW (受信イネーブル)
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);
    VPORTA.DIR |= (PIN4_bm | PIN7_bm);

    // 3. UART TXD(PA1)を出力、RXD(PA2)を入力に設定
    VPORTA.DIR |= PIN1_bm;
    VPORTA.DIR &= ~PIN2_bm;

    // 4. ステータス LED (PB2: 赤色 LED) を出力設定
    VPORTB.DIR |= PIN2_bm;
    VPORTB.OUT &= ~PIN2_bm; // 消灯

    // 5. USART0 の基本設定 (例: 115200 bps @ 20MHz または 9600 bps)
    //    USART0.BAUD = ...;
    //    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}
```

---

## 3. RS-485 半二重通信特有の物理・電気的現象と対策ノウハウ

### 3.1 「自爆エコー（Self-Reception Echo）」問題と完全遮断

* **現象**:
  * RS-485 は 2 線式差動（A/B）の半二重通信です。
  * スレーブ（マイコン）が応答パケットを送信する際、受信回路（/RE=0）が有効なままだと、**自分が TX ラインから SP485 に送り出したデータが、SP485 の RO ピンからそのまま RX 入力へループバック（自爆エコー）** します。
  * この自爆エコーデータがマイコンの UART 受信 FIFO に残留すると、次にホストから送られてくるコマンドパケットの先頭バイトと衝突・混ざり合います。
  * **実例**: `0x0440` 読み出しフリーズ問題  
    0x0400 ページの末尾文字 `'t'`（ASCII `0x74`）がエコーとして FIFO に残留 $\rightarrow$ 次のコマンド判定時に STK500 コマンド `STK_READ_PAGE` (`0x74`) と誤認 $\rightarrow$ 後続のヘッダバイトを読み出し長（2万バイト以上）と解釈して巨大ループに突入しマイコンが完全沈黙。
* **解決策（黄金比制御）**:
  1. **送信開始時**: `DE=1` かつ **/RE=1（受信回路を物理遮断）** に設定。
  2. **送信安定化待ち**: `DE=1` にしてから最初のバイトを送出するまで、微小ディレイ（数十µs〜1ms）を確保。
  3. **送信完了待ち**: `USART0.STATUS & USART_TXCIF_bm`（最後のストップビット送出完了）をポーリングで厳密に待機。（※`DREIF`（データレジスタ空）で DE を落とすと、最後の 1 バイトが物理的に切断される）。
  4. **受信復帰**: `DE=0` かつ `/RE=0` に戻す。
  5. **FIFO フラッシュ**: 万一のノイズや過渡エコーを排除するため、受信バッファを空読み（フラッシュ）する。

```c
static inline void rs485_tx_start(void) {
    VPORTA.OUT |= (PIN4_bm | PIN7_bm); // DE=1, /RE=1 (自爆エコー遮断)
    _delay_us(50);                      // トランシーバ安定化
}

static inline void rs485_tx_end(void) {
    // 最後のストップビットが送信完了するのを待つ (TXCIF)
    while (!(USART0.STATUS & USART_TXCIF_bm))
        ;
    USART0.STATUS = USART_TXCIF_bm;    // フラグクリア
    _delay_us(50);                      // バス解放マージン
    VPORTA.OUT &= ~(PIN4_bm | PIN7_bm);// DE=0, /RE=0 (受信モード復帰)

    // 受信バッファに残った過渡ノイズをフラッシュ
    while (USART0.STATUS & USART_RXCIF_bm) {
        (void)USART0.RXDATAL;
    }
}
```

### 3.2 市販 USB-RS485 ドングルの Auto-DE 特性と「フレーム一体型送信」

* **課題**:
  * PC 側に接続する市販の USB-RS485 アダプタ（FTDI, CH340, CP2102 等）の多くは、ハードウェアの Auto-DE 回路（TX 信号の立ち下がりを検知して DE を自動駆動するワンショット回路等）を搭載しています。
  * ホストプログラム（Python）側で `ser.write(header)` と `ser.write(payload)` を分割して送信すると、OS のスケジューリングや USB パケットの切れ目によって **送信途中で DE が一瞬 LOW（受信モード）へ落ちる過渡グリッチ** が発生します。
  * このグリッチにより、マイコン側では 1 バイトのビット化けやデータ欠落が引き起こされます。
* **対策**:
  * ホスト側は、**ヘッダからペイロード、CRC までを単一の `bytes` 配列として結合し、1 回の `ser.write()` / `ser.flush()` で一括送信（アトミック送信）** しなければなりません。

---

## 4. 過去 3 方式の試行と失敗の真因分析

### 4.1 試作 1: `optiboot_x` (STK500v1, 512B, 115200bps)

* **構成**: megaTinyCore 付属の `optiboot_x` を Core-D 向けにポーティング。サイズ 486 バイト（`BOOTEND = 0x02`）。
* **検証結果**:
  - 単一ページ（64 バイト）の書き込みテスト（M4）は成功（白 LED 点滅）。
  - しかし複数ページ書き込み時、**「0x0440 読み出しフリーズ問題」** が発生し実用不可。
* **真因**:
  - 送信時の `/RE=1` 制御が存在せず、自爆エコーにより受信 FIFO が汚染されていた。
  - レガシーな Optiboot の仕様として、未定義バイトを受信すると `verifySpace()` 内で 8ms WDT 自滅リセットを行うため、電源投入時やライン上の微小ノイズで即座にアプリへ逃げてしまう。

### 4.2 試作 2: `optiboot_O4` (自爆遮断 + STK500v1, 512B, 115200bps)

* **構成**: 自爆エコー遮断（DE=1, /RE=1）と TXCIF 待機、FIFO フラッシュを導入した改良版。サイズ 460 バイト。
* **検証結果**:
  - 自爆エコーの遮断により、0x0380 〜 0x0480 の 5 ページ連続読み出しが **RTT 7.7ms で完全 PASS**。読み出しフリーズは克服。
  - しかし、実スケッチ（11ページ / 676バイト）の書き込み時、**Page 0x0200（先頭ページ）の `STK_PROG_PAGE` (0x64) 実行直後に Core-D がリセット／クラッシュ（`14 FF` を返して沈黙）**。
* **真因**:
  - ATtiny1616 の Flash 消去・書き込み（Page Erase & Write）には **20ms 〜 28ms** 要する。
  - STK500v1 のステートマシンとバッファ処理において、書き込み中のマイコン内部状態（クロック、チャージポンプ、NVM コントローラ）とシリアル割り込み／ポーリングの競合、またはコードサイズ 512 バイト制約に収めるための過度なレジスタ切り詰めによるスタック破壊が発生したと推定。

### 4.3 試作 3: `optiboot_OL4` (LINAUTO / Stop-and-Wait ARQ / 1024B, 9600〜115200bps)

* **構成**: STK500v1 を脱却し、LIN Break + 0x55 同期（LINAUTO）、64B ページ単位 CRC-16、Stop-and-Wait ARQ（自動再送）、1024 バイト（`BOOTEND = 0x04`）へ拡張。
* **検証結果**:
  - ハンドシェイク（Ping, Version 取得）は成功。ベンチマーク（1バイトエコー、8バイトパケット）は 100% 成功。
  - しかし実際の書き込み時、**間欠的なタイムアウト（沈黙）が多発し、リトライを繰り返した末に書き込み失敗**。
* **真因**:
  1. **スレーブ側 `getch_payload()` のタイムアウト欠如による 1 バイト欠損デッドロック**:
     ```c
     uint8_t getch_payload(void) {
         while (!(USART0.STATUS & USART_RXCIF_bm)) ; // ★無限ループ！
         return USART0.RXDATAL;
     }
     ```
     ノイズ等でホストから送られたパケットのうち 1 バイトでも取りこぼすと、マイコンは最後の 1 バイトを待って無限ループに入る。ホスト側がタイムアウトして再送しても、スレーブは「前のパケットの続き」として吸い取るため、恒久的なズレと沈黙（デッドロック）が発生した。
  2. **Windows USB-UART による Break 信号（Baud-Rate Trick）のジッターと `ISFIF` 自爆**:
     Windows 上で `ser.baudrate` を動的に切り替えて Break 信号（LOW）を生成する際、ドライバの過渡グリッチや OS スケジューリングのジッターが発生。Core-D の `LINAUTO` ハードウェアがこれを不正エッジと判定し、`ISFIF`（Inconsistent Sync Field Flag）を立ててフレームを破棄していた。

---

## 5. ATtiny1616 内蔵機能・メモリ制御のノウハウ

### 5.1 Flash メモリと消去・書き込み時間

* **ページサイズ**: **64 バイト**（`PROGMEM_PAGE_SIZE = 64`）
* **書き込み所要時間**: **約 20ms 〜 28ms**（Page Erase & Write 時）
  * マイコンが NVMCTRL で書き込みを実行している間、CPU は停止（Stall）するか、通信処理を受け付けられません。
  * ホスト側は書き込みコマンド送出後、最低 30ms 〜 50ms のスロット時間を確保して待機する必要があります。

### 5.2 メモリマップと FUSE.BOOTEND

```text
+-----------------------+ 0x0000
|   ブートローダー      | BOOTEND = 0x02 の場合: 512B (0x0000〜0x01FF)
|   (書込保護セクション)| BOOTEND = 0x04 の場合: 1024B (0x0000〜0x03FF)
+-----------------------+ 0x0200 / 0x0400
|                       |
|   ユーザーアプリ      | アプリ開始アドレス
|   (APP セクション)    | ベクタテーブル再配置、または直接実行
|                       |
+-----------------------+ 0x3FFF (16KB End)
```

* `FUSE.BOOTEND` の値は `256 バイト単位` のブロック数を表します。
  * `BOOTEND = 0x02`: `2 * 256 = 512 バイト`（`0x0000`〜`0x01FF`）がブート領域としてハードウェア保護されます。
  * `BOOTEND = 0x04`: `4 * 256 = 1024 バイト`（`0x0000`〜`0x03FF`）がブート領域。
* **Mapped PROGMEM**:
  * ATtiny1616（AVR 0-series）では、Flash メモリがデータ空間の `0x8000` にマッピングされています。
  * ポインタ経由で Flash を読み書きする場合、`0x8000 + flash_addr` を参照する必要があります。

---

## 6. 今後のブートローダー再設計に向けた設計指針・提言

本プロジェクトの失敗から得られた教訓に基づき、次回開発において採用すべき**「最も堅牢で失敗しない設計原則」**を提言します。

### 原則 1: 複雑なプロトコルや動的同期（LINAUTO）を完全に排除する
* Windows PC の USB シリアル変換器から精密な Break 信号やマイクロ秒単位のパルスを安定生成することは困難です。
* **固定ボーレート（例: 19,200 bps または 38,400 bps / 115,200 bps）** を採用し、標準的な UART フレーム（8N1）のみで通信すべきです。

### 原則 2: スレーブ側の「受信タイムアウト & 自律復帰機構」を必ず実装する
* `while (!(USART0.STATUS & USART_RXCIF_bm));` のような無限ループを絶対に書かないこと。
* タイマー（RTC または簡易ビジーカウンタ）を用いて、パケット受信中に「一定時間（例: 20ms）次のバイトが来なければ、受信バッファを破棄してコマンド待機（アイドル状態）へ自律復帰する」ステートマシンを実装すること。

### 原則 3: Master-Slave 原則と十分なスラックタイム（Slack Time）の保守
* スレーブ（Core-D）は絶対に自発的な送信を行わず、ホストから問い合わせがあった時のみ受動的に返信すること。
* ホスト側は、スレーブの応答を受信した後、次のコマンドを送るまでに **十分なバス休止時間（スラックタイム: 10ms〜20ms、Flash 書き込み後は 30ms〜50ms）** を必ず設けること。

### 原則 4: 堅牢なパケット構造（固定長 または STX/ETX + CRC）
* パケット長を明確にし、ヘッダには必ず `MAGIC / STX` を配置。
* データ化けを検知できるシンプルなチェックサムまたは CRC を付与し、エラー時はスレーブが状態を破棄して次のパケットを待てるようにすること。

---

## 7. バックナンバー（アーカイブ）構成

隔離された過去の全コード・ツール・実機ログは以下のパスに保管されています：

```text
firmware/bootloader/archive/OneToOne_RS485/
├── README.md                          # アーカイブ時の全体案内
├── milestones.md                      # マイルストーン進捗（M0〜M5）
├── development_plan.md                # 当初の開発計画書
├── rs485_communication_sequence.md   # 通信シーケンス設計書
├── workflow_and_tools.md              # ツールチェーン設計書
│
├── src/                               # 試作1: optiboot_x (STK500v1, 512B) ソース
├── optiboot_O4/                       # 試作2: optiboot_O4 (自爆遮断, 512B) ソース・仕様
├── optiboot_OL4/                      # 試作3: optiboot_OL4 (LINAUTO, ARQ, 1024B) ソース・仕様・検証
├── scripts/                           # Python 書き込みツール (adx_rs485_flash.py 等)
├── tools/                             # 開発補助ツール (Web Flasher 等)
├── releases/                          # ビルド済みバイナリ (.hex)
└── records/                           # 各マイルストーンの実機検証ログ (M0〜M4)
```
