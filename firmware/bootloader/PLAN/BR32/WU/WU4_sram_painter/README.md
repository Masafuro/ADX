<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# WU-4: 仮想 SRAM ページペインタ (Virtual SRAM Page Painter - No Flash Risk)

本サンドボックスは、BR32 ブートローダーの本番機能である **「4 チャンク分割転送（16B $\times$ 4回 ＝ 64B）」** と **「読み戻しベリファイ（Readback Verify）」** の通信シーケンスを、**Flash への書き込みを一切行わず（SRAM 上のみで）安全・自由に動かして遊べる動作模型** です。

Flash の書き換え寿命やマイコン文鎮化の恐怖を一切感じることなく、好きな文字列やバイナリデータを Core-D の SRAM に流し込み、読み戻して 100% のデータ整合性を体感できます。

---

## 1. WU-4 の検証テーマ

1. **4 チャンク分割書き込み (`CMD_WRITE_CHUNK: 0x10`)**:
   - 64 バイトのデータを 16 バイト $\times$ 4 回（Chunk 0, 1, 2, 3）に分割してスレーブの `virtual_sram[64]` に転送。
   - スレーブは受信した 16 バイトを即時エコーバックし、書き込みの成否を応答。
2. **4 チャンク分割読み戻し (`CMD_READ_CHUNK: 0x20`)**:
   - スレーブの SRAM に蓄積された 64 バイトを、16 バイトずつ 4 回に分けて読み出し。
3. **100% ビット・パーフェクト・ベリファイ (Bit-for-Bit Verification)**:
   - 送信した 64 バイトと、読み戻した 64 バイトを完全照合（64/64 バイト一致）。
4. **「黄金の 6 大原則」の完全適用**:
   - LIN BREAK によるゼロクリア、32B 固定長、超過時の完全沈黙など、WU-3 までで確立された堅牢仕様をそのまま継承。

---

## 2. 構成ファイル一覧

```text
firmware/bootloader/PLAN/BR32/WU/WU4_sram_painter/
├── README.md              # 本ドキュメント (使い方 & 実験手順)
├── Makefile               # Ubuntu 側 avr-gcc ビルド用 Makefile
├── wu4_sram.c             # Core-D 側 仮想 SRAM ペインタファームウェア (Flash 書込なし / 1,324 Bytes)
├── wu4_sram.hex           # ビルド済みバイナリ
└── wu4_sram_bench.py      # PC 側 Python 4チャンク分割転送＆ベリファイベンチ
```

---

## 3. 実験手順 (Windows 11 ホスト PC)

### Step 1: マイコンへのファームウェア書き込み (COM20 UPDI)

PowerShell で `WU4_sram_painter` ディレクトリに移動し、以下を実行します：

```powershell
cd firmware\bootloader\PLAN\BR32\WU\WU4_sram_painter
pymcuprog write -d attiny1616 -t uart -u COM20 -f .\wu4_sram.hex --erase
```

* 書き込み完了後、Core-D の赤色 LED（PB2）が約 1 秒周期で待機点滅を開始します。
* COM21（9600 bps）に起動案内が出力されます。

---

### Step 2: 4 チャンク分割書き込み ＆ 読み戻しベリファイの一括実行 (`--paint`) ★推奨★

```powershell
python .\wu4_sram_bench.py --port COM19 --debug-port COM21 --paint
```

**実行の流れ**:
1. **[DISCOVERY]**: スレーブの生 UID を自動取得
2. **[STEP 1]**: 64 バイトのメッセージを 16B $\times$ 4 回に分けてスレーブ SRAM に書き込み
3. **[STEP 2]**: スレーブ SRAM から 16B $\times$ 4 回に分けて読み戻し
4. **[STEP 3]**: 送信データと読み戻しデータが 64/64 バイト完全一致するか検証し、アスキーアート形式でコンソールに表示！

---

### Step 3: 好きな 64 文字のメッセージを書き込んでみる (`--text`)

好きな英数字テキスト（64文字以内）を自由にペイントできます：

```powershell
python .\wu4_sram_bench.py --port COM19 --debug-port COM21 --paint --text "Hello World! ADX Core-D BR32 4-Chunk 64-Byte SRAM Painting is Fun!"
```

---

### Step 4: ランダムバイナリ 64B ストレステスト (`--stress N`)

ランダムな 64 バイトデータを連続 N 回書き込み＆読み戻しベリファイする高負荷耐久テストです：

```powershell
# 20 サイクル (20 ページ = 1,280 バイト) の高負荷ベリファイテスト
python .\wu4_sram_bench.py --port COM19 --debug-port COM21 --stress 20
```
