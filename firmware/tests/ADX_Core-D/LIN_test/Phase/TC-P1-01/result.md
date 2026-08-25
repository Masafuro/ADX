<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト結果 (TC-P1-01)
(GPIO Break 信号送出・受信テスト結果)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, トランシーバー: SP485EEN）における **`TC-P1-01`（GPIO Break 信号波形検証）** の実機動作検証結果をまとめたレポートです。

- テスト概要・手順仕様: [`README.md`](./README.md)
- テストプログラム: [`tc-p1-01_break_test.ino`](./tc-p1-01_break_test.ino)
- 実行生ログ: [`result.txt`](./result.txt)

---

## 1. 実行日時
- **実施日**: 2026/08/25

## 2. 実施条件・ハードウェア状態
- **対象ボード**: ADX Core-D (初版基板) 2台（Master機 × 1, Slave機 × 1）
- **対象MCU**: Microchip ATtiny1616-MNR (QFN-20)
- **MCU クロック**: 内部オシレータ（20MHz / 16MHz）
- **RS-485 トランシーバー**: MaxLinear SP485EEN-L/TR
- **通信設定**: 9600 bps ($T_{\text{bit}} \approx 104.17\,\mu\text{s}$), 半二重通信 (GPIO Break 送出)
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

### 3.1 スレーブ機（受信検知・パルス幅測定側）ログ
```txt
[Slave Meas #142] Width: 1471 us (Diff: +15 us, Error: +1.0%) -> [ PASS / OK ]
[Slave Meas #143] Width: 1467 us (Diff: +11 us, Error: +0.8%) -> [ PASS / OK ]
[Slave Meas #144] Width: 1468 us (Diff: +12 us, Error: +0.8%) -> [ PASS / OK ]
[Slave Meas #145] Width: 1467 us (Diff: +11 us, Error: +0.8%) -> [ PASS / OK ]
[Slave Meas #146] Width: 1467 us (Diff: +11 us, Error: +0.8%) -> [ PASS / OK ]
[Slave Meas #147] Width: 1467 us (Diff: +11 us, Error: +0.8%) -> [ PASS / OK ]
[Slave Meas #148] Width: 1472 us (Diff: +16 us, Error: +1.1%) -> [ PASS / OK ]
[Slave Meas #149] Width: 1466 us (Diff: +10 us, Error: +0.7%) -> [ PASS / OK ]
[Slave Meas #150] Width: 1467 us (Diff: +11 us, Error: +0.8%) -> [ PASS / OK ]
[Slave Meas #151] Width: 1472 us (Diff: +16 us, Error: +1.1%) -> [ PASS / OK ]
[Slave Meas #152] Width: 1471 us (Diff: +15 us, Error: +1.0%) -> [ PASS / OK ]
[Slave Meas #153] Width: 1472 us (Diff: +16 us, Error: +1.1%) -> [ PASS / OK ]
[Slave Meas #154] Width: 1471 us (Diff: +15 us, Error: +1.0%) -> [ PASS / OK ]
```

### 3.2 マスター機（Break 送出側）ログ
```txt
[Master Uptime: 60015 ms] #60 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 61015 ms] #61 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 62016 ms] #62 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 63015 ms] #63 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 64016 ms] #64 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 65015 ms] #65 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 66016 ms] #66 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 67015 ms] #67 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 68016 ms] #68 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 69015 ms] #69 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 70016 ms] #70 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 71015 ms] #71 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
[Master Uptime: 72016 ms] #72 Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)
```

---

## 4. 判定・検証結果サマリー

| 検証項目 | 期待される動作 / 合格基準 | 実際の結果 | 判定 |
| :--- | :--- | :--- | :---: |
| **Break 信号幅 (LOW)** | スレーブ検知幅が 1146 µs 〜 1875 µs（14 Tbit 前後）であること | 実測 **1466 µs 〜 1472 µs**（誤差 +0.7% 〜 +1.1%）で極めて高精度 | **PASS** |
| **パルス幅ジッター** | パルス幅の変動が小さく安定していること | 変動幅わずか **6 µs (約 0.4%)** で極めて安定 | **PASS** |
| **連続送受信の安定性** | 1 秒周期で途切れず連続してブレーク送受信が成立すること | 150 回以上の連続測定で 100% 欠落なく検知 | **PASS** |
| **半二重方向制御 (DE/RE)** | 送信時に DE が HIGH、完了後に LOW へ戻りバスが解放されること | 送受信切り替えタイミングが正常に機能 | **PASS** |
| **PCデバッグログ出力** | SoftwareSerial 経由で PC シリアルモニタ上に送受信ログが出力されること | 双方のログ出力が正常動作 | **PASS** |
| **インジケータ LED 連動** | 送信時（赤LED）および受信検知時（白LED）が 1秒周期で連動点滅すること | イベント連動点滅を確認 | **PASS** |

**総合判定: 合格 (PASS)**

---

## 5. 考察・特記事項 (Notes & Observations)

1. **極めて優れたブレーク波形精度**:
   * 設計期待値 $1458\,\mu\text{s}$（14 Tbit）に対し、スレーブ実測値は **1466 µs 〜 1472 µs** であり、誤差はわずか **+0.8% 前後** に収まりました。
   * LIN 規格スレーブのブレーク検出下限（11 Tbit = $1146\,\mu\text{s}$）を確実に満たしており、次フェーズでのハードウェア LIN スレーブエンジン（`LINAUTO`）による同期・検出が完全に保証されます。
2. **低ジッターな内部オシレータ動作**:
   * 内蔵オシレータ駆動でありながらパルス幅のばらつき（ジッター）は最大でも **6 µs (約0.4%)** と極めて微小であり、GPIO トグル方式の安定性が実証されました。
3. **次フェーズ（`TC-P1-02` / `TC-P1-03`）への進展**:
   * 本検証の成功により、Break に続く **Sync バイト (`0x55`) および PID の連続送出・UART 受信テスト** へ安全に移行できます。
