<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D RS-485 通信テスト結果 (RS-485_test)

## 1. 実行日時
- 2026/08/24

## 2. 実施条件・ハードウェア状態
- **対象ボード**: ADX Core-D (初版基板) 2台（Master機 × 1, Slave機 × 1）
- **対象MCU**: Microchip ATtiny1616-MNR (QFN-20)
- **RS-485 トランシーバー**: MaxLinear SP485EEN-L/TR
- **通信設定**: 9600 bps, 8N1, 半二重通信（GPIO手動方向制御: `no_xdir`）
- **ピン接続**:
  - RS-485 TX: `PA1` (USART0 Pin Swap: `Serial.swap(1)`)
  - RS-485 RX: `PA2` (USART0 Pin Swap: `Serial.swap(1)`)
  - DE (Driver Enable): `PA4`
  - RE (Receiver Enable): `PA7`
  - PCシリアルデバッグ (Master): `PB4` (TX), `PB5` (RX) [SoftwareSerial 9600 bps]
  - 送信LED: `PB2` (赤色) / 受信・応答LED: `PB3` (白色)
- **バス配線・ジャンパ設定**:
  - 端子台（KF142R-5.08-3P）による A-A, B-B, GND-GND ストレート接続（配線長: 約20cm）
  - 終端抵抗（H4）: 両機とも 100Ω 無効（オープン）
  - DE/RE制御ジャンパ（H2）: 独立制御設定

## 3. 実行ログ (Test Log)

```txt
[Master Uptime: 32891 ms] Sent to RS-485: Hello
Recv from RS-485: Slave Uptime: 9502 ms (Req: Hello)

[Master Uptime: 40487 ms] Sent to RS-485: hello world
Recv from RS-485: Slave Uptime: 17182 ms (Req: hello world)

[Master Uptime: 49125 ms] Sent to RS-485: hiyoko kawaii
Recv from RS-485: Slave Uptime: 25908 ms (Req: hiyoko kawaii)
```

## 4. 判定・検証結果サマリー

| 項目 | 結果 | 判定 |
| :--- | :--- | :---: |
| **半二重方向切り替え (DE/RE)** | 送信・受信の衝突なく排他切り替えが成立 | **PASS** |
| **USART0 Pin Swap (`PA1`/`PA2`)** | ハードウェアシリアル通信が正常動作 | **PASS** |
| **PCデバッグ中継 (SoftwareSerial)** | PC ⇔ Master ⇔ Slave の相互中継が正常動作 | **PASS** |
| **データ完全性 (Echo back)** | 要求メッセージおよび稼働時間タイムスタンプの送受信整合性を確認 | **PASS** |
| **LEDインジケータ連動** | 送信中(赤)・受信/応答中(白)のLED点灯が正常動作 | **PASS** |

**総合判定: RS-485 通信機能は正常に動作しています (PASS)**
