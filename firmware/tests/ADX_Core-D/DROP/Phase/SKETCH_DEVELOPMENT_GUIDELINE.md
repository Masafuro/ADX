<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D DROP-Bus スケッチ開発ガイドライン ＆ 注意事項
(DROP-Bus Sketch Development Guidelines & Precautions)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）上で DROP-Bus のテストファームウェアおよびドライバを実装する際の**必須コーディングルール、レジスタ制約、および推奨テンプレート**をまとめたガイドラインです。

---

## 1. DROP-Bus スケッチ開発の 10大鉄則 (Critical Rules)

LN-485 で確立された知見を継承しつつ、DROP-Bus 固有の自律リレー・可変長フレーム・パッシブフェイルセーフに対応した 10 の鉄則です。

```text
========================================================================================
                     DROP-Bus スケッチ開発 10大鉄則
========================================================================================
 1. 受信レジスタ読み出し順序
    - 必ず先に RXDATAH を読み、次に RXDATAL を読む（逆順は FIFO ポインタが破損）
 2. STATUS レジスタ操作における |= の絶対禁止
    - USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; （直接代入で一括クリア）
 3. ISFIF デッドロック防止と常時監視
    - 同期エラー (ISFIF) の検知・復帰は必ず while (STATUS & RXCIF) の【外側】で常時実行
 4. 一体型 Break 送出シーケンス
    - TXディスエーブル -> PA1 LOW (14 Tbit) -> PA1 HIGH (2 Tbit) -> TX再イネーブル -> 0x55 送出
 5. 半二重ターンアラウンド & slaveTxFlush()
    - 送信末尾バイト（CRC-16）送出完了を TXCIF で待機してから DE=0（受信モード）へ戻す
 6. 全ノード完全レジスタ直接制御 (Serial.begin() の完全排除)
    - Arduino コアの RX 割り込み競合を完全に排除し、決定論的（Deterministic）な低遅延を確保
 7. LEN 不正値ガード (バッファオーバーラン防御)
    - LEN > 64 を受信した瞬間、即座に受信をアボートしてバトンを落とし、パッシブSTOへ誘導
 8. CRC-16-CCITT による高速整合性検証
    - 多項式 0x1021 (初期値: 0xFFFF) を用い、LEN 〜 PAYLOAD 末尾までを確実に検証
 9. TCB0 ハードウェアタイマー連動のパッシブ STO (心中)
    - 通信途絶・CRCエラー時はタイマーをリセットせず、TCB0 満了割り込みで全出力を即時遮断
 10. Double Buffer Mailbox によるアプリと通信の疎結合化
    - アプリ層 (loop) のデータ生成と通信層の Zero-Copy 送信を 2面バッファで完全分離
========================================================================================
```

---

## 2. コア・アルゴリズム実装スニペット

### 2.1 CRC-16-CCITT 高速計算ルーチン

```cpp
/**
 * @brief CRC-16-CCITT (Poly: 0x1021, Init: 0xFFFF)
 */
uint16_t calculateCRC16(const uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}
```

---

### 2.2 TCB0 タイマーによる生存監視 ＆ パッシブ STO

```cpp
#include <avr/io.h>
#include <avr/interrupt.h>

// 安全停止フラグ
volatile bool isSafeTorqueOff = false;

/**
 * @brief TCB0 タイマー初期化 (タイムアウト: 約 20ms @ 20MHz/2)
 */
void initFailSafeTimer(uint16_t timeout_ticks) {
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;       // Periodic Interrupt モード
    TCB0.CCMP  = timeout_ticks;           // タイムアウト値
    TCB0.INTCTRL = TCB_CAPT_bm;            // 割り込み許可
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm; // 起動
}

/**
 * @brief 正常パケット受信時のタイマーリセット
 */
inline void resetFailSafeTimer() {
    TCB0.CNT = 0; // カウンタクリア
}

/**
 * @brief TCB0 タイムアウト割り込みハンドラ (パッシブ心中 STO 発動)
 */
ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm; // フラグクリア
    
    // 【パッシブ・フェイルセーフ発動】
    isSafeTorqueOff = true;
    PORTB.OUTSET = PIN2_bm;      // 赤LED 点灯 (STO警告)
    PORTB.OUTCLR = PIN3_bm;      // 白LED 消灯
    // モータ / PWM / アクチュエータ出力をすべてハードウェア LOW 遮断
}
```

---

## 3. DROP-Bus スケッチ基本テンプレート (Code Template)

同一スケッチで全ノードをビルドできる単一SKU対応の基本骨格です。

