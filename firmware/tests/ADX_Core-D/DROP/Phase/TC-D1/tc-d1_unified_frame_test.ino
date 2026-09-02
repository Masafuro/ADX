// =============================================================================
// ADX Core-D DROP-Bus Phase 1: 一体型フレーム送受信・LINAUTO同期・CRC-16検証テスト
// Test Sketch: TC-D1 (tc-d1_unified_frame_test.ino)
//
// Target Board: ADX Core-D (Microchip ATtiny1616 @ 20MHz/16MHz)
// RS-485 Transceiver: SP485EEN (DE: PA4, /RE: PA7)
//
// Copyright (c) 2026 ADX Project Contributors
// SPDX-License-Identifier: CC-BY-4.0
// =============================================================================

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// =============================================================================
// 1. テスト動作ロールの選択 (ビルド時にいずれか1つを有効化)
// =============================================================================
// #define ROLE_TX_TEST          // 送信テスト機 (1秒周期で可変長フレームを送出)
// #define ROLE_RX_DUMP          // 通常UART受信ダンプ機 (TC-D1-01: 生バイト列の確認用)
#define ROLE_RX_LINAUTO       // LINAUTO同期 & CRC-16検証機 (TC-D1-02〜04: 本番スタック検証用)

// --- 送信テスト機の動作オプション (ROLE_TX_TEST 有効時のみ) ---
// #define TX_CORRUPT_CRC        // TC-D1-03: 意図的に CRC を反転させて破損パケットを送出
// #define TX_OVERFLOW_LEN       // TC-D1-04: 意図的に LEN=0xFF の不正長パケットを送出

// =============================================================================
// 2. ハードウェアピン定義 (ADX Core-D 仕様準拠)
// =============================================================================
const int PIN_TXD   = PIN_PA1; // USART0 TXD (Alternate Pin)
const int PIN_RXD   = PIN_PA2; // USART0 RXD (Alternate Pin)
const int PIN_DE    = PIN_PA4; // RS-485 Driver Enable (HIGH=送信有効)
const int PIN_RE    = PIN_PA7; // RS-485 Receiver Enable (LOW=受信有効)
const int PIN_LED_R = PIN_PB2; // 赤色 LED (エラー / STO 警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (正常通信インジケータ)

// PCデバッグ中継用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// =============================================================================
// 3. 通信パラメータ ＆ 定数定義
// =============================================================================
const uint32_t BAUDRATE          = 9600;
#define BIT_TIME_US(baud)        (1000000UL / (baud))
#define MAX_PAYLOAD_SIZE         (64)
#define DROP_ID_SYSTEM           (0xFF)

#define TEST_SENDER_ID           (0x01)
#define TEST_TARGET_ID           (0x02)

// =============================================================================
// 4. CRC-16-CCITT 高速計算ルーチン (Poly: 0x1021, Init: 0xFFFF)
// =============================================================================
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

// =============================================================================
// 5. RS-485 送受信切り替え ＆ レジスタ送信ヘルパー
// =============================================================================
inline void setTxMode() {
    PORTA.OUTSET = PIN7_bm; // /RE = HIGH (受信無効)
    PORTA.OUTSET = PIN4_bm; // DE  = HIGH (送信有効)
    delayMicroseconds(10);  // トランシーバ安定化待ち
}

inline void setRxMode() {
    PORTA.OUTCLR = PIN4_bm; // DE  = LOW (送信無効)
    PORTA.OUTCLR = PIN7_bm; // /RE = LOW (受信有効)
    delayMicroseconds(10);  // トランシーバ安定化待ち
}

inline void sendByte(uint8_t data) {
    while (!(USART0.STATUS & USART_DREIF_bm)); // 送信バッファ空き待ち
    USART0.TXDATAL = data;
}

inline void flushTx() {
    while (!(USART0.STATUS & USART_TXCIF_bm)); // シフトレジスタ送出完了待ち
    USART0.STATUS = USART_TXCIF_bm;           // TXCIF クリア (= で直接代入)
}

