<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# WU-1: MR32 32-Byte 固定長フレーム送受信 ＆ Fast Reject 実証ベンチ

本サンドボックスは、MR32 仕様の中核である **「マジックパケット（`0x55 0xAD`）同期による 32 バイト完全固定長フレーム通信」** および **「超高速破棄（Fast Reject）」** を実機（ADX Core-D）で実証するための実験模型です。

Flash メモリへの書き込みは一切行わないため、何度実行してもマイコンが壊れる（文鎮化する）心配はありません。

---

## 🛠️ ハードウェア結線 ＆ ジャンパ設定 (ADX Core-D)

```text
 ┌──────────────────────┐                     ┌──────────────────────────┐
 │ PC (Windows / Linux) │                     │ ADX Core-D (実験基板)   │
 │                      │  USB Type-C ケーブル│  - CH342K Port A (UPDI)  │
 │                      ├────────────────────►│  - CH342K Port B (UART)  │
 │                      │                     │  - ATtiny1616-MNR        │
 │                      │                     └──────────┬───────────────┘
 │                      │                                │ 3P 端子台 (A, B, GND)
 │                      │  市販 USB-RS485 ドングル        │
 │                      ├────────────────────────────────┘
 └──────────────────────┘  (CH340 / CP2102 等)
```

1. **ジャンパ設定確認**:
   - **H2 (DE/RE 制御)**: **2-3 ショート**（DE と /RE を連動させ、PA4 で半二重制御）
   - **H3 (クロック)**: **1-2 ショート**（内部高周波オシレータ 20MHz 使用）
   - **H4 (終端抵抗)**: **OPEN**（机上・短距離テストでは無効でOK）
2. **ポート確認**:
   - PC に Core-D と USB-RS485 ドングルを接続し、認識された COM ポート番号を確認します。
   - 例:
     - `COM19`: 市販 USB-RS485 ドングル
     - `COM20`: CH342K Port A (SerialUPDI 書き込みポート)
     - `COM21`: CH342K Port B (Soft-UART デバッグログ @ 9,600 bps)

---

## 📥 手順 1: マイコンへのファームウェア書き込み

### オプション A: `pymcuprog` コマンドライン（推奨・超速）
```bash
# COM20 が UPDI ポートの場合
pymcuprog write -t uart -u COM20 -d attiny1616 -f wu1_fast_reject.hex --erase
```

### オプション B: ソースコード再ビルド（Linux / WSL）
```bash
make clean all
```

---

## 🚀 手順 2: Python ベンチマークの実行

```bash
# ポート一覧の確認
python wu1_fast_reject_bench.py --list

# ベンチマーク実行 (例: COM19 が RS-485 ポートの場合)
python wu1_fast_reject_bench.py --port COM19 --cycles 300
```

### 実行されるテスト項目:
1. **[TEST 1] Single Ping Check**:
   - `CMD_BOOT_PING` (0x10) を送信し、マイコン（ATtiny1616, Flash 16KB, Page 64B）の 32B 応答と CRC16 を検証。
   - 応答時、基板上の **赤色 LED (PB2) がトグル点灯/消灯** します。
2. **[TEST 2] Fast Reject Suite**:
   - TC-1: 先頭ゴミバイト注入 $\rightarrow$ 完全沈黙確認
   - TC-2: 偽マジックバイト（`0x55 0x00`） $\rightarrow$ 完全沈黙確認
   - TC-3: 他ノード宛て（`DST_ID = 0x02`） $\rightarrow$ 0.2µs 判定・完全沈黙確認
   - TC-4: CRC 破損パケット $\rightarrow$ 完全沈黙確認
   - TC-5: 1,000 バイト連続ファジング直後の即時 Ping 自己治癒確認
3. **[TEST 3] Long-Run Stability & Jitter Test**:
   - 300 サイクル連続 Ping-Pong。往復遅延（RTT）とジッター（$\sigma$）を統計計測。

---

## 📝 手順 3: 結果の記録

ベンチマークスクリプトの実行完了時に画面に表示される統計レポートをコピーし、  
以下のレポートファイルに貼り付けて記録・コミットします：

📄 **[`firmware/sandbox/records/WU1_fast_reject_report.md`](../records/WU1_fast_reject_report.md)**
