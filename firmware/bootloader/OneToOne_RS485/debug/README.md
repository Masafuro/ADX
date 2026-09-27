# ADX Core-D RS-485 ブートローダー プロトコル詳細診断環境

## 1. 概要と問題提起
ADX Core-D（ATtiny1616-MNR）に対する 1-to-1 RS-485 ブートローダー（Optiboot_x / STK500v1 互換）において、複数ページ（11ページ: 676バイト）の書き込みを実施した際、以下の現象が発生しています：

```text
[15:25:35] [STAGE 3: PASS] Entered programming mode successfully.
[15:25:35] [STAGE 4: WRITE] Programming 11 page(s) (64 bytes/page)...
[15:25:35] [STAGE 4: PASS] All pages programmed to flash successfully.
[15:25:35] [STAGE 5: VERIFY] Verifying 11 page(s) against HEX binary...
[15:25:37] [ERROR] Stage 5 失敗: ページ 0x440 の読み出しに失敗しました。
```

テスト①（白色点滅: 2ページ / 104B）やテスト②（赤白点滅: 2ページ / 120B）では成功する一方、11ページ（0x0200〜0x04A3）のスケッチにおいて、**10ページ目にあたる `0x0440` の読み出しでタイムアウト（Core-D からの応答途絶）** が発生しています。

本ディレクトリは、この問題をあてずっぽうの推測ではなく、**ミリ秒単位のパケット追跡・Raw HEX ダンプ・単体コマンド分離テスト** を通じて精密に特定・解決するための専用調査環境です。

---

## 2. 考えられる仮説一覧

| No | 仮説 | 検証手法 |
|---|---|---|
| **H1** | **`STK_LOAD_ADDRESS` の失敗**<br>Stage 5 の 0x0440 読み出し時、直前の `STK_LOAD_ADDRESS`（0x55 0x40 0x04 0x20）で応答が返っておらず、次の `STK_READ_PAGE` が同期ズレを起こしている。 | `debug_flasher.py` または Web Flasher の詳細HEXログで `STK_LOAD_ADDRESS` の応答を確認。 |
| **H2** | **RS-485 半二重のバス切り替え衝突（ターンアラウンドタイム）**<br>Core-D が `STK_LOAD_ADDRESS` の `0x14 0x10` を返した後、ホストが `STK_READ_PAGE` を送信するタイミングが早すぎて、Core-D が受信イネーブルに戻っていない。 | パケット間のディレイ（Quiet time）を 10ms → 30ms に広げて検証。 |
| **H3** | **メモリマップド PROGMEM アドレスの境界問題**<br>`optiboot_x.c` 内で `address.word += MAPPED_PROGMEM_START;`（0x8000加算）を行っているが、`0x0440` 周辺で特定のポインタ操作またはレジスタ破損が発生している。 | `debug_flasher.py --read-only --single-page 0x0440` で単体読み出しテスト。 |
| **H4** | **Stage 4 書き込み自体の不完全性**<br>Stage 4 の書き込みが完了したように見えて、実は 0x0440 の書き込み時に NVM コントローラがビジーのままハング、あるいは WDT が依然として干渉している。 | 単一ページ書き込み＆即時ベリファイテスト（`test_page_write_and_read`）。 |

---

## 3. 診断ツールの使い方

### ツール①: Python プロトコル診断スクリプト (`debug_flasher.py`)
ホスト PC のターミナル（PowerShell / Bash）から直接 COM ポートを指定して実行し、全送受信パケットの Raw HEX と RTT を完全記録します。

```bash
# 準備: pyserial が必要です
pip install pyserial

# 1. 0x0440 単一ページの読み出しテスト（書き込みを行わず読み出しのみ）
python debug_flasher.py --port COM19 --single-page 0x0440

# 2. 全 11 ページの読み出し専用監査（0x0200 から 11 ページを順に読み出し）
python debug_flasher.py --port COM19 --read-only --start 0x0200 --pages 11

# 3. 1ページごとの書き込み＆即時ベリファイ（どのページで止まるかを完全特定）
python debug_flasher.py --port COM19 --hex ../releases/test_rs485_serial.hex
```

### ツール②: Web Flasher の詳細 Raw パケットログ（GUI）
ブラウザ（GitHub Pages / ローカル HTML）のコンソールヘッダに **「詳細Rawパケットログ (HEX)」** チェックボックスを追加しました。
これを有効にすると、以下のようにすべての送信 `[TX]` と受信 `[RX]` が 16 進数で表示されます：

```text
[TX] 55 40 04 20 (STK_LOAD_ADDRESS 0x0440)
[RX] 14 10 (RTT: 4ms)
[TX] 74 00 40 46 20 (STK_READ_PAGE 64B)
[RX] 14 [64 bytes data...] 10 (RTT: 12ms)
```

これにより、ブラウザ上でも **「どのバイトで Core-D が黙り込んだのか」** を正確に可視化できます。
