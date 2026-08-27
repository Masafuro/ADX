
# LN-485 Publish Mailbox Pattern: ダブルバッファ構成による完全非同期設計

## 1. 概要
産業用PLCの「I/Oリフレッシュ」と同等のアーキテクチャを採用し、メールボックス内に「フロントバッファ（通信層用）」と「バックバッファ（アプリ層用）」の2つの領域を持たせる。
これにより、アプリケーション層での書き込み・チェックサム計算中に Master Broker からの読み出し要求が衝突するリスク（Torn Read）を構造的に完全に排除する。

## 2. データ構造の再設計
バッファを2面（`0` と `1`）用意し、通信層が現在どちらを読み出すべきかを `activeIdx`（0 または 1）で管理する。

```cpp
struct DoubleBufferMailbox {
    uint8_t  topicId;
    uint8_t  length;
    
    // 2面バッファ: [0] と [1]
    uint8_t  payload[2][8];
    uint8_t  checksum[2];
    
    // 現在通信層がアクセスしてよい（最新の）バッファのインデックス
    volatile uint8_t activeIdx;
};

DoubleBufferMailbox mailbox[MAX_TOPICS];

```

## 3. アプリケーション側の処理 (Publisher)

アプリケーション側は、現在「アクティブではない方（裏バッファ）」に対してゆっくりとデータを書き込む。書き込み完了後、一瞬だけ割り込みを禁止してインデックスを切り替える。

```cpp
void publishTopic(uint8_t id, const void* data, uint8_t len) {
    // 1. 次に書き込むべき「裏バッファ」のインデックスを計算 (0なら1, 1なら0)
    uint8_t nextIdx = mailbox[id].activeIdx ^ 1;

    // 2. 裏バッファへの書き込みと計算 (※割り込み許可状態なのでブロックしない)
    // この間、通信層は表バッファ(activeIdx)を安全に読み出し続けることができる
    memcpy(mailbox[id].payload[nextIdx], data, len);
    mailbox[id].checksum[nextIdx] = calculateClassicChecksum(data, len);
    mailbox[id].length = len;

    // 3. インデックスの切り替え (極小のクリティカルセクション)
    // 8bitマイコンでの8bit変数代入は元々アトミックだが、意図を明確にするためcli()を使用
    uint8_t sreg = SREG;
    cli();
    mailbox[id].activeIdx = nextIdx; // ここから通信層は新しいデータを読み出す
    SREG = sreg;
}

```

## 4. 通信層側の処理 (ISR または 高速ポーリング)

通信層は、Master Broker から要求が来た瞬間に、その時点の `activeIdx` を見て表バッファをそのままバスへ送出する。

```cpp
void onMasterPollReceived(uint8_t id) {
    // 現在アクティブなバッファのインデックスを取得
    uint8_t currentIdx = mailbox[id].activeIdx;
    
    // 表バッファからデータを即座に送信 (コピー不要で超高速)
    sendPayload(
        mailbox[id].payload[currentIdx],
        mailbox[id].length,
        mailbox[id].checksum[currentIdx]
    );
}

```

## 5. ダブルバッファ方式のメリット

1. **ゼロ・ブロッキング:** チェックサム計算などの「比較的重い処理」をしている間も割り込みを禁止しないため、通信の取りこぼしや遅延が一切発生しない。
2. **ゼロ・コピー (通信層):** 通信層は `memcpy` 等でデータを安全な領域に退避させる必要がなく、直接ポインタ（インデックス）を参照してバスに流せるため、50µsの厳しいレスポンススペースに余裕で間に合う。
3. **PLCライクな堅牢性:** データの書き換え途中で通信層が読み出してしまう「Torn Read」が原理的に発生しない。

