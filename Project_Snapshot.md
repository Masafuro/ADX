<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Project Snapshot

## 0. プロジェクト
- **ADX**: Advanced Devices eXtended
- It's a form factor for Next Generation Industrial Arduino ecosystem.
- **1st Goal**: Start Crowd Supply campaign.
- **License**: Tri-license model (Docs: CC BY 4.0, Hardware: CERN-OHL-P-v2, Firmware: MIT)

## 1. 現状 (Hardware & Firmware)

### 1.1 ADX Core-D (`hardware/ADX_Core-D`)
- 2026/08/17 : 発注完了
- 2026/08/23 : 基板到着・基本動作検証完了（SerialUPDI書き込み、SoftwareSerial通信、LED点灯、12MHz外部オシレーター動作確認完了）。
- 2026/08/24 : RS-485 半二重通信テスト完了（2台のADX Core-D実機間でのMaster/Slave双方向通信・GPIO手動方向制御の正常動作を確認）。※IDCケーブル未着のためCARD接続検証は待機中。
- 2026/08/25 : LN-485 Phase 1 / Phase 2 実機検証完了（GPIO Break 送出、スレーブ LINAUTO ハードウェア自動ボーレート同期・PIDパリティ検証 PASS）。
- 2026/08/26 : LN-485 Phase 3 実機検証完了（Type A: Master-Pub → Slave-Sub 実証、Subscriber 受信・Classic Checksum 照合・LED コマンド制御・不正 CS 破棄 PASS）。
- 2026/08/26 : LN-485 Phase 4 実機検証完了（Type B: Slave-Pub → Master-Sub 実証、Double Buffer Mailbox による完全非同期・Zero-Copy 送信、Master Broker 15ms タイムアウト自律復帰 PASS 【★Master Broker MVP 完成】）。
- 2026/08/27 : LN-485 Phase 5 実機検証完了（Type C: Slave A Pub → Slave B Sub 実証、3台実機構成でのスレーブ間ダイレクト制御、Master Broker プロミスキャス傍受監視、STATE_IGNORE_PAYLOAD による ISFIF 自爆防止 PASS 【★LN-485 UP/CS 完成】）。
LN-485ブートローダの開発のための専用ボード

### 1.2 Prototyping CARD (`hardware/CARD/Prototyping`)
- 2026/08/17 : 発注完了
- 2026/08/23 : 基板到着済み（IDCケーブル未着のため動作確認は待機中）
8748フォームファクタ対応の拡張プロトタイピング基板

### 1.3 Firmware & Software (`firmware/`)
- 2026/08/23 : ADX Core-D 単体動作確認テスト完了（LED・シリアル通信・オシレータ検証: [`firmware/tests/ADX_Core-D/`](firmware/tests/ADX_Core-D/)）
- 2026/08/24 : RS-485 半二重通信テスト完了（Master/Slave双方向エコーバック検証: [`firmware/tests/ADX_Core-D/RS-485_test/`](firmware/tests/ADX_Core-D/RS-485_test/)）
- 2026/08/25 : LN-485 Phase 1 (Break/UART) ＆ Phase 2 (LINAUTO 自動同期) 実機検証 PASS ([`firmware/tests/ADX_Core-D/LIN_test/Phase/`](firmware/tests/ADX_Core-D/LIN_test/Phase/))
- 2026/08/26 : LN-485 Phase 3 (Type A: Master-Pub ＆ Slave Subscriber 実装) 実機検証 PASS ([`firmware/tests/ADX_Core-D/LIN_test/Phase/TC-P3/`](firmware/tests/ADX_Core-D/LIN_test/Phase/TC-P3/))
- 2026/08/26 : LN-485 Phase 4 (Type B: Slave-Pub ＆ Double Buffer Mailbox ＆ Master Broker MVP) 実機検証 PASS ([`firmware/tests/ADX_Core-D/LIN_test/Phase/TC-P4/`](firmware/tests/ADX_Core-D/LIN_test/Phase/TC-P4/))
- 2026/08/27 : LN-485 Phase 5 (Type C: スレーブ間直接通信 ＆ Master傍受監視 ＆ LN-485 UP/CS 完成) 実機検証 PASS ([`firmware/tests/ADX_Core-D/LIN_test/Phase/TC-P5/`](firmware/tests/ADX_Core-D/LIN_test/Phase/TC-P5/))
- LN-485通信スタック（Phase 6: ハードウェアXDIR自動方向制御・多重ボーレートスイープ・ロバストネス評価へ移行）、SerialUPDI対応ブートローダ、Arduino/C++ BSPの準備

## 2. その他
- [History](https://github.dev/Masafuro/ADX/blob/main/Project_Snapshot.md)


