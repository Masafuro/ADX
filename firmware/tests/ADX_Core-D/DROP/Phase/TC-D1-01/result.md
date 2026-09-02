<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# DROP-Bus TC-D1-01 実機検証レポート (Verification Report)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）における **DROP-Bus TC-D1-01（一体型フレーム生成 ＆ 通常UART生バイト列受信ダンプテスト）** の実機検証結果を記録した公式レポートです。

---

## 1. 検証環境

* **テスト日時:** 2026-09-02
* **対象ボード:** ADX Core-D 初版基板 $\times$ 2台
* **搭載MCU:** Microchip ATtiny1616 (内蔵 20MHz/16MHz RC発振器)
* **RS-485 トランシーバー:** SP485EEN (DE: PA4, /RE: PA7)
* **通信速度:** 9600 bps
* **テストスケッチ:** [`fixed.ino`](./fixed.ino) / [`tc-d1-01_raw_frame_dump.ino`](./tc-d1-01_raw_frame_dump.ino)
* **デバッグモニタ:** SoftwareSerial (PB5:RX, PB4:TX / 9600 bps)

---

## 2. 実機実行ログ

### Node 1（送信機側）
```text
16:59:54.7 > [TX] Frame Sent.
16:59:55.7 > [TX] Frame Sent.
16:59:56.7 > [TX] Frame Sent.
16:59:57.7 > [TX] Frame Sent.
16:59:58.7 > [TX] Frame Sent.
16:59:59.7 > [TX] Frame Sent.
```

### Node 2（受信機側）
```text
17:00:07.9 > 
17:00:07.9 > [RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0xCC 0x2A [Node 2] Heartbeat... PC Connection is OK.
17:00:08.9 > 
17:00:08.9 > [RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0xCC 0x2A 
17:00:09.9 > [RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0xCC 0x2A 
17:00:11.0 > [RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0xCC 0x2A [Node 2] Heartbeat... PC Connection is OK.
17:00:12.0 > 
17:00:12.0 > [RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0xCC 0x2A 
17:00:13.0 > [RX DUMP] 0x00(BREAK) 0x55 0x04 0x02 0x01 0x11 0x22 0x33 0x44 0xCC 0x2A 
```

---

## 3. 受信バイト列の詳細照合

送信データ列に対する受信データのバイト単位の照合結果です。

| フィールド | 送信設定値 | 受信データ | 判定 | 備考 |
| :--- | :---: | :---: | :---: | :--- |
| **Break Field** | 14 Tbit LOW | `0x00(BREAK)` | **PASS** | `USART_FERR_bm`（Framing Error）として正常検知 |
| **Sync Byte** | `0x55` | `0x55` | **PASS** | LINAUTO 計測用交番パターンを正確に受信 |
| **LEN** | `0x04` | `0x04` | **PASS** | ペイロード長（4バイト）が一致 |
| **TARGET_ID** | `0x02` | `0x02` | **PASS** | 宛先バトンIDが一致 |
| **SENDER_ID** | `0x01` | `0x01` | **PASS** | 送信元バトンIDが一致 |
| **PAYLOAD[0..3]** | `0x11 0x22 0x33 0x44` | `0x11 0x22 0x33 0x44` | **PASS** | 4バイトのペイロードが完全一致 |
| **CRC-16 High** | `0xCC` | `0xCC` | **PASS** | 多項式 `0x1021` による計算値と完全一致 |
| **CRC-16 Low** | `0x2A` | `0x2A` | **PASS** | 多項式 `0x1021` による計算値と完全一致 |

---

## 4. 確立された実装ノウハウと技術的成果

1. **SoftwareSerial 干渉の完全排除（配列バッファリング ＆ 50ms無音一括ダンプ）**
   高速なハードウェア UART 受信ループ内で直接 `pcSerial.print()` を呼ぶことをやめ、`rxBuf[]` への格納のみに専念。無音期間（50ms超過）を検出してから一括出力することで、バッファオーバーランや受信取りこぼしを完全に解消。
2. **PA2 (RX) ピンのプルアップ（`PORT_PULLUPEN_bm`）有効化**
   バス解放時（Hi-Z）における不定電位・ノイズ誤検出をハードウェア的に防止。
3. **`USART0.CTRLA = 0` による割り込み完全無効化**
   Arduino コアの ISR 横取りを排除し、完全なポーリング制御を確立。
4. **送信側での `Serial.flush()` の徹底**
   末尾の CRC-16（`0x2A`）がバス上に送出完了するまで確実に待機してから `DE=0` に戻すことで、パケット破損を防止。

---

## 5. 総合判定

全項目において仕様通りのバイト列が 1.0 秒周期で安定して連続受信されており、DROP-Bus の一体型フレーム（Break 〜 CRC-16）の物理送受信が実証されました。

**判定: PASS（完全合格）**
