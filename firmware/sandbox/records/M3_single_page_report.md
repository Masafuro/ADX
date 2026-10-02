<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M3 実機検証レポート: MR32 単一 Flash ページ (64B) 物理書き込み ＆ CRC 照合実証

**実施日**: 2026-10-02  
**対象ハードウェア**: [ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/archive/CORE-D/proposal.md)  
**通信規格**: MR32（115,200 bps, 8N1, マジックパケット `0x55 0xAD` 同期, 32B 完全固定長）  
**検証環境**: 
- **PC 側ポート**: RS-485 (市販 USB-RS485 ドングル @ 115,200 bps), SerialUPDI (CH342K Port A), Soft-UART (CH342K Port B @ 9,600 bps)
- **基板ジャンパ**: H2 (2-3 連動), H3 (1-2 内部OSC 20MHz), H4 (OPEN: 終端抵抗無効)
**検証スクリプト**: [`firmware/sandbox/M_milestones/M3_single_page/m3_flash_bench.py`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M3_single_page/m3_flash_bench.py)  
**ファームウェア**: [`firmware/sandbox/M_milestones/M3_single_page/m3_flash.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M3_single_page/m3_flash.hex) (NVMCTRL 物理 Flash 書込コア)

---

## 1. 検証の目的とテスト項目

Milestone 3（M3）は、構想段階にある MR32 プロトコルにおいて、**マイコン（ATtiny1616）の物理 Flash メモリ（NVMCTRL）への実消去・書き込み（OTW コア機能）** を初めて実機で実証する本番ゲートです：

1. **自爆防止ガード（ブートローダー保護）の実証**:
   - 基礎設計（[`EXPERIMENTAL_CONDITIONS_BASE_DESIGN.md`](../M_milestones/EXPERIMENTAL_CONDITIONS_BASE_DESIGN.md)）に基づき、ブートローダー保護領域である `Page 0..63`（アドレス `0x0000`〜`0x0FFF`、計 4,096 バイト / 4KB）への書き込み要求に対し、マイコンがハードウェア保護を発動して書き込みを拒絶（`STATUS_ERR_PARAM`）することを確認。
2. **物理 Flash 1 ページ（64 バイト）実書き込み**:
   - `CMD_BOOT_WRITE_CHUNK` (0x11) により 16 バイト $\times$ 4 チャンクを転送。
   - 4 チャンク受領の瞬間に ATtiny1616 の NVMCTRL が `PAGEERASEWRITE` コマンドを実行し、安全領域の物理 Flash メモリ（Page 64: `0x1000`〜`0x103F`）の消去・書き込みを約 2.5ms で完了することを確認。
   - 書き込み完了後に `NVMCTRL_CMD_NONE_gc` による明示的クリーンアップを確認。
3. **物理 Flash 読み出し ＆ Bit-for-Bit 完全一致照合**:
   - `CMD_BOOT_READ_CHUNK` (0x12) により、書き込まれた物理 Flash メモリ（`0x1000`〜`0x103F`）から直接 64 バイトを読み出し、**送信データと 1 ビットの狂いもなく 100.0% 完全一致（Bit-for-Bit Perfect Match）** することを確認。
4. **ハードウェア Flash CRC-16-CCITT 一括照合**:
   - `CMD_BOOT_CRC_CHECK` (0x13) により、物理 Flash メモリの全 64 バイトを走査して算出した CRC16 が、ホスト側元データの期待値と完全一致することを確認。
5. **書き込み所要時間の実測**:
   - Flash 消去＋書き込み処理を含む 1 ページ全体の所要時間を実測し、目標（$< 35\,\text{ms}$）を達成することを確認。

---

## 2. 実機ベンチマーク測定結果 (生ログエビデンス)

### 2.1 テストベンチ実行サマリー

```text
PS C:\Users\User> python m3_flash_bench.py --port COM22 --page 64
[INIT] Opening RS-485 port COM22 @ 115200 bps (8N1)...
[INIT] Serial port opened successfully.

=================================================================
 [TEST 1] Single Ping Check
=================================================================
 [PASS] Ping OK in 6.19 ms | MCU: 0x1616 (ATtiny1616), Flash: 16KB, Page: 64B

=================================================================
 [TEST 2] Bootloader Protection Guard (Attempt Write to Page 0)
=================================================================
 [PASS] Self-Programming Guard Activated! Target Page 0 safely REJECTED with STATUS_ERR_PARAM (0x03) in 6.18 ms.

=================================================================
 [TEST 3] Physical Flash Page Write (Page 64 / 64 Bytes)
=================================================================
 Target Physical Page : 64 (Address: 0x1000 - 0x103F)
 Expected 64B CRC16   : 0x6CA1
  Chunk 0: mask=0x01 status=OK RTT=6.35ms
  Chunk 1: mask=0x03 status=OK RTT=6.22ms
  Chunk 2: mask=0x07 status=OK RTT=6.29ms
  Chunk 3: mask=0x0F status=PAGE_DONE (NVM Flash Written!) RTT=10.74ms

 [PASS] Physical Flash Page 64 committed in 31.60 ms!
        Flash Hardware Readback CRC16: 0x6CA1 (Expected: 0x6CA1)
        CRC16 Match Confirmed immediately by MCU!

=================================================================
 [TEST 4] Physical Flash Readback & Bit-for-Bit Verification
=================================================================
  Chunk 0: Read 16B in 6.55ms (HEX: 414458204d523332204d696c6573746f)
  Chunk 1: Read 16B in 6.45ms (HEX: 6e6520333a205265616c205068797369)
  Chunk 2: Read 16B in 6.57ms (HEX: 63616c20466c61736820506167652057)
  Chunk 3: Read 16B in 6.77ms (HEX: 72697465206973204f4b203230323621)
-----------------------------------------------------------------
 [RESULT] TX ASCII: 'ADX MR32 Milestone 3: Real Physical Flash Page Write is OK 2026!'
 [RESULT] RX ASCII: 'ADX MR32 Milestone 3: Real Physical Flash Page Write is OK 2026!'
 [RESULT] TX CRC: 0x6CA1 | Physical Flash CRC: 0x6CA1
 >>> ★ 100% BIT-FOR-BIT PERFECT MATCH ON PHYSICAL FLASH (64/64 Bytes) ★ <<<

=================================================================
 [TEST 5] Hardware CRC Check Command (CMD_BOOT_CRC_CHECK: 0x13)
=================================================================
 Page: 64 | Reported Flash CRC: 0x6CA1 in 6.55 ms
 [PASS] Flash CRC Command Matched Expected Checksum Exactly!

=================================================================
                 MILESTONE 3 FINAL SUMMARY
=================================================================
 Test 1 (Ping Check)           : PASS
 Test 2 (Bootloader Protection): PASS
 Test 3 (Physical Flash Write) : PASS (31.60 ms)
 Test 4 (Bit-for-Bit Readback) : PASS
 Test 5 (Hardware Flash CRC)   : PASS
 Estimated 16KB Full OTW Time  : 8.09 s
-----------------------------------------------------------------
 >>> ★ MILESTONE 3: GRADE A+ (OFFICIALLY PASSED) ★ <<<
=================================================================
[INFO] Port closed.
```

### 2.2 テスト項目判定マトリクス

| テスト項目 | 検証内容 / コマンド | 期待される挙動 | 実測結果 | 判定 |
| :--- | :--- | :--- | :---: | :---: |
| **Test 1: 生存確認** | `CMD_BOOT_PING` (0x10) | ATtiny1616 デバイス情報返信 | 0x1616, 16KB Flash, 64B Page (6.19ms) | **PASS** |
| **Test 2: 自爆保護ガード** | Page 0 への `CMD_BOOT_WRITE_CHUNK` | 書込拒絶・エラー応答（0x03） | STATUS_ERR_PARAM 即座拒絶 (6.18ms) | **PASS** |
| **Test 3: Flash 物理書込** | Page 64 へ 4 チャンク書込 ＆ NVMCTRL 実行 | `STATUS_PAGE_DONE (0x10)` 返信 | 31.60ms / CRC 0x6CA1 一致 (Chunk 3 RTT 10.74ms) | **PASS** |
| **Test 4: 物理 Flash 読出** | `CMD_BOOT_READ_CHUNK` で 4 チャンク読出 | 64/64 バイト Bit-for-Bit 100% 一致 | 送信・受信文字列完全合致 (64/64B) | **PASS** |
| **Test 5: Flash CRC16 照合** | `CMD_BOOT_CRC_CHECK` (0x13) | 物理 Flash CRC16 完全一致 | Reported 0x6CA1 == Expected 0x6CA1 (6.55ms) | **PASS** |

### 2.3 物理 Flash 書き込み所要時間 ＆ 性能分析

| 測定項目 | 実測値 | 目標値 / 仕様 | 判定 |
| :--- | :---: | :---: | :---: |
| **1 ページ (4 チャンク + Flash 消去書込) 所要時間** | **31.60 ms** | $< 35.0\,\text{ms}$ | **PASS** |
| **物理 Flash 書き込み単体時間 (NVMCTRL 実行差分)** | **約 4.44 ms** | $< 5.0\,\text{ms}$ (消去書込2.5ms + CRC計算) | **PASS** |
| **12KB アプリケーション空間 (192 ページ) 実装換算時間** | **6.07 秒** | $< 7.0\,\text{秒}$ | **PASS** |
| **16KB 全空間 (256 ページ) 換算理論時間** | **8.09 秒** | $< 9.0\,\text{秒}$ | **PASS** |
| **Flash 読み戻し Bit-for-Bit 一致率** | **100.00% (64/64 Bytes)** | 100.00% | **PASS** |

---

## 3. デバッグモニタ (Soft-UART PB4 @ 9,600 bps) 観測ログ

```text
=======================================================
   ADX Core-D M3: MR32 Physical Flash Page Writer
=======================================================
 [CONFIG] PA1=TX, PA2=RX, PA4=DE, PA7=/RE, PB2=LED, PB3=ACT
 [DEVICE] ATtiny1616-MNR (Flash: 16KB, Page: 64B)
 [PROTECT] Pages 0..63 (0x0000 - 0x0FFF, 4KB) LOCKED
 [TARGET] Application Space (Pages 64..255) WRITABLE
 [MR32] 115,200 bps 8N1, Magic=0x55 0xAD
 Ready for Flash Self-Programming...
```

---

## 4. 総合評価 ＆ 結論

- **判定**: `GRADE A+ (ROCK SOLID)` / `PASS`
- **所見**:
  - MR32 プロトコル経由での ATtiny1616 物理 Flash メモリ書き換え（NVMCTRL PAGEERASEWRITE）が実機で完全成功。
  - 自爆防止保護（Pages 0..63 ロック）が正常に機能し、ブートローダーの安全性が確認された。
  - 通常チャンク（約 6.3ms）に対し、最終チャンクが 10.74ms を記録したことは、**マイコン内部で NVMCTRL が物理消去・書き込み（約 2.5ms）と Flash CRC16 計算を実際に実行した紛れもない物理的証拠**である。
  - 次期ステップ（M4: 12KB フル OTW ファームウェア更新 ＆ アプリケーション自動起動）へ進出可能。
