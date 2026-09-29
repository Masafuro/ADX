<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Android Break Probe 改善改修計画書
## Native Half-Baud Atomic Engine ＆ モバイル最適化設計

**文書ID**: PLAN-ADX-AND-REV-001  
**策定日**: 2026-09-29  
**対象モジュール**: `software/sandbox/adx-break-probe-android`  
**実機試験結果**: [`probe_test_result1.md`](./probe_test_result1.md) (SC-01K, Android 9, USB2RS485)  
**上位マイルストーン仕様書**: [`ADX_ANDROID_BREAK_PROBE_MILESTONES.md`](./ADX_ANDROID_BREAK_PROBE_MILESTONES.md)

---

## 1. 改修の背景と目的

### 1.1 第 1 回実機試験（2026-09-29）の総括
* **達成された成果**:
  - Android 9 実機（Galaxy Note8）において、アプリ起動（M1 PASS）および USB-OTG ドングル認識・OS 権限取得（M2 PASS）を完全達成。
  - **P10（Half-Baud 方式）において、スレーブからの応答パケット（`01 14 00 00...`）の受信に成功。Android $\leftrightarrow$ マイコン間の有線物理通信パイプラインが開通していることが実証された。**
* **抽出された課題**:
  1. **通信安定性（課題 A）**:
     - WebSerial のコードをそのまま踏襲した結果、`port.close()` $\rightarrow$ `open(9600)` $\rightarrow$ `close()` $\rightarrow$ `open(19200)` という無理な再オープンが走り、OS USB ドライバと受信スレッドの再初期化オーバーヘッドで通信が不安定（1/3 回成功、20 回テストでタイムアウト）になった。
     - また、直流的な `setBreak(true)` 方式（P01〜P09）は、市販 USB-RS485 ドングルの Auto-DE 回路により DE が落ちて全滅した。
  2. **UI 操作性（課題 B）**:
     - Disconnect ボタンが画面端からはみ出し、スライダー操作時に画面全体が横スクロールして操作しづらい。
     - ドングル抜去時に自動で `DISCONNECTED` 表示に戻らない。

### 1.2 改修目的
**「ATtiny1616 の LIN AUTO による鉄壁の自動クロック校正 ＆ ノイズ自己治癒力」** を最大限に発揮させるため、WebSerial の再オープン方式を完全撤廃し、**Android ネイティブの特権を生かした「Native Half-Baud Atomic Engine（ポート常時オープン ＆ USB コントロール転送による動的ボーレート変更）」** へ改修する。あわせてスマホ縦画面に最適化したレスポンシブ UI を実装する。

---

## 2. コア改修項目（設計詳細）

```mermaid
graph TD
    subgraph "Before (WebSerial Re-open 方式: 不安定)"
        B1["Port Close<br/>(USB切断)"] --> B2["Thread 破棄"]
        B2 --> B3["Port Open(9600)<br/>(再初期化)"]
        B3 --> B4["Write 0x00"]
        B4 --> B5["Port Close<br/>(また切断)"]
        B5 --> B6["Port Open(19200)<br/>(再初期化)"]
        B6 --> B7["Write Frame"]
        B7 -.->|"遅延大・スレッド再開遅れ"| B8["✕ 応答取りこぼし (Timeout)"]
    end

    subgraph "After (Native Half-Baud Atomic Engine: 理想)"
        A1["Port 常時 OPEN (受信スレッド常時待機)"]
        A1 --> A2["setParameters(9600)<br/>[USB コントロール転送 1発: 数µs]"]
        A2 --> A3["write(0x00)<br/>[完璧な 18~20bit LOW パルス射出]"]
        A3 --> A4["SystemClock.sleep(12ms)<br/>[物理 UART 送信完了待ち]"]
        A4 --> A5["setParameters(19200)<br/>[瞬時に 19200bps 復帰]"]
        A5 --> A6["write(Packet: 0x55 + 32B Frame)<br/>[アトミック送信]"]
        A6 -->|"常時スタンバイの受信スレッド"| A7["★ 100% 漏らさず即座にキャッチ (PASS)"]
    end
```

---

### 改修項目 1: Native Half-Baud Atomic Engine の実装 (`UsbSerialBridge.kt`)

ネイティブ層（Kotlin）に、LIN BREAK 生成からフレーム送信までをミリ秒精度でアトミックに実行する専用トランザクション関数を新設する。

