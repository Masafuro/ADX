<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# WU-5-1: WebSerial BREAK 挙動検証サンドボックス (WebSerial Break Probe)

本サンドボックスは、WebSerial API を用いたブラウザ版 Web Flasher（GitHub Pages 展開）の基礎技術として、**ブラウザ（Google Chrome / Microsoft Edge）から RS-485 バス経由でハードウェア BREAK 信号を生成し、スレーブの `LINAUTO` アームおよび 32 バイト往復通信を 100% 成立させる手法を確立・実証した** サンドボックス環境です。

**詳細な検証レポート ＆ 物理的真因の解明報告書**: [`WU5_1_REPORT.md`](./WU5_1_REPORT.md)

---

## 1. 検証成果と結論

1. **Win32 SetCommBreak の非同期位相破壊の解明**:
   - WebSerial API の `port.setSignals({ break: true/false })`（Win32 `SetCommBreak`）は、CH342K の内部 UART TX クロック位相を狂わせ、最初の 1〜2 バイトが `0x40 0x20`（ビット鏡像反転）化して 31 バイトに欠損する原因となっていたことを完全立証。
2. **WebSerial Half-Baud Re-open Trick による完全勝利**:
   - `close()` $\rightarrow$ `open(9600)` $\rightarrow$ `write(0x00)` $\rightarrow$ `close()` $\rightarrow$ `open(19200)` $\rightarrow$ `write(0x55 + frame)` により、**UART ハードウェア自体の正規クロック同期で LIN Break を生成**。
   - **20 サイクル連続ストレステストで 20/20 (100.0%) PERFECT MATCH**、および **全 7 条件のタイミングスイープで 21/21 (100.0%) PERFECT MATCH** を達成。

---

## 2. 動作環境・要件

- **対応ブラウザ**: Google Chrome または Microsoft Edge（Chromium 系、WebSerial API サポート環境）
- **公開 URL**: `https://masafuro.github.io/ADX/`（GitHub Pages）
- **ターゲットハードウェア**:
  - ADX Core-D（ATtiny1616-MNR）
  - ファームウェア: [`firmware/wu5_diag.hex`](./firmware/wu5_diag.c) (透明診断スケルトン)
  - RS-485 ポート: `COM19` @ 19,200 bps (8N1)
  - デバッグモニタ: `COM21` @ 9,600 bps (PB4 Soft-UART)

---

## 3. UI ツールと機能一覧

ブラウザ画面（`index.html`）には、以下の検証・診断ツールが搭載されています：

| ボタン名 | 機能・検証内容 |
| :--- | :--- |
| **🎯 Tuned Half-Baud Probe (P10)** | 最適化された Half-Baud Re-open（待機 15ms/12ms）による単発プローブ。 |
| **🏆 Half-Baud 20-Cycle Stress Test** | 250ms インターバルで連続 20 回の Half-Baud トランザクションを実行し、耐久成功率を測定。 |
| **🔬 Half-Baud Timing Sweep (5~30ms)** | 待機時間（5, 8, 12, 15, 20, 25, 30 ms）を各 3 回ずつテストし、最適タイミングを自動診断。 |
| **🧪 Run Multi-Pattern Break Explorer** | 考えられる 10 通りの Break 生成アプローチを自動で網羅実行し、診断レポートを出力。 |
| **📈 Delimiter Sweep (0.5~50ms)** | Delimiter（復帰待機時間）の長さを変えて耐性を検証。 |
| **📊 Pulse Width Sweep (0.7~5ms)** | Break パルス幅の長さを変えて耐性を検証。 |
| **⚡ Standard Probe** | スライダーで指定した Break 幅・Delimiter による単発プローブ。 |
| **⏹ Stop** | 実行中のスイープ・ベンチマークを安全に中断。 |

---

## 4. ファイル構成

- [`index.html`](./index.html): 単一完結型 Web アプリケーション（HTML5, Vanilla CSS, WebSerial API, CRC16 算出エンジン、統計計算）
- [`WU5_1_REPORT.md`](./WU5_1_REPORT.md): **詳細検証レポート（Win32 非同期破壊の証明と Half-Baud トリックの完全勝利）**
- [`wu5_diag_probe.py`](./wu5_diag_probe.py): Python 比較スクリプト（Method 1: Half-Baud vs Method 2: send_break）
- [`firmware/wu5_diag.c`](./firmware/wu5_diag.c): 透明診断ファームウェア ソースコード
- [`firmware/wu5_diag.hex`](./firmware/wu5_diag.hex): コンパイル済みバイナリ（1,074 Bytes）
