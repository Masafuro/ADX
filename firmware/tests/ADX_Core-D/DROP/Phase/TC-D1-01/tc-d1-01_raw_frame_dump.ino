// =============================================================================
// ADX Core-D DROP-Bus Step 1: 一体型フレーム生成 ＆ 通常UART受信ダンプテスト
// Test Sketch: TC-D1-01 (tc-d1-01_raw_frame_dump.ino)
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
const int PIN_LED_R = PIN_PB2; // 赤色 LED
const int PIN_LED_W = PIN_PB3; // 白色 LED

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信パラメータ ---
const uint32_t BAUDRATE   = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud)) // 9600bps: 約104.17 µs

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
// 一体型 Break 送出 (14 Tbit LOW + 2 Tbit HIGH + 0x55 送出)
void sendDropBreak(uint32_t baud) {
    uint16_t tBit = BIT_TIME_US(baud);
    
    Serial.flush(); // 直前の送信完了を確実に待機
    setTxMode();
    
    // TX を一時ディスエーブルして PA1 を GPIO LOW 駆動
    USART0.CTRLB &= ~USART_TXEN_bm;
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;  // TX 再有効化
    Serial.write(0x55);             // Sync Byte 送出
}

// 固定4バイトのテストフレーム送信
void transmitTestFrame() {
    const uint8_t len = 4;
    const uint8_t target_id = 0x02;
    const uint8_t sender_id = 0x01;
    const uint8_t payload[4] = {0x11, 0x22, 0x33, 0x44};
    
    // 1. Break (14 Tbit) + Delimiter (2 Tbit) + Sync (0x55)
    sendDropBreak(BAUDRATE);
    
    // 2. 論理ヘッダ送信
    Serial.write(len);
    Serial.write(target_id);
    Serial.write(sender_id);
    
    // 3. ペイロード送信
    for (uint8_t i = 0; i < len; i++) {
        Serial.write(payload[i]);
    }
    
    // 4. CRC-16 計算 (対象: LEN + TARGET + SENDER + PAYLOAD = 7 bytes)
    uint8_t crcBuf[7] = {len, target_id, sender_id, payload[0], payload[1], payload[2], payload[3]};
    uint16_t crc = calculateCRC16(crcBuf, 7);
    
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
    delay(3000); // シリアルモニタを開く時間を確保
    pcSerial.println(F("\r\n=========================================="));
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D1-01 Test      "));
    pcSerial.println(F("=========================================="));

#ifdef ROLE_TX
    // マスター側: Arduino Serial オブジェクトを利用
    pinMode(PIN_RXD, INPUT);
    Serial.swap(1);
    Serial.begin(BAUDRATE);
    
    pcSerial.println(F("Role: [TRANSMITTER (Node 1)]"));
    pcSerial.println(F("Sending Frame every 1000ms:"));
    pcSerial.println(F("  [Break] 0x55 | LEN:0x04 | TGT:0x02 | SND:0x01 | DATA:11 22 33 44 | CRC\r\n"));
#else
    // スレーブ側: 割り込み競合を防ぐため Serial.begin() は使わずレジスタ直接設定
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
    PORTA.DIRSET = PIN1_bm; // PA1 (TX) = OUTPUT
    PORTA.DIRCLR = PIN2_bm; // PA2 (RX) = INPUT
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm; // PA2 の入力バッファ有効化・プルアップ
    
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLA = 0; // 割り込み完全無効化
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.BAUD  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));
    USART0.CTRLB = USART_RXEN_bm; // 受信のみ有効
    
    pcSerial.println(F("Role: [RAW DUMP RECEIVER (Node 2)]"));
    pcSerial.println(F("Listening on RS-485 bus (9600 bps)...\r\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
uint32_t lastTxTime = 0;
uint32_t lastRxByteTime = 0;

void loop() {
#ifdef ROLE_TX
    // --- 【送信機動作】 1秒間隔でフレーム送出 ---
    if (millis() - lastTxTime >= 1000) {
        lastTxTime = millis();
        
        PORTB.OUTTGL = PIN3_bm; // 白LED トグル
        pcSerial.println(F("[TX] Frame Sent."));
        transmitTestFrame();
    }
#else
    // --- 【受信機動作】 バッファリングして後で一括出力 ---
    static uint8_t rxBuf[64];
    static bool errBuf[64];
    static uint8_t rxIdx = 0;
    
    // SoftwareSerial 生存確認用ハートビート（3秒間隔）
    static uint32_t lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 3000) {
        lastHeartbeat = millis();
        pcSerial.println(F("[Node 2] Heartbeat... PC Connection is OK."));
    }

    // 1. ハードウェアバッファからデータを吸い上げて配列に保存
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        lastRxByteTime = millis();
        PORTB.OUTTGL = PIN3_bm; // 受信で白LED トグル
        
        if (rxIdx < sizeof(rxBuf)) {
            rxBuf[rxIdx] = rxLow;
            errBuf[rxIdx] = (rxHigh & USART_FERR_bm) ? true : false;
            rxIdx++;
        }
    }

    // 2. フレーム終了判定 (最後のデータから50ms経過) で一括ダンプ出力
    if (rxIdx > 0 && (millis() - lastRxByteTime > 50)) {
        pcSerial.println();
        pcSerial.print(F("[RX DUMP] "));
        
        for (uint8_t i = 0; i < rxIdx; i++) {
            pcSerial.print(F("0x"));
            if (rxBuf[i] < 0x10) pcSerial.print(F("0"));
            pcSerial.print(rxBuf[i], HEX);
            
            if (errBuf[i]) {
                pcSerial.print(F("(BREAK)"));
            }
            pcSerial.print(F(" "));
        }
        rxIdx = 0; // バッファリセット
    }
#endif
}
