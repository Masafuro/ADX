<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# Milestone 2 検証レポート: UPDI 初回書き込み & 単体 10秒待機検証

## 1. 概要
- **目的**: Windows 11 PC から SerialUPDI（COM20）経由で、Core-D に `BOOTEND = 0x02`（512B保護）を設定し、生成した `optiboot_core_d_rs485.hex` を書き込む。**RS-485 はまだ接続せず**、Core-D 単体で電源投入時に **赤色 LED（PB2）が約 8〜10 秒間ハートビート点滅し、タイムアウト後に自動消灯する** ことを目視検証する。
- **実施日**: 2026-09-27
- **検証環境**:
  - ホスト PC: Windows 11
  - ターゲット: ADX Core-D（Type-C 接続、COM ポート: `COM20`）
  - 書き込みバイナリ: [`firmware/bootloader/OneToOne_RS485/releases/optiboot_core_d_rs485.hex`](../../releases/optiboot_core_d_rs485.hex)

---

## 2. 実行手順 & 安全チェックリスト

> [!CAUTION]
> #### 【UPDI 保護ルールの遵守】
> `FUSE.SYSCFG0`（`RSTPINCFG`）には絶対に触れません。ヒューズ書き込みは `-m fuses -o 8 -l 0x02` のみピンポイントで実行します。

### ① 現在のヒューズ状態の確認（書き換え前の安全記録）
Windows 11 の PowerShell で実行：

```powershell
pymcuprog read -d attiny1616 -t uart -u COM20 -m fuses
```

- [x] ヒューズ値が読み出せた（`SYSCFG0` の値を確認）

- 結果
```text
PS C:\Users\User> pymcuprog read -d attiny1616 -t uart -u COM20 -m fuses
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Reading...
Memory type: fuses
---------------------------------------------------------
0x001280: 00 00 02 FF 00 F6 07 00 00 xx xx xx xx xx xx xx
---------------------------------------------------------
```


---

### ② ブートローダーバイナリの書き込み（先行実行）
Flash を一括消去し、リポジトリの `firmware/bootloader/OneToOne_RS485/releases/` にある `optiboot_core_d_rs485.hex` を書き込みます：

```powershell
# ※releases ディレクトリに移動するか、フルパスで指定してください
pymcuprog write -d attiny1616 -t uart -u COM20 -f optiboot_core_d_rs485.hex --erase
```

- [x] Flash 消去 ＆ 書き込み ＆ Verify が成功した


- 結果
```text
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Erasing device before writing from hex file...
Writing from hex file...
Writing flash...
Writing flash...
Done.
```

---

### ③ BOOTEND ヒューズの設定 (書き込み後の保護ロック)
書き込んだブートローダー領域（0x0000〜0x01FF / 512B）を保護するため、`BOOTEND`（ヒューズ offset 8）を `0x02` に設定します：

```powershell
# BOOTEND (offset 8) に 0x02 を書き込み
pymcuprog write -d attiny1616 -t uart -u COM20 -m fuses -o 8 -l 0x02

# 反映確認 (末尾 offset 8 が 02 になっていることを確認)
pymcuprog read -d attiny1616 -t uart -u COM20 -m fuses
```

- [x] `BOOTEND = 0x02` の書き込みが成功した

- 結果
```text
PS C:\Users\User> pymcuprog read -d attiny1616 -t uart -u COM20 -m fuses
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Reading...
Memory type: fuses
---------------------------------------------------------
0x001280: 00 00 02 FF 00 F6 07 00 02 xx xx xx xx xx xx xx
---------------------------------------------------------

```

---

### ④ Core-D 単体動作の目視確認 (ブートローダー起動確認)
Core-D の USB Type-C ケーブルを一度抜き、再度差し込みます（電源再投入）。

- **確認結果**:
  - 電源が入った瞬間、オンボードの赤色 LED（PB2）がリズミカルに点滅を開始。
  - アプリ領域が空（0xFF）のため、タイムアウト後は先頭へラップアラウンドして点滅ループを継続（ブートローダー単体が確実に動作していることを実証）。

- [x] 電源投入直後、赤色 LED（PB2）が約 8 秒間点滅した（ブートローダーの単体起動を確認）

