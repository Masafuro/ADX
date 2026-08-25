<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D LN-485 テスト: TC-P1-02
(Sync 0x55 & PID 送出・受信検証テスト)

本ディレクトリには、**ADX Core-D**（MCU: ATtiny1616, トランシーバー: SP485EEN）における LN-485 規格の **マスター側 LIN Header（Break + Sync `0x55` + PID）送出およびスレーブ通常 UART 受信検証テスト (`TC-P1-02`)** のスケッチ、テスト実施手順、および結果記録シートをまとめています。

---

## 1. テスト概要 (Test Overview)

* **テストID**: `TC-P1-02`
* **対象フェーズ**: Phase 1（マスターヘッダ送出 & 基本波形・UART受信検証）
* **検証目的**:
  マスター機において、GPIO Break 信号（14 Tbit LOW + 1 Tbit HIGH）に続き、LIN 規格に準拠した **Sync キャラクタ (`0x55`)** および **正当なパリティを持つ PID (Protected Identifier)** を RS-485 差動バス上へ連続送信し、スレーブ機の標準 UART でこれらが欠落・化けなく受信できることを確認する。
* **テストスケッチ**: [`tc-p1-02_header_test.ino`](./tc-p1-02_header_test.ino)

```text
       ┌── Break (14 Tbit LOW) ──┐ ┌─ Delimiter (1 Tbit HIGH) ─┐ ┌── Sync (0x55) ──┐ ┌── PID (0x81) ──┐
───────┘                         └──┘                          └─── 01010101b ─────┴─── P0,P1 + ID ───┴───────
  Idle                                                                                                  Idle
```

### パリティ方程式 (LIN Standard)
* ID = `0x01`（6-bit: `000001b`）の場合:
  * $P_0 = ID_0 \oplus ID_1 \oplus ID_2 \oplus ID_4 = 1 \oplus 0 \oplus 0 \oplus 0 = 0$
  * $P_1 = \text{NOT}(ID_1 \oplus ID_3 \oplus ID_4 \oplus ID_5) = \text{NOT}(0 \oplus 0 \oplus 0 \oplus 0) = 1$
  * **$\text{PID} = 0\text{x01} \mid (0 \ll 6) \mid (1 \ll 7) = 0\text{x81}$**

---

## 2. テスト環境および接続構成

### 2.1 使用機材
* **マスター機**: ADX Core-D #1 (MCU: ATtiny1616-MNR, 内部OSC 20MHz/16MHz) ── Header 送出側
* **スレーブ機**: ADX Core-D #2 (MCU: ATtiny1616-MNR, 内部OSC 20MHz/16MHz) ── Header 受信・解析側
* **ホストPC**: Windows 11 (Arduino IDE 1.8.x / 2.x + megaTinyCore)
* **外部測定機器**: 不要（PC シリアルモニタで自動検証）

### 2.2 結線およびジャンパ設定
1. **RS-485 バス配線**:
   * 2台の ADX Core-D の 3P 端子台（KF142R-5.08-3P）をストレート接続（A-A, B-B, GND-GND, 配線長 約20cm）。
2. **ジャンパ設定**:
   * `H4` (終端抵抗 100Ω): **オープン（無効）**
   * `H2` (DE/RE制御): **独立制御**（PA4=DE, PA7=\RE）
   * `H3` (クロック源): **内部オシレータ**（デフォルト設定）

---

## 3. 開発環境・書き込み設定 (megaTinyCore)

| 項目 | 設定値 |
| :--- | :--- |
| **Board** | `ATtiny1616/1606/816/806/416/406` |
| **Chip** | `ATtiny1616` |
| **Clock Source** | `20MHz internal` または `16MHz internal` |
| **Programmer** | `SerialUPDI - 230400 baud` (CH342K Port A) |

---

## 4. テスト実施手順 (Action)

