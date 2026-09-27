<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Optiboot_OL4 開発進捗・実験ログ・検証記録レポート (v2.0)

**最終更新**: 2026-09-27  
**対象ターゲット**: ADX Core-D (Microchip ATtiny1616-MNR, RS-485: SP485EEN)  
**通信プロトコル**: 1-to-1 RS-485 Stop-and-Wait ARQ (9,600 bps, 8N1, 64B Page CRC-16)  
**実装ファイル**: [`src/optiboot_ol4.c`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/src/optiboot_ol4.c) (814 Bytes / 1024 Bytes)  
**配布バイナリ**: [`releases/optiboot_ol4_core_d.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/releases/optiboot_ol4_core_d.hex)  
**デモアプリ**: [`releases/demo_app.hex`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/releases/demo_app.hex)  
**新アップローダー**: [`tools/adx_rs485_upload.py`](file:///home/ubuntu/AgentWorkspace/ADX/github/ADX/firmware/bootloader/OneToOne_RS485/optiboot_OL4/tools/adx_rs485_upload.py)  

---

## 1. プロジェクトの総括と技術的ブレークスルー

### 1.1 真因の完全究明（エビデンスに基づく確定）
これまで「電源投入直後だけ数回通信でき、その後二度と応答しなくなる」という最大のミステリーに直面していたが、動かぬ証拠（ネットリスト精査・COM21テレメトリ・ベンチマーク）に基づき真因を完全特定：
1. **DE ピンの浮遊電荷問題 [ハードウェア真因]**:
   - 当初、コードが PA3 を操作していたが、回路設計上の真の DE は **PA4**、/RE は **PA7** であった（PA3 は外部水晶 EXTCLK）。
   - 未初期化の PA4（Hi-Z）が、電源投入直後の寄生容量チャージによって数秒間だけ HIGH になり、放電して LOW に落ちた瞬間に二度と返信できなくなっていた。
   - **対策**: PA4 (DE) と PA7 (/RE) の明示駆動により 100% 根絶。
2. **19200 bps & LIN Break ジッター [プロトコル要因]**:
   - Windows USB シリアルからミリ秒未満の正確な Break 信号（1042 µs）を生成するのは OS スケジューリング上ジッターが大きく、同期失敗の原因となっていた。
   - **対策**: **9,600 bps（1ビット 104.2 µs）** への適正化と、通常 UART フレーム（STX/ETX/CRC）への刷新。

### 1.2 ベンチマークによる実証
- **Step 1 (1文字エコー)**: 20/20 (100.0%) PASS、RTT 48.5ms 均一。物理層・トランシーバ切替の完全健全性を証明。
- **Step 2 (8バイトパケット)**: 30/30 (100.0%) PASS、RTT 19.65ms、**ジッターわずか 0.88ms**。ランダムペイロード 100% 一致。

---

## 2. Optiboot_OL4 v2.0 の完成

ユーザー合意方針「**CRCチェックと再送（Stop-and-Wait ARQ）の徹底**」に基づき、ブートローダー本体と PC 側アップローダーを完全刷新。

### 2.1 ブートローダー仕様
* **コードサイズ**: **814 バイト**（1024 バイト制限に対し 210 バイトの安全マージン）
* **機能**:
  * 起動後 1.0 秒の PING 待機タイムアウト（何も来なければ直ちに 0x0400 へジャンプ）
  * 64 バイト Flash ページ単位の CRC-16-CCITT 検証
  * BOOTEND 保護（0x0000〜0x03FF への書き込み拒絶ガード）
  * Unified Flash Memory (`MAPPED_PROGMEM_START` 0x8000) へのダイレクトロード ＆ NVMCTRL `PAGEERASEWRITE`

### 2.2 アップローダー CLI (`adx_rs485_upload.py`)
* Intel HEX 自動パース ＆ 64B ページ分割
* 接続ハンドシェイク（バージョン確認、ATtiny1616 シグネチャ確認）
* Stop-and-Wait ARQ（最大 5 回自動リトライ、プログレスバー表示）
* マイコン内部 CRC による全ページ一括ベリファイ
* ユーザーアプリケーション起動コマンド（`CMD_BOOT_APP`）

---

## 3. 実機検証フロー

### Step 1: Optiboot_OL4 v2.0 ブートローダーの書き込み (COM20 UPDI)
```powershell
pymcuprog write -d attiny1616 -t uart -u COM20 -f releases\optiboot_ol4_core_d.hex --erase
```

### Step 2: RS-485 経由でのデモアプリケーション書き込み (COM19)
```powershell
python tools\adx_rs485_upload.py --port COM19 --debug-port COM21 releases\demo_app.hex
```

### Step 3: 起動確認
- 書き込み完了後、Core-D 上の **赤色 LED（PB2）が 1 秒周期でチカチカ点滅**。
- COM21 テレメトリに `[APP HEARTBEAT] Tick=... (LED Toggled)` が出力される。