```cpp
// =============================================================================
// ADX Core-D DROP-Bus Node Firmware Template
// =============================================================================
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// --- ノード識別設定 (単一SKU: EEPROM/DIPスイッチ等で動的取得可能) ---
#define MY_NODE_ID    (0x01)
#define NEXT_NODE_ID  (0x02)
#define IS_INITIATOR  (1)    // 起動時に初回バトンを点火するノードなら 1

// --- ハードウェアピン定義 (ADX Core-D 仕様準拠) ---
const int PIN_TXD = PIN_PA1; // USART0 TXD
const int PIN_RXD = PIN_PA2; // USART0 RXD
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH=送信)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW=受信)
const int LED_R   = PIN_PB2; // 赤色 LED (STO 警告)
const int LED_W   = PIN_PB3; // 白色 LED (通信アクティブ)

// デバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信パラメータ ---
const uint32_t BAUDRATE = 9600;
#define BIT_TIME_US(baud)    (1000000UL / (baud))
#define MAX_PAYLOAD_SIZE     (64)

// --- 受信ステートマシン定義 ---
enum DropRxState {
    STATE_WAIT_BREAK_SYNC,
    STATE_RECEIVE_HEADER,
    STATE_RECEIVE_PAYLOAD,
    STATE_RECEIVE_CRC
};
DropRxState rxState = STATE_WAIT_BREAK_SYNC;

// 受信バッファ
uint8_t rxBuffer[MAX_PAYLOAD_SIZE + 8];
uint8_t rxIndex = 0;
uint8_t rxPayloadLen = 0;

// 送信バッファ (Double Buffer Mailbox)
uint8_t txPayload[2][MAX_PAYLOAD_SIZE];
uint8_t txPayloadLen[2] = {4, 4};
volatile uint8_t activeTxIdx = 0;

// =============================================================================
// 低レイヤ制御ヘルパー関数
// =============================================================================
inline void setTxMode() {
    PORTA.OUTSET = PIN7_bm; // RE=HIGH (受信無効)
    PORTA.OUTSET = PIN4_bm; // DE=HIGH (送信有効)
    delayMicroseconds(10);
}

inline void setRxMode() {
    PORTA.OUTCLR = PIN4_bm; // DE=LOW (送信無効)
    PORTA.OUTCLR = PIN7_bm; // RE=LOW (受信有効)
    delayMicroseconds(10);
}

inline void sendByte(uint8_t data) {
    while (!(USART0.STATUS & USART_DREIF_bm));
    USART0.TXDATAL = data;
}

inline void flushTx() {
    while (!(USART0.STATUS & USART_TXCIF_bm));
    USART0.STATUS = USART_TXCIF_bm; // TXCIF クリア (= で直接代入)
}

// 一体型 Break 送出
void sendDropBreak(uint32_t baud) {
    uint16_t tBit = BIT_TIME_US(baud);
    setTxMode();
    USART0.CTRLB &= ~USART_TXEN_bm; // TX ディスエーブル (GPIO解放)
    
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;  // TX 再イネーブル
    sendByte(0x55);                 // Sync Byte 送出
}

// パケット送信
void transmitDropFrame(uint8_t target_id, const uint8_t *payload, uint8_t len) {
    sendDropBreak(BAUDRATE);
    
    // ヘッダ送信
    sendByte(len);
    sendByte(target_id);
    sendByte(MY_NODE_ID);
    
    // CRC-16 計算用バッファ構築
    uint8_t crcBuf[MAX_PAYLOAD_SIZE + 3];
    crcBuf[0] = len;
    crcBuf[1] = target_id;
    crcBuf[2] = MY_NODE_ID;
    for (uint8_t i = 0; i < len; i++) {
        crcBuf[3 + i] = payload[i];
        sendByte(payload[i]);
    }
    
    // CRC-16 送出
    uint16_t crc = calculateCRC16(crcBuf, len + 3);
    sendByte((uint8_t)(crc >> 8));
    sendByte((uint8_t)(crc & 0xFF));
    
    flushTx();
    setRxMode();
}

// =============================================================================
// 初期化ルーチン (setup)
// =============================================================================
void setup() {
    // GPIO 初期化
    PORTA.DIRSET = PIN4_bm | PIN7_bm; // DE, /RE = OUTPUT
    PORTB.DIRSET = PIN2_bm | PIN3_bm; // LED_R, LED_W = OUTPUT
    setRxMode();
    
    pcSerial.begin(9600);
    pcSerial.println(F("=== ADX Core-D DROP-Bus Node Initialized ==="));
    
    // USART0 レジスタ直接初期化 (LINAUTO モード)
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc; // PA1:TX, PA2:RX
    PORTA.DIRSET = PIN1_bm;                        // PA1 = OUTPUT
    PORTA.DIRCLR = PIN2_bm;                        // PA2 = INPUT
    
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
    USART0.BAUD  = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    
    // TCB0 タイマー初期化
    initFailSafeTimer(50000); // タイムアウト設定
    
    sei(); // 全体割り込み許可
    
#if IS_INITIATOR
    delay(500); // 全ノード起動待機
    pcSerial.println(F("[INITIATOR] Firing initial baton..."));
    uint8_t initData[4] = {0x01, 0x02, 0x03, 0x04};
    transmitDropFrame(NEXT_NODE_ID, initData, 4);
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
    if (isSafeTorqueOff) {
        // STO 発動中: 沈黙を維持
        return;
    }
    
    // --- 1. 同期エラー (ISFIF) デッドロック防止監視 ---
    if (USART0.STATUS & USART_ISFIF_bm) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxState = STATE_WAIT_BREAK_SYNC;
    }
    
    // --- 2. 受信ステートマシン ---
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        // ステートマシン遷移とパケット受信処理 (TC-D1 / TC-D2 実装)
    }
    
    // --- 3. アプリケーション層 (Double Buffer Mailbox への非同期書き込み) ---
    // センサ計測やモータ制御演算をここで自由に実行
}
```
