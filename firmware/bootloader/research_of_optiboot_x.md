# optiboot_x の研究と ADX への適用

## 1. 概要
ADX のメイン MCU である Microchip **ATtiny1616-MNR** 向けに、SpenceKonde 氏の `megaTinyCore` に同梱される極小ブートローダー **`optiboot_x`** のコード精査および適用可能性の調査を実施しました。

- 参照リポジトリ: [megaTinyCore optiboot_x](https://github.com/SpenceKonde/megaTinyCore/tree/0f883addb31b71e8febaf493369ae259065a1e9f/megaavr/bootloaders/optiboot_x)
- 参照コード: [`firmware/sample/optiboot_x/`](../sample/optiboot_x/)

---

## 2. 調査結果サマリー

### ① ハードウェア RS-485 送信自動制御（XDIR）のビルトイン
`optiboot_x.c` には既に `#if defined(RS485) && RS485 > 0` の分岐があり、ATtiny1616 内蔵 USART の RS-485 モード（`CTRLA = 1`）を利用するコードが実装されています。これにより、ソフトウェアでの GPIO トグルや送信完了ポーリングを行わずとも、ハードウェアが自動的に送信時 HIGH / 送信完了後 LOW へ DE ピンを切り替えます。

### ② ADX ハードウェアとのピン完全一致
`pin_defs_x.h` の `UARTTX=A1`（PORTMUX によるオルタネートピン）定義と、ADX Core-D / CORE-S の結線が完全に一致しています。

| 信号 | optiboot_x 定義 (`UARTTX=A1`) | ADX Core-D / CORE-S 結線 | 備考 |
| :--- | :--- | :--- | :--- |
| **TXD** | `PA1` | `PA1/D` (MCU Pin 20 $\rightarrow$ SP485 Pin 4 DI) | 送信データ |
| **RXD** | `PA2` | `PA2` (MCU Pin 1 $\rightarrow$ SP485 Pin 1 RO) | 受信データ |
| **DE** | `PA4` | `PA4/DE` (MCU Pin 5 $\rightarrow$ SP485 Pin 3 DE) | ハードウェア XDIR 自動制御 |
| **/RE** | 未定義（GPIO） | `PA7/RE` (MCU Pin 8 $\rightarrow$ SP485 Pin 2 /RE) | 初期化時に LOW 出力固定が必要 |

### ③ メモリ配置と保護
- ブートローダー配置: `0x0000` 〜 `0x01FF`（512バイト）
- ヒューズ保護: `BOOTEND = 0x02` (2 × 256B = 512B) により、自己書き換え破壊をハードウェア防止
- アプリケーション配置: `0x0200` 〜（残余 Flash ~15.5KB）
- タイムアウト: ウォッチドッグタイマ（WDT）による約 1 秒の自動タイムアウト後、アプリへ即時ジャンプ

---

## 3. 次の開発ステップ

先行実機が存在する **ADX Core-D** をターゲットとし、まずは「1対1 の RS-485 ブートローダー」の完全動作を目指します。

詳細な開発計画およびマイルストーンについては、以下の開発計画書を参照してください：

👉 **[ADX Core-D 用 1-to-1 RS-485 ブートローダー開発計画書](./OneToOne_RS485/development_plan.md)**