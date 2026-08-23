<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D Hardware Verification & Test Suite

このディレクトリには、**ADX Core-D**（MCU: ATtiny1616）のハードウェア初期疎通確認（Bring-up）、単体機能テスト、出荷検査向けファームウェアおよびその実行検証結果をまとめています。

---

## 1. テスト一覧

| テスト名 | 対象機能 | 概要 |
| :--- | :--- | :--- |
| [`led_softserial_test`](./led_softserial_test/) | GPIO (PB2, PB3), SoftwareSerial (PB4, PB5) | シリアルコマンドによるLED点灯制御と双方向通信テスト |

---

## 2. ハードウェア検証レポート (Verification Report)

### 2.1 テスト環境
- **対象ボード**: ADX Core-D (初版基板)
- **搭載MCU**: Microchip ATtiny1616
- **ホスト環境**: Windows 11
- **開発環境**: Arduino IDE 1.8.x
- **BSP / Core**: [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore)
- **書き込み方式**: SerialUPDI (専用COMポート)
- **通信方式**: SoftwareSerial (9600 bps / 専用COMポート)

---

### 2.2 検証結果サマリー

| 項目 | 結果 | 判定 | 備考 |
| :--- | :---: | :---: | :--- |
| **UPDIプログラム書き込み** | 成功 | **PASS** | megaTinyCore経由でのSerialUPDI書き込みが正常動作 |
| **SoftwareSerial通信** | 成功 | **PASS** | 9600 bpsでのコマンド受信・応答送信が正常動作 |
| **GPIO制御 (PB2, PB3)** | 成功 | **PASS** | LEDの個別/同時点灯・消灯をコマンド制御可能 |
| **誤書き込みフェールセーフ** | 成功 | **PASS** | ソフトウェアシリアル用COMポートへのUPDI書き込み試行時、安全にエラー停止 |
| **UPDIポート通信フェールセーフ** | 成功 | **PASS** | UPDI用COMポートへシリアルモニターで接続しても無反応（ポート分離が確認） |

---

### 2.3 詳細検証内容

#### 1. 機能動作の確認
- Arduino IDEのシリアルモニター（9600 bps）からコマンド `0` 〜 `6` を送信し、各機能が意図通り動作することを確認。
  - `0`: ヘルプメニューの表示
  - `1` / `2`: PB2 LED ON / OFF
  - `3` / `4`: PB3 LED ON / OFF
  - `5` / `6`: PB2 & PB3 同時 ON / OFF
  - 未定義コマンドに対する `[ERR]` 応答

#### 2. ポート切り替えとプログラミング/通信の分離
- Windows 11環境において、UPDI用とSoftwareSerial用の2つのCOMポートを切り替えることで、プログラム書き込みと対話通信の両立を確認。

#### 3. フェールセーフ動作の確認
- **シリアルポートへの誤書き込み防止**:
  SoftwareSerial側のCOMポートを選択した状態でUPDI書き込みを実行した場合、マイコンやツールチェーンが不正なシーケンスを検知し、適切にエラーで中断・停止することを確認。
- **UPDIポートの通信分離**:
  UPDI側のCOMポートに対してシリアルモニターを開いて通信を試みた場合、マイコン側からの応答はなく、通信ラインの独立性と安全性を確認。