### 【Step 1】 スレーブ機（Header 受信側）の準備と書き込み
1. [`tc-p1-02_header_test.ino`](./tc-p1-02_header_test.ino) の 9 行目をコメントアウトします。
   ```cpp
   // #define ROLE_MASTER    // スレーブ（受信）機用
   ```
2. スレーブ用 ADX Core-D（#2）の SerialUPDI ポート（CH342K Port A）へ書き込みます。
3. 書き込み完了後、スレーブ機の SoftwareSerial ポート（CH342K Port B）をシリアルモニタ（**9600 bps**）で開きます。
4. 以下の待機画面が表示されます。
   ```txt
   ==============================================
   === ADX Core-D LN-485 Test: TC-P1-02       ===
   === [SLAVE] Header Reception & Parser      ===
   ==============================================
   [Config] BaudRate: 9600 bps (Standard UART Mode)
   [Target] Expected Frame: [Break/0x00] + [0x55] + [PID: 0x81 (ID:0x01)]
   [Status] Listening on RS-485 bus...
   ```

### 【Step 2】 マスター機（Header 送出側）の準備と書き込み
1. [`tc-p1-02_header_test.ino`](./tc-p1-02_header_test.ino) の 9 行目を有効化します。
   ```cpp
   #define ROLE_MASTER       // マスター（送出）機用
   ```
2. マスター用 ADX Core-D（#1）の SerialUPDI ポート（CH342K Port A）へ書き込みます。
3. マスター機が 1 秒周期で Header 送出を開始し、赤LED（PB2）が点滅します。
4. マスター側のシリアルモニタ（Port B）に以下のログが表示されます。
   ```txt
   [Master Uptime: 1000 ms] #1 Sent Header: [Break] + [0x55] + [PID: 0x81]
   [Master Uptime: 2000 ms] #2 Sent Header: [Break] + [0x55] + [PID: 0x81]
   ```

### 【Step 3】 スレーブ側シリアルモニタでの受信・パース結果確認
1. スレーブ側のシリアルモニタに、マスターからのヘッダ受信ログが以下のように出力されることを確認します。
   ```txt
   [Slave Recv #1] Raw Bytes (3B): [0x00, 0x55, 0x81] -> Sync(0x55): OK, PID(0x81 / ID:0x01) Parity: OK -> [ PASS / OK ]
   [Slave Recv #2] Raw Bytes (3B): [0x00, 0x55, 0x81] -> Sync(0x55): OK, PID(0x81 / ID:0x01) Parity: OK -> [ PASS / OK ]
   [Slave Recv #3] Raw Bytes (3B): [0x00, 0x55, 0x81] -> Sync(0x55): OK, PID(0x81 / ID:0x01) Parity: OK -> [ PASS / OK ]
   ```
2. ヘッダを受信するたびに、スレーブ基板上の白LED（PB3）が一瞬点滅します。

---

## 5. 合否判定基準 (OK / NG Criteria)

| 判定 | 判定基準 | 備考 |
| :---: | :--- | :--- |
| **【 OK 】** | ・スレーブ側で Break（`0x00` または FE）に続き、**`0x55` (Sync)** および **`0x81` (PID)** が確実に受信される。<br>・PID のパリティ計算（$P_0=0, P_1=1$）が一致し、シリアルモニタ上に `[ PASS / OK ]` が連続表示される。<br>・マスターの赤LED（送信）およびスレーブの白LED（受信）が 1 秒周期で連動点滅する。 | **合格 (PASS)** |
| **【 NG 】** | ・Sync バイトが `0x55` 以外の値に化けている、または受信されない。<br>・PID バイトが破損し、パリティ不一致（`PID Parity: FAIL`）となる。<br>・送信途中で DE が LOW に落ちてデータ末尾が欠落する。 | **不合格 (FAIL)** |

---

## 6. テスト結果の記録

実機テストを実施した後は、[**`result.md`**](./result.md) にシリアルログおよび合否判定を記録してください。
