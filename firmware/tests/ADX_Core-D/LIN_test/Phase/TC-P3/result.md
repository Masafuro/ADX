<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト結果 (TC-P3)
(Type A [Master Pub → Slave Sub] 実証 ＆ Slave Subscriber 実装結果)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, トランシーバー: SP485EEN）における **`TC-P3`（Type A 通信実証 ＆ Slave Subscriber 実装）** の実機動作検証結果をまとめるレポートです。

- テスト概要・手順仕様: [`README.md`](./README.md)
- 技術解析・修正レポート: [`fixed_report.md`](./fixed_report.md)
- テストプログラム: [`fixed.ino`](./fixed.ino) / [`tc-p3_master_pub_slave_sub.ino`](./tc-p3_master_pub_slave_sub.ino)

---

## 1. 実行日時・実施環境
- **実施日**: 2026/08/26
- **対象ボード**: ADX Core-D 2台（Master機 × 1, Slave機 × 1）
- **対象MCU**: Microchip ATtiny1616-MNR (QFN-20)
- **MCU クロック**: 内部オシレータ（20MHz / 16MHz）
- **RS-485 トランシーバー**: MaxLinear SP485EEN-L/TR
- **通信設定**: 9600 bps, 半二重通信 (Master: GPIO Break / Slave: LINAUTO モード)
- **ピン接続**:
  - RS-485 TX: `PA1` (USART0 Pin Swap: `PORTMUX_USART0_ALTERNATE_gc`)
  - RS-485 RX: `PA2` (USART0 Pin Swap: `PORTMUX_USART0_ALTERNATE_gc`)
  - DE (Driver Enable): `PA4`
  - RE (Receiver Enable): `PA7`
  - PCシリアルデバッグ: `PB4` (TX), `PB5` (RX) [SoftwareSerial 9600 bps]
  - 制御LED: `PB2` (赤色: ID=0x02 コマンド連動), `PB3` (白色: ID=0x03 コマンド連動)
- **バス配線・ジャンパ設定**:
  - 3P端子台による A-A, B-B, GND-GND ストレート接続
  - 終端抵抗（H4）: 両機とも 100Ω 無効（オープン）
  - DE/RE制御ジャンパ（H2）: 独立制御設定

---

## 2. 実行ログ (Test Log)

### 2.1 スレーブ機（Subscriber 受信 ＆ LED 制御側）ログ
```txt
[Slave Frame #12] PID: 0xC4 (ID: 0x04) | Payload: [0x12, 0x34, 0x56, 0x78] | CS: 0xEA -> [ PASS / OK ]
[Slave Frame #13] PID: 0x42 (ID: 0x02) | Payload: [0x01] | CS: 0xFE -> [ PASS / OK ]
  └─> [Action] Red LED turned ON
[Slave Frame #14] PID: 0x42 (ID: 0x02) | Payload: [0x00] | CS: 0xFF -> [ PASS / OK ]
  └─> [Action] Red LED turned OFF
[Slave Frame #15] PID: 0x03 (ID: 0x03) | Payload: [0x01] | CS: 0xFE -> [ PASS / OK ]
  └─> [Action] White LED turned ON
[Slave Frame #16] PID: 0x03 (ID: 0x03) | Payload: [0x00] | CS: 0xFF -> [ PASS / OK ]
  └─> [Action] White LED turned OFF
[Slave Frame #17] PID: 0x42 (ID: 0x02) | Payload: [0x01] | CS: 0x01 -> [ FAIL / CS MISMATCH ] (Discarded)
  └─> [Safety] Frame discarded, no LED change.
[Slave Frame #18] PID: 0xC4 (ID: 0x04) | Payload: [0x12, 0x34, 0x56, 0x78] | CS: 0xEA -> [ PASS / OK ]
[Slave Frame #19] PID: 0x42 (ID: 0x02) | Payload: [0x01] | CS: 0xFE -> [ PASS / OK ]
  └─> [Action] Red LED turned ON
[Slave Frame #20] PID: 0x42 (ID: 0x02) | Payload: [0x00] | CS: 0xFF -> [ PASS / OK ]
  └─> [Action] Red LED turned OFF
[Slave Frame #21] PID: 0x03 (ID: 0x03) | Payload: [0x01] | CS: 0xFE -> [ PASS / OK ]
  └─> [Action] White LED turned ON
```

