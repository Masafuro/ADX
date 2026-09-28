# ADX Core-D RS-485 シリアル出力 & エコーバック テストスケッチ

## 1. 概要
ADX Core-D（ATtiny1616）と PC（Web Serial モニター / Arduino IDE シリアルモニタ）の間で、RS-485 差動通信による双方向テキスト送受信（1 秒周期のハートビート送信 ＋ キー入力エコーバック）を行うサンプルスケッチです。

## 2. ハードウェア・ピンマップ
| 信号 / 機能 | Core-D ピン番号 | 物理ポート | 説明 |
| :--- | :--- | :--- | :--- |
| **白色 LED** | 9 | `PB3` | ハートビート点滅（1 秒周期で反転） |
| **赤色 LED** | 8 | `PB2` | ブートローダー状態表示用（本スケッチ実行中は常時 OFF） |
| **RS-485 TX** | 15 | `PA1` | USART0 送信（トランシーバー DI へ接続） |
| **RS-485 RX** | 16 | `PA2` | USART0 受信（トランシーバー RO へ接続） |
| **RS-485 RE/DE** | 14 | `PA7` | 送信・受信方向切替（HIGH: 送信, LOW: 受信） |

## 3. ファイル構成
- [`rs485_serial_echo.ino`](./rs485_serial_echo.ino): Arduino IDE（megaTinyCore）用スケッチ
- [`../../src/test_rs485_serial.c`](../../src/test_rs485_serial.c): スタンドアロン AVR-C ソースコード
- [`../../releases/test_rs485_serial.hex`](../../releases/test_rs485_serial.hex): コンパイル済み Intel HEX ファイル（アドレス `0x0200` 配置、Web Flasher ドラッグ＆ドロップまたは UPDI 書き込み用）

## 4. 動作仕様
1. **起動時**:
   - ボーレート `115,200 bps`（8N1）でシリアルを初期化。
   - `ADX Core-D RS-485 Serial Monitor Ready!` という起動アナウンスを送信。
2. **通常稼働時（1 秒ごと）**:
   - 白色 LED（PB3）を反転（点灯/消灯）。
   - `[ADX Core-D] Heartbeat packet #<count> | White LED: ON/OFF` を RS-485 送信。
3. **エコーバック（PC からの入力検知時）**:
   - Web Serial モニターやシリアルモニタから送信された文字を受信し、即座に `>>> Echo received: '<char>'` として送り返します。

## 5. Arduino IDE での書き込み手順
1. ボードマネージャで **megaTinyCore** を導入します。
2. ツールメニューから以下を選択します：
   - **Board**: `ATtiny1616 / 1606 / 816 / 416`
   - **Chip**: `ATtiny1616`
   - **Clock**: `20MHz internal` (または `16MHz`)
   - **Programmer**: `SerialUPDI` (または Optiboot 経由書き込み)
3. 本フォルダの `rs485_serial_echo.ino` を開き、書き込みを実行します。

## 6. Web Serial Flasher での書き込み手順
1. ブラウザで [Web Flasher](https://masafuro.github.io/ADX/flasher/) を開きます。
2. `releases/test_rs485_serial.hex` を画面中央の枠にドラッグ＆ドロップします。
3. 「ファームウェア書き込み開始」をクリックし、Core-D の電源を ON にして書き込みます。
4. 書き込み完了後、「📟 RS-485 シリアルモニター」タブを開いて通信を確認します。