#### Kotlin 実装仕様 (`UsbSerialBridge.kt`)
```kotlin
@JavascriptInterface
fun executeNativeHalfBaudTransaction(
    packetHex: String,
    flushWaitMs: Long = 12L,
    delimMs: Long = 2L
): String {
    val port = serialPort ?: return jsonError("Port not open")
    
    return try {
        // 1. 直前の受信ゴミバッファをクリア
        rxBufferLock.withLock { rxQueue.clear() }

        // 2. 9600bps へ瞬時切り替え (ポートは開いたまま)
        port.setParameters(9600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)

        // 3. 0x00 送出 (Auto-DE 回路が確実に反応し、18~20bit時間の綺麗な物理 LOW を射出)
        port.write(byteArrayOf(0x00), 100)

        // 4. 9600bps 1文字送信の物理完了待ち (10 bit / 9600bps = 1.04ms + マージン)
        SystemClock.sleep(flushWaitMs)

        // 5. 19200bps へ瞬時復帰 (USB コントロール転送 1 発)
        port.setParameters(19200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)

        // 6. Delimiter (HIGH レベル維持時間)
        if (delimMs > 0) {
            SystemClock.sleep(delimMs)
        }

        // 7. 本番 33 バイト (0x55 Sync + 32B Frame) をアトミック一括送信
        val packetBytes = hexStringToByteArray(packetHex)
        port.write(packetBytes, 200)

        jsonSuccess()
    } catch (e: Exception) {
        Log.e(TAG, "Native Half-Baud transaction failed", e)
        jsonError(e.message ?: "Transaction error")
    }
}
```

* **メリット**:
  - `port.close()` / `port.open()` が一切発生しないため、OS ドライバの再初期化遅延がゼロ。
  - バックグラウンド受信スレッド（`SerialInputOutputManager`）が常時稼働しているため、マイコンからの 32 バイト返信を先頭 1 バイトも取りこぼさずに受信可能。
  - Python 版ベンチマーク（`ser.baudrate = 9600`）と完全に等価なシーケンスを Android 上で再現。

---

### 改修項目 2: WebSerial Shim の最適化 (`android-serial-shim.js`, `index.html`)

* `index.html` の `executeBr32Transaction` において、`window.AndroidBridge` が存在する場合は、従来の WebSerial 再オープン分岐ではなく、上記の `executeNativeHalfBaudTransaction()` を呼び出すように透過接続する。
* これにより、Web UI 上の「PROBE HALF-BAUD」や「HALF-BAUD SWEEP」、「MULTI-PATTERN EXPLORER」がすべてネイティブの超高速・安定エンジンで実行される。

---

### 改修項目 3: モバイル・レスポンシブ UI の最適化 (`index.html`)

スマートフォン（縦画面: 幅 360px〜420px 前後）での操作性を抜本的に改善する。

1. **ビューポート固定 ＆ 横スクロール禁止**:
   ```html
   <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
   ```
   ```css
   html, body {
     overflow-x: hidden;
     max-width: 100vw;
     padding: 12px 8px; /* モバイル向け余白調整 */
   }
   ```
2. **ヘッダー ＆ ボタン配置のレスポンシブ化**:
   - `header`: 画面幅が狭い場合はタイトルとステータス表示を縦積みまたはコンパクト表示。
   - `btnConnect` / `btnDisconnect`: 幅 100% または均等 50% ずつで配置し、右端へのはみ出しを根絶。
   - スライダーコンポーネント: タッチ操作しやすいようにスライダートラック高さを 8px に拡大、パディングを調整。
3. **USB 物理切断イベントの双方向同期**:
   - ネイティブから `notifyJsStatus("DETACHED")` を受信した際、Web UI 側の `btnDisconnect.click()` 相当の処理を自動トリガーし、画面のステータス表示が確実に `DISCONNECTED`（灰色）に戻るように配線。

---

## 3. 改修マイルストーンと検証計画

| Milestone | 改修作業内容 | 合否判定基準 (Pass Criteria) |
| :---: | :--- | :--- |
| **REV-1** | **UI モバイル最適化** | ・スマホ画面で横スクロール（左右の揺れ）がゼロになること。<br>・Disconnect ボタンがはみ出さず、スライダー操作が軽快に行えること。<br>・ドングル抜去時に自動で `DISCONNECTED` に戻ること。 |
| **REV-2** | **Native Half-Baud Engine 実装** | ・`UsbSerialBridge.kt` に `executeNativeHalfBaudTransaction` を実装。<br>・ポートを閉じることなく、一瞬で 9600 $\rightarrow$ 19200 切り替えが成功すること。 |
| **REV-3** | **実機通信ベンチマーク (M4 Gate 再判定)** | ・「PROBE HALF-BAUD」実行時、**32 バイト応答がタイムアウトなしで 100% 受信されること**。<br>・Hex Dump に `01 14 ...` が表示され、**`CRC: OK`** となること。<br>・20 回連続テストで成功率 **90% 以上** を達成すること。 |

---

## 4. 総括 ＆ 次のアクション

本改修により、WebSerial のブラウザ制約から完全に解放され、**「Python 版と同じ決定論的 LIN BREAK」** が Android 上で実現します。

改修計画にご同意いただけましたら、直ちにコードの改修・APK の再ビルド（REV-1 ＆ REV-2）に着手します。
