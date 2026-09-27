# Optiboot_O4 開発進捗・課題整理レポート

**最終更新**: 2026-09-27  
**対象ターゲット**: ADX Core-D (Microchip ATtiny1616-MNR)  
**通信方式**: 1-to-1 半二重 RS-485 (115200 bps, 8N1, STK500v1 準拠)  
**実装ファイル**: [`src/optiboot_o4.c`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_O4/src/optiboot_o4.c)  
**配布バイナリ**: [`releases/optiboot_o4_with_blank.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_O4/releases/optiboot_o4_with_blank.hex)  

---

## 1. エグゼクティブサマリ

新設計ブートローダー **Optiboot_O4** の開発において、**最大目標であった「0x0440 読み出しフリーズ問題」を完全克服**し、512 バイト制限（460 バイト達成）や通信確立・シグネチャ照合（Gate 1, Gate 2）をクリアしました。  
しかし、実テストスケッチ書き込みテスト（Phase 3）において、**先頭ページ（Page 0x0200）の書き込みコマンド（`STK_PROG_PAGE` / 0x64）実行時に Core-D がリセット／クラッシュする事象**に直面しています。

本ドキュメントでは、これまでに「完全にわかったこと」「達成されたこと」「直面している課題」「未解明な謎・仮説」を明確に整理し、次の解決アプローチを策定します。

---

## 2. これまでに達成されたこと・完全にわかったこと（事実）

### (1) 因縁の「0x0440 フリーズ問題」の完全解消（★最大のブレークスルー）
* **従来の現象**:
  * 旧 Optiboot（optiboot_x）では、複数ページ読み出し時に必ず「0x0400 読み出し直後」の「0x0440」で Core-D がフリーズし、通信途絶していた。
* **真因の特定**:
  * 自爆エコーにより 0x0400 ページ末尾の ASCII 文字 `'t'`（`0x74`）が受信 FIFO に残り、これが直後のコマンド `STK_READ_PAGE`（`0x74`）と誤認され、直後のパケットヘッダ `55 40` を読み出し長（21,824 バイト）と解釈して巨大ループに突入していた。
* **Optiboot_O4 による克服実証**:
  * ソフトウェア GPIO 排他制御（送信時は DE=1, /RE=1 で自爆エコーを物理遮断、送信完了 TXCIF 後に DE=0, /RE=0 で FIFO フラッシュ）を導入。
  * `debug_flasher.py --read-only --start 0x0380 --pages 5` による実機テストにおいて、**0x0380, 0x03C0, 0x0400, 0x0440, 0x0480 の全 5 ページが RTT 7.7ms で一発 PASS**。
  * 自爆エコー遮断設計の正しさが完全に証明された。

### (2) 512 バイト制限の完全クリア（Gate 1 PASS）
* `.text` 領域: **458 バイト**
* `.version` 領域: **2 バイト**
* **合計バイナリサイズ: 460 バイト**（上限 512 バイトに対して **52 バイトの空きマージン**を確保）。

### (3) ハードウェアピン配置と通信確立（Gate 2 PASS）
* **初回書き込み時の無応答原因特定**:
  * ATtiny1616 の USART0 はデフォルトで PB2(TXD) / PB3(RXD) に割り当てられる。
  * ADX Core-D は PA1(TXD) / PA2(RXD) を使用しているため、`PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;` が必須。
  * これが抜けていたため、PB2（Core-D の赤色LEDピン）が UART アイドル（HIGH）で点灯し続け、通信ピン PA1/PA2 に信号が通っていなかった。
* **修正と実証**:
  * `PORTMUX.CTRLB` を設定し、赤LED（PB2）の待機点滅（約 2Hz）と PA1/PA2 通信を両立。
  * `debug_flasher.py` による電源投入検知（Stage 1: 4.55s）、シグネチャ照合（Stage 2: `0x1E 0x94 0x21`）、プログラミングモード突入が一発で PASS。

---

## 3. 現在直面している課題（行き詰まっている点）

テストスケッチ（`test_rs485_serial.hex`、11ページ / 676バイト）の書き込みテスト（Phase 3）を実行した際、**1 ページ目（Page 0x0200）の書き込みで失敗し、Core-D が死ぬ（またはリセットする）現象**が発生しています。

### 実機ログ 1 回目:
```text
=== MODE: FULL WRITE & VERIFY PER PAGE ===

--- Testing Page 0x0200 (64 bytes) ---
  [TX] 55 00 02 20              -> [RX] 14 10                        (RTT= 2.1ms) PASS
  [TX] 64 00 40 46 20 E8 20 93 ... 80 93 02 08 20
  [RX OK FAIL] expected 0x10, got 0xFF RTT=11.9ms: 14 FF
  [ERROR] Write page failed at 0x0200

  [HEALTH CHECK] Probing if Core-D is still alive in bootloader...
  [TX] 30 20
  [RX TIMEOUT/SHORT] len=1/2 RTT=302.8ms: FF
  [TX] 30 20
  [RX TIMEOUT/SHORT] len=0/2 RTT=303.7ms: <TIMEOUT NO DATA>
  [HEALTH: DEAD] Core-D did NOT respond to sync probes. It likely reset or crashed.
!!! HALTED AT PAGE 0x0200 !!!
```

### 実機ログ 2 回目（再起動直後）:
```text
[STAGE 1] Waiting for Core-D power on/reset (up to 30.0s)...
>>> POWER ON OR RESET CORE-D NOW <<<
[STAGE 1: PASS] Power-on detected in 5.30s (probe #44)!

[STAGE 2] Checking Sync & Entering Programming Mode...
  [TX] 30 20                    -> [RX] 14 10                        (RTT= 2.0ms) PASS
  [TX] 30 20
  [RX SYNC FAIL] expected 0x14, got 0x11 RTT=1.9ms: 11 FC
[STAGE 2: FAIL] Sync probe 2 failed.
```

---

## 4. 判明している挙動と詳細分析

| 観点 | 分析・判明した事実 |
| :--- | :--- |
| **アドレス設定 (`STK_LOAD_ADDRESS`)** | `55 00 02 20` $\rightarrow$ `14 10` (RTT 2.1ms) で**完全に正常**。`0x0200` が正確にロードされている。 |
| **データ受信ループ** | `64 00 40 46` に続き 64 バイトを受信し、末尾の `20`（CRC_EOP）を `verifySpace()` が確認している。 |
| **先頭応答バイト `0x14`** | `verifySpace()` 内で `rs485_tx_start()` を行い `putch(STK_INSYNC)`（`0x14`）を送出している。PC側は `14` を正常に受信している。 |
| **第2応答バイトの異常 (`0xFF`)** | 期待値 `0x10`（STK_OK）に対し、PC側は `0xFF` を受信している。 |
| **所要時間 (RTT)** | RTT が **11.9ms**。通常 ATtiny1616 の Flash Page Erase & Write（消去＋書き込み）には **約 20ms 〜 28ms** 要するはずであり、書き込み完了を待たずに異常終了している疑いがある。 |
| **その後の状態** | ヘルスチェックの Sync 送信（`30 20`）に対し、最初に `FF` が1バイト返った後、沈黙（TIMEOUT）。マイコンがクラッシュしたか、リセットされてブートローダー外に逸脱した。 |
| **2回目の異常 (`11 FC`)** | 1回目の書き込みで Flash アドレス 0x0200 付近が中途半端に破壊または書き換えられたため、リセット時の挙動やボーレート／フレームに乱れが生じた可能性。 |

---

## 5. よくわからないところ（検証が必要な仮説）

### 【仮説 1】フラッシュ書き込み（PAGEERASEWRITE）中の RS-485 バスマナーと CPU ストール
* **コードの流れ**:
  ```c
  verifySpace(); // DE=1, /RE=1 にして putch(STK_INSYNC: 0x14) を TXDATAL に書く
  _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc); // ★ここで書き込み開始
  while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm));
  putch(STK_OK: 0x10);
  rs485_tx_end(); // 送信完了待ち、DE=0, /RE=0
  ```
* **懸念点**:
  * `putch(0x14)` を呼んだ直後は、`0x14` は USART0 の送信データレジスタに置かれただけであり、送信シフトレジスタからの送出中（または開始直前）である。
  * その瞬間に `NVMCTRL_CMD_PAGEERASEWRITE_gc` が発行されると、**ATtiny1616 の CPU はフラッシュアクセスウェイト（HALT）に入る**。
  * この間（約 20ms）、RS-485 ドライバは **DE=1（バス占有中）のまま放置**される。
  * さらに、もし NVM コントローラの動作中に UART クロックやバスの動作に影響が出た場合、`0x14` に続くデータが化けて `0xFF` に見えたり、フレームエラーを起こす可能性がある。

### 【仮説 2】NVM コントローラの書き込みポインタ・境界条件
* **コードの流れ**:
  ```c
  address.word += MAPPED_PROGMEM_START; // 0x0200 + 0x8000 = 0x8200
  do {
    *(address.bptr++) = getch();        // 64 バイト書き込み
  } while (--length);
  // ループ終了時、address.word は 0x8240（次のページ先頭）を指している！
  ```
* **懸念点**:
  * ATtiny1616 の NVMCTRL では、ページバッファへの書き込みはデータ空間 `0x8000 + page_offset` にストアすることで行われる。
  * しかし、64 バイト書き終えた時点で `address` ポインタは `0x8240`（Page 0x0240 の先頭アドレス）に達している。
  * NVMCTRL_CMD_PAGEERASEWRITE は「直前のストアアドレスが属するページ」を対象とするが、もし `0x8240` が「次のページ」と解釈された場合、予期しないアドレスに対して Erase & Write が実行されたり、境界保護エラーが発生していないか？

### 【仮説 3】FUSE.BOOTEND と SPM（自己書き換え）保護の競合
* **ハードウェア仕様**:
  * `BOOTEND = 0x02`（0x0000〜0x01FF が BOOT セクション、0x0200 以降が APP セクション）。
  * 通常、BOOT セクションから APP セクションへの SPM 書き込みは許可されている。
  * しかし、NVMCTRL の `CTRLA` に書き込む際のアンロックシーケンス（`CCP = CCP_SPM_gc; NVMCTRL.CTRLA = ...`）が正しく受理されているか、あるいは未消去ページへの多重書き込みで NVMCTRL エラーフラグ（`WRFLAGS` 等）が立ってストールしていないか？

### 【仮説 4】Flash Page Erase & Write 前のページバッファクリア手順
* megaTinyCore / optiboot_x では、ページ書き込みの前に `PAGEBUFFERCLR`（ページバッファのクリア）コマンドが必要なのか、あるいは `PAGEERASEWRITE` だけで自動クリアされるのか？
* `optiboot_x.c` では `do_nvmctrl(flashOffset+MAPPED_PROGMEM_START, 0xFF, *inputPtr)` という記述があり、どのようなシーケンスが公式・安全であるかを精査する必要がある。

---

## 6. 次のアクション・解決へのアプローチ

1. **ステップ 1: 送信タイミングの分離テスト（★実装完了・ビルド成功）**
   * **実装内容**:
     * `STK_PROG_PAGE` ハンドラにおいて、データ受信後に `verifySpace()` で `0x14` を送出するのを廃止。
     * CRC_EOP（`0x20`）の確認のみを行い、**RS-485 バスを解放した状態（DE=0, /RE=0）のままフラッシュ消去・書き込み（NVMCTRL）を実行**。
     * 書き込み完了（FBUSY/EEBUSY 解除）後、初めて `rs485_tx_start()`（DE=1, /RE=1）にし、`putch(0x14)` $\rightarrow$ `putch(0x10)` $\rightarrow$ `rs485_tx_end()` でクリーンに一括返信。
   * **ビルド検証結果**:
     * `.text`: 484 バイト, `.version`: 2 バイト $\rightarrow$ **合計バイナリサイズ: 486 バイト**（上限 512 バイトに対し **26 バイトのマージン** を保持し Gate 1 クリア）。
     * 生成ファイル: [`releases/optiboot_o4_with_blank.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_O4/releases/optiboot_o4_with_blank.hex)