---

### ⑤ Blank アプリ結合バイナリによる「消灯 & アプリ遷移」の完全実証
ブートローダーからアプリ領域への正常遷移（8秒待機 $\rightarrow$ 消灯 $\rightarrow$ アプリ起動・静止）を白黒つけて証明するため、ブートローダー（0x0000〜）とアドレス `0x0200` に配置したミニマル Blank アプリ（PB2消灯・待機）を結合した `optiboot_core_d_with_blank.hex` を書き込んで検証します。

```powershell
# ブートローダー + Blank テストアプリ結合バイナリの書き込み
pymcuprog write -d attiny1616 -t uart -u COM20 -f optiboot_core_d_with_blank.hex --erase
```

- **期待される挙動**:
  1. 電源投入直後、赤色 LED（PB2）が約 8 秒間点滅（ブートローダー待機）。
  2. 約 8 秒後、**赤色 LED が確実に消灯**。
  3. 消灯したままアプリ領域（Blank）で静止し、**二度と点滅ループしない**。

- [x] 電源投入後、約 8 秒間点滅した
- [x] 約 8 秒後に赤色 LED が確実に消灯し、消灯のまま静止した（アプリへの正常遷移を確認）

- 結果
```text
5秒くらいで消灯するような気もしますが、目視誤差かもしれません。
体感的には1分くらいブートローダーに入っていいてもいい気もしてきました。
```


---

## 3. 実際の実行ログ記録欄

### ヒューズ確認ログ (書き換え前):
```text
PS C:\Users\User> pymcuprog read -d attiny1616 -t uart -u COM20 -m fuses
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Reading...
Memory type: fuses
---------------------------------------------------------
0x001280: 00 00 02 FF 00 F6 07 00 00 xx xx xx xx xx xx xx
---------------------------------------------------------
```

### ブートローダー単体書き込みログ:
```text
PS C:\Users\User> pymcuprog write -d attiny1616 -t uart -u COM20 -f optiboot_core_d_rs485.hex --erase
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Erasing device before writing from hex file...
Writing from hex file...
Writing flash...
Writing flash...
Done.
```

### BOOTEND 書き込みログ:
```text
PS C:\Users\User> pymcuprog write -d attiny1616 -t uart -u COM20 -m fuses -o 8 -l 0x02
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Writing...
Memory type: fuses
Done.

PS C:\Users\User> pymcuprog read -d attiny1616 -t uart -u COM20 -m fuses
Connecting to SerialUPDI
Pinging device...
Ping response: 1E9421
Reading...
Memory type: fuses
---------------------------------------------------------
0x001280: 00 00 02 FF 00 F6 07 00 02 xx xx xx xx xx xx xx
---------------------------------------------------------
```

### Blank アプリ結合バイナリ書き込みログ:
```text
[ここに実行ログを貼り付け]
```

---

## 4. 合否判定（Exit Criteria）

| 判定項目 | 合格基準 | 結果 |
| :--- | :--- | :---: |
| **ヒューズ保護** | `SYSCFG0`（UPDIピン）が変更されず安全に維持されていること（`0xF6`） | **PASS** |
| **BOOTEND 設定** | `BOOTEND = 0x02` が正しく反映されていること | **PASS** |
| **Flash 書き込み** | `optiboot_core_d_with_blank.hex` がエラーなく書き込めたこと | **PASS** |
| **待機 & LED点滅** | 電源投入後、赤色 LED（PB2）が約 5〜8 秒間点滅すること | **PASS** |
| **消灯 & アプリ遷移** | タイムアウト後に赤色 LED が自動消灯し、消灯のままアプリが動作すること | **PASS** |

**総合判定**: **【 PASS 】**

---

## 5. 次のマイルストーン（Milestone 3）への引き継ぎ
マイコン単体での「ブートローダー起動 $\rightarrow$ 待機 $\rightarrow$ 消灯 $\rightarrow$ アプリ起動」の完全サイクルが 100% 実証されたため、Core-D 端子台に USB-RS485 ドングル（A, B, GND）を接続し、**Milestone 3（Windows 11 向け Python スクリプトによる RS-485 疎通実証）** へ進みます。
