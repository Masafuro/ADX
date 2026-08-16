# ADX Core V0 TEST

## 1. Visual inspection

- 顕微鏡による外観検査

## 2. COLD TEST

- テスト項目
  - [200_cold_test.md](200_cold_test.md)
- 必要なもの
  - ADX Core V0 Board
  - テスター
  - 記録用Github

## 3. HOT TEST

- テスト項目
  - [300_hot_test.md](300_hot_test.md)
- 必要なもの
  - ADX Core V0 Board
  - IDC20pinリボンケーブル
  - IDC Breakout Board
  - Analog Discovery 3
  - [USB-UPDI シリアル変換アダプタ](https://www.switch-science.com/products/9833?srsltid=AfmBOorsQxXZl9mzRVWr1nGvxjNZj7mHYrLwM3wYnoYn2lLD-4AgsqZT)
    - ★未発注
    - Adafruit UPDI Friend HVは挙動が怪しいため不可。
    - 12Vパルスリセットは必要になったら考える。
  - RS-485検査用スケッチ
    - [スケッチ仕様](301_RS485_test_sketch_spec.md)
    - [テストスケッチ](302_RS485_test_sketch_01.ino)
  - 記録用Github

