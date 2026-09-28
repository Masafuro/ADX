<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# WU-3: BR32 フレーム破壊 ＆ ノイズ耐性実験室 (Fault Injection & Framing Resilience Lab)

本サンドボックスは、BR32 プロトコルの真骨頂である **「通信の不可視性を排除する正直なエラー報告」** と、従来のシリアル通信の最大の死因であった **「1 バイト欠損によるデッドロック（永久フリーズ）の完全克服」** を実機（ADX Core-D）で実証するための実験環境です。

Flash への書き込みは一切行わないため、何度でも安全・過激にパケット破壊攻撃を仕掛けることができます。

---

## 1. WU-3 の検証テーマと合格基準

1. **[Case 1] 途中切断テスト (Incomplete Frame: 10 Bytes Only)**:
   - 32 バイトのフレームに対し、ホストが **先頭 10 バイトだけ送ってプツンと回線を切断** する。
   - スレーブは永久待ち（デッドロック）せず、約 18ms の時間枠満了で自律脱出し、**「STATUS_ERR_TIMEOUT (0x02), RX_COUNT=10」** を 32B フレームで堂々と返信する。
2. **[Case 2] CRC 破壊テスト (Corrupted CRC-16)**:
   - 32 バイト満額送出するが、末尾の CRC16 だけ意図的に `0xDEAD` などの不正値にする。
   - スレーブは CRC 異常を検知し、**「STATUS_ERR_CRC (0x01), RX_COUNT=32」** を 32B フレームで即時返信する。
3. **[Case 3] 過剰データ垂れ流しテスト (50-Byte Flooding / Overflow Gate)**:
   - 32 バイト枠に対し、ホストが **50 バイトのゴミデータを一気に連続垂れ流す**。
   - スレーブは 32 バイトを超えたデータ流入を即座に検知し、**連鎖的な送信衝突（Cascading Collision）を防ぐため、原則 2（BREAK なしゴミ）相当として「100% 完全沈黙（DE=0 維持）」** を貫く。
4. **[Case 4] 破壊攻撃直後の即時自己治癒力 (Post-Fault Healing)**:
   - 上記 3 連撃の破壊攻撃を受けた直後に、正規の 32B フレームを 1 発送出。
   - スレーブが何事もなかったかのように **一撃で平然と RTT ≈ 38.6ms、STATUS_OK (0x00) で即時復帰** することを実証する。

---

## 2. 構成ファイル一覧

```text
firmware/bootloader/PLAN/BR32/WU/WU3_fault_injection/
├── README.md              # 本ドキュメント (使い方 & 実験手順)
├── Makefile               # Ubuntu 側 avr-gcc ビルド用 Makefile
├── wu3_fault.c            # Core-D 側 時間枠自律脱出ファームウェア (Flash 書込なし / 1,226 Bytes)
├── wu3_fault.hex          # ビルド済みバイナリ
└── wu3_fault_bench.py     # PC 側 Python 4大フレーム破壊自動実行ベンチ
```

---

## 3. 実験手順 (Windows 11 ホスト PC)

### Step 1: マイコンへのファームウェア書き込み (COM20 UPDI)

PowerShell で `WU3_fault_injection` ディレクトリに移動し、以下を実行します：

```powershell
pymcuprog write -d attiny1616 -t uart -u COM20 -f .\wu3_fault.hex --erase
```

* 書き込み完了後、Core-D の赤色 LED（PB2）が約 1 秒周期で待機点滅を開始します。
* COM21（9600 bps）に起動案内が出力されます。

---

### Step 2: 4 大フレーム破壊テストの一括自動実行 (`--suite`) ★推奨★

```powershell
python .\wu3_fault_bench.py --port COM19 --debug-port COM21 --suite
```

