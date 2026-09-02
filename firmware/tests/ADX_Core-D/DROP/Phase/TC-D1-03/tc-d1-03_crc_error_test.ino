// =============================================================================
// ADX Core-D DROP-Bus Step 3: CRC-16 誤り検出 ＆ 不正パケット破棄テスト
// Test Sketch: TC-D1-03 (tc-d1-03_crc_error_test.ino)
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

// =============================================================================
// 【役割設定】
// 送信機（Node 1）に書き込む場合 : #define ROLE_TX を有効化
// 受信機（Node 2）に書き込む場合 : // #define ROLE_TX のようにコメントアウト
// =============================================================================
// #define ROLE_TX

// --- ハードウェアピン定義 (ADX Core-D 仕様準拠) ---
const int PIN_TXD   = PIN_PA1; // USART0 TXD (RS-485 DI)
const int PIN_RXD   = PIN_PA2; // USART0 RXD (RS-485 RO)
const int PIN_DE    = PIN_PA4; // RS-485 Driver Enable (HIGH=送信有効)
const int PIN_RE    = PIN_PA7; // RS-485 Receiver Enable (LOW=受信有効)
const int PIN_LED_R = PIN_PB2; // 赤色 LED (CRC エラー / STO 警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (正常受信インジケータ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信パラメータ ---
const uint32_t BAUDRATE   = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud)) // 9600bps: 約104.17 µs
#define MAX_PAYLOAD_SIZE  (64)

// =============================================================================
// CRC-16-CCITT 計算関数 (Poly: 0x1021, Init: 0xFFFF)
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
// RS-485 方向制御ヘルパー
// =============================================================================
inline void setTxMode() {
    PORTA.OUTSET = PIN7_bm; // /RE = HIGH (受信無効)
    PORTA.OUTSET = PIN4_bm; // DE  = HIGH (送信有効)
    delayMicroseconds(10);
}

inline void setRxMode() {
    PORTA.OUTCLR = PIN4_bm; // DE  = LOW (送信無効)
    PORTA.OUTCLR = PIN7_bm; // /RE = LOW (受信有効)
    delayMicroseconds(10);
}

#ifdef ROLE_TX
// =============================================================================
// 送信機専用関数 (実績のある Serial API 準拠)
// =============================================================================
void sendDropBreak(uint32_t baud) {
    uint16_t tBit = BIT_TIME_US(baud);
    
    Serial.flush();
    setTxMode();
    
    USART0.CTRLB &= ~USART_TXEN_bm;
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;
    Serial.write(0x55);             // Sync Byte
}

// テストフレーム送信関数 (corruptCRC が true の場合、意図的に CRC を反転破損)
void transmitTestFrame(uint8_t target_id, uint8_t sender_id, const uint8_t *payload, uint8_t len, bool corruptCRC) {
    // 1. Break (14 Tbit) + Delimiter (2 Tbit) + Sync (0x55)
    sendDropBreak(BAUDRATE);
    
    // 2. ヘッダ送信 (LEN, TARGET_ID, SENDER_ID)
    Serial.write(len);
    Serial.write(target_id);
    Serial.write(sender_id);
    
    // 3. ペイロード送信
    for (uint8_t i = 0; i < len; i++) {
        Serial.write(payload[i]);
    }
    
    // 4. CRC-16 計算 (対象: LEN + TARGET + SENDER + PAYLOAD)
    uint8_t crcBuf[MAX_PAYLOAD_SIZE + 3];
    crcBuf[0] = len;
    crcBuf[1] = target_id;
    crcBuf[2] = sender_id;
    for (uint8_t i = 0; i < len; i++) {
        crcBuf[3 + i] = payload[i];
    }
    uint16_t crc = calculateCRC16(crcBuf, len + 3);
    
    // エラー注入: CRC ビット反転
    if (corruptCRC) {
        crc ^= 0xFFFF; // 全ビット反転
    }
    
    // 5. CRC-16 送信 (High then Low)
    Serial.write((uint8_t)(crc >> 8));
    Serial.write((uint8_t)(crc & 0xFF));
    
    // 6. 送信完了待機 & バス解放
    Serial.flush();
    setRxMode();
}
#endif

