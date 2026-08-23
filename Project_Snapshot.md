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
- 2026/08/23 : 基板到着・基本動作検証完了（SerialUPDI書き込み、SoftwareSerial通信、LED点灯、12MHz外部オシレーター動作確認完了）。IDCケーブル未着のためCARD接続検証は待機中。
LN-485ブートローダの開発のための専用ボード

### 1.2 Prototyping CARD (`hardware/CARD/Prototyping`)
- 2026/08/17 : 発注完了
- 2026/08/23 : 基板到着済み（IDCケーブル未着のため動作確認は待機中）
8748フォームファクタ対応の拡張プロトタイピング基板

### 1.3 Firmware & Software (`firmware/`)
- 2026/08/23 : ADX Core-D 単体動作確認テスト完了（LED・シリアル通信・オシレータ検証: [`firmware/tests/ADX_Core-D/`](firmware/tests/ADX_Core-D/)）
- LN-485通信スタック、SerialUPDI対応ブートローダ、Arduino/C++ BSPの準備

## 2. その他
- [History](https://github.dev/Masafuro/ADX/blob/main/Project_Snapshot.md)


