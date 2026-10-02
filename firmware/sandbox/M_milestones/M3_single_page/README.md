<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Milestone 3 (M3): MR32 単一 Flash ページ (64B) 物理書き込み ＆ CRC 照合実証

本マイルストーンは、MR32 仕様における **物理 Flash メモリの Self-Programming（消去・書き込み）および CRC 照合** を実機（ADX Core-D）で初めて実証する本番ゲートです。

---

## 🛡️ 鉄壁の自爆防止ガード（先行実験フェーズ: 4KB 保護設計）

ファームウェア（`m3_flash.c`）には、先行実験フェーズの基礎設計（[`EXPERIMENTAL_CONDITIONS_BASE_DESIGN.md`](../EXPERIMENTAL_CONDITIONS_BASE_DESIGN.md)）に基づき、自爆を 100% 防ぐハードウェア保護境界が設定されています：

```text
 0x0000 ┌──────────────────────────────────────┐
        │  Pages 0..63 (4,096 Bytes / 4KB)      │
        │  【LOCKED: 実験ブートローダー保護領域】│ ← 書き込み試行時は即座に REJECT (0x03)
 0x1000 ├──────────────────────────────────────┤
        │  Pages 64..255 (12,288 Bytes / 12KB)  │
        │  【WRITABLE: アプリケーション領域】   │ ← Page 64 (0x1000) への実書き込みを検証
 0x3FFF └──────────────────────────────────────┘
```

万一、Flash 書き込み実験で異常が生じても、ADX Core-D は **CH342K Port A (SerialUPDI)** からいつでも 1 秒で再書き込みできるため、物理的な文鎮化リスクはゼロです。

---

## 🛠️ ハードウェア結線 ＆ ジャンパ設定 (ADX Core-D)

WU-1 / WU-3 とまったく同一です：
- **H2 (DE/RE)**: 2-3 ショート（連動制御）
- **H3 (クロック)**: 1-2 ショート（内部 20MHz）
- **H4 (終端抵抗)**: OPEN
- **COM ポート例**:
  - `COM20`: CH342K Port A (SerialUPDI 書き込みポート)
  - `COM21`: CH342K Port B (Soft-UART デバッグログ @ 9,600 bps)
  - `COM22`: 市販 USB-RS485 ドングル (CH340 @ 115,200 bps)

---

## 📥 手順 1: マイコンへのファームウェア書き込み

```powershell
# COM20 が UPDI ポートの場合
pymcuprog write -t uart -u COM20 -d attiny1616 -f m3_flash.hex --erase
```

---

## 🚀 手順 2: Python ベンチマークの実行

```powershell
# COM22 が RS-485 ポートの場合 (安全テスト領域: Page 64 へ書き込み)
python m3_flash_bench.py --port COM22 --page 64
```

### 実行される 5 大テスト項目:
1. **[TEST 1] Single Ping**: マイコンとの疎通確認（ATtiny1616, Flash 16KB, Page 64B）。
2. **[TEST 2] Bootloader Protection Guard**:
   - 保護領域 `Page 0` への書き込みを試行し、マイコンが安全に拒絶（`STATUS_ERR_PARAM: 0x03`）することを確認。
3. **[TEST 3] Physical Flash Page Write**:
   - `Page 64`（物理アドレス `0x1000`〜`0x103F`）へ 16B $\times$ 4 チャンクを転送。
   - ATtiny1616 の NVMCTRL が物理 Flash メモリの消去＆書き込み（約 2.5ms）を実行し、`STATUS_PAGE_DONE` (0x10) を返信することを確認。
4. **[TEST 4] Bit-for-Bit Readback Verification**:
   - `CMD_BOOT_READ_CHUNK` (0x12) で書き込まれた物理 Flash から直接 64 バイトを読み出し、**送信データと 1 ビットの狂いもなく 100.0% 完全一致（Bit-for-Bit Match）** することを証明。
5. **[TEST 5] Hardware Flash CRC Check Command**:
   - `CMD_BOOT_CRC_CHECK` (0x13) により、物理 Flash メモリ全体の CRC16 が期待値と完全一致することを証明。

---

## 📝 手順 3: 結果の記録

ベンチマークスクリプトの実行完了時に画面に表示されたレポートをコピーし、  
以下のレポートファイルに貼り付けて記録します：

📄 **[`firmware/sandbox/records/M3_single_page_report.md`](../../records/M3_single_page_report.md)**
