<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: MIT
-->

# Milestone 2 トラブルシューティング & 調査計画書 (debug.md)

## 1. 発生現象
- **現象**: Milestone 2 において、SerialUPDI 経由でのブートローダー書き込みおよび `BOOTEND=0x02` 設定は正常終了したが、Core-D の電源再投入時に **赤色 LED（PB2）の点滅が目視確認できない**。

---

## 2. 原因調査と分析（3つの有力因果関係）

コード精査および Core-D の回路ネットリスト解析を行った結果、以下の **3つの決定的な原因** が特定されました。

### ① 【主因 1】LED ピンの出力設定（DIR）が完全にスキップされていた
`optiboot_x.c` のソースコード（L429-432）を確認したところ、LED の出力ピン設定は以下のプリプロセッサ条件で囲まれていました：

```c
#if (LED_START_FLASHES > 0) || defined(LED_DATA_FLASH) || defined(LED_START_ON)
  /* Set LED pin as output */
  LED_PORT.DIR |= LED;
#endif
```

- **問題点**:
  Makefile のコンパイルフラグには `-DLED=B2` と `-DBLINK_WAIT_LED` のみを指定しており、上記 3 つのマクロのいずれも定義されていませんでした。
- **結果**:
  **`LED_PORT.DIR |= LED;`（出力モード化）が一度も実行されず、PB2 はマイコン起動時のデフォルトである「入力ピン（Hi-Z）」のまま** でした。入力ピンに対してトグル操作（`LED_PORT.IN |= LED;`）を行っても、外部ピンの電圧は変化せず、LED が点灯・点滅することはありませんでした。

---

### ② 【主因 2】USB 抜き差し時のリセット要因（BOD）によるブートローダースキップ
`optiboot_x.c` のエントリー条件判定（L384-388）を確認：

```c
#if !defined(ENTRYCOND_REQUIRE)
  #define ENTRYCOND_REQUIRE 0x35
#endif
if ((ch & 0x08) || !(ch & ENTRYCOND_REQUIRE)) {
  // Start the app.
  watchdogConfig(WDT_PERIOD_OFF_gc);
  __asm__ __volatile__("jmp 0x0200\n\t");
}
```

- **問題点**:
  `ENTRYCOND_REQUIRE` のデフォルト値 `0x35` は、`PORF(0x01) | EXTRF(0x04) | SWRF(0x10) | UPDIRF(0x20)` です。
  **`BODF`（Brown-out Reset = `0x02`）が含まれていません**。
- **結果**:
  Core-D の Type-C ケーブルを抜き差しした際、電源ラインのコンデンサ残電によって電圧が緩やかに低下し、チップが **BOD（電圧低下リセット）** で再起動すると、`ch & 0x35` が `0`（偽）となり、**ブートローダーを実行せずに即座にアプリ領域（0x0200）へジャンプして終了** してしまいます。

---

### ③ 【要因 3】点滅カウンタ周期の調整
待機ループ（`getch()`）内のカウンタ `loop`（16bit = 65,536 回）および `loop_h >= 8`（合計 524,288 ループ）は、内蔵オシレータの分周比によっては点滅周期が長くなりすぎ、最初の数秒間点灯しないように見える可能性がありました。

---

## 3. 是正・改善計画

以下の是正措置を `optiboot_x.c` および `Makefile` に適用します：

1. **LED ポートの確実な出力初期化**:
   ブートローダー開始時に、マクロ条件に関わらず `LED_PORT.DIR |= LED;` を確実に実行し、初期状態を消灯（`LED_PORT.OUT &= ~LED;`）に設定する。
2. **起動直後の明示的スタートフラッシュ（`LED_START_FLASHES=3`）**:
   電源投入直後にまず 3 回高速点滅（ピピッ、ピピッ）させて「ブートローダーが確実に起動したこと」を目視確認可能にする。
3. **BOD リセットをエントリー条件に許容**:
   `-DENTRYCOND_REQUIRE=0x37`（PORF 0x01 + BODF 0x02 + EXTRF 0x04 + SWRF 0x10 + UPDIRF 0x20）を設定し、USB ケーブルの抜き差しでも確実にブートローダーを起動させる。
4. **待機中ハートビート点滅の周期最適化**:
   待機中の点滅周期を約 200ms（約 2.5Hz）の心地よい点滅速度に最適化。

---

## 4. 検証ステップ & 実施結果
- [x] 修正コードを適用し、`avr-gcc` で再ビルド。
- [x] サイズが **512 バイト以下（<= 512B）** を維持していることを確認。
  - **実績**: `.text` = 472B, `.version` = 2B $\rightarrow$ **合計 474 Bytes（38 バイトの余裕）でクリア！**
- [x] 新バイナリ [`optiboot_core_d_rs485.hex`](../../releases/optiboot_core_d_rs485.hex) を出力。
- [ ] Windows 11 PC から新バイナリを Core-D に書き込み。
- [ ] USB 抜き差しで、赤色 LED（PB2）の約 8〜10 秒待機点滅を目視確認。