// =============================================================================
// 初期化ルーチン (setup)
// =============================================================================
void setup() {
    // GPIO 出力設定
    PORTA.DIRSET = PIN4_bm | PIN7_bm; // DE, /RE = OUTPUT
    PORTB.DIRSET = PIN2_bm | PIN3_bm; // LED_R, LED_W = OUTPUT
    PORTB.OUTCLR = PIN2_bm | PIN3_bm; // 消灯
    setRxMode();
    
    // PC デバッグ用 SoftwareSerial 初期化 (9600 bps)
    pcSerial.begin(9600);
    delay(3000); // シリアルモニタ待機
    pcSerial.println(F("\r\n=============================================="));
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D1-03 Test          "));
    pcSerial.println(F("   (CRC-16 Error Detection & Recovery Test)   "));
    pcSerial.println(F("=============================================="));

#ifdef ROLE_TX
    // マスター側: Arduino Serial オブジェクト利用
    pinMode(PIN_RXD, INPUT);
    Serial.swap(1);
    Serial.begin(BAUDRATE);
    
    pcSerial.println(F("Role: [TRANSMITTER (Node 1)]"));
    pcSerial.println(F("Pattern Schedule:"));
    pcSerial.println(F("  Cycle 1..3 : Valid Frames (LEN=4B, CRC OK)"));
    pcSerial.println(F("  Cycle 4    : [ERROR INJECTION] Corrupted CRC Frame"));
    pcSerial.println(F("  Cycle 5..  : Valid Frames (Recovery Confirmation)\r\n"));
#else
    // スレーブ側: ハードウェア LINAUTO モード レジスタ直接設定
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
    PORTA.DIRSET = PIN1_bm; // PA1 (TX) = OUTPUT
    PORTA.DIRCLR = PIN2_bm; // PA2 (RX) = INPUT
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm; // PA2 プルアップ有効化
    
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLA = 0; // 割り込み完全無効化
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.BAUD  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));
    
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    
    pcSerial.println(F("Role: [SLAVE LINAUTO RECEIVER (Node 2)]"));
    pcSerial.println(F("Status: WFB=1. Monitoring for Valid and Corrupted Frames...\r\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
uint32_t lastTxTime = 0;
uint32_t txCycle = 0;

void loop() {
#ifdef ROLE_TX
    // --- 【送信機動作】 1.2秒間隔で正常 / 破損フレームを計画送出 ---
    if (millis() - lastTxTime >= 1200) {
        lastTxTime = millis();
        txCycle++;
        
        const uint8_t len = 4;
        const uint8_t payload[4] = {0x11, 0x22, 0x33, 0x44};
        
        // 4サイクルごとに意図的な CRC 反転破損を注入
        bool injectError = ((txCycle % 4) == 0);
        
        if (injectError) {
            PORTB.OUTSET = PIN2_bm; // 赤LED 点灯 (エラー送出表示)
            PORTB.OUTCLR = PIN3_bm;
            pcSerial.print(F("[TX #"));
            pcSerial.print(txCycle);
            pcSerial.println(F("] ★ [TEST: CRC CORRUPT] Sending Frame with Inverted CRC (0x33D5)..."));
            transmitTestFrame(0x02, 0x01, payload, len, true);
        } else {
            PORTB.OUTSET = PIN3_bm; // 白LED 点灯 (正常送出表示)
            PORTB.OUTCLR = PIN2_bm;
            pcSerial.print(F("[TX #"));
            pcSerial.print(txCycle);
            pcSerial.println(F("] [NORMAL] Sending Valid Frame (CRC: 0xCC2A)..."));
            transmitTestFrame(0x02, 0x01, payload, len, false);
        }
    }

#else
    // --- 【スレーブ LINAUTO 受信機動作】 ---
    static uint8_t rxBuf[MAX_PAYLOAD_SIZE + 5];
    static uint8_t rxIdx = 0;
    static uint32_t lastRxByteTime = 0;
    static uint32_t rxPassCount = 0;
    static uint32_t rxCrcErrorCount = 0;
    
    // 1. ISFIF 常時監視
    if (USART0.STATUS & USART_ISFIF_bm) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
    
    // 生存確認ハートビート (5秒間隔)
    static uint32_t lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 5000) {
        lastHeartbeat = millis();
        pcSerial.println(F("[Node 2] Heartbeat: Listening on RS-485 bus..."));
    }

    // 2. 受信 FIFO からデータを吸い上げて配列に保存
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        lastRxByteTime = millis();
        
        if (rxIdx < sizeof(rxBuf)) {
            rxBuf[rxIdx++] = rxLow;
        }
    }

    // 3. フレーム終了判定 (50ms無音) で解析実行
    if (rxIdx > 0 && (millis() - lastRxByteTime > 50)) {
        if (rxIdx >= 5) {
            uint8_t len = rxBuf[0];
            uint8_t target_id = rxBuf[1];
            uint8_t sender_id = rxBuf[2];
            
            if (len <= MAX_PAYLOAD_SIZE && rxIdx == (uint8_t)(len + 5)) {
                uint16_t receivedCrc = ((uint16_t)rxBuf[len + 3] << 8) | rxBuf[len + 4];
                uint16_t calculatedCrc = calculateCRC16(rxBuf, len + 3);
                
                if (receivedCrc == calculatedCrc) {
                    // 【PASS: 正常パケット】
                    rxPassCount++;
                    PORTB.OUTSET = PIN3_bm; // 白LED 点灯
                    PORTB.OUTCLR = PIN2_bm; // 赤LED 消灯
                    
                    pcSerial.print(F("[RX PASS #"));
                    pcSerial.print(rxPassCount);
                    pcSerial.print(F("] LEN="));
                    pcSerial.print(len);
                    pcSerial.print(F("B, Target=0x0"));
                    pcSerial.print(target_id, HEX);
                    pcSerial.print(F(", Sender=0x0"));
                    pcSerial.print(sender_id, HEX);
                    pcSerial.print(F(" | CRC=0x"));
                    pcSerial.print(receivedCrc, HEX);
                    pcSerial.println(F(" [MATCH] -> Packet Accepted!"));
                    
                } else {
                    // 【DETECTED: CRC 破損パケット検知 ＆ 破棄】
                    rxCrcErrorCount++;
                    PORTB.OUTSET = PIN2_bm; // 赤LED 点灯 (エラー警告)
                    PORTB.OUTCLR = PIN3_bm; // 白LED 消灯
                    
                    pcSerial.print(F("★ [RX CRC ERROR #"));
                    pcSerial.print(rxCrcErrorCount);
                    pcSerial.print(F("] Recv=0x"));
                    pcSerial.print(receivedCrc, HEX);
                    pcSerial.print(F(" != Calc=0x"));
                    pcSerial.print(calculatedCrc, HEX);
                    pcSerial.println(F(" [MISMATCH] -> FRAME DISCARDED (Safety Preserved)!"));
                }
            }
        }
        
        // 次フレーム待機へ再アーム
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
#endif
}
