// =============================================================================
// ADX Core-D DROP-Bus Step 2: スレーブ LINAUTO 自動同期 ＆ CRC-16 検証テスト
// Test Sketch: TC-D1-02 (tc-d1-02_slave_linauto_test.ino)
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
const int PIN_LED_R = PIN_PB2; // 赤色 LED (エラー / STO 警告)
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
    
    Serial.flush(); // 直前の送信完了を確実に待機
    setTxMode();
    
    USART0.CTRLB &= ~USART_TXEN_bm; // TX ディスエーブル
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;  // TX 再有効化
    Serial.write(0x55);             // Sync Byte 送出 (LINAUTO 自動校正用)
}

void transmitTestFrame(uint8_t target_id, uint8_t sender_id, const uint8_t *payload, uint8_t len) {
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
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D1-02 Test          "));
    pcSerial.println(F("   (Slave LINAUTO Auto-Baud & CRC-16 Verify)  "));
    pcSerial.println(F("=============================================="));

#ifdef ROLE_TX
    // マスター側: Arduino Serial オブジェクト利用
    pinMode(PIN_RXD, INPUT);
    Serial.swap(1);
    Serial.begin(BAUDRATE);
    
    pcSerial.println(F("Role: [TRANSMITTER (Node 1)]"));
    pcSerial.println(F("Transmitting Frames (LEN=4, 8, 16B) every 1000ms...\r\n"));
#else
    // スレーブ側: ハードウェア LINAUTO モード レジスタ直接設定
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
    PORTA.DIRSET = PIN1_bm; // PA1 (TX) = OUTPUT
    PORTA.DIRCLR = PIN2_bm; // PA2 (RX) = INPUT
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm; // PA2 プルアップ有効化
    
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLA = 0; // 割り込み完全無効化
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.BAUD  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE)); // 初期BAUD値
    
    // LINAUTO モード有効化 (Break + 0x55 でボーレート自動校正)
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
    // WFB=1 (Wait for Break) セット & エラーフラグ一括クリア (直接代入)
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    
    pcSerial.println(F("Role: [SLAVE LINAUTO RECEIVER (Node 2)]"));
    pcSerial.print(F("[Config] Initial USART0.BAUD = 0x"));
    pcSerial.println(USART0.BAUD, HEX);
    pcSerial.println(F("[Status] WFB=1. Listening for Break + 0x55 on RS-485 bus...\r\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
uint32_t lastTxTime = 0;
uint8_t txCycle = 0;

void loop() {
#ifdef ROLE_TX
    // --- 【送信機動作】 1秒間隔で可変長テストフレームを送出 ---
    if (millis() - lastTxTime >= 1000) {
        lastTxTime = millis();
        txCycle++;
        
        uint8_t len = 4;
        uint8_t payload[MAX_PAYLOAD_SIZE];
        
        // サイクルごとにペイロード長を変更 (4B -> 8B -> 16B)
        if (txCycle % 3 == 1) {
            len = 4;
            payload[0] = 0x11; payload[1] = 0x22; payload[2] = 0x33; payload[3] = 0x44;
        } else if (txCycle % 3 == 2) {
            len = 8;
            for (uint8_t i = 0; i < len; i++) payload[i] = (uint8_t)(0xA0 + i);
        } else {
            len = 16;
            for (uint8_t i = 0; i < len; i++) payload[i] = (uint8_t)(0x20 + i);
        }
        
        PORTB.OUTTGL = PIN3_bm; // 白LED トグル
        pcSerial.print(F("[TX #"));
        pcSerial.print(txCycle);
        pcSerial.print(F("] Sent Frame: LEN="));
        pcSerial.print(len);
        pcSerial.print(F("B, Target=0x02, Sender=0x01\r\n"));
        
        transmitTestFrame(0x02, 0x01, payload, len);
    }

#else
    // --- 【スレーブ LINAUTO 受信機動作】 ---
    static uint8_t rxBuf[MAX_PAYLOAD_SIZE + 5]; // LEN(1) + TARGET(1) + SENDER(1) + DATA(N) + CRC(2)
    static uint8_t rxIdx = 0;
    static uint32_t lastRxByteTime = 0;
    static uint32_t rxPacketCount = 0;
    static uint32_t crcErrorCount = 0;
    
    // 1. ISFIF (同期エラー) 常時監視・デッドロック防止 (10大鉄則 第6条)
    if (USART0.STATUS & USART_ISFIF_bm) {
        PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
        pcSerial.println(F("\r\n[ERR] USART0.STATUS.ISFIF Detected! Auto-rearming WFB=1..."));
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; // クリア & 再アーム
        rxIdx = 0;
        PORTB.OUTCLR = PIN2_bm;
    }
    
    // 生存確認用ハートビート (5秒間隔)
    static uint32_t lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 5000) {
        lastHeartbeat = millis();
        pcSerial.println(F("[Node 2] Heartbeat: LINAUTO Engine Active & Listening..."));
    }

    // 2. 受信 FIFO からデータを吸い上げて配列に保存 (低速 I/O 排除)
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得 (10大鉄則 第3条)
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        lastRxByteTime = millis();
        
        if (rxIdx < sizeof(rxBuf)) {
            rxBuf[rxIdx++] = rxLow;
        }
    }

    // 3. フレーム終了判定 (最後のデータ受信から 50ms 経過で解析実行)
    if (rxIdx > 0 && (millis() - lastRxByteTime > 50)) {
        // 必要最小サイズ: LEN(1) + TARGET(1) + SENDER(1) + CRC(2) = 5 バイト
        if (rxIdx >= 5) {
            uint8_t len = rxBuf[0];
            uint8_t target_id = rxBuf[1];
            uint8_t sender_id = rxBuf[2];
            
            // バッファオーバーラン防御ガード (10大鉄則 第8条)
            if (len > MAX_PAYLOAD_SIZE) {
                PORTB.OUTSET = PIN2_bm;
                pcSerial.print(F("\r\n[GUARD] Invalid LEN="));
                pcSerial.print(len);
                pcSerial.println(F(" (>64). Buffer overrun prevented!"));
            } else if (rxIdx == (uint8_t)(len + 5)) {
                // 受信バイト長が LEN と完全一致
                uint16_t receivedCrc = ((uint16_t)rxBuf[len + 3] << 8) | rxBuf[len + 4];
                
                // CRC-16 計算 (対象: LEN 〜 PAYLOAD 末尾)
                uint16_t calculatedCrc = calculateCRC16(rxBuf, len + 3);
                
                if (receivedCrc == calculatedCrc) {
                    // 【PASS】 LINAUTO 同期 ＆ CRC-16 完全一致
                    rxPacketCount++;
                    PORTB.OUTSET = PIN3_bm; // 白LED 点灯
                    PORTB.OUTCLR = PIN2_bm; // 赤LED 消灯
                    
                    pcSerial.print(F("[RX PASS #"));
                    pcSerial.print(rxPacketCount);
                    pcSerial.print(F("] Calibrated BAUD=0x"));
                    pcSerial.print(USART0.BAUD, HEX);
                    pcSerial.print(F(" | LEN="));
                    pcSerial.print(len);
                    pcSerial.print(F("B, Target=0x0"));
                    pcSerial.print(target_id, HEX);
                    pcSerial.print(F(", Sender=0x0"));
                    pcSerial.print(sender_id, HEX);
                    pcSerial.print(F(" | CRC=0x"));
                    pcSerial.print(receivedCrc, HEX);
                    pcSerial.print(F(" [MATCH] Data: [ "));
                    for (uint8_t i = 0; i < len; i++) {
                        pcSerial.print(F("0x"));
                        if (rxBuf[3 + i] < 0x10) pcSerial.print(F("0"));
                        pcSerial.print(rxBuf[3 + i], HEX);
                        pcSerial.print(F(" "));
                    }
                    pcSerial.println(F("]"));
                    
                } else {
                    // 【FAIL】 CRC 不一致
                    crcErrorCount++;
                    PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
                    PORTB.OUTCLR = PIN3_bm;
                    
                    pcSerial.print(F("[CRC ERROR #"));
                    pcSerial.print(crcErrorCount);
                    pcSerial.print(F("] Recv=0x"));
                    pcSerial.print(receivedCrc, HEX);
                    pcSerial.print(F(", Calc=0x"));
                    pcSerial.print(calculatedCrc, HEX);
                    pcSerial.println(F(" -> Discarded!"));
                }
            } else {
                // バイト数不一致 (パケット途絶)
                pcSerial.print(F("[LEN MISMATCH] Expected "));
                pcSerial.print(len + 5);
                pcSerial.print(F(" bytes, but received "));
                pcSerial.print(rxIdx);
                pcSerial.println(F(" bytes. Discarded."));
            }
        } else {
            pcSerial.print(F("[MALFORMED] Packet too short ("));
            pcSerial.print(rxIdx);
            pcSerial.println(F(" bytes). Discarded."));
        }
        
        // 次の Break を待機するため WFB=1 を再アーム & バッファ初期化
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
#endif
}
