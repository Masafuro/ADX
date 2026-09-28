<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BR32 Warm Up (WU) サンドボックス実験環境

本ディレクトリは、BR32 プロトコルおよびブートローダー開発における **「自由な動作模型・実験サンドボックス（WU-0 〜 WU-4）」** のコード資産を管理します。

本番マイルストーン（M0〜M6）の開発中に行き詰まったり挙動が怪しくなった時、**いつでもここに戻って物理層や通信の素の挙動を即座に確認できる安全地帯（ホームグラウンド）** です。Flash 書き込みを行わないため、何度動かしてもマイコンが壊れる心配はありません。

---

## 1. WU サンドボックス一覧

| 番号 | サンドボックス名 | 目的・検証テーマ | フォルダ / スクリプト |
| :---: | :--- | :--- | :--- |
| **WU-0** | **PC 側 BREAK 生成実験ベンチ** | PC（Windows / USB-RS485）から安定して Break を打つ 3 大手法の比較・特定 | [`WU0_break_generator/`](./WU0_break_generator/) |
| **WU-1** | **32B エコー ＆ ハートビート統計** | 周期可変（1s〜100ms）での 32B 送受信・ジッターリアルタイム観察・配線揺らし | [`WU1_echo_telemetry/`](./WU1_echo_telemetry/) |
| **WU-2** | **意地悪 SIGROW ゲート・チェッカー** | 不正 UID やゴミデータに対するスレーブの「鉄壁の沈黙」と自己治癒力の体感 | [`WU2_sigrow_fuzzing/`](./WU2_sigrow_fuzzing/) |
| **WU-3** | **フレーム破壊 ＆ ノイズ耐性実験室** | 10B 欠損・50B 垂れ流し・CRC 破壊に対する時間枠自律脱出（非デッドロック）の実証 | [`WU3_fault_injection/`](./WU3_fault_injection/) |
| **WU-4** | **仮想 SRAM ページペインタ** | Flash 書き込みゼロで、SRAM 上の 64B 配列に 4 チャンク分割蓄積・読出遊び | [`WU4_sram_painter/`](./WU4_sram_painter/) |

---

## 2. 共通ハードウェア設定 (Core-D)

- **RS-485 ポート**: `COM19`（19,200 bps または 115,200 bps）
  - TX: `PA1`, RX: `PA2`, DE: `PA4`, /RE: `PA7`
- **UPDI 書き込みポート**: `COM20`（SerialUPDI）
- **デバッグテレメトリポート**: `COM21`（PB4 TX, 9600 bps Soft-UART）
- **ステータス LED**: `PB2`（赤色 LED）