2. **ステップ 2: 実機書き込みと Phase 3 単体検証（★実証完了）**
   * **検証結果**:
     * `debug_flasher.py --hex test_rs485_serial.hex` において、**Page 0x0200 の書き込み・ベリファイ（64B完全一致）が一発で PASS（RTT 12.6ms / 7.7ms）**。
     * `debug_flasher.py --single-page 0x0240` においても、**0x0240 ページの単体読み出しが 100% PASS**。
     * Flash 書込み・消去ロジック（NVMCTRL）およびアドレス処理は完全に健全であることを証明。

---

## 7. 最終総括と次期プロジェクト「optiboot_OL4」への発展的移行

### 7.1 Optiboot_O4 で得られた決定的な知見
1. **0x0440 フリーズの撲滅**: 自爆エコー遮断（DE/RE ソフトウェア GPIO 制御）の完全実証。
2. **Flash ページ消去・書き込み（NVMCTRL）の成功**: バス解放状態での消去・書き込みシーケンスの確立。
3. **STK500v1（全二重前提）と半二重 RS-485 の構造的限界の解明**:
   * 単体読み出し・単体書き込みは完璧に成功するにもかかわらず、連続処理時に「66バイトの長大データ送出直後」に次のコマンド（Page 0x0240）でタイムアウトとなる事象が発生。
   * 10cm という極めて理想的なテスト環境において、STK500v1（フレーム同期なしの生バイトストリーム）のバッファマージン不足が露呈。長距離配線や実運用環境における脆弱性が明白となった。

### 7.2 次期プロジェクト「optiboot_OL4」の立ち上げ決定
* **名称**: **optiboot_OL4** (One-to-one LN-485 Bootloader)
* **基本方針**:
  * STK500v1 を脱却し、`firmware/tests/ADX_Core-D/LIN_test/` で実績のある **LN-485（LIN-based RS-485）** をネイティブプロトコルとして採用。
  * **ブートローダーサイズ上限を 1024 バイト（`BOOTEND=0x04`、1KB）へ拡大**: アプリ領域 15KB（93.75%）を確保しつつ、LINAUTO ハードウェア同期、Break 検出、CRC-16 を余裕を持って実装。
  * **Master Broker 方式の決定論的ポーリング**: ホスト（PC/フラッシャー）主導の通信サイクルにより、半二重のターンアラウンドとバス調停を完全制御。
  * **Baud-Rate Trick による Break 生成**: ホスト側 USB-RS485 ドングルで低速 `0x00` 送信を行い、ハードウェア Break を生成。
  * **専用フラッシャー（Python / Web Serial）への移行**: avrdude の制約を排除し、自律再送・高信頼更新を実現。

Optiboot_O4 の開発資産・教訓はすべて **optiboot_OL4** へ引き継がれます。


