
# トラブルシューティングレビュー：Slave AのISFIF問題とステートマシン導入

## 1. 事象の概要
ADX Core-D LN-485 Test Firmware (Phase 5) において、Slave A が定期的に `ISFIF Sync Error!` を出力し、Masterからの要求（ID=0x03, 0x04等）に一切応答しない問題が発生した。
事前の検証によりハードウェア環境は正常であり、また取得したログから「Master と Slave B 間の通信は完璧に成立している」ことが確認できたため、原因は Slave A 単体のファームウェアロジックにあると特定された。

## 2. 根本原因（ISFIF自爆のメカニズム）
RS-485バス上で、他ノード宛て（例：Slave B宛てのID=0x02）の通信が行われている際の「Slave A の受信無視ロジック」に致命的な欠陥があった。

1. **他ノード宛てIDの受信:** Slave A が ID=0x02 を受信し、「自分宛てではない」と正しく判定する。
2. **誤った待機復帰:** その直後、Slave A は直ちに `USART_WFB_bm`（次のブレーク信号待機モード）をセットしてしまう。
3. **ペイロードの誤認:** バス上には Master から Slave B へ向けた「データ（ペイロード）」が継続して流れてくるが、待機モードに入った Slave A のハードウェアは、これを「新しいブレーク信号・同期信号」として読み込んでしまう。
4. **ISFIFの発生:** 当然、ペイロードデータは同期用の `0x55` ではないため、Slave A は「同期フィールド不整合」と判断して `ISFIF` エラーフラグを立ててフリーズ（またはリカバリーを反復）していた。

## 3. 解決策
Slave A の受信ロジックに **ステートマシン（状態遷移）** を導入し、「無視モード」を実装する。

*   **状態の定義:** `STATE_WAIT_HEADER`（ヘッダ待機）と `STATE_IGNORE_PAYLOAD`（ペイロード無視）の2つの状態を設ける。
*   **無視モードの挙動:** 自分に関係のない ID を受信した場合、`WFB` をセットせずに `STATE_IGNORE_PAYLOAD` へ移行し、バス上をペイロードが通り過ぎるのを待つ。
*   **安全な復帰（タイムアウト）:** 無視モード移行から一定時間（20ms）経過後、タイムアウト処理によって安全に `WFB` をセットし、次の正規のヘッダ待機（`STATE_WAIT_HEADER`）へ復帰させる。

## 4. 修正コードの要点

### ステートマシンの定義とタイムアウト処理
```cpp
enum SlaveARxState {
  STATE_WAIT_HEADER,
  STATE_IGNORE_PAYLOAD
};
SlaveARxState slaveAState = STATE_WAIT_HEADER;
uint32_t rxIgnoreStartTime = 0;

void loop() {
  uint32_t now = millis();

  // 無視モードのタイムアウト復帰処理（20msで安全に待機モードへ戻る）
  if (slaveAState == STATE_IGNORE_PAYLOAD && (now - rxIgnoreStartTime > 20)) {
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    slaveAState = STATE_WAIT_HEADER;
  }
  // ... (以降の処理)
}

```

### 無視状態への移行ロジック

```cpp
        // 関連するIDの処理（Type B, Type C）
        if (currentID == TOPIC_TYPE_B_UPTIME) {
          // ... 応答処理 ...
        } else if (currentID == TOPIC_TYPE_C_TRIGGER) {
          // ... 送信処理 ...
        } 
        // 無関係なIDの場合は WFB をセットせず「無視モード」へ移行
        else {
          slaveAState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
        }

```

### 【副次的な修正】送信フラグクリアの不具合防止

OR代入(`|=`)による意図しないフラグクリアを防ぐため、単純代入(`=`)に変更。

```cpp
void slaveTxFlush() {
  while (!(USART0.STATUS & USART_TXCIF_bm)); 
  USART0.STATUS = USART_TXCIF_bm;  // |= ではなく = を使用する
}

```

