<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# WU-5-1 検証レポート: WebSerial API による RS-485 BREAK 挙動 ＆ IDENTIFY レビュー

**実施日**: 2026-09-28  
**対象ハードウェア**: ADX Core-D (Microchip ATtiny1616-MNR, SP485EEN)  
**配線環境**: 50cm (25cm+25cm WAGO 差込形コネクタ中継・3線 A/B/GND・両端 120Ω 終端抵抗 ON)  
**通信ポート**: COM19 (RS-485 @ 19,200 bps, 8N1)  
**ホスト実行環境**: Google Chrome / Microsoft Edge (WebSerial API)  
**検証アプリ**: [`firmware/bootloader/PLAN/BR32/WU/WU5_1_web_break_probe/index.html`](../../WU/WU5_1_web_break_probe/index.html)  
**ターゲットファームウェア**: [`firmware/bootloader/PLAN/BR32/M4_1/m4_1_flash.hex`](../../M4_1/m4_1_flash.hex) (1,022 Bytes / M4-1 極小ブートローダー)  

---

## 1. 検証目的と合格基準

本検証は、GitHub Pages に展開予定のブラウザ版 Web Flasher（WU-5）の開発に向け、WebSerial API 経由での物理層 BREAK 信号制御とスレーブ応答の信頼性を先行実証することを目的とする。

### 合格判定基準 (Review Gates)
- [ ] **Gate 1: WebSerial ハードウェア BREAK アーム**
  - `port.setSignals({ break: true / false })` により、スレーブの `LINAUTO` ハードウェアが確実にトリガーされること。
- [ ] **Gate 2: 32B 完全フレーム往復 ＆ 自己同定 (IDENTIFY)**
  - `0x55` (Sync) + 32B `CMD_IDENTIFY` に対し、スレーブから 32B 応答フレームを受信し、CRC-16-CCITT が合致すること。
  - スレーブ UID: `0x30 53 51 46 33 34 29 29 14 21` を正常取得できること。
- [ ] **Gate 3: 連続 50 サイクルベンチマーク (200ms メトロノーム)**
  - 成功率 $\ge 98.0\%$（目標 100.0%）。
  - RTT 平均値が Python 実行時（約 38〜40ms）と同等であり、ブラウザ起因の異常な遅延・ハングアップが発生しないこと。

---

## 2. 実機測定結果 (Live Telemetry Log)

*(※実機ブラウザでの実行後にログを転記)*

```text
(Pending user live run)
```

---

## 3. レビュー所見・技術評価

*(※実機テスト結果を踏まえて分析・記載)*
