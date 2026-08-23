<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D オシレーター動作テスト結果 (osc_test)

## 1. 実行日時
- 2026/08/23(Sun) 22:10

## 2. 実施条件・ハードウェア状態
- **対象ボード**: ADX Core-D (初版基板)
- **対象ピン**: PA3 (EXTCLK)
- **ジャンパ設定**: 1×3P ジャンパピン（H3）を `osc` 側にショート（12MHz 水晶発振器 TFOM12M4RHKCNT2T の出力を PA3 に供給）

## 3. 実行ログ (Test Log)
```txt
--- PA3 オシレーター動作テスト開始 ---
クロック検出ステータス (EXTS): 検出成功 (STABLE)
RTCカウンタ遷移: 36876 -> 27573 -> 18238 -> 8897 -> 65102
発振確認結果: OK (パルスをカウントしています)

判定: PA3のオシレーターは正常に動作しています！ <<<

```