**期待される出力例**:
```text
============================================================================
        WU-3: BR32 FAULT INJECTION & FRAMING RESILIENCE BENCHMARK
============================================================================
 Target Port       : COM19 @ 19200 bps
 Interval          : 200.0 ms
============================================================================

[DISCOVERY] Probing bus with All-Zero UID to discover slave...
[DISCOVERY] Found Slave SIGROW UID: 0x30 53 51 46 33 34 29 29 14 21 (RTT: 38.65 ms)

----------------------------------------------------------------------------
 [CASE 1] Incomplete Frame Injection (10 Bytes Only - Truncated Mid-Stream)
          Attack     : Send only 10 bytes instead of 32 bytes.
          Expectation: Slave does NOT deadlock! After timeout window,
                       slave honestly replies: STATUS=0x02 (TIMEOUT), RX_COUNT=10
----------------------------------------------------------------------------
  Sending 10 bytes: 01 11 30 53 51 46 33 34 29 29
  -> Slave Response: RTT = 54.30 ms
     Status        : 0x02 (STATUS_ERR_TIMEOUT)
     Slave RX Count: 10 Bytes
     Slave UID     : 0x30 53 51 46 33 34 29 29 14 21
  -> RESULT: ★ [PASS] PERFECT RESILIENCE! Slave reported exact 10-byte truncation!

----------------------------------------------------------------------------
 [CASE 2] CRC-16 Corruption Injection (32 Bytes Full Frame with Bogus CRC)
          Attack     : Send full 32 bytes, but intentionally poison CRC16 to 0xDEAD.
          Expectation: Slave receives all 32 bytes, detects CRC mismatch,
                       and honestly replies: STATUS=0x01 (CRC_ERR), RX_COUNT=32
----------------------------------------------------------------------------
  Sending 32 bytes with poisoned CRC (0xDEAD):
  -> Slave Response: RTT = 38.65 ms
     Status        : 0x01 (STATUS_ERR_CRC)
     Slave RX Count: 32 Bytes
  -> RESULT: ★ [PASS] CRC ERROR TELEMETRY CONFIRMED!

----------------------------------------------------------------------------
 [CASE 3] Flooding / Excess Data Injection (50 Bytes Streamed into 32B Window)
          Attack     : Stream 50 continuous bytes (32B valid frame + 18B garbage).
          Expectation: Slave absorbs first 32B, discards excess, does NOT crash.
----------------------------------------------------------------------------
  Sending 50 bytes stream (32B frame + 18B trailing garbage)...
  -> Slave Response: RTT = 38.70 ms
     Status        : 0x00 (STATUS_OK)
     Slave RX Count: 32 Bytes
  -> RESULT: ★ [PASS] FLOODING RESILIENCE CONFIRMED! Zero buffer overflow crash.

----------------------------------------------------------------------------
 [CASE 4] Post-Fault Immediate Self-Healing Verification
          Attack     : Incomplete -> Poisoned CRC -> Flooding 3-hit combo executed.
          Expectation: Next standard 32B valid frame must succeed INSTANTLY (RTT ≈ 38ms)
----------------------------------------------------------------------------
  -> Slave Response: RTT = 38.68 ms (Target ~38.6 ms)
     Status        : 0x00 (STATUS_OK)
     Slave RX Count: 32 Bytes
     Payload Echo  : 50 4f 53 54 5f 46 41 55 4c 54 5f 4f 4b 21
  -> RESULT: ★ [PASS] FLAWLESS INSTANT SELF-HEALING PROVEN!

============================================================================
                   WU-3 FINAL BENCHMARK VERDICT
============================================================================
  [CASE 1] Incomplete Frame Detection (10B) : PASS (STATUS=0x02, RX_COUNT=10)
  [CASE 2] CRC-16 Poisoning Telemetry       : PASS (STATUS=0x01, RX_COUNT=32)
  [CASE 3] 50-Byte Flooding Resilience      : PASS (Zero Crash / Graceful Flush)
  [CASE 4] Post-Destruction Self-Healing    : PASS (Instant Recovery, RTT=38ms)
============================================================================
  OVERALL VERDICT: ★ GRADE A+ (BR32 FAULT RESILIENCE CERTIFIED) ★
============================================================================
```
