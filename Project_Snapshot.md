<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Project Snapshot

## 0. プロジェクト概要 & コンセプト

- **ADX**: Advanced Devices eXtended
- **コア・アイデンティティ**: **"Enclosure-Friendly"（筐体適合性）** を軸とした次世代オープンソース・モジュラーハードウェア規格。
- **現在の提供価値とパラダイムシフト**:
  - Arduino が切り拓いた「机の上のプロトタイピング」を、実際の筐体や過酷な現実空間（屋外、防水ケース、車載、工場配電盤、冷凍設備、通信インフラ）へスムーズに連れ出すための物理的・電気的アーキテクチャの確立。
  - **「最もシンプルで、最もタフで、安価な1枚のコア（ADX CORE-A: $28）」** と、**「現場のRS-485トラブルを一撃で解決するスマートツール（RPR4 Smart Probe: $25）」** の二頭立てを主軸に、モジュラー型 CARD システムと連携して普遍的なエッジコンピューティング環境を提供。
- **直近のゴール**: Crowd Supply キャンペーンのローンチ（ADX Starter Kit: $49〜$55）
- **ライセンス体系**: Tri-license モデル（Docs: CC BY 4.0, Hardware: CERN-OHL-P-v2, Firmware: MIT）

---

## 1. ハードウェア開発進捗 (Hardware Status)

### 1.1 ADX CORE-A (`hardware/CORE-A`) 【絶対標準機 / 旗艦コア】
- **役割**: 机上開発から酷暑・極寒・工場配電盤まで、1枚ですべてを網羅するADXエコシステムの絶対標準コントローラー。
- **主要機能 & アーキテクチャ**:
  - **5V専任・自己発熱ゼロ**: 高電圧DCDCをコアボードから追放し、真夏の密閉ボックスでも動く -40℃〜+105℃ フルIndustrial耐熱を実現。
  - **完全絶縁 RS-485**: Mornsun `TDA51S485HC`（2500V耐圧、絶縁電源内蔵）＋ 前面スクリューレス・スプリング端子台（DB142R-5.08）。
  - **USB Type-C 6P ＋ 逆流阻止バルブ**: TI `LM66100`（理想ダイオード）により、現場通電中に作業用PCを挿してもPCを100%保護。
  - **20P ラッチ付き Eject Header**: 車載・工場の振動脱落を完全防止、5V/2A双方向幹線プレーン。
  - **超低消費電力 BMC**: ATtiny412 によるナノアンペアスリープ制御 ＆ スマホPWAからの完全自動OTW書き換え。
  - **メインMCU**: Microchip ATtiny1616-MNR（工業用Nグレード: -40℃〜+105℃）。
- **進捗実績・コスト**:
  - 2026/10/01: CORE-A 基本構想書（`concept.md`）および技術仕様書（`spec.md`）策定完了。
  - 2026/10/01: rev1 ネットリスト出力完了。車載グレードAEC-Q102 LED（-40℃〜+110℃）、TI LM66100、TDA51S485HC、牛角ヘッダ選定完了。
  - **JLCPCB 100pcs PCBA見積もり確定**: **$626.27（送料込・1台あたり約 $6.26 / 想定販売価格 $28.00、粗利率 74.8%）**。

### 1.2 RPR4 Smart Probe (`hardware/RPR4_Smart_Probe`) 【スマートアナライザ ＆ プログラマ】
- **役割**: 保守員・組み込み開発者のための完全絶縁マルチツール。RS-485のブラックボックス問題を粉砕し、スマホOTWを実現するポケットデバイス。
- **主要機能**:
  - **コントローラー**: Raspberry Pi RP2040（Dual Cortex-M0+ @ 133MHz）。
  - **完全絶縁インターフェース**: Mornsun `TDA51S485HC` による 2500V ガルバニック絶縁。
  - **RP2040 PIO（プログラマブルI/O）による超高速解析**:
    - 通信ボーレート自動検出（Auto-Baud: 9600〜115200bps等）。
    - プロトコル自動デコード（Modbus RTU、DMX512、LN-485）。
  - **専用 Web PWA アプリ連携**: スマホのブラウザ上でRS-485波形・トラフィック・エラーをロジアナ表示。
  - **完全自動 OTW 書き換え**: BREAK信号送出とOptibootプロトコルによる3秒ファームウェア更新。