### 2.2 マスター機（Master-Publish 送出側）ログ
```txt
[Master Pattern #84] Sent Type A: ID=0x04, Data=[0x12, 0x34, 0x56, 0x78] (Multi-byte Test)
[Master Pattern #85] Sent Type A: ID=0x02, Data=[0x01] (Red LED ON)
[Master Pattern #86] Sent Type A: ID=0x02, Data=[0x00] (Red LED OFF)
[Master Pattern #87] Sent Type A: ID=0x03, Data=[0x01] (White LED ON)
[Master Pattern #88] Sent Type A: ID=0x03, Data=[0x00] (White LED OFF)
[Master Pattern #89] [TEST: CORRUPTED CS] Sent Type A: ID=0x02, Data=[0x01] + Inverted Checksum
[Master Pattern #90] Sent Type A: ID=0x04, Data=[0x12, 0x34, 0x56, 0x78] (Multi-byte Test)
[Master Pattern #91] Sent Type A: ID=0x02, Data=[0x01] (Red LED ON)
[Master Pattern #92] Sent Type A: ID=0x02, Data=[0x00] (Red LED OFF)
[Master Pattern #93] Sent Type A: ID=0x03, Data=[0x01] (White LED ON)
[Master Pattern #94] Sent Type A: ID=0x03, Data=[0x00] (White LED OFF)
[Master Pattern #95] [TEST: CORRUPTED CS] Sent Type A: ID=0x02, Data=[0x01] + Inverted Checksum
[Master Pattern #96] Sent Type A: ID=0x04, Data=[0x12, 0x34, 0x56, 0x78]
```

---

## 3. 判定・検証結果サマリー (Phase 3 全 3 項目)

| テストID | 検証項目 | 期待される動作 / 合格基準 | 実際の結果 | 判定 |
| :---: | :--- | :--- | :--- | :---: |
| **`TC-P3-01`** | **ミニマム・スケジューラ ＆ Master-Publish 送出** | 1.5秒周期で Type A フレームが連続送出され、送信完了後に DE が即座に LOW に戻ること | Pattern #84〜#96 の連続送出およびスレーブでの全フレーム安定受信を確認 | **PASS** |
| **`TC-P3-02`** | **Slave Subscriber ペイロード受信 ＆ チェックサム検証** | PID 受信後にデータと CS を受信し、Classic Checksum が一致して `[ PASS / OK ]` となること | 1バイトコマンド（ID=0x02, 0x03）および4バイトデータ（ID=0x04）の完全受信・CS 一致を確認 | **PASS** |
| **`TC-P3-03`** | **コマンド連動 LED 制御 ＆ 不正チェックサム破棄** | 正常フレームで赤/白 LED が制御され、不正 CS フレームで LED が変化せず安全に破棄されること | 正常フレームでの LED 点灯/消灯制御、および破損 CS 時の安全破棄・自律復帰を確認 | **PASS** |

**総合判定: 合格 (PASS)**

---

## 4. 考察・特記事項 (Notes & Observations)

1. **スレーブ側初期化における Arduino Core 割り込み競合の完全解消**:
   - スレーブ側で `Serial.begin()` を呼び出すと megaTinyCore の RX 割り込みハンドラが自動登録され、ハードウェア `RXDATAL` のデータを吸い上げて `STATUS.RXCIF` を勝手にクリアしてしまう競合が発生していました。
   - `Serial.begin()` を排除し、`PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;` および `PORTA` レジスタ直接制御で初期化を行うことで、ポーリング駆動のハードウェア受信が 100% 安定して動作することが実証されました（詳細は [`fixed_report.md`](./fixed_report.md) および [`fixed.ino`](./fixed.ino) 参照）。
2. **Type A (Master-Publish) 通信フレームの完全実証**:
   - 単一バイトコマンドだけでなく、複数バイト（4バイトテレメトリ）フレームも欠落なく受信・チェックサム計算（$\sum \text{Data} + \text{CS} \equiv 0\text{xFF}$）が成立することが実証されました。
3. **不正チェックサム（ビット破損）に対する堅牢なフェイルセーフ**:
   - マスターが意図的に反転させた不正チェックサムを送信した際、スレーブが瞬時に `[ FAIL / CS MISMATCH ]` を検知し、アクチュエータ（LED）を一切動作させずにフレームを破棄して `WFB=1` に自律復帰できることが確認されました。
4. **Phase 4 への展開**:
   - 本検証により、スレーブ側の受信・解析エンジン（Subscriber）が完成しました。次の Phase 4（Type B: Slave-Publish）では、スレーブ側が Publisher としてバス権（DE制御）を取得して応答送信を行う処理の実証に進みます。
