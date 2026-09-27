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