// 一体型 Break 信号生成 (14 Tbit LOW + 2 Tbit HIGH + 0x55 送出)
void sendDropBreak(uint32_t baud) {
    uint16_t tBit = BIT_TIME_US(baud);
    
    setTxMode();
    USART0.CTRLB &= ~USART_TXEN_bm; // TX ディスエーブル (GPIO 出力へ開放)
    
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;  // TX 再イネーブル
    delayMicroseconds(5);           // グリッチ防止マージン
    sendByte(0x55);                 // Sync Byte 送出
}

// 一体型フレーム送信関数
void transmitDropFrame(uint8_t target_id, uint8_t sender_id, const uint8_t *payload, uint8_t len) {
    sendDropBreak(BAUDRATE);
    
#ifdef TX_OVERFLOW_LEN
    sendByte(0xFF); // 不正長 (255バイト)
#else
    sendByte(len);
#endif

    sendByte(target_id);
    sendByte(sender_id);
    
    // CRC-16 計算用バッファ (LEN + TARGET_ID + SENDER_ID + PAYLOAD)
    uint8_t crcBuf[MAX_PAYLOAD_SIZE + 3];
    crcBuf[0] = len;
    crcBuf[1] = target_id;
    crcBuf[2] = sender_id;
    for (uint8_t i = 0; i < len; i++) {
        crcBuf[3 + i] = payload[i];
        sendByte(payload[i]);
    }
    
    uint16_t crc = calculateCRC16(crcBuf, len + 3);
#ifdef TX_CORRUPT_CRC
    crc ^= 0xFFFF; // CRC 反転破損
#endif

    sendByte((uint8_t)(crc >> 8));   // CRC High Byte
    sendByte((uint8_t)(crc & 0xFF));  // CRC Low Byte
    
    flushTx();
    setRxMode();
}

// =============================================================================
// 6. 受信ステートマシン定義 (ROLE_RX_LINAUTO 用)
// =============================================================================
#ifdef ROLE_RX_LINAUTO

enum RxState {
    STATE_WAIT_BREAK_SYNC, // Break + 0x55 同期待機
    STATE_RECEIVE_LEN,     // LEN 受信 & バッファガード
    STATE_RECEIVE_HEADER,  // TARGET_ID, SENDER_ID 受信
    STATE_RECEIVE_PAYLOAD, // ペイロード受信
    STATE_RECEIVE_CRC      // CRC-16 受信 & 照合
};

RxState rxState = STATE_WAIT_BREAK_SYNC;
uint8_t rxPayload[MAX_PAYLOAD_SIZE];
uint8_t rxLen = 0;
uint8_t rxTargetId = 0;
uint8_t rxSenderId = 0;
uint8_t rxPayloadIndex = 0;
uint8_t rxCrcHigh = 0;
uint8_t rxHeaderIndex = 0;
uint32_t rxPacketCount = 0;
uint32_t crcErrorCount = 0;

void resetRxState() {
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; // 一括直接代入
    rxState = STATE_WAIT_BREAK_SYNC;
    rxPayloadIndex = 0;
    rxHeaderIndex = 0;
}

#endif

// =============================================================================
// 7. 初期化ルーチン (setup)
// =============================================================================
void setup() {
    // GPIO ピン初期化
    PORTA.DIRSET = PIN4_bm | PIN7_bm; // DE, /RE = OUTPUT
    PORTB.DIRSET = PIN2_bm | PIN3_bm; // LED_R, LED_W = OUTPUT
    PORTB.OUTCLR = PIN2_bm | PIN3_bm; // LED 消灯
    setRxMode(); // 初期状態は受信モード
    
    pcSerial.begin(9600);
    delay(100);
    pcSerial.println(F("\r\n=============================================="));
    pcSerial.println(F("   ADX Core-D DROP-Bus Phase 1 Test (TC-D1)   "));
    pcSerial.println(F("=============================================="));

    // USART0 ピンリマップ (PA1:TXD, PA2:RXD)
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
    PORTA.DIRSET = PIN1_bm; // PA1 = OUTPUT
    PORTA.DIRCLR = PIN2_bm; // PA2 = INPUT

#ifdef ROLE_TX_TEST
    pcSerial.println(F("Role: [TRANSMITTER (ROLE_TX_TEST)]"));
  #ifdef TX_CORRUPT_CRC
    pcSerial.println(F("Mode: ★ Corrupt CRC Injection Enabled (TC-D1-03)"));
  #elif defined(TX_OVERFLOW_LEN)
    pcSerial.println(F("Mode: ★ LEN Overflow (0xFF) Injection Enabled (TC-D1-04)"));
  #else
    pcSerial.println(F("Mode: Normal Unified Frame Transmission"));
  #endif

    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLB = USART_TXEN_bm; // 送信のみ有効
    USART0.BAUD  = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));

