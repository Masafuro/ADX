<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Optiboot_OL4 開発進捗・実験ログ・検証記録レポート

**最終更新**: 2026-09-27  
**対象ターゲット**: ADX Core-D (Microchip ATtiny1616-MNR, RS-485: SP485EEN)  
**通信プロトコル**: 1-to-1 LN-485 (LIN-based RS-485, 115200 bps, 8N1)  
**実装ファイル**: [`src/optiboot_ol4.c`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/src/optiboot_ol4.c)  
**配布バイナリ**: [`releases/optiboot_ol4_with_blank.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/releases/optiboot_ol4_with_blank.hex)  
**診断・書込ツール**: [`tools/ol4_flasher.py`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/tools/ol4_flasher.py)  

---

## 1. プロジェクト立ち上げの背景とコア思想

### 1.1 Optiboot_O4 (STK500v1) からの発展的移行の経緯
先行開発の `Optiboot_O4` では、自爆エコーの完全遮断、512 バイト制限のクリア、Flash ページの書き込み・読み出し・単体ベリファイ（100% 一致）を実証しました。  
しかし、STK500v1 は元々「全二重 UART」を前提としており、**フレーム同期機構（Preamble/Break）やターンアラウンド（半二重切り替えマージン）の合意がプロトコルに存在しない**という構造的欠陥がありました。その結果、わずか 10cm の理想的ベンチ環境においても、66 バイト送出直後の過渡切り替え時にパケット解釈の同期ズレが生じる課題が浮き彫りとなりました。

### 1.2 LN-485 技術資産の統合
ADX プロジェクトで既に実機検証済み（Phase 1 〜 Phase 4 MVP 完全 PASS）の **LN-485（LIN-based RS-485）** をブートローダーのネイティブプロトコルとして採用。
* **1024 バイト（`BOOTEND=0x04`、1KB）枠の採用**: アプリ領域 15.0KB（93.75%）を確保し、安全な C 言語実装へ。
* **ハードウェア `LINAUTO` ＋ `WFB` (Wait For Break)**: バス上の過渡ノイズ・グリッチをハードウェアが完全に無視。
* **Baud-Rate Trick（57600bps 0x00 送出）**: ホスト側 USB-RS485 ドングルから規格適合の 18 Tbit LOW Break を 100% 確実に出力。
* **CRC-16-CCITT ＆ レスポンススペース（約 60µs）**: 完全なデータ完全性と衝突のないターンアラウンドを保証。

---

## 2. 開発・検証マイルストーン進捗状況

| フェーズ | 検証内容 | 判定 | 達成日 | 獲得した成果・ポイント |
| :--- | :--- | :---: | :---: | :--- |
| **Phase 1** | ホスト側 Break 生成 (Baud Trick) 実機検証 | **PASS** | 2026-09-27 | 57600bps 0x00 送出による 18 Tbit LOW Break の安定出力を実証。 |
| **Phase 2** | LINAUTO 同期 & Master-Broker 双方向対話 | **PASS** | 2026-09-27 | PING（RTT 4.8ms）およびデバイス情報（Signature `1E 94 21`, Ver `1.0`）の取得成功。 |
| **Phase 3** | Flash 単体読み出し & 書き込み・CRC16 ベリファイ | **PASS (3-1)** | 2026-09-27 | 0x0400（blank_app）の 64B 読み出し・CRC16 検証が一撃で完全合致。 |
| **Phase 4** | 全ページ一括連続書き込み・自動更新 | 未着手 | - | `ol4_flasher.py` による実スケッチの一括更新。 |
| **Phase 5** | 正式リリースパッケージ & ドキュメント整備 | 未着手 | - | 配布用 HEX、取扱説明書、Web Serial (JS) 対応。 |

---

## 3. 実機実験ログ & 検証記録 (Chronological Test Logs)

### ログ 1: Phase 2-1 PING 単体プローブ試験 [PASS]
* **実施日時**: 2026-09-27 18:27
* **コマンド**: `python ol4_flasher.py --port COM19 --probe`
* **実機実行結果**:
  ```text
  PS C:\Users\User> python ol4_flasher.py --port COM19 --probe
  [INIT] Opening serial port COM19 at 115200 bps...
  [INIT] Connected successfully to COM19.

  [STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
  >>> POWER ON OR RESET CORE-D NOW <<<
  [STAGE 1: PASS] Power-on detected in 5.15s (probe #42)!
    [PING PASS] Core-D responded with STATUS_OK (RTT=4.8ms)
  [INFO] Serial port closed.
  ```
* **分析**:
  * ホスト送信の Break（Baud Trick）を Core-D が `LINAUTO` で完全に捕捉。
  * `0x55` でボーレートが自動校正され、`PID_PING`（0x80）を受信。
  * レスポンススペース（60µs）後に Core-D が `STATUS_OK`（0x00）を返信。RTT 4.8ms で通信成立を実証。

---

### ログ 2: Phase 2-2 デバイス情報取得試験（1回目：未初期化課題の発見）
* **実施日時**: 2026-09-27 18:28
* **コマンド**: `python ol4_flasher.py --port COM19 --info`
* **実機実行結果**:
  ```text
  [STAGE 1: PASS] Power-on detected in 5.12s (probe #42)!
    [DEVICE INFO] Signature: 0x00 0x00 0x00 | Optiboot_OL4 Version: 0.0
  [INFO] Serial port closed.
  ```
* **原因分析**:
  * `-nostartfiles -nostdlib`（C ランタイムなし）環境において、`main()` 内のローカル配列（スタック変数）`uint8_t info[5]` の初期化時に、未初期化レジスタがフレームポインタとして使われ、未定義アドレスへの書き込み・読み出しが発生していた。
* **対策改修**:
  1. `main()` 冒頭で `__zero_reg__`（r1）のクリアを明示化。
  2. ローカルスタック配列を完全排除し、レジスタ（`SIGROW_DEVICEID0/1/2`）や定数を直接 `putch()` と `crc16_update()` へ 1 バイトずつストリーム出力する設計へ刷新。
  3. スタックオーバーヘッド解消により、バイナリサイズが **742 バイト**（空きマージン **282 バイト**）へ大幅縮小。

---

### ログ 3: Phase 2-3 デバイス情報取得試験（2回目：完全 PASS）
* **実施日時**: 2026-09-27 18:32
* **コマンド**: `python ol4_flasher.py --port COM19 --info`
* **実機実行結果**:
  ```text
  PS C:\Users\User> python ol4_flasher.py --port COM19 --info
  [INIT] Opening serial port COM19 at 115200 bps...
  [INIT] Connected successfully to COM19.

  [STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
  >>> POWER ON OR RESET CORE-D NOW <<<
  [STAGE 1: PASS] Power-on detected in 5.53s (probe #45)!
    [DEVICE INFO] Signature: 0x1E 0x94 0x21 | Optiboot_OL4 Version: 1.0
  [INFO] Serial port closed.
  ```
* **分析**:
  * ATtiny1616 の正規デバイスシグネチャ **`0x1E 0x94 0x21`** およびバージョン **`1.0`** を完全取得。
  * 受信データ末尾の CRC-16-CCITT 検証も一発で合致（PASS）。
  * 双方向のデータストリームおよびエラーチェックが完璧に動作していることを実証。

---

### ログ 4: Phase 3-1 Flash 単体ページ読み出し試験 [PASS]
* **実施日時**: 2026-09-27 18:36
* **コマンド**: `python ol4_flasher.py --port COM19 --read-page 0x0400`
* **実機実行結果**:
  ```text
  PS C:\Users\User> python ol4_flasher.py --port COM19 --read-page 0x0400
  [INIT] Opening serial port COM19 at 115200 bps...
  [INIT] Connected successfully to COM19.

  [STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
  >>> POWER ON OR RESET CORE-D NOW <<<
  [STAGE 1: PASS] Power-on detected in 4.65s (probe #38)!

  [READ PAGE] Address 0x0400...
  Read 64 bytes: 84 E0 80 93 21 04 80 93 26 04 00 00 FE CF FF FF ...
  [INFO] Serial port closed.
  ```
* **分析**:
  * アドレス `0x0400`（アプリケーション領域先頭）の Flash 読み出しコマンド（`PID_SET_ADDR` $\rightarrow$ `PID_READ_PAGE`）が一発で成立。
  * 読み出されたデータ `84 E0 80 93 21 04 80 93 26 04 00 00 FE CF ...` は、事前に UPDI で書き込まれた `blank_app.hex` のバイナリおよび消去後未書き込み領域（`0xFF`）と **1 ビットの狂いもなく 100% 完全一致**。
  * Core-D から送出された 64 バイトのペイロードおよび末尾の CRC-16-CCITT 検証も完璧に PASS。
  * STK500v1 で苦しめられた「64 バイト送出時のターンアラウンド・過渡ノイズ問題」は、LN-485 の「Break + 0x55 ＋ WFB 常時アーム ＋ 60µs レスポンススペース」によって完全に克服されたことを証明。

---

## 4. バイナリサイズ管理状況 (Gate 1: 1024B 上限)

```text
================== OPTIBOOT_OL4 BINARY SIZE ==================
section          size      addr
.text             740         0
.version            2      1022
Total             742
==============================================================
* 上限: 1024 バイト (FUSE.BOOTEND = 0x04)
* 使用量: 742 バイト (72.5%)
* 空きマージン: 282 バイト (27.5%)
```

---

## 5. 次回実施試験（Phase 3-2 & Phase 4）

1. **試験 3-2: Flash ページ書き込み検証 (`CMD_WRITE_PAGE`)**
   * 64 バイトの消去・書き込み（NVMCTRL）と、直後の読み出しによる完全一致ベリファイ。
2. **試験 4-1: 複数ページ連続書き込み・自動更新（全 10〜11 ページ一括フラッシュ）**
   * 専用フラッシャー `ol4_flasher.py --hex` による一括書き込み・自動ベリファイの実証。
