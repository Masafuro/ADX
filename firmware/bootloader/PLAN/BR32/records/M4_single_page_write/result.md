# Milestone 4 (M4): 単一 Flash ページ消去・書き込み ＆ 非同期分離実証 実機エビデンス

- **実施日時**: 2026-09-28
- **対象デバイス**: Microchip ATtiny1616-MNR on ADX Core-D
- **スレーブ固有 UID**: `0x30 53 51 46 33 34 29 29 14 21`
- **対象 Flash ページ**: Page 16 (`0x0400` 〜 `0x043F`, 64 Bytes)
- **テストポート**: COM19 (RS-485 @ 19200 bps), COM20 (SerialUPDI), COM21 (Soft-UART PB4 @ 9600 bps)

---

## 1. 検証結果サマリー

| 検証項目 | 期待動作 | 実測結果 | 判定 |
| :--- | :--- | :--- | :---: |
| **スレーブ自己同定** | `CMD_IDENTIFY` で UID 取得 | 取得完了 (RTT: 75.50 ms) | **PASS** |
| **ベースライン読出** | 書き込み前の Flash データ読み出し | 読み出し完了 (64B) | **PASS** |
| **4チャンク書込 ＆ エコー** | 16B $\times$ 4 チャンク書き込み ＆ 即時 ACK | 4/4 チャンク ACK 受信 (RTT: ~104ms) | **PASS** |
| **非同期 Flash コミット** | ACK 送出直後の 25ms 消去書き込み | Flash 消去書き込み実行成功 | **PASS** |
| **物理 Flash ベリファイ** | 物理 Flash からの 64B 読出照合 | 読出時に無応答 (No Resp) | **FAIL (自爆)** |
| **保護領域セキュリティ** | Page 0 (`0x0000`) への書き込み遮断 | `STATUS_ERR_PROTECT (0x03)` 返信 | **PASS** |

**総合評価**: **CONDITIONAL PASS (物理書き込み成功 ＆ コード自己上書き自爆を特定 $\rightarrow$ M4-1 へ昇華)**

---

## 2. 実行ログ（自爆現象の観測）

```text
PS C:\Users\User> python .\m4_flash_bench.py --port COM19 --debug-port COM21
[DEBUG-MON] Listening on COM21 @ 9600 bps (Soft-UART PB4)
[INIT] Opening RS-485 Serial Port COM19 @ 19200 bps...
[INIT] Connected successfully to COM19.
============================================================================
     MILESTONE 4: BR32 SINGLE FLASH PAGE ERASE/WRITE BENCHMARK
============================================================================
 Target Port       : COM19 @ 19200 bps
 Target Flash Page : Page 16 (0x0400)
 Protected Area    : Pages 0..15 (0x0000..0x03FF)
============================================================================

[DISCOVERY] Probing bus with All-Zero UID to discover slave...
  |-> [COM21] [CMD: IDENTIFY] Responding UID
[DISCOVERY] Found Slave SIGROW UID: 0x30 53 51 46 33 34 29 29 14 21 (RTT: 75.50 ms)

============================================================================
       STEP 1: BASELINE READ OF PAGE 16 (BEFORE WRITE)
============================================================================
  |-> [COM21] [CMD: READ_CHUNK] Read 16B from Physical Flash [0x0400]
  |-> [COM21] [CMD: READ_CHUNK] Read 16B from Physical Flash [0x0410]
  |-> [COM21] [CMD: READ_CHUNK] Read 16B from Physical Flash [0x0420]
  |-> [COM21] [CMD: READ_CHUNK] Read 16B from Physical Flash [0x0430]
 [Current Flash 64B] (Hex): b1 06 c9 f7 5b de 83 ed 97 e8 de c0 89 a1 0d a5 6e a4 f6 2d f3 70 7f 2e f7 01 91 2f 11 92 9a 95 e9 f7 9a a1 9a 83 1b 83 47 2d 50 e0 9a 01 64 e0 22 0f 33 1f 6a 95 e1 f7 21 60 2c 83 9e 01 2b 5f
 [Current Flash 64B] (ASCII): '....[...........n..-.p...../............G-P...d.".3.j...!`,...+_'

============================================================================
       STEP 2: WRITING 4 CHUNKS & COMMITTING TO PHYSICAL FLASH
============================================================================
  |-> [COM21] [CMD: WRITE_CHUNK] Page 0x10 Chunk #0 buffered in SRAM
  [Chunk 0] TX: 'ADX FLASH M4 OK!' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 104.60 ms)
  |-> [COM21] [CMD: WRITE_CHUNK] Page 0x10 Chunk #1 buffered in SRAM
  [Chunk 1] TX: 'PAGE 16 @ 0x0400' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 105.22 ms)
  |-> [COM21] [CMD: WRITE_CHUNK] Page 0x10 Chunk #2 buffered in SRAM
  [Chunk 2] TX: 'ATtiny1616 NVM  ' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 104.02 ms)
  |-> [COM21] [CMD: WRITE_CHUNK] Page 0x10 Chunk #3 buffered in SRAM
  [Chunk 3] TX: 'BR32 PROTO 2026!' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 104.33 ms)
  |-> [COM21] |-> [ASYNC NVM] Committing Page 0x10 (Flash Addr 0x0400) Erase & Write...
  |-> [COM21] |-> [ASYNC NVM] Flash write complete! (Slack time ~158ms remaining)

============================================================================
       STEP 3: READING PHYSICAL FLASH & VERIFYING BIT-FOR-BIT
============================================================================
  [Read Chunk 1] FAILED! Status: No Resp
[FAIL] Step 3 Read back failed.
```

---

## 3. 原因究明と得られたナレッジ

1. **第 1 段階（書き込みスルー）**:
   - 初回実行時、`nvm_commit_page_hw` がアドレス `0x0522`（APP 領域）にあったため、`BOOTEND = 0x04`（`0x0400` 境界）のハードウェア保護回路により SPM コマンドが完全無視（スルー）された。
2. **第 2 段階（アプローチ 2 適用 ＆ 自爆）**:
   - `nvm_commit_page_hw` をアドレス `0x01A8`（BOOT 領域）へ移動したことで、ハードウェア保護の門が開き、物理 Flash（`0x0400`）の消去・書き込みが完全に成功した。
   - しかし、プログラム全体が 1,506 バイトあったため、**`0x0400` 〜 `0x043F` には現在実行中の `main()` 関数のコマンド処理コードが配置されていた**。
   - その結果、物理 Flash への書き込みが成功した瞬間にマイコン自身の実行コードがテスト文字列（`"ADX FLASH M4 OK!..."`）で上書きされ、直後の Step 3 で暴走・自爆した。
3. **M4-1 への昇華**:
   - 本現象の解明により、「ブートローダー全体を `0x0400`（1,024 バイト）未満に収めなければならない」という絶対的要件が確立され、Milestone 4-1（1,022 バイト極小ブートローダー）の開発・大勝利へと繋がった。
