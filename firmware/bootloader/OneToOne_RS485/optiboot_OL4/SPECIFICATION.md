<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Optiboot_OL4 通信プロトコル仕様書 (Specification)

本ドキュメントは、**ADX Core-D**（Microchip ATtiny1616-MNR）向け 1-to-1 LN-485 ブートローダー **Optiboot_OL4** の通信仕様、フレームフォーマット、シーケンス、およびメモリマップを定義します。

---

## 1. システム全体構成 & メモリマップ

### 1.1 メモリマップ (`BOOTEND = 0x04`)
ATtiny1616 の 16KB Flash メモリ（0x0000 〜 0x3FFF）を以下のように分割します。

```text
+-----------------------+ 0x0000
|   Optiboot_OL4        | 1024 Bytes (1 KB)
|   (BOOT セクション)   | FUSE.BOOTEND = 0x04
+-----------------------+ 0x0400
|                       |
|   ユーザーアプリ      | 15,360 Bytes (15 KB / 93.75%)
|   (APP セクション)    |
|                       |
+-----------------------+ 0x3FFF
```

* リセット直後は MCU ハードウェアにより `0x0000`（Optiboot_OL4）から起動します。
* ブートローダー終了時、またはタイムアウト時は `0x0400`（ユーザーアプリ先頭）へジャンプします。

### 1.2 ハードウェアピンアサイン

| ピン | 機能 | レジスタ設定 | 動作モード |
| :--- | :--- | :--- | :--- |
| **PA1** | USART0 TXD | `PORTMUX.CTRLB = PORTMUX_USART0_ALTERNATE_gc;`<br>`VPORTA.DIR \|= (1 << 1);` | 通常 High（アイドル） |
| **PA2** | USART0 RXD | 同上 / `VPORTA.DIR &= ~(1 << 2);` | LINAUTO 自動ボーレート検出 |
| **PA3** | RS-485 DE | `VPORTA.DIR \|= (1 << 3);` | Active HIGH（送信時のみ 1） |
| **PA7** | RS-485 /RE | `VPORTA.DIR \|= (1 << 7);` | Active LOW（受信待機時 0, 送信時 1 で自爆遮断） |
| **PB2** | 赤色 LED | `VPORTB.DIR \|= (1 << 2);` | Active HIGH（待機時 2Hz 点滅, 通信時消灯/同期） |

---

## 2. 物理層 & 通信パラメータ

* **通信方式**: 半二重差動通信（RS-485, SP485EEN トランシーバー）
* **基本ボーレート**: **115200 bps**（8N1）
* **自動校正**: ATtiny1616 内蔵 `LINAUTO` エンジンによる Sync（`0x55`）キャリブレーション
* **レスポンススペース（Response Space）**: **50 µs 〜 100 µs**（ターンアラウンド待機）
* **ブレーク信号（Break）**: **14 Tbit 以上のドミナント（LOW）** ＋ **2 Tbit デリミタ（HIGH）**

---

## 3. ホスト側 LIN Break 生成方式（Baud-Rate Trick）

標準的な USB-RS485 シリアルドングル（CH340, FTDI, CP2102 等）の「Auto-Direction 回路」を正常に駆動しつつハードウェア Break を生成するため、**ボーレート一時変更手法（Baud-Rate Trick）** を採用します。

```text
[Host TX シーケンス]
  1. ボーレートを 57600 bps に切り替え
  2. 0x00 を 1 バイト送信 (スタート 0 + データ 00000000 = 計 9 bit LOW)
     --> 115200 bps 側の Core-D から見ると【18 Tbit の連続 LOW (Break)】として観測される！
  3. 送信完了 (t_wait = 180µs)
  4. ボーレートを 115200 bps に復帰
  5. 0x55 (Sync キャラクタ) を送信
```

これにより、OS 固有の API やドングルの回路特性に依存せず、**100% 確実に RS-485 バス上へ規格適合 Break が出力**されます。

---

## 4. フレームフォーマット & PID 定義

すべての通信は、ホスト（Master Broker）が送出する **LIN ヘッダ** から始まります。

### 4.1 LIN ヘッダ構成 (Host -> Bus)
```text
+-------------------+-----------------+---------------+
| Break (18 Tbit L) | Sync (0x55)     | PID (1 Byte)  |
+-------------------+-----------------+---------------+
```
* **PID (Protected Identifier)**: 6ビットのコマンド ID（ID0〜ID5）＋ 2ビットのパリティ（P0, P1）
  * $P_0 = ID_0 \oplus ID_1 \oplus ID_2 \oplus ID_4$
  * $P_1 = \neg (ID_1 \oplus ID_3 \oplus ID_4 \oplus ID_5)$

