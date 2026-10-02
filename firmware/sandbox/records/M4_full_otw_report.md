<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M4 実機検証レポート: MR32 12KB フル OTW ファームウェア更新 ＆ アプリケーション自動起動実証

**実施日**: 2026-10-02  
**対象ハードウェア**: [ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/hardware/archive/CORE-D/proposal.md)  
**通信規格**: MR32（115,200 bps, 8N1, マジックパケット `0x55 0xAD` 同期, 32B 完全固定長）  
**検証環境**: 
- **PC 側ポート**: RS-485 (市販 USB-RS485 ドングル @ 115,200 bps), SerialUPDI (CH342K Port A), Soft-UART (CH342K Port B @ 9,600 bps)
- **基板ジャンパ**: H2 (2-3 連動), H3 (1-2 内部OSC 20MHz), H4 (OPEN: 終端抵抗無効)
- **検証スクリプト**: [`firmware/sandbox/M_milestones/M4_full_otw/m4_otw_bench.py`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M4_full_otw/m4_otw_bench.py)  
- **ファームウェア**: [`firmware/sandbox/M_milestones/M4_full_otw/m4_bootloader.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M4_full_otw/m4_bootloader.hex) (OTW コア ＆ 自動ジャンプ機構)  
- **転送対象イメージ**: [`firmware/sandbox/M_milestones/M4_full_otw/app_12k.bin`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/sandbox/M_milestones/M4_full_otw/app_12k.bin) (12,288 Bytes / 192 Pages)

---

## 1. 検証の目的とテスト項目

Milestone 4（M4）は、MR32 プロトコルにおける **実用規模ファームウェア（12KB / 192 ページ）の連続 OTW（Over-The-Wire）書き換えと、新ファームウェアの自動起動（ジャンプ）** を実機で実証するマイルストーンです：

1. **12KB（全 192 ページ / 768 チャンク）の連続エラーフリー転送**:
   - `Page 64` 〜 `Page 255`（計 12,288 バイト）を RS-485 経由で途絶えることなく連続書き込み。
2. **ページごとのハードウェア Flash 消去・書き込み ＆ CRC16 即時照合**:
   - ATtiny1616 の NVMCTRL が各ページで `PAGEERASEWRITE` を実行し、Flash ハードウェア CRC16 が期待値と完全一致することを確認。
3. **転送所要時間の実測 ＆ 目標達成判定**:
   - 12KB 全域の転送・書き込み完了所要時間が **目標値（$< 7.0\,\text{秒}$）** を達成することを確認（M3 の実測 31.6ms/P から約 6.0 秒と予測）。
4. **スポット Bit-for-Bit 物理 Flash 照合**:
   - アプリケーション先頭ページ（Page 64, 65）および最終ページ（Page 255）を読み出し、100% 完全一致を確認。
5. **ユーザーアプリケーション自動起動実証**:
   - `CMD_BOOT_APP_EXEC (0x14)` により、ブートローダーが `0x1000`（Page 64）へソフトウェアジャンプ（`ijmp`）し、ユーザーアプリ（LED 交互点滅 ＆ Soft-UART テレメトリ）が正常に起動することを確認。

---

## 2. 実機ベンチマーク測定結果 (生ログエビデンス)

### 2.1 テストベンチ実行サマリー

```text
PS C:\Users\User> python m4_otw_bench.py --port COM22 --image app_12k.bin
[INIT] Opening RS-485 port COM22 @ 115200 bps (8N1)...
[INIT] Serial port opened successfully.

=================================================================
 [TEST 1] Target Ping & Identity Check
=================================================================
 [PASS] Ping OK in 6.47 ms | MCU: 0x1616 (ATtiny1616), Flash: 16KB, Page: 64B

=================================================================
 [TEST 2] Bootloader Protection Guard (Attempt Write to Page 0)
=================================================================
 [PASS] Self-Programming Guard Activated! Target Page 0 safely REJECTED with STATUS_ERR_PARAM (0x03) in 6.17 ms.

=================================================================
 [TEST 3] 12KB Full OTW Flash Transfer (Pages 64..255 / 192 Pages)
=================================================================
 Total Payload Size  : 12288 Bytes (192 Pages)
 Memory Target Range : Address 0x1000 - 0x3FFF
 Streaming 192 Flash pages via MR32 RS-485...
  [=========================] 100.0% | Page 255/255 | RTT=30.5ms | 2.01 KB/s

 [PASS] 12KB Full OTW Flash Completed in 5.96 seconds!
        Average Page Time : 30.50 ms / page
        Effective OTW Rate: 2.01 KB/s (Target < 7.0s: PASS)

=================================================================
 [TEST 4] Spot Bit-for-Bit Readback Verification (Pages 64, 65, 255)
=================================================================
  [PASS] Page  64 (0x1000): 100% Bit-for-Bit Match (64/64 Bytes)
  [PASS] Page  65 (0x1040): 100% Bit-for-Bit Match (64/64 Bytes)
  [PASS] Page 255 (0x3FC0): 100% Bit-for-Bit Match (64/64 Bytes)
 >>> ★ ALL SPOT-CHECK PAGES MATCHED 100.0% WITH FLASH ROM ★ <<<

=================================================================
 [TEST 5] Launch User Application (CMD_BOOT_APP_EXEC: 0x14)
=================================================================
 Sending Execution Command to Core-D...
 [PASS] Bootloader acknowledged Launch Command (STATUS_OK) in 6.12 ms!
        Bootloader is performing indirect jump (ijmp) to 0x1000...
-----------------------------------------------------------------
 [ACTION] Please check the physical board:
   1. Visual: Red LED (PB2) and White LED (PB3) should alternate blink rapidly (150ms).
   2. Monitor (COM21 @ 9600 bps): Soft-UART should print user app banner:
      '🎉 ADX Core-D USER APPLICATION LAUNCHED SUCCESSFULLY! 🎉'

=================================================================
                 MILESTONE 4 FINAL SUMMARY
=================================================================
 Test 1 (Target Ping Check)    : PASS
 Test 2 (Bootloader Protection): PASS
 Test 3 (12KB Full OTW Flash)  : PASS (5.96 s / avg 30.5ms/p)
 Test 4 (Spot Bit-for-Bit Match): PASS
 Test 5 (User App Execution)   : PASS
-----------------------------------------------------------------
 >>> ★ MILESTONE 4: GRADE A+ (OFFICIALLY PASSED) ★ <<<
 Full 12KB OTW Firmware Update & Auto-Execution Confirmed!
=================================================================
[INFO] Port closed.
```

### 2.2 テスト項目判定マトリクス

| テスト項目 | 検証内容 / コマンド | 期待される挙動 | 実測結果 | 判定 |
| :--- | :--- | :--- | :---: | :---: |
| **Test 1: 生存確認** | `CMD_BOOT_PING` (0x10) | ATtiny1616 デバイス情報返信 | 0x1616, 16KB Flash, 64B Page (6.47ms) | **PASS** |
| **Test 2: 自爆保護ガード** | Page 0 への `CMD_BOOT_WRITE_CHUNK` | 書込拒絶・エラー応答（0x03） | STATUS_ERR_PARAM 即座拒絶 (6.17ms) | **PASS** |
| **Test 3: 12KB フル OTW** | Pages 64..255 (192P) 連続書込 | 192P 全完走 ＆ 各ページ CRC16 一致 | **5.96 秒** 完走 (平均 30.50ms/P, 2.01 KB/s) | **PASS** |
| **Test 4: スポット読出照合** | Pages 64, 65, 255 物理 Flash 読出 | 64/64 バイト Bit-for-Bit 100% 一致 | 3 ページすべて 100.0% 完全一致 | **PASS** |
| **Test 5: アプリ自動起動** | `CMD_BOOT_APP_EXEC` (0x14) | STATUS_OK ＆ 0x1000 へジャンプ | 6.12ms 応答 ＆ アプリ自動起動確認 | **PASS** |

### 2.3 OTW 転送性能分析

| 測定項目 | 実測値 | 目標値 / 仕様 | 判定 |
| :--- | :---: | :---: | :---: |
| **12KB (192 ページ) 総書き換え所要時間** | **5.96 秒** | $< 7.0\,\text{秒}$ (目標 6.0 秒前後) | **GRADE A+ (PASS)** |
| **1 ページあたり平均所要時間** | **30.50 ms** | $< 35.0\,\text{ms}$ | **PASS** |
| **実効 OTW スループット** | **2.01 KB/s** | $> 1.8\,\text{KB/s}$ | **PASS** |
| **スポット Flash 読み戻し一致率** | **100.00%** | 100.00% | **PASS** |

---

## 3. アプリケーション起動観測ログ (Soft-UART PB4 @ 9,600 bps)

CH342K Port B (COM21) から受信されたリアルタイム実行ログ：

```text
19:26:15.2 >  Ready for Full OTW Programming...
19:27:08.8 > 
19:27:08.8 > [BOOT] Shutting down bootloader services...
19:27:08.8 > [BOOT] Jumping to User Application @ 0x1000...
19:27:09.0 > 
19:27:09.0 > 
19:27:09.0 > ===============================================================
19:27:09.1 >    🎉 ADX Core-D USER APPLICATION LAUNCHED SUCCESSFULLY! 🎉
19:27:09.2 > ===============================================================
19:27:09.2 >  [ENTRY] Address: 0x1000 (Flash Page 64 / Application Space)
19:27:09.3 >  [INDICATORS] PB2 (Red) & PB3 (White) alternating blink active
19:27:09.4 >  [STATUS] Milestone 4 OTW Auto-Execution Verified!
19:27:09.4 > ===============================================================
19:27:09.5 > 
19:27:11.0 > [APP HEARTBEAT] Alive count: 1s | Running at 20MHz
19:27:12.6 > [APP HEARTBEAT] Alive count: 2s | Running at 20MHz
19:27:14.2 > [APP HEARTBEAT] Alive count: 3s | Running at 20MHz
19:27:15.7 > [APP HEARTBEAT] Alive count: 4s | Running at 20MHz
19:27:17.3 > [APP HEARTBEAT] Alive count: 5s | Running at 20MHz
```

---

## 4. 総合評価 ＆ 結論

- **判定**: `GRADE A+ (OFFICIALLY PASSED)` / `PASS`
- **所見**:
  - MR32 プロトコルによる **12KB（192 ページ / 768 チャンク）の連続ストリーミング OTW ファームウェア更新が 5.96 秒（実効 2.01 KB/s）で完全完走**。
  - MR32 仕様目標である「OTW 所要時間 10 秒未満」を大幅に上回る、**5秒台での 12KB フル書き換え** を達成した。
  - ブートローダーからアドレス `0x1000` への自動ソフトウェアジャンプ（`ijmp`）が完全に機能し、新ファームウェアの正常起動（LED 交互点滅 ＆ 連続ハートビート）を実機で実証。
  - これにより、MR32 の OTW ファームウェア更新メカニズムの中核技術は実機実証フェーズを完全に通過した。
