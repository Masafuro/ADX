# Milestone 4-1 (M4-1): 極小ブートローダー (<1024B) 単一 Flash ページ書き込み実証 実機エビデンス

- **実施日時**: 2026-09-28
- **対象デバイス**: Microchip ATtiny1616-MNR on ADX Core-D
- **スレーブ固有 UID**: `0x30 53 51 46 33 34 29 29 14 21`
- **対象 Flash ページ**: Page 16 (`0x0400` 〜 `0x043F`, 64 Bytes)
- **バイナリサイズ**: 1,022 Bytes (`_etext = 0x03FE` < `0x0400`)
- **テストポート**: COM19 (RS-485 @ 19200 bps), COM20 (SerialUPDI)

---

## 1. 検証結果サマリー

| 検証項目 | 期待動作 | 実測結果 | 判定 |
| :--- | :--- | :--- | :---: |
| **バイナリサイズ制約** | 1,024 バイト以内 (`_etext < 0x0400`) | **1,022 バイト (`0x03FE`)** (2B マージン) | **PASS** |
| **スレーブ自己同定** | `CMD_IDENTIFY` で UID 取得 | 取得完了 (RTT: 38.87 ms) | **PASS** |
| **ベースライン読出** | 書き込み前の Flash データ読み出し | 読み出し完了 (**全 64B が 0xFF 消去状態**) | **PASS** |
| **4チャンク書込 ＆ エコー** | 16B $\times$ 4 チャンク書き込み ＆ 即時 ACK | 4/4 チャンク ACK 受信 (RTT: 39.02〜39.90 ms) | **PASS** |
| **非同期 Flash コミット** | ACK 送出直後の 25ms 消去書き込み | **自爆・フリーズゼロ、平然と待機正常復帰** | **PASS** |
| **物理 Flash ベリファイ** | 物理 Flash からの 64B 読出照合 | **64/64 バイト 100% 完全一致 (Bit-for-Bit Match)** | **PASS** |
| **保護領域セキュリティ** | Page 0 (`0x0000`) への書き込み遮断 | **`STATUS_ERR_PROTECT (0x03)` 返信** | **PASS** |

**総合評価**: **GRADE A+ (MILESTONE 4-1 100% ACHIEVED)**

---

## 2. 実機実行ログ

```text
PS C:\Users\User> python .\m4_1_flash_bench.py --port COM19
[INIT] Opening RS-485 Serial Port COM19 @ 19200 bps...
[INIT] Connected successfully to COM19.
============================================================================
    MILESTONE 4-1: BR32 COMPACT BOOTLOADER (<1024B) BENCHMARK
============================================================================
 Target Port       : COM19 @ 19200 bps
 Target Flash Page : Page 16 (0x0400)
 Protected Area    : Pages 0..15 (0x0000..0x03FF)
 Bootloader Constraint: Size <= 1024B (Strictly Pages 0..15)
============================================================================

[DISCOVERY] Probing bus with All-Zero UID to discover slave...
[DISCOVERY] Found Slave SIGROW UID: 0x30 53 51 46 33 34 29 29 14 21 (RTT: 38.87 ms)

============================================================================
       STEP 1: BASELINE READ OF PAGE 16 (BEFORE WRITE)
============================================================================
 [Current Flash 64B] (Hex): ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 [Current Flash 64B] (ASCII): '................................................................'

============================================================================
       STEP 2: WRITING 4 CHUNKS & COMMITTING TO PHYSICAL FLASH
============================================================================
  [Chunk 0] TX: 'ADX FLASH M4 OK!' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 39.90 ms)
  [Chunk 1] TX: 'PAGE 16 @ 0x0400' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 39.19 ms)
  [Chunk 2] TX: 'ATtiny1616 NVM  ' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 39.02 ms)
  [Chunk 3] TX: 'BR32 PROTO 2026!' -> ACK [STATUS_OK] (Echo: MATCH, RTT: 39.58 ms)

============================================================================
       STEP 3: READING PHYSICAL FLASH & VERIFYING BIT-FOR-BIT
============================================================================
 [Expected Data] : 41 44 58 20 46 4c 41 53 48 20 4d 34 20 4f 4b 21 50 41 47 45 20 31 36 20 40 20 30 78 30 34 30 30 41 54 74 69 6e 79 31 36 31 36 20 4e 56 4d 20 20 42 52 33 32 20 50 52 4f 54 4f 20 32 30 32 36 21
 [Physical Flash]: 41 44 58 20 46 4c 41 53 48 20 4d 34 20 4f 4b 21 50 41 47 45 20 31 36 20 40 20 30 78 30 34 30 30 41 54 74 69 6e 79 31 36 31 36 20 4e 56 4d 20 20 42 52 33 32 20 50 52 4f 54 4f 20 32 30 32 36 21
 [Byte Match]    : 64 / 64 Bytes (100.0%)
 [VERIFY RESULT] : >>> 100% BIT-FOR-BIT PERFECT MATCH! <<<

============================================================================
       STEP 4: SECURITY TEST - WRITE TO PROTECTED PAGE 0 (0x0000)
============================================================================
  Attempted write to Page 0 -> Slave returned: STATUS_ERR_PROTECT (0x03)
  [SECURITY PASS] Slave correctly rejected write to protected bootloader area!

============================================================================
               MILESTONE 4-1 FINAL SCORECARD & RATING
============================================================================
 1. Discovery & SIGROW UID Check   : PASS (UID: 0x30535146333429291421)
 2. 4-Chunk 64B Write & Echo       : PASS
 3. Physical Flash 64B Verification: PASS (100% Match)
 4. Bootloader Area Protection     : PASS (STATUS_ERR_PROTECT)
----------------------------------------------------------------------------
 >>> FINAL GRADE: GRADE A+ (MILESTONE 4-1 100% ACHIEVED) <<<
============================================================================
```

---

## 3. 技術的考察と意義

1. **ブートローダーの完全自己完結**:
   - `BOOTEND = 0x04`（1024 バイト）のハードウェア境界線に対し、ブートローダーの全コードが `0x0000` 〜 `0x03FE`（1,022 バイト）の中に完全に収まった。
   - これにより、ハードウェアセキュリティの SPM 実行権限を 100% 維持しつつ、ユーザーアプリ領域（Page 16 / `0x0400` 以降）への安全な書き込みを達成。

2. **物理 Flash への完全なデータ刻印**:
   - Step 1 の「全 0xFF」から、Step 2 の書き込みを経て、Step 3 での「64 バイト全 512 ビット完全一致（100% Bit-for-Bit Match）」が実機にて完全に証明された。

3. **超高速・極小ジッターの非同期分離**:
   - 往復遅延時間（RTT）は 38.87ms 〜 39.90ms（ジッター $\sigma < 0.5\,\text{ms}$）。
   - 即時 ACK 返信完了後の 25ms Flash 消去書き込みが完全にマイコン内部で自律完結し、次フレームへの影響は皆無であった。
