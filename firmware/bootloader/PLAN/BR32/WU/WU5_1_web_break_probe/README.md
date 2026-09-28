<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# WU-5-1: WebSerial BREAK 挙動検証プローブ (WebSerial Break Probe)

本サンドボックスは、WebSerial API を用いたブラウザ版 Web Flasher（GitHub Pages 展開予定）の基礎技術として、**ブラウザ（Google Chrome / Microsoft Edge）から RS-485 バス経由でハードウェア BREAK 信号を生成し、スレーブの `LINAUTO` アームおよび 32 バイト往復通信が安定して成立するか** を先行検証・レビューするための単一完結型 Web アプリケーションです。

マイコン側は、現在書き込まれている **M4-1（1,022 バイト極小ブートローダー）のままで 100% テスト可能**（追加のファームウェア書き込み不要）です。

---

## 1. 検証テーマと背景

1. **WebSerial API の BREAK 信号制御能力の実証**:
   - WebSerial API では `port.setSignals({ break: true })` および `port.setSignals({ break: false })` により、OS 固有のシリアル Break 制御（Windows では Win32 `SetCommBreak` / `ClearCommBreak` API）を直接操作できます。
   - USB-シリアル変換 IC（CH342K）を介して、ATtiny1616 の `LINAUTO` ハードウェアをアームする十分なパルス幅（標準 2.0ms ≒ 19200bps で約 38 ビット分）がブラウザの JavaScript からジッターなく送出できるかを検証します。
2. **ブラウザ ↔ RS-485 双方向 32B フレーム往復ジッター測定**:
   - `Sync (0x55)` + 32B `CMD_IDENTIFY` を送出し、スレーブからの 32B 応答フレーム受信・CRC-16-CCITT 検証を行います。
   - 200ms メトロノームでの連続 50 サイクルベンチマークにより、成功率（%）、RTT 平均値、ジッター（$\sigma$）をリアルタイムに自動算出して可視化します。

---

## 2. 動作環境・要件

- **対応ブラウザ**: Google Chrome または Microsoft Edge（Chromium 系、WebSerial API サポート環境）
- **実行コンテキスト**: WebSerial API のセキュリティ仕様上、**`http://localhost:*`** または **`https://*`** のセキュアコンテキストである必要があります（`file:///` ではブラウザのセキュリティ設定によりシリアル API がブロックされる場合があります）。
- **ターゲットハードウェア**:
  - ADX Core-D（ATtiny1616-MNR）
  - 現在のファームウェア: **M4-1 極小ブートローダー（`m4_1_flash.hex`）**
  - RS-485 ポート: `COM19` @ 19,200 bps (8N1)
  - スレーブ UID: `0x30 53 51 46 33 34 29 29 14 21`

---

## 3. 起動と実行手順

### Step 1: ローカル HTTP サーバーの起動

Windows PowerShell またはターミナルで本ディレクトリに移動し、Python の標準 HTTP サーバーを起動します。

```powershell
# WU5_1_web_break_probe ディレクトリで実行
cd C:\Users\User\path\to\firmware\bootloader\PLAN\BR32\WU\WU5_1_web_break_probe
python -m http.server 8080
```

### Step 2: ブラウザでアクセス

Google Chrome または Microsoft Edge を開き、以下の URL にアクセスします：

```text
http://localhost:8080/index.html
```

### Step 3: シリアルポート接続

1. 画面上部の **「🔌 Connect Serial Port (COM19)」** ボタンをクリックします。
2. ブラウザのシリアルポート選択ポップアップが表示されるので、**COM19（USB-Enhanced-SERIAL CH342 等）** を選択して「接続」を押します。
3. ヘッダーのステータスインジケーターが **「CONNECTED (19200)」**（エメラルド色）に点灯すれば準備完了です。

### Step 4: 単発プローブ実行 (Single Probe)

1. **「⚡ Probe Slave (Single Break + CMD_IDENTIFY)」** ボタンをクリックします。
2. コンソールに `[TX]`（Break + Sync + IDENTIFY フレーム）および `[RX PASS]` が出力され、スレーブの生 UID が画面にバッジ表示されることを確認します。
   - 期待値 UID: `0x30 53 51 46 33 34 29 29 14 21`
   - 期待値 RTT: 約 `38` 〜 `42` ms

### Step 5: 連続 50 サイクルベンチマーク (200ms Metronome)

1. **「🚀 Run 50-Cycle Benchmark (200ms Metronome)」** ボタンをクリックします。
2. 200ms 周期で自動的に 50 回のプローブが連続実行されます。
3. 統計カードで以下を確認・レビューします：
   - **Total Probes**: 50
   - **Success Rate**: 100.0%
   - **Last RTT**: ~39 ms
   - **Jitter (σ)**: ±1〜3 ms 程度

---

## 4. パラメータ調整サンドボックス機能

UI 上のスライダーにより、リアルタイムに以下の物理層タイミングを可変にして限界耐性をテストできます：

| パラメータ | 調整範囲 | デフォルト | 役割・検証意図 |
| :--- | :---: | :---: | :--- |
| **Break Pulse Width** | 0.5 ms 〜 10.0 ms | **2.0 ms** | 19200bps でのスレーブ LINAUTO 検出最小パルス幅の特定 |
| **Delimiter Gap** | 0.1 ms 〜 5.0 ms | **0.5 ms** | Break 解除から Sync バイト（0x55）送出までのアイドル時間余裕 |

---

## 5. ファイル構成

- [`index.html`](./index.html): 単一完結型 Web アプリケーション（HTML5, Vanilla CSS, WebSerial API, CRC16 算出エンジン、統計計算）
- [`README.md`](./README.md): 本検証プローブの取扱説明書
