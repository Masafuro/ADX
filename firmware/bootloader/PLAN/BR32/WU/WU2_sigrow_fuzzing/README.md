<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# WU-2: BR32 SIGROW ゲート・チェッカー ＆ UID ファジング実験室 (Fuzzing & Silence Lab)

本サンドボックスは、BR32 プロトコルにおける **マルチドロップ RS-485 バスの要である「SIGROW 宛先照合ゲート」と「鉄壁の沈黙」** を実機（ADX Core-D）で実証するための実験環境です。

複数台のスレーブが同一バスにぶら下がるマルチドロップ環境において、**「自分宛てではないパケットに対して 1 ビットたりとも喋らず完全沈黙を守ること」**、そして **「悪意あるノイズやゴミデータ、他者宛てパケットをどれほど浴びてもデッドロックせず自己治癒すること」** は、バス衝突を防ぎプロトコルの信頼性を担保する絶対条件です。

Flash への書き込みは一切行わないため、何度でも安全・過激にファジング攻撃を仕掛けることができます。

---

## 1. WU-2 の検証テーマと合格基準

1. **自己同定ゲートの受諾（Accept All-Zero Broadcast）**:
   - `TARGET_SIGROW` が「全 10 バイト 0x00」のとき、スレーブは要求を受諾し、自身の生 UID を載せた 32B フレームを返信する。
2. **名指しユニキャストゲートの受諾（Accept Targeted Unicast）**:
   - `TARGET_SIGROW` が「スレーブ自身の UID」と完全一致するとき、スレーブは要求を受諾し、32B フレームを返信する。
3. **鉄壁の沈黙（Complete Bus Silence on Unauthorized Frames）**:
   - 以下の 4 大意地悪パケットに対し、スレーブは **DE=1（送信モード）に一切切り替えず、1 ビットも喋らず完全沈黙（0 バイト送出）** を貫く：
     - **3-A**: 1 ビットだけ改変した UID（境界値テスト）
     - **3-B**: 全 0xFF UID
     - **3-C**: 他人の UID（例: `0xAA BB CC DD EE ...`）
     - **3-D**: ランダムゴミ UID（ノイズ模倣）
4. **耐久ファジング ＆ 自己治癒力（Stress Fuzzing & Instant Self-Healing）**:
   - ランダム UID パケットを 50〜100 回連続で高速連打（200ms 周期）する。
   - スレーブが全弾に対して 100% 沈黙を守り抜くこと。
   - 連打直後に「正しい UID」のパケットを 1 発送出されたとき、**スレーブがハングアップやデッドロックを一切起こさず、一撃で平然と 100% 正常応答を返す（自己治癒）** ことを実証する。
5. **仕様書 3.2 節（正式スレーブ返信フレーム）の完全準拠**:
   - `[0] STATUS` (0x00=OK)
   - `[1] ECHO_SEQ`
   - `[2] RX_COUNT` (32)
   - `[3] DEV_STATE` (0x01=Targeted, 0x00=Broadcast)
   - `[4..13] MY_SIGROW[10]` (生 UID)
   - `[14..15] COUNTER` (受諾回数カウンタ)
   - `[16..29] EXTRA[14]` (エコーバック)
   - `[30..31] CRC16` (XMODEM)

---

## 2. 構成ファイル一覧

```text
firmware/bootloader/PLAN/BR32/WU/WU2_sigrow_fuzzing/
├── README.md              # 本ドキュメント (使い方 & 実験手順)
├── Makefile               # Ubuntu 側 avr-gcc ビルド用 Makefile
├── wu2_fuzz.c             # Core-D 側 ゲート＆沈黙ファームウェア (Flash 書込なし / 1,214 Bytes)
├── wu2_fuzz.hex           # ビルド済みバイナリ
└── wu2_fuzz_bench.py      # PC 側 Python 4大テスト自動実行ベンチ
```

---

## 3. 実験手順 (Windows 11 ホスト PC)

### Step 1: マイコンへのファームウェア書き込み (COM20 UPDI)

PowerShell で `WU2_sigrow_fuzzing` ディレクトリに移動し、以下を実行します：

```powershell
pymcuprog write -d attiny1616 -t uart -u COM20 -f .\wu2_fuzz.hex --erase
```

* 書き込み完了後、Core-D の赤色 LED（PB2）が約 1 秒周期で点滅を開始します（待機ハートビート）。
* COM21（9600 bps）に起動案内が出力されます。

---

### Step 2: 4 大テスト自動ベンチマークの一括実行 (`--suite`) ★推奨★

```powershell
python .\wu2_fuzz_bench.py --port COM19 --debug-port COM21 --suite
```

**実行されるテストの流れ**:
1. **[DISCOVERY]**: 全 0x00 パケットを送出し、接続中の Core-D の生 UID を自動取得。
2. **[TEST 1]**: 全 0x00 自己同定プローブ $\rightarrow$ `[PASS]`（即時応答）
3. **[TEST 2]**: 取得した生 UID 名指しプローブ $\rightarrow$ `[PASS]`（即時応答）
4. **[TEST 3]**: 4 つの意地悪 UID（1ビット反転、全0xFF、他人UID、ゴミUID）を順次送出 $\rightarrow$ **マイコンが完全沈黙を守り、バス上に 0 バイトであることを確認 $\rightarrow$ `[PASS]`**
5. **[TEST 4]**: 50 回のランダム UID ファジング連打 $\rightarrow$ 全 50 回沈黙 $\rightarrow$ 直後に正しい UID を 1 発送出 $\rightarrow$ **一撃で平然と 38ms で復帰（自己治癒実証）$\rightarrow$ `[PASS]`**

---

### Step 3: より過激な 100 連打ファジングテスト (`--fuzz-count 100`)

```powershell
python .\wu2_fuzz_bench.py --port COM19 --debug-port COM21 --suite --fuzz-count 100
```

100 回の猛烈なノイズ攻撃にもスレーブのステートマシンが一切乱れず、100% の沈黙と完全な自己治癒を証明します。

---

### Step 4: 単発プローブによる沈黙の目視観察 (`--single`)

特定の意地悪パケットを単発で送り、COM21 のデバッグ出力と RS-485 の沈黙を観察できます：

```powershell
# 1ビット反転 UID を送信 (スレーブは沈黙し、COM21 に [SILENCE] と表示)
python .\wu2_fuzz_bench.py --port COM19 --debug-port COM21 --single --target bitflip

# 他人 UID を送信 (スレーブは沈黙)
python .\wu2_fuzz_bench.py --port COM19 --debug-port COM21 --single --target fake

# 正しい UID を送信 (スレーブは応答)
python .\wu2_fuzz_bench.py --port COM19 --debug-port COM21 --single --target valid
```