#elif defined(ROLE_RX_DUMP)
    pcSerial.println(F("Role: [RAW DUMP RECEIVER (ROLE_RX_DUMP) - TC-D1-01]"));
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLB = USART_RXEN_bm; // 通常UART受信
    USART0.BAUD  = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));

#elif defined(ROLE_RX_LINAUTO)
    pcSerial.println(F("Role: [LINAUTO & CRC RECEIVER (ROLE_RX_LINAUTO) - TC-D1-02..04]"));
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
    USART0.BAUD  = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
#endif

    pcSerial.println(F("Initialization complete. Starting loop...\r\n"));
}

// =============================================================================
// 8. メインループ (loop)
// =============================================================================
uint32_t lastTxTime = 0;
uint8_t testLenIndex = 0;
const uint8_t TEST_LENS[] = {1, 8, 16, 32, 64};
const uint8_t TEST_LEN_COUNT = sizeof(TEST_LENS) / sizeof(TEST_LENS[0]);

void loop() {

#ifdef ROLE_TX_TEST
    // --- 【送信機動作】 1秒間隔で可変長フレームを送出 ---
    if (millis() - lastTxTime >= 1000) {
        lastTxTime = millis();
        
        uint8_t len = TEST_LENS[testLenIndex];
        testLenIndex = (testLenIndex + 1) % TEST_LEN_COUNT;
        
        // 送信テストデータ生成
        uint8_t payload[MAX_PAYLOAD_SIZE];
        for (uint8_t i = 0; i < len; i++) {
            payload[i] = (uint8_t)(0x10 + i);
        }
        
        PORTB.OUTTGL = PIN3_bm; // 白LED トグル
        pcSerial.print(F("[TX] Sending Frame: LEN="));
        pcSerial.print(len);
        pcSerial.print(F("B, Target=0x"));
        pcSerial.print(TEST_TARGET_ID, HEX);
        pcSerial.print(F(", Sender=0x"));
        pcSerial.println(TEST_SENDER_ID, HEX);
        
        transmitDropFrame(TEST_TARGET_ID, TEST_SENDER_ID, payload, len);
    }

#elif defined(ROLE_RX_DUMP)
    // --- 【通常UARTダンプ動作 (TC-D1-01)】 生バイト列のダンプ ---
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        PORTB.OUTTGL = PIN3_bm; // 白LED トグル
        pcSerial.print(F("0x"));
        if (rxLow < 0x10) pcSerial.print(F("0"));
        pcSerial.print(rxLow, HEX);
        if (rxHigh & USART_FERR_bm) {
            pcSerial.print(F("(BREAK/FERR)"));
        }
        pcSerial.print(F(" "));
    }