- **進捗実績・コスト**:
  - 2026/10/01: rev1 ネットリスト確定。PCBA原価 約 $7.00、想定販売価格 $25.00。

### 1.3 CARD シリーズ (`hardware/CARD/`) 【モジュラー拡張基板】
「電源の保護・変換・昇圧・降圧は、電源を供給するCARD側が背負う」という単一責任の原則に基づき展開。

- **AAA 2S Power CARD (乾電池自律運用)**:
  - 単4電池×2本から低暗電流昇圧DCDCでクリーンな5Vを生成。BMC連動でDCDCを完全停止。
- **DC 12V〜48V Power CARD (産業・インフラ電源)**:
  - 工場配電盤（24V）、車載（12V/24V）、通信基地局（-48V）等の広入力をトランス絶縁5V変換。TVS/サージ保護集約。
- **Prototyping CARD (ユニバーサル試作基板)**:
  - 20P Eject Header 中継パススルー ＋ 2.54mmピッチ試作エリア。基板受領済み。

---

## 2. ソフトウェア ＆ クラウド進捗 (Software Ecosystem)

- **PWA ＋ クラウドAPI アーキテクチャ確立**:
  - Androidネイティブアプリの保守地獄（16KBページ問題、OSセキュリティ制約）を完全回避。
  - ブラウザ1本で iOS / Android / PC 全対応。
  - 現場での定数調整: RS-485経由でATtiny1616のEEPROMパラメータを直接書き換え（所要時間0.1秒、オフライン対応）。
  - クラウドコンパイルAPI: サーバー側 `arduino-cli` が1秒でビルドして `.hex` を返送。
- **LN-485 通信スタック**:
  - Phase 1 〜 Phase 5 までの実機検証完了（実機3台によるマルチドロップ通信、Zero-Copy Mailbox、自動タイムアウト復帰）。

---

## 3. ガイドライン ＆ 規格策定 (Standard & Guidelines)

- **ADX ハードウェア・フォームファクタ開発ガイドライン (`docs/ja/adx_formfactor_guidelines.md`)**:
  - RFC 2119 準拠の3段階要件定義（MUST / SHOULD / MAY）を策定。
  - 【MUST】Power CARDの逆流阻止義務（電池破裂防止）、Pin 5アイソレーション帯厳守。
  - 【SHOULD】ラッチ付き Eject Header、産業用トランス絶縁DCDC、8748寸法、-40℃〜+105℃耐熱。
  - 【MAY】非絶縁DCDC、ボックスヘッダ、自由な基板形状、独自センサー実装。

---

## 4. 直近のマイルストーン & 今後の課題 (Next Milestones)

1. **CORE-A rev1 のアートワーク設計 ＆ 試作発注**:
   - 8748サイズ内での前面端子台・Type-C・牛角Eject Headerの配置最適化。
2. **RPR4 Smart Probe のファームウェア開発（PIOロジアナ ＆ OTWエンジン）**:
   - RP2040 PIO による Auto-Baud 計測および Modbus/DMX パケット解析エンジンの実装。
3. **PWA 開発環境のプロトタイピング**:
   - WebSerial API を用いた RPR4 通信 ＆ クラウドコンパイルAPI連携の検証。
4. **Crowd Supply キャンペーン準備**:
   - **ADX Mobile Starter Kit**（CORE-A ＋ RPR4 ＋ Power CARD: $49〜$55）の企画資料・デモ動画の構成FIX。
