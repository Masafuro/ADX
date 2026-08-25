<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト結果 (TC-P1-02)
(Sync 0x55 & PID 送出・受信テスト結果)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, トランシーバー: SP485EEN）における **`TC-P1-02`（Sync 0x55 & PID 送出・受信検証）** の実機動作検証結果をまとめたレポートです。

- テスト概要・手順仕様: [`README.md`](./README.md)
- テストプログラム: [`tc-p1-02_header_test.ino`](./tc-p1-02_header_test.ino)
- 実行生ログ: [`result.txt`](./result.txt)

---

## 1. 実行日時
- **実施日**: 2026/08/25

## 2. 実施条件・ハードウェア状態
- **対象ボード**: ADX Core-D (初版基板) 2台（Master機 × 1, Slave機 × 1）
- **対象MCU**: Microchip ATtiny1616-MNR (QFN-20)
- **MCU クロック**: 内部オシレータ（20MHz / 16MHz）
- **RS-485 トランシーバー**: MaxLinear SP485EEN-L/TR
- **通信設定**: 9600 bps ($T_{\text{bit}} \approx 104.17\,\mu\text{s}$), 半二重通信 (Header 送出)
- **テスト対象ヘッダ**:
  - Break: 14 Tbit LOW + 1 Tbit HIGH
  - Sync: `0x55` (01010101b)
  - PID: `0xC1` (ID: `0x01`, Parity: $P_0=1, P_1=1$)
- **ピン接続**:
  - RS-485 TX: `PA1` (USART0 Pin Swap: `Serial.swap(1)`)
  - RS-485 RX: `PA2` (USART0 Pin Swap: `Serial.swap(1)`)
  - DE (Driver Enable): `PA4`
  - RE (Receiver Enable): `PA7`
  - PCシリアルデバッグ: `PB4` (TX), `PB5` (RX) [SoftwareSerial 9600 bps]
  - 送信LED (Master): `PB2` (赤色)
  - 受信検知LED (Slave): `PB3` (白色)
- **バス配線・ジャンパ設定**:
  - 3P端子台（KF142R-5.08-3P）による A-A, B-B, GND-GND ストレート接続（配線長: 約20cm）
  - 終端抵抗（H4）: 両機とも 100Ω 無効（オープン）
  - DE/RE制御ジャンパ（H2）: 独立制御設定

---

## 3. 実行ログ (Test Log)

### 3.1 スレーブ機（受信・パース側）ログ
```txt
[Slave Recv #44] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #45] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #46] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #47] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #48] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #49] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #50] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #51] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #52] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #53] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #54] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #55] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
[Slave Recv #56] Raw Bytes (3B): [0x00, 0x55, 0xC1] -> Sync(0x55): OK, PID(0xC1 / ID:0x01) Parity: OK -> [ PASS / OK ]
```

### 3.2 マスター機（Header 送出側）ログ
```txt
[Master Uptime: 59018 ms] #59 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 60018 ms] #60 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 61018 ms] #61 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 62018 ms] #62 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 63018 ms] #63 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 64018 ms] #64 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 65018 ms] #65 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 66018 ms] #66 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 67018 ms] #67 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 68018 ms] #68 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 69018 ms] #69 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 70018 ms] #70 Sent Header: [Break] + [0x55] + [PID: 0xC1]
[Master Uptime: 71018 ms] #71 Sent Header: [Break] + [0x55] + [PID: 0xC1]
```

---

## 4. 判定・検証結果サマリー

| 検証項目 | 期待される動作 / 合格基準 | 実際の結果 | 判定 |
| :--- | :--- | :--- | :---: |
| **Sync バイト受信** | `0x55` (01010101b) が文字化けせずに完全受信されること | 毎回欠落なく `0x55` を受信 | **PASS** |
| **PID 受信 & パリティ一致** | PID=`0xC1` が受信され、$P_0, P_1$ のパリティ計算が一致すること | $P_0=1, P_1=1$ 合致を検知 | **PASS** |
| **フレーム整合性** | `[0x00, 0x55, 0xC1]` の 3バイト列が安定して受信されること | 56回以上の連続測定で 100% 成立 | **PASS** |
| **半二重方向制御 (DE/RE)** | ヘッダ送信完了まで DE=HIGH が維持され、完了直後に LOW に戻ること | 送信末尾欠落なくバス解放完了 | **PASS** |
| **インジケータ LED 連動** | 送信時（赤LED）および受信検知時（白LED）が 1秒周期で連動点滅すること | イベント連動点滅を確認 | **PASS** |

**総合判定: 合格 (PASS)**

---

## 5. 考察・特記事項 (Notes & Observations)

1. **LIN ヘッダ送出シーケンスの完全成立**:
   * Master 機の GPIO Break 送出（14 Tbit LOW + 1 Tbit HIGH）$\rightarrow$ `0x55` 送信 $\rightarrow$ PID（`0xC1`）送信 $\rightarrow$ `flush()` $\rightarrow$ `DE=0` の一連のシーケンスが完璧に機能しました。
2. **パリティ整合性の完全確認**:
   * ID = `0x01` に対する LIN 規格パリティ $P_0=1, P_1=1$（PID = `0xC1`）がスレーブ側で 100% 正当として判定され、RS-485 差動バス経由での 1 ビットの誤りもない伝送が実証されました。
3. **Phase 1（マスターヘッダ生成・基本UART検証）の全完了**:
   * `TC-P1-01`（Break波形）、`TC-P1-02`（Sync/PID送出）、`TC-P1-03`（スレーブUART受信）の全要件が達成されました。
   * これにより、次はいよいよ ATtiny1616 のハードウェア LIN スレーブエンジン（`LINAUTO` モード）を直接稼働させる **Phase 2 (`TC-P2-01` 〜 `TC-P2-05`)** へと進むことができます。
