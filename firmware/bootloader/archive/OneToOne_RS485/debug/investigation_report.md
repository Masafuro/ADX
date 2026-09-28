# ADX Core-D RS-485 0x0440 読み出し失敗 調査記録 (Investigation Report)

## 1. 調査の背景と目的
11ページ（676B）のファームウェア書き込み時、Stage 4（フラッシュ書き込み）完了後の Stage 5（ベリファイ）において、10ページ目にあたる `0x0440` の読み出しでタイムアウトが発生する問題について、再現実験とパケット解析を行い、根本原因を特定・恒久対策する。

---

## 2. 調査タイムライン & ログ

### [2026-09-27] 初期症状
- **現象**:
  ```text
  [STAGE 3: PASS] Entered programming mode successfully.
  [STAGE 4: WRITE] Programming 11 page(s) (64 bytes/page)...
  [STAGE 4: PASS] All pages programmed to flash successfully.
  [STAGE 5: VERIFY] Verifying 11 page(s) against HEX binary...
  [ERROR] Stage 5 失敗: ページ 0x440 の読み出しに失敗しました。
  ```
- **WDT停止改修後の結果**:
  - `STK_ENTER_PROGMODE` 受信時に `watchdogConfig(WDT_PERIOD_OFF_gc)` を実施したが、症状変わらず `0x0440` で停止。
  - $\rightarrow$ **「WDTタイムアウト」以外の要因が支配的である可能性が極めて高い。**

---

## 3. 検証項目 & 診断マトリクス

| テスト項目 | コマンド / 手順 | 期待される判定情報 |
|---|---|---|
| **Test A: 0x0440 単体読み出し** | `python debug_flasher.py --port COM19 --single-page 0x0440` | 書き込みを経ずに 0x0440 が読めるか？（読めればハード/メモリ空間の問題ではない） |
| **Test B: 全ページ読み出し監査** | `python debug_flasher.py --port COM19 --read-only --start 0x0200 --pages 11` | 何ページ目で読み出しが止まるか？（連続読み出しによるバッファ/ディレイ問題の特定） |
| **Test C: 1ページ書き込み&即時ベリファイ** | `python debug_flasher.py --port COM19 --hex ../releases/test_rs485_serial.hex` | 書き込み直後の読み出しがどのページで失敗するか？ |
| **Test D: Web Flasher 詳細Rawログ** | ブラウザで「詳細Rawパケットログ (HEX)」をONにして実行 | 送信したパケットとCore-Dが返したRawバイト列の完全追跡 |

---

## 4. 診断結果記録欄（ユーザー実行待ち）
（※診断ツール実行後に結果を追記）

- Test A
```text
PS C:\Users\User> python debug_flasher.py --port COM19 --single-page 0x0440
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 6.04s (probe #50)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.2ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!

=== TARGET TEST: SINGLE PAGE 0x0440 ===
  [TX] 55 40 04 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 74 00 40 46 20           -> [RX] 14 29 0D 0A 00 5B 41 44 58 20 43 6F 72 65 2D 44 5D 20 48 65 61 72 74 62 65 61 74 20 70 61 63 6B 65 74 20 23 00 20 7C 20 57 68 69 74 65 20 4C 45 44 3A 20 4F 4E 0D 0A 00 20 7C 20 57 68 69 74 65 20 10 (RTT= 7.9ms) PASS
Read 64 bytes: 29 0D 0A 00 5B 41 44 58 20 43 6F 72 65 2D 44 5D ...
[INFO] Serial port closed.
```

- Test B

```text

PS C:\Users\User> python debug_flasher.py --port COM19 --read-only --start 0x0200 --pages 11
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 6.53s (probe #54)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 1.8ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.1ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 1.8ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!

[STAGE: EXIT] Leaving programming mode...
  [TX] 51 20                    -> [RX] 14 10                        (RTT= 2.2ms) PASS

[FINISH] Test session ended.
[INFO] Serial port closed.

```

- Test C

```text
PS C:\Users\User> python debug_flasher.py --port COM19 --hex test_rs485_serial.hex
[INIT] Opening serial port COM19 at 115200 bps...
[INIT] Connected successfully to COM19.

[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 0.00s (probe #1)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
  [TX] 75 20                    -> [RX] 14 1E 94 21 10               (RTT= 2.6ms) PASS
  Device Signature: 0x1E 0x94 0x21
  [TX] 50 20                    -> [RX] 14 10                        (RTT= 1.9ms) PASS
[STAGE 2: PASS] Successfully entered programming mode!
[INFO] Serial port closed.
Traceback (most recent call last):
  File "C:\Users\User\debug_flasher.py", line 329, in <module>
    main()
    ~~~~^^
  File "C:\Users\User\debug_flasher.py", line 289, in main
    hex_data = parse_intel_hex(args.hex)
  File "C:\Users\User\debug_flasher.py", line 242, in parse_intel_hex
    with open(hex_path, 'r') as f:
         ~~~~^^^^^^^^^^^^^^^
FileNotFoundError: [Errno 2] No such file or directory: 'test_rs485_serial.hex'

```

- Memo
  - ごくまれに、CORE-Dの再起動をしておらず、ブートローダーの赤点滅もしていないのに、debu_flasher.pyが成功している時がある。