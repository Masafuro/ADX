<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# Milestone 3 検証レポート: Windows 11 向け Python CLI による RS-485 疎通実証

## 1. 概要
- **目的**: 
  - USB-RS485 ドングル（COM19）と Core-D 端子台（A, B, GND）を結線し、Windows 11 上の専用ポーリングスクリプト [`adx_rs485_ping.py`](../../scripts/adx_rs485_ping.py) を用いて、ブートローダーとホスト間の双方向 RS-485 半二重通信（115,200 bps）が 100% 成立することを実証する。
- **実施日**: 2026-09-27
- **検証環境**:
  - ホスト PC: Windows 11
  - アダプタ: 2Mbps 対応 USB-RS485 アダプタ（COM ポート: `COM19`）
  - ターゲット: ADX Core-D（COM ポート: `COM20` ※給電・デバッグ用）
  - ハードウェア半二重制御: PA1(TX), PA2(RX), PA4(XDIR / DE 自動制御), PA7(/RE 常時受信 LOW)
  - 疎通スクリプト: [`firmware/bootloader/OneToOne_RS485/scripts/adx_rs485_ping.py`](../../scripts/adx_rs485_ping.py)

---

## 2. 物理結線手順

USB-RS485 ドングルと Core-D 端子台を以下のように結線します：

```text
[USB-RS485 ドングル]                [ADX Core-D 端子台]
     A (D+)       ------------------       A
     B (D-)       ------------------       B
     GND          ------------------       GND
```

> [!IMPORTANT]
> - **GND の接続**: ノイズ耐性と同相電位差を抑えるため、`GND` 同士は必ず接続してください。
> - **A / B の極性**: RS-485 ドングルによっては `A` と `B` の極性定義（+/-）が逆転している場合があります。もし通信が通らない場合は、端子台側で `A` と `B` を入れ替えてお試しください。

---

## 3. 検証手順

### ① スクリプトの実行（PC 側待機）
Windows 11 の PowerShell で、リポジトリルートから以下のコマンドを実行します：

```powershell
python firmware/bootloader/OneToOne_RS485/scripts/adx_rs485_ping.py COM19
```

スクリプトが起動すると、ポートを開いて自動的にポーリング送信を開始します：
```text
=================================================================
  ADX Core-D 1-to-1 RS-485 Bootloader Ping Utility (M3)
=================================================================
Target Port : COM19
Baud Rate   : 115200 bps (8N1)
Timeout     : 30 seconds
-----------------------------------------------------------------
[INFO] Port COM19 opened successfully.
[INFO] Waiting for Core-D to power on / reset...
>>> Power on or reset the Core-D NOW (Red LED PB2 will start blinking) <<<

[POLLING] Sending STK_GET_SYNC probe... (1s / 30s) ...
```

---

### ② Core-D の電源投入（またはリセット）
スクリプトが待機状態に入っている状態で、**Core-D の USB Type-C ケーブルを抜き差し**（電源再投入）します。

- **期待されるシーケンス**:
  1. Core-D の電源が入った瞬間、赤色 LED（PB2）が点滅を開始。
  2. スクリプトが送信しているプローブ（`STK_GET_SYNC: 0x30 0x20`）を Core-D が受信。
  3. Core-D（Optiboot）がハードウェア XDIR 制御により直ちに `0x14 0x10`（`STK_INSYNC` + `STK_OK`）を返信。
  4. スクリプトが応答を受信し、コンソールに `[PASS]` とバージョン情報が表示される。

---

## 4. 実際の実行ログ記録欄

```text
PS C:\Users\User> python .\adx_rs485_ping.py COM19
=================================================================
  ADX Core-D 1-to-1 RS-485 Bootloader Ping Utility (M3)
=================================================================
Target Port : COM19
Baud Rate   : 115200 bps (8N1)
Timeout     : 30 seconds
-----------------------------------------------------------------
[INFO] Port COM19 opened successfully.
[INFO] Waiting for Core-D to power on / reset...
>>> Power on or reset the Core-D NOW (Red LED PB2 will start blinking) <<<

[POLLING] Sending STK_GET_SYNC probe... (1s / 30s[POLLING] Sending STK_GET_SYNC probe... (2s / 30s[POLLING] Sending STK_GET_SYNC probe... (3s / 30s[POLLING] Sending STK_GET_SYNC probe... (4s / 30s[POLLING] Sending STK_GET_SYNC probe... (5s / 30s) .

=================================================================
  [PASS] ADX Bootloader is ALIVE on RS-485!
=================================================================
Response Received : 0x14 (STK_INSYNC) 0x10 (STK_OK)
Round-Trip Time   : 83.8 ms
=================================================================
[RESULT] Milestone 3 RS-485 Communication Test: PASS
```

> [!IMPORTANT]
> **【重要仕様: RS-485 逆接時の挙動とトラブルシューティング】**:
> - **逆接時の挙動**: RS-485 の A と B が逆接されている場合、バスのアイドル電位が反転して常時 Break 信号（LOW レベル）が入力されるため、文字受信トリガーにより **「電源投入直後に赤色 LED（PB2）が点滅せず即座に消灯したまま待機する（一切書き込みを受け付けず、8秒後にアプリへ脱落する）」** という挙動を示します。
> - **判定・対処法**: **「電源を入れた瞬間に赤 LED が点滅せず即座に消灯して通信が通らない」場合は、100% の確率で A と B が逆接** されています。端子台側で A と B を入れ替えることで直ちに復旧します。
> - *(※ブートローダーを軽量（<= 512B）に保つため、ソフトウェアでの極性自動反転等は行わず、この特徴的な消灯挙動を「明確な誤結線シグネチャ」として仕様化します)*

---

## 5. 合否判定（Exit Criteria）

| 判定項目 | 合格基準 | 結果 |
| :--- | :--- | :---: |
| **物理結線 & ポートオープン** | USB-RS485 ドングル（COM19）が正常にオープンできること | **PASS** |
| **STK500 同期応答** | 電源投入時、即座に `0x14 0x10`（INSYNC + OK）を受信すること | **PASS** |
| **ハードウェア自動制御** | Core-D の PA4(XDIR) による半二重送信切り替えが正常に機能すること | **PASS** |
| **ボーレート整合** | 115,200 bps で文字化け・フレームエラーなく通信できること | **PASS** |

**総合判定**: **【 PASS 】**

---

## 6. 次のマイルストーン（Milestone 4）への引き継ぎ
RS-485 双方向通信が 100% 成立したため、**Milestone 4: RS-485 経由での avrdude アプリ書き込み（Over-The-Wire 実証）** に進みます。
テストスケッチ（オンボード白LED PB3 点滅）を avrdude で RS-485（COM19）経由で書き込み、Flash Read-back Verify とアプリ起動（白LED点滅）を実証します。
