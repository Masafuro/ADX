<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Project Snapshot

## 0. プロジェクト概要 & コンセプト

- **ADX**: Advanced Devices eXtended
- **コア・アイデンティティ**: **"Enclosure-Friendly"（筐体適合性）** を軸とした次世代オープンソース・モジュラーハードウェア規格。
- **現在の探求テーマと提供価値**:
  - Arduino が切り拓いた「机の上のプロトタイピング」を、実際の筐体や過酷な現実空間（屋外、防水ケース、建築・舞台・設備現場）へスムーズに連れ出すための物理的・電気的アーキテクチャの確立。
  - **「音響・PA機器のように現場で直感的に数珠つなぎ（デイジーチェーン）配線できるフィールドネットワーク（CORE-S）」** と、**「DCDCを捨ててeFuse/OVPに特化し、12V誤挿入にも耐える5V大電力・バッテリー一体型デバイス（CORE-U）」** の2大ラインナップを主軸に、Enclosure-Friendly な 8748 フォームファクタをベースにした具体的な製品価値を確立・実証中。
- **直近のゴール**: Crowd Supply キャンペーンのローンチ
- **ライセンス体系**: Tri-license モデル（Docs: CC BY 4.0, Hardware: CERN-OHL-P-v2, Firmware: MIT）

---

## 1. ハードウェア開発進捗 (Hardware Status)

### 1.1 ADX CORE-S (`hardware/CORE-S`) 【現場・差動通信標準機】
- **役割**: 12V/24V電源、長距離ノイズ環境、多点デイジーチェーン配線に対応するADXの現場標準コントローラー。
- **主要機能**:
  - DC 12V/24V 広入力 DCDC（TI TPS5430）
  - デイジーチェーン対応 LN-485（RS-485半二重 / MaxLinear SP485EEN）
  - サブMCU（BMC: ATtiny412）による自律電源ゲーティング（`VDD_SW`）およびブート制御
  - メインMCU: Microchip ATtiny1616-MNR
- **進捗状況**:
  - rev1 〜 rev3 の設計レビューおよびシャットダウン・パワーマネジメント改善完了。
  - rev4 回路図・ネットリスト精査および試作発注準備進行中。

### 1.2 ADX CORE-U (`hardware/CORE-U`) 【汎用・5V大電力・バッテリー連携機】
- **役割**: 5V/2A（10Wクラス）の大電力を安全に扱い、筐体内蔵バッテリーCARDとの連携に特化した普及・汎用コントローラー。
- **主要機能**:
  - DCDCをあえてオミットし、Raspberry Pi等の高品位5V ACアダプターを前提化
  - 削ったコスト・面積を **大電流 eFuse ＋ 12V耐圧過電圧保護（OVP）** に再投資
  - 単一DCジャックによる物理排他（バッテリーCARD接続時のACアダプター誤挿入・多重衝突を構造的にゼロ化）
  - USB Type-C（WCH CH342K）による SerialUPDI 書込 ＆ デュアルUARTデバッグ
  - 8748 Form Factor、メインMCU: ATtiny1616-MNR
- **進捗状況**:
  - 2026/09/24: rev0 構想策定（`concept.md`）およびネットリスト一次評価完了。
  - 2026/09/24: 開発指針書（`review/rev0/development_guidelines.md`）策定完了。OVP回路選定と次期rev1ネットリスト作成へ移行。

### 1.3 ADX Core-D (`hardware/Core-D`) 【R&D 実証・実験専用ボード】
- **役割**: LN-485（LIN-based RS-485）通信スタックおよびブートローダー先行開発のための実証ボード（※量産製品ラインナップ外）。
- **進捗実績**:
  - 2026/08/17 : 初版PCBA発注完了
  - 2026/08/23 : 基板受領、基本動作検証完了（SerialUPDI、SoftwareSerial、LED、12MHz外部オシレーター動作確認 PASS）。
  - 2026/08/24 : RS-485 半二重通信テスト完了（実機2台での双方向通信 PASS）。
  - 2026/08/25 : LN-485 Phase 1 / Phase 2 検証完了（Break送出、LINAUTO自動ボーレート同期 PASS）。
  - 2026/08/26 : LN-485 Phase 3 (Master-Pub → Slave-Sub) ＆ Phase 4 (Slave-Pub → Master-Sub、Double Buffer Mailbox、Master Broker MVP) 検証 PASS。
  - 2026/08/27 : LN-485 Phase 5 (実機3台構成スレーブ間直接通信 ＆ Master傍受監視、LN-485 UP/CS 完成) 検証 PASS。
  - 開発実験用としての役割を完了し、得られた知見を CORE-S / CORE-U の設計へ完全反映済み。

### 1.4 CARD シリーズ (`hardware/CARD/`)
- **ADX Prototyping CARD**:
  - 8748フォームファクタ準拠の拡張ユニバーサル試作基板。基板受領済み。
- **ADX Battery / Power CARD**:
  - 乾電池やLiPoバッテリーから昇降圧してCORE-UのDCジャックへ5Vを供給するバッテリー拡張CARDの構想・設計検討中。

---

## 2. ファームウェア & 通信スタック進捗 (Firmware & Software)

- **LN-485 通信スタック**:
  - Phase 1 〜 Phase 5 までの実機検証完了（実機3台によるマルチドロップ通信、Zero-Copy Mailbox、自動タイムアウト復帰）。
- **開発ツール & ブートローダー**:
  - WCH CH342K による SerialUPDI 高速書き込み検証完了。
  - SoftwareSerial デバッグログ環境確立済み。

---

## 3. 直近のマイルストーン & 今後の課題 (Next Milestones)

1. **CORE-U rev1 の回路確定**:
   - 12V誤挿入対策（OVP）の回路方式確定（TMI6240 + ツェナーシャットダウン回路 vs 5V専用OVP統合IC）とネットリスト出力。
2. **CORE-S rev4 の最終FIX ＆ 試作発注**:
   - BMC・DCDC周りの定数・パターン最適化と試作検証。
3. **8748 筐体適合性（Enclosure-Friendly）の実機検証**:
   - 3Dプリントケース、タカチ防水ケースへのM3ネジ締結トルク試験、L型DCプラグ・IDCリボンケーブルの配線クリアランス検証。
4. **Crowd Supply プロジェクト準備**:
   - プロジェクト申請資料（`memo/merketing/crowd_supply_application.md`）のアップデート。
