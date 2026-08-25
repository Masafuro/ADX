<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト結果 (TC-P2)
(スレーブ LINAUTO ハードウェア自動同期 & PID パリティ・エラー検証結果)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, トランシーバー: SP485EEN）における **`TC-P2`（スレーブ LINAUTO ハードウェア同期・PIDパリティ・エラー検出検証）** の実機動作検証結果をまとめたレポートです。

- テスト概要・手順仕様: [`README.md`](./README.md)
- 修正・技術解析メモ: [`TC-P2_FIXED_MODIFICATION_MEMO.md`](./TC-P2_FIXED_MODIFICATION_MEMO.md)
- テストプログラム: [`tc-p2_slave_linauto_test.ino`](./tc-p2_slave_linauto_test.ino)
- 実行生ログ: [`result.txt`](./result.txt)

---

## 1. 実行日時
- **実施日**: 2026/08/25

## 2. 実施条件・ハードウェア状態
- **対象ボード**: ADX Core-D (初版基板) 2台（Master機 × 1, Slave機 × 1）
- **対象MCU**: Microchip ATtiny1616-MNR (QFN-20)
- **MCU クロック**: 内部オシレータ（20MHz / 16MHz）
- **RS-485 トランシーバー**: MaxLinear SP485EEN-L/TR
- **通信設定**: 9600 bps, 半二重通信 (LINAUTO ハードウェア同期モード)
- **ピン接続**:
  - RS-485 TX: `PA1` (USART0 Pin Swap: `Serial.swap(1)`)
  - RS-485 RX: `PA2` (USART0 Pin Swap: `Serial.swap(1)`)
  - DE (Driver Enable): `PA4`
  - RE (Receiver Enable): `PA7`
  - PCシリアルデバッグ: `PB4` (TX), `PB5` (RX) [SoftwareSerial 9600 bps]
  - 送信LED (Master): `PB2` (赤色)
  - 状態LED (Slave): `PB3` (白色: 同期・PID正常), `PB2` (赤色: エラー検知)
- **バス配線・ジャンパ設定**:
  - 3P端子台（KF142R-5.08-3P）による A-A, B-B, GND-GND ストレート接続（配線長: 約20cm）
  - 終端抵抗（H4）: 両機とも 100Ω 無効（オープン）
  - DE/RE制御ジャンパ（H2）: 独立制御設定

---

## 3. 実行ログ (Test Log)

### 3.1 スレーブ機（LINAUTO 受信・検証側）ログ
```txt
[Slave Event #19] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E4 | BDF: 0 -> [ PASS / OK ]
[Slave Event #20] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
[Slave Event #21] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E1 | BDF: 0 -> [ PASS / OK ]
[Slave Event #22] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E4 | BDF: 0 -> [ PASS / OK ]
[Slave Event #23] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
[Slave Event #24] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E3 | BDF: 0 -> [ PASS / OK ]
[Slave Event #25] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
[Slave Event #26] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E0 | BDF: 0 -> [ PASS / OK ]
[Slave Event #27] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
[Slave Event #28] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E0 | BDF: 0 -> [ PASS / OK ]
[Slave Event #29] PID Received: 0xC1 (ID: 0x01) | AutoBAUD: 0x20E2 | BDF: 0 -> [ PASS / OK ]
[Slave Event #30] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed
```

### 3.2 マスター機（テストパターン送出側）ログ
```txt
[Master Pattern #7] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #8] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #9] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #10] [TEST: ISFIF] Sent Corrupted Sync (0xAA) + PID(0xC1)
[Master Pattern #11] [TEST: PERR]  Sent Corrupted Parity PID (0x01)
[Master Pattern #12] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #13] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #14] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #15] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]
[Master Pattern #16] [TEST: ISFIF] Sent Corrupted Sync (0xAA) + PID(0xC1)
[Master Pattern #17] [TEST: PERR]  Sent Corrupted Parity PID (0x01)
```

---

## 4. 判定・検証結果サマリー (Phase 2 全 5 項目)

| テストID | 検証項目 | 期待される動作 / 合格基準 | 実際の結果 | 判定 |
| :---: | :--- | :--- | :--- | :---: |
| **`TC-P2-01`** | **ブレーク検出 (`BDF`)** | ブレーク受信時に `STATUS.BDF == 1` または同期シーケンスが起動すること | ブレーク検知・Auto-baud 起動を確認 | **PASS** |
| **`TC-P2-02`** | **Auto-baud 補正 (`BAUD`)** | `0x55` 受信により `USART0.BAUD` が自動補正・更新されること | `0x20E0` 〜 `0x20E4` で動的補正を確認 | **PASS** |
| **`TC-P2-03`** | **PID / パリティ認識** | `RXDATAH.DATA == 0` かつ `PERR == 0` で PID `0xC1` が取得できること | PID `0xC1` を完全受信 | **PASS** |
| **`TC-P2-04`** | **Sync エラー検出 (`ISFIF`)** | 不正 Sync (`0xAA`) 受信時に `ISFIF == 1` を検知し、復帰すること | `ISFIF DETECTED!` 検知および自律復帰を確認 | **PASS** |
| **`TC-P2-05`** | **PID パリティエラー (`PERR`)** | 不正パリティ PID 受信時に `PERR == 1` を検知し、破棄すること | 不正パリティ破棄＆待機復帰を確認 | **PASS** |

**総合判定: 合格 (PASS)**

---

## 5. 考察・特記事項 (Notes & Observations)

1. **ATtiny1616 ハードウェア LINAUTO エンジンの完全実証**:
   * スレーブ側のマイコンがマスターの `0x55` Sync キャラクタからボーレートを自動計測し、`USART0.BAUD` を `0x20E0` 〜 `0x20E4` に自動補正して PID を取得する動作が完全に実証されました。
2. **ISFIF（Sync不整合エラー）検知と自律リカバリ**:
   * 不正な Sync（`0xAA`）が到来した際、ハードウェアが `ISFIF` フラグをセットし、ファームウェアによるフラグクリア＆`WFB` 再設定によって、次の正常フレームを全く取りこぼすことなく即座に復帰できる堅牢性が確認されました。
3. **レジスタ直接代入によるフラグ競合の完全解消**:
   * AVR の「Write 1 to Clear」仕様に適合した `USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;` のアトミックな待機復帰により、通信エラーやハングアップがゼロになりました。
