<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# WU-1: BR32 32-Byte 固定長フレーム送受信 ＆ エコーバック統計サンドボックス

本サンドボックスは、BR32 プロトコルのコア仕様である **「固定 32 バイトフレーム」の双方向通信** を実機（ADX Core-D）で実証するための実験環境です。

Break + Sync に続けて、PC から 32 バイトの BR32 フレームを送信し、マイコンが自身の **生 SIGROW（10 バイト UID）** を埋め込んで 32 バイトで即時エコーバックします。Flash への書き込みは一切行わないため、何度でも安全に通信路の安定性をテストできます。

---

## 1. WU-1 の検証テーマ

1. **完全 32 バイト固定フレーム送受信**:
   - Master (PC) $\rightarrow$ Slave (Core-D): 32 バイト
   - Slave (Core-D) $\rightarrow$ Master (PC): 32 バイト
2. **CRC-16-CCITT（多項式 0x1021、初期値 0xFFFF）の実機リアルタイム検証**:
   - 往復ともに CRC-16 をハードウェア/ソフトウェアで計算し、1 ビットの誤りも許さない健全性を確認。
3. **マイコン自身の「生 SIGROW（Unique Serial ID 10バイト）」の自己同定**:
   - マイコン内部の Signature Row（`SIGROW_SERNUM0`〜`9`）を読み出し、応答フレームの UID 領域に載せて PC へ返送。
4. **最優先レスポンス設計による「超低 RTT（往復遅延）」の達成**:
   - Soft-UART デバッグ出力を「RS-485 返信後」に回すことで、真の 32B 往復遅延（19,200 bps で約 35〜40ms）を達成。
5. **推奨 200ms 周期（5.0 Hz）での長周期バス安定性テスト**:
   - WU-0 で導き出された「200ms 黄金周期」を適用し、フルフレームでのジッター $\sigma$ とパケットロスゼロを実証。

---

## 2. 構成ファイル一覧

```text
firmware/bootloader/PLAN/BR32/WU/WU1_echo_telemetry/
├── README.md              # 本ドキュメント (使い方 & 実験手順)
├── Makefile               # Ubuntu 側 avr-gcc ビルド用 Makefile
├── wu1_echo.c             # Core-D 側 32B エコーファームウェア (C言語, 1,058 Bytes)
├── wu1_echo.hex           # ビルド済みバイナリ (Flash 書込なし / 安全)
└── wu1_echo_bench.py      # PC 側 Python 32B テストベンチ & 統計アナライザ
```

---

## 3. 実験手順 (Windows 11 ホスト PC)

### Step 1: マイコンへのファームウェア書き込み (COM20 UPDI)

PowerShell で `WU1_echo_telemetry` ディレクトリに移動し、以下を実行します：

```powershell
pymcuprog write -d attiny1616 -t uart -u COM20 -f .\wu1_echo.hex --erase
```

* 書き込み完了後、Core-D の赤色 LED（PB2）が約 1 秒周期で点滅を開始します。
* COM21（9600 bps）に、起動案内とマイコン自身の生 UID が出力されます。

---

### Step 2: 単発プローブ実行でマイコンの生 UID を確認 (`--single`)

```powershell
python .\wu1_echo_bench.py --port COM19 --debug-port COM21 --single
```

**期待される出力例**:
```text
[SEND] 32-Byte Frame (CMD=0x01, SEQ=1):
       HEX: 55 01 01 00 00 00 00 00 00 00 00 00 00 12 34 0e 41 44 58 5f 43 4f 52 45 5f 44 5f 4f 4b 21 xx xx

[RECV] Status: PASS (100% CRC Valid)
       RTT: 38.45 ms
       Raw RX (32B): 55 81 01 1E 20 44 ...
       --------------------------------------------------------
       Slave SIGROW UID : 0x1E 0x20 0x44 0xXX ... (10 Bytes)
       Response CMD     : 0x81 (Echo OK)
       Response SEQ     : 1
       Slave Frame Count: 1
       Payload Len      : 14
       Payload Data     : 41 44 58 5f 43 4f 52 45 5f 44 5f 4f 4b 21 ('ADX_CORE_D_OK!')
       CRC-16           : 0xXXXX (OK)
       --------------------------------------------------------
```

---

### Step 3: 長周期バス安定性テスト (`--stability`)

WU-0 で確定した推奨周期 `200.0 ms` で、1,000 回の 32B フルフレーム連続送受信を実施します：

```powershell
python .\wu1_echo_bench.py --port COM19 --debug-port COM21 --stability --count 1000 --interval 200
```

* 1,000 回中何回成功したか（成功率 %）
* 32B 送受信における真の RTT（平均、最小、最大、ジッター）
* 応答間隔ジッター $\sigma$、ヒストグラム分布

---

### Step 4: 対話メニューモード

オプションなしで起動すると、手動でテストできる対話メニューが表示されます：

```powershell
python .\wu1_echo_bench.py --port COM19 --debug-port COM21
```

* `1`: 単発 32B 送信＆UID 取得
* `S`: 安定性テスト（1,000回）
* `O`: 最適インターバル検査くん（80ms〜400ms スイープ）
* `Q`: 終了