### 4.2 コマンド PID 定義表

| PID (Hex) | 6bit ID | コマンド名 | 通信パターン | 説明 |
| :---: | :---: | :--- | :---: | :--- |
| **`0x80`** | `0x00` | `CMD_PING` | Master-Pub $\rightarrow$ Slave-Pub | 生存確認・状態取得（ACK 応答） |
| **`0xC1`** | `0x01` | `CMD_GET_INFO` | Master-Header $\rightarrow$ Slave-Pub | デバイス ID・シグネチャ・バージョン取得 |
| **`0x42`** | `0x02` | `CMD_SET_ADDR` | Master-Pub $\rightarrow$ Slave-Pub | Flash アドレス設定（2バイト Target Addr） |
| **`0x03`** | `0x03` | `CMD_WRITE_PAGE` | Master-Pub $\rightarrow$ Slave-Pub | 64B Flash ページ書き込み（データ + CRC16） |
| **`0xC4`** | `0x04` | `CMD_READ_PAGE` | Master-Header $\rightarrow$ Slave-Pub | 64B Flash ページ読み出し要求 |
| **`0x85`** | `0x05` | `CMD_REBOOT` | Master-Pub (No Resp) | アプリケーション（0x0400）へ起動ジャンプ |

---

## 5. ペイロード構造 & CRC16

### 5.1 ペイロードフレーム
* **Master-Publish ペイロード**:
  ```text
  [Length (1B)] + [Payload (N Bytes)] + [CRC16_H] + [CRC16_L]
  ```
* **Slave-Publish 応答ペイロード**:
  ```text
  [Status (1B)] + [Length (1B)] + [Payload (N Bytes)] + [CRC16_H] + [CRC16_L]
  ```
  * `Status`:
    * `0x00`: OK
    * `0x01`: CRC_ERROR
    * `0x02`: ADDR_ERROR
    * `0x03`: FLASH_ERROR

### 5.2 CRC-16-CCITT 仕様
* 多項式: $x^{16} + x^{12} + x^5 + 1$ (`0x1021`)
* 初期値: `0xFFFF`
* 反転: なし

---

## 6. 代表的トランザクション・シーケンス

### 6.1 PING (生存確認・コールドスタート脱出)
```text
Host                                                Core-D
 │                                                    │
 ├──[Break] + [0x55] + [PID 0x80] ───────────────────>│ (LINAUTO 同期 & WFB 解除)
 │                                                    │ (50µs レスポンススペース)
 │<── [Status: 0x00] + [Len: 0x00] + [CRC16] ────────┤ (DE=1 送信後、TXCIF 待機して DE=0)
 │                                                    │
```

### 6.2 WRITE_PAGE (64バイト書き込み)
```text
Host                                                Core-D
 │                                                    │
 ├──[Break] + [0x55] + [PID 0x03] ───────────────────>│
 ├──[Len: 64] + [Data: 64B] + [CRC16] ───────────────>│ (受信完了 & CRC 検証)
 │                                                    │ (バス解放 DE=0 のまま Flash Erase & Write)
 │                                                    │ (書き込み完了後、50µs 待機)
 │<── [Status: 0x00] + [Len: 0x00] + [CRC16] ────────┤
 │                                                    │
```

### 6.3 READ_PAGE (64バイト読み出し)
```text
Host                                                Core-D
 │                                                    │
 ├──[Break] + [0x55] + [PID 0xC4] ───────────────────>│ (ヘッダのみ送信)
 │                                                    │ (50µs レスポンススペース)
 │<── [Status: 0x00] + [Len: 64] + [64B] + [CRC16] ──┤ (DE=1 送信後、TXCIF 待機して DE=0)
 │                                                    │
```

---

## 7. スレーブ（Core-D）の堅牢性・自律復帰設計

1. **`WFB` (Wait For Break) 常時アーム**:
   * アイドル時および通信終了後は常に `USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;` を代入。
   * Break がない限り一切の受信 FIFO は動作せず、バス上のノイズを完全に無視。
2. **ウォッチドッグ（WDT）によるフェイルセーフ**:
   * コールドスタート時は 8 秒 WDT を起動。ホストからの通信がない場合は自動的に `0x0400` へジャンプ。
   * 通信確立後は WDT を安全値（またはリフレッシュ）で管理し、異常時は即座にクリーンリセット。
3. **自爆エコーの完全物理遮断**:
   * 送信時は `DE=1, /RE=1` でローカルレシーバを物理遮断。
   * 送信完了（`TXCIF`）後に `DE=0, /RE=0` に復帰し、RX FIFO のゴミを掃討。
