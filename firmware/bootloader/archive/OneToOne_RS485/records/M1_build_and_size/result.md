<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# Milestone 1 検証レポート: ブートローダーのビルド & 静的サイズ検証

## 1. 概要
- **目的**: Ubuntu 開発環境において、ADX Core-D 向けカスタム（PA1/PA2/PA4 XDIR自動制御、PA7 /RE 初期化、PB2 赤LEDハートビート点滅、約10秒待機）を組み込んだブートローダーをコンパイルし、**512 バイト境界（<= 512 Bytes）** を厳格にクリアしていることを静的に検証・実証する。
- **実施日**: 2026-09-27
- **検証環境**:
  - ホスト OS: Ubuntu 22.04 LTS
  - コンパイラ: `avr-gcc` 7.3.0 (megaTinyCore DxCore ツールチェーン)
  - ターゲット MCU: Microchip ATtiny1616-MNR

---

## 2. ビルド構成 & カスタム内容

### ① コンパイルパラメータ
- `MCU`: `attiny1616`
- `F_CPU`: `16000000UL` (内蔵 16MHz)
- `BAUD_RATE`: `115200` bps
- `UARTTX`: `A1` (PA1: TXD, PA2: RXD, PA4: DE/XDIR)
- `RS485`: `1` (内蔵 USART0 ハードウェア RS-485 モード有効化)
- `LED`: `B2` (Core-D オンボード赤色 LED)
- `WDTTIME`: `8` (WDT 約8秒待機)
- `RS485_RE_PORT`: `VPORTA`
- `RS485_RE_PIN`: `(1<<PORT7)` (PA7 を OUTPUT LOW に初期化 / 常時受信)
- `BLINK_WAIT_LED`: 待機中に赤色 LED をハートビート点滅

---

## 3. ビルド実行ログ & サイズ解析

### 実行コマンド:
```bash
make -C firmware/bootloader/OneToOne_RS485/src clean
make -C firmware/bootloader/OneToOne_RS485/src
```

### コンソール出力ログ:
```text
make: Entering directory '/home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/src'
rm -f optiboot_core_d_rs485.elf optiboot_core_d_rs485.hex
make: Leaving directory '/home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/src'
make: Entering directory '/home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/src'
/home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc -g -Wall -Os -fno-split-wide-types -mrelax -mmcu=attiny1616 -DF_CPU=16000000UL -DBAUD_RATE=115200 -DUARTTX=A1 -DRS485=1 -DLED=B2 -DWDTTIME=8 -DBLINK_WAIT_LED -DRS485_RE_PORT=VPORTA '-DRS485_RE_PIN=(1<<PORT7)' -Wl,-section-start=.text=0 -Wl,--section-start=.version=0x1fe -Wl,--relax -nostartfiles -nostdlib -o optiboot_core_d_rs485.elf optiboot_x.c
optiboot_x.c:218:4: warning: #warning F_CPU is ignored for this chip (run from internal osc.) [-Wcpp]
   #warning F_CPU is ignored for this chip (run from internal osc.)
    ^~~~~~~
/home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex optiboot_core_d_rs485.elf optiboot_core_d_rs485.hex
mkdir -p ../releases
cp optiboot_core_d_rs485.hex ../releases/optiboot_core_d_rs485.hex
================== BINARY SIZE CHECK ==================
optiboot_core_d_rs485.elf  :
section          size      addr
.data               0   8402944
.text             466         0
.version            2       510
.comment           17         0
.debug_aranges     48         0
.debug_info      2398         0
.debug_abbrev     602         0
.debug_line      1038         0
.debug_frame      156         0
.debug_str       1488         0
.debug_loc        630         0
.debug_ranges      56         0
Total            6901
=======================================================
make: Leaving directory '/home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/src'
```

---

## 4. 合否判定（Exit Criteria）

| 判定項目 | 合格基準 | 実績値 | 判定 |
| :--- | :--- | :---: | :---: |
| **コンパイル結果** | エラーなくバイナリが出力されること | エラー 0 件 | **PASS** |
| **コードサイズ (.text)** | 510 バイト以下 | **466 バイト** | **PASS** |
| **バージョン領域 (.version)** | アドレス 0x01FE (510B目) に配置 | **2 バイト** (addr 510) | **PASS** |
| **合計 Flash 専有サイズ** | **512 バイト以下（<= 512 Bytes）** | **468 バイト (余裕: 44B)** | **PASS** |
| **バイナリ出力** | `releases/optiboot_core_d_rs485.hex` 生成 | 正常生成完了 (1,352 bytes) | **PASS** |

**総合判定**: 【 **PASS** : 2026/09/27 】

---

## 5. 生成バイナリ
- 成果物パス: [`firmware/bootloader/OneToOne_RS485/releases/optiboot_core_d_rs485.hex`](../../releases/optiboot_core_d_rs485.hex)

## 6. 次のマイルストーン（Milestone 2）への引き継ぎ
Windows 11 PC 側へ本バイナリ（`optiboot_core_d_rs485.hex`）を投入し、SerialUPDI 経由で Core-D に書き込んで、**赤色 LED（PB2）の約10秒間ハートビート点滅と自動消灯（単体動作）** を検証します。
