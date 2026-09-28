<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# WU-0: PC 側 RS-485 BREAK 生成実験ベンチ (PC Break Generation Sandbox)

本サンドボックスは、PC（Windows 11 / `pyserial` / 市販 USB-RS485 アダプタ）から送出される **LIN BREAK 信号（最低 13 ビット時間の物理 LOW パルス）の生成手法** を比較・検証するための実験環境です。

マイコン（Core-D）のハードウェアが 100% 確実に Break を検知できる「最もブレない Break 生成テクニック」を特定します。

---

## 1. 比較・実験する 3 大 Break 手法

| 手法名 | 仕組み | メリット | 潜在リスク |
| :--- | :--- | :--- | :--- |
| **Method 1: Half-Baud Trick**<br>(半速ボーレート切替) | 19,200 bps の半速（9,600 bps）に切り替えて `0x00` を送信 $\rightarrow$ 19,200 bps に復帰。通常速度換算で **約 18〜20 ビット時間の綺麗な LOW パルス** を生成。 | 純粋な UART ハードウェア波形が出るためパルス幅が極めて安定。 | Windows ドライバのボーレート切り替え時に微小グリッチが出る場合がある。 |
| **Method 2: `send_break()` API**<br>(OS ハードウェア Break) | Windows API (`SetCommBreak` / `ClearCommBreak`) で TX ラインを直接 LOW 駆動。 | コードが最もシンプル（1行）。ボーレート変更なし。 | Windows のタイマー精度（1ms〜15ms）に依存するため、パルス幅にジッターが生じやすい。 |
| **Method 3: ゼロデータ連打方式**<br>(同一ボーレート 0x00 送出) | ボーレートを変えずに通常速度のまま `0x00` を送出。 | 切り替えオーバーヘッドがゼロ。 | 単一の `0x00` は 9 ビット LOW なので 13 ビット要件に満たない（マイコン側の判定条件次第）。 |

---

## 2. 構成ファイル一覧

```text
firmware/bootloader/PLAN/BR32/WU/WU0_break_generator/
├── README.md              # 本ドキュメント (使い方 & 実験手順)
├── Makefile               # Ubuntu 側 avr-gcc ビルド用 Makefile
├── wu0_probe.c            # Core-D 側 Break 観測プローブファームウェア (C言語)
├── wu0_probe.hex          # ビルド済みバイナリ (880 Bytes / Flash書込なし)
└── wu0_break_bench.py     # PC 側 Python ベンチマーク ＆ インタラクティブツール
```

---

## 3. 実験・遊び方の手順 (Windows 11 ホスト PC)

### Step 1: マイコン側プローブの書き込み (COM20 UPDI)
PowerShell で以下を実行して、`wu0_probe.hex` を Core-D に書き込みます：

```powershell
pymcuprog write -d attiny1616 -t uart -u COM20 -f .\wu0_probe.hex --erase
```
* 書き込み完了後、Core-D の赤色 LED（PB2）が約 1 秒周期でゆっくり点滅を開始します。
* COM21（9600 bps）を開いておくと、起動案内ログが出力されます。

---

### Step 2: PC 側ベンチマークスクリプトの実行 (COM19 / COM21)

```powershell
python .\wu0_break_bench.py --port COM19 --debug-port COM21
```

コンソールに以下の対話メニューが表示されます：

```text
=======================================================================
[DEBUG-MON] Listening on COM21 @ 9600 bps (Soft-UART PB4)
[INIT] Opening RS-485 Serial Port COM19 @ 19200 bps...
[INIT] Connected successfully to COM19.

--- [WU-0 Interactive Control Menu] ---
  [1] Send Break via Method 1 (Half-Baud Trick)
  [2] Send Break via Method 2 (pyserial send_break API)
  [3] Send Break via Method 3 (Normal Baud 0x00 Data)
  [B] Run Automated Benchmark (30 iterations each)
  [Q] Exit
---------------------------------------
> 
```

---

### Step 3: 手動インタラクティブ遊び

* **`1` を押して Enter**:
  - Method 1（Half-Baud）で Break 送出。
  - マイコンの **赤色 LED（PB2）がチカッとトグル点滅**！
  - 画面に `|-> [COM21] [BREAK #1 OK] AutoBAUD=0x02B5 Byte=0x11` が流れ、RTT（約 10〜15ms）と応答バイトが表示されます。
* **`2` を押して Enter**:
  - Method 2（`send_break()`）で Break 送出。
  - 同様に赤色 LED が反応するか観察。
* **`3` を押して Enter**:
  - Method 3（Normal 0x00）で Break 送出。
  - 13 ビット要件を満たさずに無視されるか、あるいは拾えるかを観察。

---

### Step 4: 自動ベンチマーク（30連打比較テスト）

* **`B` を押して Enter**:
  - Method 1 〜 3 をそれぞれ 30 回連続で送出。
  - 最終結果として、以下のような比較表が出力されます：

```text
============================================================================
                       BENCHMARK COMPARISON REPORT
============================================================================
 Method                       | Success   | Rate    | Avg RTT   | Jitter σ 
----------------------------------------------------------------------------
 Method 1 (Half-Baud Trick)   | 30/30     | 100.0%  |  60.60ms  |   0.57ms 
 Method 2 (send_break API)    |  0/30     |   0.0%  |    -      |    -     
 Method 3 (Normal 0x00 Byte)  |  0/30     |   0.0%  |    -      |    -     
============================================================================

>>> RECOMMENDED BREAK GENERATOR: Method 1 (Half-Baud Trick) <<<
```

---

### Step 5: 長周期バス安定性・ジッター統計テスト (`--stability`)

1,000 回の定周期 Break プローブを送信し、応答間隔のばらつき（標準偏差 $\sigma$、ヒストグラム）を統計評価します：

```powershell
python .\wu0_break_bench.py --port COM19 --debug-port COM21 --stability --count 1000 --interval 200
```

---

### Step 6: 最適インターバル検査くん（`--optimize` / `--sweep`）

周期（80ms〜400ms）を自動スイープし、最もジッターが小さく安定する「黄金の動作周期」を全自動で探索・可視化します：

```powershell
python .\wu0_break_bench.py --port COM19 --debug-port COM21 --optimize
```

* `--sweep-start`, `--sweep-end`, `--sweep-step`, `--sweep-count` で探索範囲やプローブ数をカスタマイズ可能です。
* 完了後、バスタブ曲線（U字型ジッター特性）のアスキーチャートと、推奨される最適周期が表示されます。

