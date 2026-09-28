<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# ADX Bootloader & Over-The-Wire (OTW) Flashing

本ディレクトリは、ADX 各モジュール（ADX Core-D / CORE-S 等、Microchip ATtiny1616-MNR 搭載）向けのブートローダーおよび RS-485 経由ファームウェア書き込み（OTW: Over-The-Wire）関連のリソースを管理します。

---

## 1. ナレッジベース & 確定設計値（必読）

2026年9月に実施された 1-to-1 RS-485 ブートローダー開発（`optiboot_x`, `optiboot_O4`, `optiboot_OL4`）において得られた、**確定ピンアサイン・レジスタ初期化コード・半二重通信制御ルール（自爆エコー対策）・失敗要因分析・今後の推奨設計原則** はすべて以下のナレッジベースに集約されています。

👉 **[RS-485 ブートローダー開発 ナレッジベース & 設定値集約書](./RS485_BOOTLOADER_KNOWLEDGE_BASE.md)**

### 重要ハードウェア諸元クイックリファレンス
- **MCU**: Microchip ATtiny1616-MNR
- **RS-485 トランシーバー**: Exar / MaxLinear SP485EEN
- **ピンアサイン**:
  - `PA1`: USART0 TXD (`PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc` 必須)
  - `PA2`: USART0 RXD
  - `PA4`: RS-485 DE (Driver Enable, Active HIGH) ※`PA3` は EXTCLK であり DE ではない
  - `PA7`: RS-485 /RE (Receiver Enable, Active LOW, 送信時 HIGH で自爆遮断)
  - `PB2`: オンボード赤色 LED
  - `PB3`: オンボード白色 LED
  - `PB4`: デバッグ TX (CH342K Port B SoftwareSerial 9600 bps)
  - `PB5`: デバッグ RX (CH342K Port B SoftwareSerial 9600 bps)

---

## 2. バックナンバー・試作アーカイブ (`archive/`)

過去に開発・検証された試作ブートローダーおよびホストツールのファイル群は、検証エビデンスおよびコード資産としてバックナンバー隔離されています。

👉 **[archive/OneToOne_RS485/](./archive/OneToOne_RS485/)**

| 試作方式 | 概要・プロトコル | ステータス | 課題・教訓 |
| :--- | :--- | :---: | :--- |
| **optiboot_x (M0〜M5)** | STK500v1, 512B (`BOOTEND=0x02`), 115200bps | アーカイブ | 単体ページ書込は動作したが、自爆エコー未遮断による 0x0440 読出フリーズが発生。 |
| **optiboot_O4** | STK500v1 + 自爆遮断, 512B, 115200bps | アーカイブ | 読出フリーズは克服したが、Flash消去書込(0x0200)時にマイコンがクラッシュ。 |
| **optiboot_OL4** | LINAUTO / Stop-and-Wait ARQ, 1024B (`BOOTEND=0x04`) | アーカイブ | 受信タイムアウト欠如によるデッドロック、Windows Break ジッターによる ISFIF 自爆。 |

---

## 3. 次世代 BR32 プロトコル ＆ ブートローダー開発計画 (`PLAN/BR32/`)

昨日の試作（`OneToOne_RS485`）の反省と知見を踏まえ、**「バスクロック思想」「時間枠厳格読取（待たない）」「応答最優先（即時ACK）」「2機能 SIGROW アーキテクチャ」** を統合した次世代プロトコル **BR32** の仕様策定を進めています。

- 👉 **[BR32 プロトコル哲学・基本構想 (`PLAN/BR32/concept.md`)](./PLAN/BR32/concept.md)**
- 👉 **[BR32 プロトコル開発仕様書 (`PLAN/BR32/specification.md`)](./PLAN/BR32/specification.md)**
- 👉 **[BR32 段階的開発マイルストーン計画書 (`PLAN/BR32/milestones.md`)](./PLAN/BR32/milestones.md)**

---

## 4. 関連調査ドキュメント

- [optiboot_x の研究と ADX への適用](./research_of_optiboot_x.md)
