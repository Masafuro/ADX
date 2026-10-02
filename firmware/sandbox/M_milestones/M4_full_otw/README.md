<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Milestone 4 (M4): MR32 12KB フル OTW ファームウェア更新 ＆ アプリケーション自動起動実証

本マイルストーンは、MR32 仕様における **12KB（全 192 ページ）の連続 OTW（Over-The-Wire）書き換えおよび新ファームウェアの自動起動（ジャンプ）** を実機（ADX Core-D）で実証する本番ゲートです。

---

## 🛡️ メモリ区画設計 ＆ 安全性

先行実験フェーズの基礎設計（[`EXPERIMENTAL_CONDITIONS_BASE_DESIGN.md`](../EXPERIMENTAL_CONDITIONS_BASE_DESIGN.md)）に基づき、以下のメモリ配置を採用しています：

```text
 0x0000 ┌──────────────────────────────────────┐
        │  Pages 0..63 (4,096 Bytes / 4KB)      │
        │  【LOCKED: 実験ブートローダー保護領域】│ ← 書き込み試行時は即座に REJECT (0x03)
 0x1000 ├──────────────────────────────────────┤
        │  Pages 64..255 (12,288 Bytes / 12KB)  │
        │  【WRITABLE: アプリケーション領域】   │ ← 全 192 ページを OTW で連続書き換え
 0x3FFF └──────────────────────────────────────┘
```

- **ブートローダー（`m4_bootloader.hex`）**: サイズ 1,702 Bytes（Page 0〜26）。Page 64 まで約 2.4KB の完全な安全マージンを確保。
- **ユーザーアプリケーション（`user_app.elf`）**: アドレス `0x1000`（Page 64）から開始。未書き込み領域を `0xFF` でパディングした 12,288 バイトの完全イメージ（`app_12k.bin`）を転送。
- **自動起動（ジャンプ機構）**: 全書き込み完了後、ホストから `CMD_BOOT_APP_EXEC (0x14)` を受領すると、ブートローダーが `ijmp` 命令でアドレス `0x1000` へソフトウェアジャンプしてユーザーアプリを自動起動します。

---

## 🛠️ ハードウェア結線 (ADX Core-D)

- **H2 (DE/RE)**: 2-3 ショート（連動制御）
- **H3 (クロック)**: 1-2 ショート（内部 20MHz）
- **H4 (終端抵抗)**: OPEN
- **接続 COM ポート**:
  - `COM20`: CH342K Port A (SerialUPDI ブートローダー書込ポート)
  - `COM21`: CH342K Port B (Soft-UART デバッグモニタ @ 9,600 bps 8N1)
  - `COM22`: 市販 USB-RS485 ドングル (CH340 @ 115,200 bps 8N1)

---

## 📥 手順 1: ブートローダーの書き込み (SerialUPDI)

PowerShell にて実行します：

```powershell
# COM20 が UPDI ポートの場合
pymcuprog write -t uart -u COM20 -d attiny1616 -f m4_bootloader.hex --erase
```

---

## 🚀 手順 2: 12KB フル OTW ベンチマーク ＆ 自動起動の実行 (RS-485)

```powershell
# COM22 が RS-485 ポートの場合
python m4_otw_bench.py --port COM22 --image app_12k.bin
```

### 実行される 5 大テスト項目:
1. **[TEST 1] Target Ping**: マイコンとの通信確認（ATtiny1616, 16KB Flash, 64B Page）。
2. **[TEST 2] Bootloader Protection Guard**:
   - 保護領域 `Page 0` への書き込みを試行し、マイコンが安全に拒絶（`STATUS_ERR_PARAM: 0x03`）することを確認。
3. **[TEST 3] 12KB Full OTW Flash Transfer**:
   - `Page 64` 〜 `Page 255`（計 192 ページ / 768 チャンク）を連続ストリーミング転送。
   - ページごとに NVMCTRL 消去・書き込み（PAGEERASEWRITE）を実行し、Flash ハードウェア CRC16 を即時照合。
   - 目標所要時間: **約 5.0 秒 〜 6.5 秒以内**。
4. **[TEST 4] Spot Bit-for-Bit Readback Verification**:
   - `Page 64`（アプリ先頭）、`Page 65`、`Page 255`（末尾）の物理 Flash を読み出し、100% Bit-for-Bit 一致を確認。
5. **[TEST 5] Launch User Application (`CMD_BOOT_APP_EXEC: 0x14`)**:
   - アプリ起動コマンドを発行し、ブートローダーが `0x1000` へジャンプ！

---

## 💡 手順 3: ユーザーアプリケーション起動の確認

起動コマンド送信後、Core-D 上で以下が確認できます：
1. **LED 視覚確認**:
   - **赤 LED (PB2)** と **白 LED (PB3)** が **150ms 周期で高速交互点滅** を開始します。
2. **Soft-UART モニタ確認 (COM21 @ 9,600 bps)**:
   - Tera Term 等で COM21 を開いている場合、以下の起動ログが出力されます：
     ```text
     ===============================================================
        🎉 ADX Core-D USER APPLICATION LAUNCHED SUCCESSFULLY! 🎉
     ===============================================================
      [ENTRY] Address: 0x1000 (Flash Page 64 / Application Space)
      [INDICATORS] PB2 (Red) & PB3 (White) alternating blink active
      [STATUS] Milestone 4 OTW Auto-Execution Verified!
     ===============================================================
     [APP HEARTBEAT] Alive count: 1s | Running at 20MHz
     ```

---

## 📝 手順 4: 結果の記録

実行完了後、画面に表示されたサマリーを以下のレポートに記録します：
📄 **[`firmware/sandbox/records/M4_full_otw_report.md`](../../records/M4_full_otw_report.md)**