#elif defined(ROLE_RX_LINAUTO)
    // --- 【LINAUTO & CRC検証機動作 (TC-D1-02 〜 04)】 ---
    
    // 1. ISFIF (同期エラー) 常時監視・デッドロック防止 (10大鉄則 第3条)
    if (USART0.STATUS & USART_ISFIF_bm) {
        PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
        pcSerial.println(F("\r\n[ERR] ISFIF (Inconsistent Sync Field) Detected! Recovering..."));
        resetRxState();
        PORTB.OUTCLR = PIN2_bm;
    }
    
    // 2. 受信 FIFO 読み出しループ
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得 (第1条)
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        switch (rxState) {
            case STATE_WAIT_BREAK_SYNC:
                // Break + 0x55 完了後、最初のデータバイトは LEN
                rxLen = rxLow;
                
                // 【バッファオーバーラン防御ガード (TC-D1-04)】
                if (rxLen > MAX_PAYLOAD_SIZE) {
                    PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
                    pcSerial.print(F("\r\n[GUARD] Invalid LEN="));
                    pcSerial.print(rxLen);
                    pcSerial.println(F(" (>64). Buffer overrun prevented! Aborting frame..."));
                    resetRxState();
                    PORTB.OUTCLR = PIN2_bm;
                } else {
                    rxHeaderIndex = 0;
                    rxState = STATE_RECEIVE_HEADER;
                }
                break;
                
            case STATE_RECEIVE_HEADER:
                if (rxHeaderIndex == 0) {
                    rxTargetId = rxLow;
                    rxHeaderIndex = 1;
                } else if (rxHeaderIndex == 1) {
                    rxSenderId = rxLow;
                    rxPayloadIndex = 0;
                    if (rxLen == 0) {
                        rxState = STATE_RECEIVE_CRC; // ペイロード長0なら直ちにCRCへ
                    } else {
                        rxState = STATE_RECEIVE_PAYLOAD;
                    }
                }
                break;
                
            case STATE_RECEIVE_PAYLOAD:
                rxPayload[rxPayloadIndex++] = rxLow;
                if (rxPayloadIndex >= rxLen) {
                    rxHeaderIndex = 0; // CRC High/Low 読み出し用カウンタに流用
                    rxState = STATE_RECEIVE_CRC;
                }
                break;
                
            case STATE_RECEIVE_CRC:
                if (rxHeaderIndex == 0) {
                    rxCrcHigh = rxLow;
                    rxHeaderIndex = 1;
                } else {
                    uint8_t rxCrcLow = rxLow;
                    uint16_t receivedCrc = ((uint16_t)rxCrcHigh << 8) | rxCrcLow;
                    
                    // CRC-16 計算 (LEN + TARGET_ID + SENDER_ID + PAYLOAD)
                    uint8_t crcBuf[MAX_PAYLOAD_SIZE + 3];
                    crcBuf[0] = rxLen;
                    crcBuf[1] = rxTargetId;
                    crcBuf[2] = rxSenderId;
                    for (uint8_t i = 0; i < rxLen; i++) {
                        crcBuf[3 + i] = rxPayload[i];
                    }
                    uint16_t calculatedCrc = calculateCRC16(crcBuf, rxLen + 3);
                    
                    // CRC 照合
                    if (receivedCrc == calculatedCrc) {
                        // 【PASS】 正常受信
                        rxPacketCount++;
                        PORTB.OUTSET = PIN3_bm; // 白LED 点灯
                        PORTB.OUTCLR = PIN2_bm; // 赤LED 消灯
                        
                        pcSerial.print(F("[RX PASS #"));
                        pcSerial.print(rxPacketCount);
                        pcSerial.print(F("] LEN="));
                        pcSerial.print(rxLen);
                        pcSerial.print(F("B, Target=0x"));
                        pcSerial.print(rxTargetId, HEX);
                        pcSerial.print(F(", Sender=0x"));
                        pcSerial.print(rxSenderId, HEX);
                        pcSerial.print(F(", CRC=0x"));
                        pcSerial.print(receivedCrc, HEX);
                        pcSerial.print(F(" [MATCH] Data: [ "));
                        for (uint8_t i = 0; i < rxLen && i < 8; i++) {
                            pcSerial.print(F("0x"));
                            if (rxPayload[i] < 0x10) pcSerial.print(F("0"));
                            pcSerial.print(rxPayload[i], HEX);
                            pcSerial.print(F(" "));
                        }
                        if (rxLen > 8) pcSerial.print(F("..."));
                        pcSerial.println(F("]"));
                        
                    } else {
                        // 【FAIL】 CRC 不一致 (TC-D1-03)
                        crcErrorCount++;
                        PORTB.OUTSET = PIN2_bm; // 赤LED 点灯 (エラー警告)
                        PORTB.OUTCLR = PIN3_bm;
                        
                        pcSerial.print(F("[CRC ERROR #"));
                        pcSerial.print(crcErrorCount);
                        pcSerial.print(F("] Recv=0x"));
                        pcSerial.print(receivedCrc, HEX);
                        pcSerial.print(F(", Calc=0x"));
                        pcSerial.print(calculatedCrc, HEX);
                        pcSerial.println(F(" -> Frame Discarded!"));
                    }
                    
                    // 次のフレーム待機へ復帰 (WFB=1 再アーム)
                    resetRxState();
                }
                break;
        }
    }
#endif

}
