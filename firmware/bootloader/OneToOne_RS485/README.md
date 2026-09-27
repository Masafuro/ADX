<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# ADX Core-D 1-to-1 RS-485 ブートローダー プロジェクト

ADX Core-D（Microchip **ATtiny1616-MNR** 搭載）向け、2線式差動 **RS-485 半二重通信（115,200 bps）経由でのファームウェア書き込み（Over-The-Wire: OTW）** を実現する堅牢なブートローダーおよびホストツールのリポジトリです。

---

## 1. プロジェクト概要

- **ターゲット MCU**: ATtiny1616-MNR (Flash 16KB, SRAM 2KB, EEPROM 256B)
- **RS-485 トランシーバー**: Exar / MaxLinear **SP485EEN**
  - TXD: `PA1` (USART0 Alternate)
  - RXD: `PA2` (USART0 Alternate)
  - XDIR (DE): `PA4` (ハードウェア自動送信方向制御)
  - /RE: `PA7` (常時 LOW = 受信有効)
- **インジケータ LED**:
  - **赤色 LED (`PB2`)**: ブートローダー待機中（電源投入後 約 5〜8 秒間点滅）
  - **白色 LED (`PB3`)**: ユーザーアプリケーション稼働インジケータ
- **ブートローダー方式**: `optiboot_x` (STK500v1 プロトコル互換, サイズ 486B <= 512B)
- **ヒューズ保護**: `BOOTEND = 0x02` (アドレス `0x0000`〜`0x01FF` のブートローダー領域をハードウェア保護)
- **アプリケーション配置**: アドレス `0x0200` (512 バイト境界) 以降

---

## 2. ディレクトリ構成

```
OneToOne_RS485/
├── README.md                          # 本ドキュメント (全体案内 & クイックスタート)
├── milestones.md                      # 段階的開発マイルストーン計画 & 進捗状況 (Gate 方式)
├── workflow_and_tools.md              # 開発・検証ワークフロー & CLI ツールチェーン設計書
├── rs485_communication_sequence.md   # RS-485 通信シーケンス & プロトコル詳細設計書 (Stage 0-6)
├── development_plan.md                # 初期開発計画書
│
├── releases/                          # 配布用バイナリ
│   ├── optiboot_core_d_rs485.hex      # ブートローダー単体 (486B, 0x0000〜)
│   ├── optiboot_core_d_with_blank.hex # 初回書込用 (ブートローダー + Blank アプリ結合)
│   └── test_blink_white_led.hex       # OTW 実証用テストアプリ (0x0200〜, 白LED点滅)
│
├── scripts/                           # ホスト PC (Windows 11 / Linux) 向けツール
│   ├── adx_rs485_flash.py             # 【推奨】7ステージ堅牢 RS-485 フラッシュ書き込みツール
│   ├── adx_rs485_ping.py              # RS-485 疎通確認・バージョン取得ツール (M3)
│   └── adx_debug_probe.py             # 開発デバッグ用パケットプローブ
│
├── src/                               # ブートローダー & スケッチ ソースコード
│   ├── optiboot_x.c                   # ブートローダー本体 (ノイズ耐性強化・RS485対応)
│   ├── pin_defs_x.h                   # Core-D ピンアサイン & レジスタ定義
│   ├── stk500.h                       # STK500v1 コマンド定数定義
│   ├── blank_app.c                    # テスト用 Blank アプリケーション
│   ├── test_blink_white_led.c         # 白色 LED 点滅テストスケッチ
│   ├── merge_hex.py                   # Intel HEX 結合スクリプト
│   └── Makefile                       # Ubuntu 側ビルド & サイズ検証 Makefile
│
└── records/                           # 各マイルストーンの実機検証レポート (エビデンス)
    ├── M0_env_setup/result.md         # M0: 環境構築 & SerialUPDI 導通確認 (PASS)
    ├── M1_build_and_size/result.md    # M1: avr-gcc ビルド & 512B 静的サイズ検証 (PASS)
    ├── M2_updi_standalone/result.md   # M2: UPDI 初回書込 & 単体待機・消灯検証 (PASS)
    ├── M3_rs485_ping/result.md        # M3: RS-485 疎通実証 & A/B 逆接特性解明 (PASS)
    └── M4_otw_upload/result.md        # M4: RS-485 経由アプリ書き込み・二重起動実証 (PASS)
```

---

## 3. クイックスタートガイド (ファームウェア書き込み)

### 3.1 前提環境 (Windows 11 ホスト PC)
- Python 3.8 以上
- `pyserial` ライブラリ:
  ```powershell
  pip install pyserial
  ```
- 市販 USB-RS485 アダプタ（COM ポート確認、例: `COM19`）
- Core-D 端子台（A, B, GND）と USB-RS485 アダプタが正しく接続されていること

### 3.2 アプリケーションの書き込み手順

1. **PowerShell で書き込みツールを実行**:
   ```powershell
   python .\scripts\adx_rs485_flash.py COM19 .\releases\test_blink_white_led.hex
   ```
   コンソールに `[STAGE 1/6: POLLING]` と表示され、待機状態になります。

2. **Core-D の電源を ON にする（またはリセット）**:
   - オンボード赤色 LED（PB2）がチカチカ点滅を開始します。
   - スクリプトが自動で同期を検知し、書き込み・ベリファイ・アプリ起動まで一気に完了します。

3. **動作確認**:
   - 書き込み完了直後、オンボード **白色 LED（PB3）が 500ms 周期で点滅** を開始します。
   - 電源再投入時も、約 5 秒間の赤点滅待機（通信がなければ消灯）を経て、自動的に白点滅（アプリ）へ移行します。

---

## 4. トラブルシューティング & ハードウェア診断ルール

| 現象 | 推定原因 | 対処方法 |
| :--- | :--- | :--- |
| **電源を入れた瞬間、赤 LED が点滅せず即座に消灯する** | **RS-485 の A と B が逆接** されています（常時 Break 検出）。 | Core-D 端子台側で **`A` と `B` の線を入れ替えてください**。 |
| **赤 LED は約 5 秒間点滅するが、Python 側がタイムアウトする** | 1. COM ポート番号の間違い。<br/>2. GND 配線の未接続。<br/>3. A/B 線の断線。 | 1. デバイスマネージャーで COM ポートを確認。<br/>2. GND 結線を確認。<br/>3. `adx_rs485_ping.py` で疎通確認を実行。 |
| **`Access is denied` (COM ポートオープン失敗)** | 他のプログラム（シリアルモニタ、avrdude 等）が COM ポートを占有中。 | 該当ツールを終了し、再度スクリプトを実行してください。 |

---

## 5. マイルストーン進捗状況

| マイルストーン | 検証内容 | 合格基準 | 状態 |
| :---: | :--- | :--- | :---: |
| **M0** | 接続・環境準備 | SerialUPDI 導通確認 | **PASS** |
| **M1** | Ubuntu ビルド & サイズ検証 | バイナリ `<= 512B` (実績: 486B) | **PASS** |
| **M2** | UPDI 初回書き込み & 単体検証 | 赤点滅(5〜8s) $\rightarrow$ 自動消灯 | **PASS** |
| **M3** | RS-485 疎通確認 (Ping) | `0x14 0x10` 受信 & RTT 計測 | **PASS** |
| **M4** | RS-485 経由アプリ書き込み (OTW) | フラッシュ書込 $\rightarrow$ 白LED点滅 | **PASS** |
| **M5** | Windows 11 配布パッケージ化 | ワンクリック / 1コマンド自動化 | **進行中** |
