<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト: TC-P1-01
(GPIO Break 信号波形検証テスト)

本ディレクトリには、**ADX Core-D**（MCU: ATtiny1616, トランシーバー: SP485EEN）における LN-485 規格の基礎となる **マスター側 GPIO Break 信号波形送出テスト (`TC-P1-01`)** のスケッチ、テスト実施手順、および結果記録シートをまとめています。

> **💡 測定機器（オシロスコープ等）が手元にない場合でも実施可能:**
> 2台目の ADX Core-D（スレーブ機）を **「波形自己測定アナライザ」** として動作させることで、マイコン自身が Break の LOW パルス幅（マイクロ秒）を計測し、PC シリアルモニタ上に数値と OK/NG 判定をリアルタイム表示できます。

---

## 1. テスト概要 (Test Overview)

* **テストID**: `TC-P1-01`
* **対象フェーズ**: Phase 1（マスターヘッダ送出 & 基本波形・UART受信検証）
* **検証目的**:
  マスター機において、GPIO トグル方式を用いて LIN 規格に準拠した **「14ビット時間の Dominant (LOW) ＋ 1ビット時間の Recessive (HIGH / Delimiter)」** のブレーク信号が RS-485 差動バス上へ正確に出力されることを確認する。
* **テストスケッチ**: [`tc-p1-01_break_test.ino`](./tc-p1-01_break_test.ino)

```text
       ┌────────── 14 Tbit LOW (約1458 µs @ 9600bps) ──────────┐  ┌─ 1 Tbit HIGH (約104 µs) ─┐
       │                                                        │  │                         │
───────┘                                                        └──┘                         └───────
  Idle (HIGH)               Break Field (LOW)                   Delimiter                   Idle (HIGH)
```

---

## 2. テスト環境および接続構成

### 2.1 使用機材
* **マスター機**: ADX Core-D #1 (MCU: ATtiny1616-MNR, 内部OSC 20MHz/16MHz) ── Break 送出側
* **スレーブ機**: ADX Core-D #2 (MCU: ATtiny1616-MNR, 内部OSC 20MHz/16MHz) ── Break 測定アナライザ側
* **ホストPC**: Windows 11 (Arduino IDE 1.8.x / 2.x + megaTinyCore)
* **外部測定機器**: 不要（※オシロスコープがある場合は併用可能）

### 2.2 結線およびジャンパ設定
1. **RS-485 バス配線**:
   * 2台の ADX Core-D の 3P 端子台（KF142R-5.08-3P）をストレート接続（A-A, B-B, GND-GND, 配線長 約20cm）。
2. **ジャンパ設定**:
   * `H4` (終端抵抗 100Ω): **オープン（無効）**
   * `H2` (DE/RE制御): **独立制御**（PA4=DE, PA7=\RE）
   * `H3` (クロック源): **内部オシレータ**（デフォルト設定）

---

## 3. 開発環境・書き込み設定 (megaTinyCore)

Arduino IDE のボード設定は以下のように設定します。

| 項目 | 設定値 |
| :--- | :--- |
| **Board** | `ATtiny1616/1606/816/806/416/406` |
| **Chip** | `ATtiny1616` |
| **Clock Source** | `20MHz internal` または `16MHz internal` |
| **Programmer** | `SerialUPDI - 230400 baud` (CH342K Port A) |

---

## 4. テスト実施手順 (Action)

### 【Step 1】 スレーブ機（測定アナライザ）の準備と書き込み
1. [`tc-p1-01_break_test.ino`](./tc-p1-01_break_test.ino) の 9 行目をコメントアウトします。
   ```cpp
   // #define ROLE_MASTER    // スレーブ（測定アナライザ）機用
   ```
2. スレーブ用 ADX Core-D（#2）の SerialUPDI ポート（CH342K Port A）へ書き込みます。
3. 書き込み完了後、スレーブ機の SoftwareSerial ポート（CH342K Port B）をシリアルモニタ（**9600 bps**）で開きます。
4. 以下の待機画面が表示されます。
   ```txt
   ==============================================
   === ADX Core-D LN-485 Test: TC-P1-01       ===
   === [SLAVE] Break Pulse Width Analyzer     ===
   ==============================================
   [Target] Expected Break LOW: 1458 us (14 Tbit @ 9600 bps)
   [Pass Range] 1146 us (11 Tbit) <= Width <= 1875 us (18 Tbit)
   [Status] Waiting for Master Break pulses on PA2 (RXD)...
   ```

### 【Step 2】 マスター機（Break 送出側）の準備と書き込み
1. [`tc-p1-01_break_test.ino`](./tc-p1-01_break_test.ino) の 9 行目を有効化します。
   ```cpp
   #define ROLE_MASTER       // マスター（送出）機用
   ```
2. マスター用 ADX Core-D（#1）の SerialUPDI ポート（CH342K Port A）へ書き込みます。
3. マスター機が 1 秒周期で Break パルスの送出を開始し、赤LED（PB2）が点滅します。

### 【Step 3】 スレーブ側シリアルモニタでの測定結果確認
1. スレーブ側のシリアルモニタに、1 秒ごとに以下のようにマイクロ秒単位の実測値と判定が表示されることを確認します。
   ```txt
   [Slave Meas #1] Width: 1456 us (Diff: -2 us, Error: -0.1%) -> [ PASS / OK ]
   [Slave Meas #2] Width: 1458 us (Diff:  0 us, Error:  0.0%) -> [ PASS / OK ]
   [Slave Meas #3] Width: 1456 us (Diff: -2 us, Error: -0.1%) -> [ PASS / OK ]
   ```
2. パルスを受信するたびに、スレーブ基板上の白LED（PB3）が一瞬点滅します。

---

## 5. 合否判定基準 (OK / NG Criteria)

| 判定 | 判定基準 | 備考 |
| :---: | :--- | :--- |
| **【 OK 】** | ・スレーブ側実測値が **1146 µs（11 Tbit）以上 かつ 1875 µs（18 Tbit）以下**（理想値: 約 1458 µs）の範囲内である。<br>・シリアルモニタ上に `[ PASS / OK ]` が連続して表示される。<br>・マスターの赤LED（送信）およびスレーブの白LED（検知）が 1 秒周期で連動点滅する。 | **合格 (PASS)** |
| **【 NG 】** | ・実測値が 1146 µs 未満（$<11\,\text{Tbit}$）である（ブレーク長不足）。<br>・スレーブ側で全くパルスが検知されない（`Width: 0` または無応答）。<br>・シリアルモニタ上に `[ FAIL / NG ]` が表示される。 | **不合格 (FAIL)** |

---

## 6. テスト結果の記録

実機テストを実施した後は、[**`result.md`**](./result.md) に測定データ・シリアルログ・合否判定を記録してください。
