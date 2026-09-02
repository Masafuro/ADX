// =============================================================================
// ADX Core-D DROP-Bus Phase 2: 2ノード自律分散バトンリレー ＆ 相互購読テスト
// Test Sketch: TC-D2-01 (tc-d2-01_ping_pong_relay.ino)
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
// 【ノード識別設定】
// Node 1 に書き込む場合 : #define NODE_ID (1)
// Node 2 に書き込む場合 : #define NODE_ID (2)
// =============================================================================
#define NODE_ID (1)

// --- ハードウェアピン定義 (ADX Core-D 仕様準拠) ---
const int PIN_TXD   = PIN_PA1; // USART0 TXD (RS-485 DI)
const int PIN_RXD   = PIN_PA2; // USART0 RXD (RS-485 RO)
const int PIN_DE    = PIN_PA4; // RS-485 Driver Enable (HIGH=送信有効)
const int PIN_RE    = PIN_PA7; // RS-485 Receiver Enable (LOW=受信有効)
const int PIN_LED_R = PIN_PB2; // 赤色 LED (エラー / STO 警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (バトン受領・通信アクティブ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信パラメータ ---
const uint32_t BAUDRATE   = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud)) // 9600bps: 約104.17 µs
#define MAX_PAYLOAD_SIZE  (64)

// --- ノード別バトン設定 (論理 Pub/Sub 定義) ---
#if (NODE_ID == 1)
  const uint8_t MY_SLOT_ID      = 0x01; // 自ノードの発話バトンID (Publisher)
  const uint8_t NEXT_TARGET_ID  = 0x02; // 次にバトンを渡す宛先ID (Target)
  const bool    IS_IGNITER      = true; // Node 1 が起動時点火役
#elif (NODE_ID == 2)
  const uint8_t MY_SLOT_ID      = 0x02; // 自ノードの発話バトンID (Publisher)
  const uint8_t NEXT_TARGET_ID  = 0x01; // 次にバトンを渡す宛先ID (Target)
  const bool    IS_IGNITER      = false; // Node 2 はバトン受領待ち
#else
  #error "Invalid NODE_ID! Please set NODE_ID to 1 or 2."
#endif

// 送信・受信用メールボックス
uint8_t myTxData[4] = {0x10, 0x20, 0x30, (uint8_t)(NODE_ID * 0x11)};
uint8_t rxMailbox[4] = {0, 0, 0, 0};
uint32_t relayCycleCount = 0;

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

// 一体型 Break 送出関数 (14 Tbit LOW + 2 Tbit HIGH + 0x55 送出)
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

// バトンフレーム送信関数 (Publish & Pass)
void transmitBatonFrame(uint8_t target_id, uint8_t sender_id, const uint8_t *payload, uint8_t len) {
    sendDropBreak(BAUDRATE);
    
    Serial.write(len);
    Serial.write(target_id);
    Serial.write(sender_id);
    
    for (uint8_t i = 0; i < len; i++) {
        Serial.write(payload[i]);
    }
    
    uint8_t crcBuf[MAX_PAYLOAD_SIZE + 3];
    crcBuf[0] = len;
    crcBuf[1] = target_id;
    crcBuf[2] = sender_id;
    for (uint8_t i = 0; i < len; i++) {
        crcBuf[3 + i] = payload[i];
    }
    uint16_t crc = calculateCRC16(crcBuf, len + 3);
    
    Serial.write((uint8_t)(crc >> 8));
    Serial.write((uint8_t)(crc & 0xFF));
    
    Serial.flush(); // 送出完了待機 (10大鉄則 第7条)
    setRxMode();
}

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
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D2-01 Relay Test    "));
    pcSerial.println(F("   (2-Node Autonomous Baton Pass & Pub/Sub)   "));
    pcSerial.println(F("=============================================="));
    pcSerial.print(F("Node Configuration: [NODE_ID = "));
    pcSerial.print(NODE_ID);
    pcSerial.print(F("] SlotID=0x0"));
    pcSerial.print(MY_SLOT_ID, HEX);
    pcSerial.print(F(" -> TargetID=0x0"));
    pcSerial.println(NEXT_TARGET_ID, HEX);

    // ハードウェア Serial 初期化 (送信で使用)
    pinMode(PIN_RXD, INPUT);
    Serial.swap(1);
    Serial.begin(BAUDRATE);

    // 受信用レジスタ直接初期化 (LINAUTO モード)
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

    // --- 点火シーケンス (Node 1 のみ初期バトンを投入) ---
    if (IS_IGNITER) {
        pcSerial.println(F("[IGNITER] Firing Initial Baton into Bus in 500ms..."));
        delay(500);
        
        relayCycleCount = 1;
        PORTB.OUTSET = PIN3_bm; // 白LED 点灯
        transmitBatonFrame(NEXT_TARGET_ID, MY_SLOT_ID, myTxData, 4);
        
        pcSerial.print(F("[TX #1 (IGNITION)] Passed Baton to Target=0x0"));
        pcSerial.println(NEXT_TARGET_ID, HEX);
        
        // 直ちに受信待機へ遷移
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    } else {
        pcSerial.println(F("[LISTENER] Waiting for incoming Baton from Node 1...\r\n"));
    }
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
    static uint8_t rxBuf[MAX_PAYLOAD_SIZE + 5];
    static uint8_t rxIdx = 0;
    static uint32_t lastRxByteTime = 0;

    // 1. ISFIF 常時監視・デッドロック防止 (10大鉄則 第6条)
    if (USART0.STATUS & USART_ISFIF_bm) {
        PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
        pcSerial.println(F("\r\n[ERR] ISFIF Detected! Rearming WFB=1..."));
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
        PORTB.OUTCLR = PIN2_bm;
    }

    // 2. 受信 FIFO からデータを吸い上げて配列に保存 (低速 I/O 排除)
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
        lastRxByteTime = millis();
        
        if (rxIdx < sizeof(rxBuf)) {
            rxBuf[rxIdx++] = rxLow;
        }
    }

    // 3. フレーム終了判定 (50ms無音) で解析 ＆ バトンパス処理
    if (rxIdx > 0 && (millis() - lastRxByteTime > 50)) {
        if (rxIdx >= 5) {
            uint8_t len = rxBuf[0];
            uint8_t target_id = rxBuf[1];
            uint8_t sender_id = rxBuf[2];
            
            // バッファオーバーラン防御
            if (len <= MAX_PAYLOAD_SIZE && rxIdx == (uint8_t)(len + 5)) {
                uint16_t receivedCrc = ((uint16_t)rxBuf[len + 3] << 8) | rxBuf[len + 4];
                uint16_t calculatedCrc = calculateCRC16(rxBuf, len + 3);
                
                if (receivedCrc == calculatedCrc) {
                    // 【CRC 一致: 正常フレーム受信】
                    PORTB.OUTCLR = PIN2_bm; // 赤LED 消灯
                    
                    // --- ① Common Subscriber (トピック購読処理) ---
                    for (uint8_t i = 0; i < len && i < 4; i++) {
                        rxMailbox[i] = rxBuf[3 + i];
                    }
                    
                    // --- ② 発話権判定 (自身宛てのバトンか？) ---
                    if (target_id == MY_SLOT_ID) {
                        relayCycleCount++;
                        PORTB.OUTTGL = PIN3_bm; // 白LED トグル
                        
                        // ログ表示
                        pcSerial.print(F("[RELAY #"));
                        pcSerial.print(relayCycleCount);
                        pcSerial.print(F("] Recv from Sender=0x0"));
                        pcSerial.print(sender_id, HEX);
                        pcSerial.print(F(" Data:[ "));
                        for (uint8_t i = 0; i < len; i++) {
                            pcSerial.print(F("0x"));
                            if (rxBuf[3 + i] < 0x10) pcSerial.print(F("0"));
                            pcSerial.print(rxBuf[3 + i], HEX);
                            pcSerial.print(F(" "));
                        }
                        pcSerial.print(F("] -> Passing Baton to Target=0x0"));
                        pcSerial.println(NEXT_TARGET_ID, HEX);
                        
                        // 送信データ更新 (カウンタをインクリメント)
                        myTxData[3] = (uint8_t)(relayCycleCount & 0xFF);
                        
                        // レスポンススペース (相手の DE=0 待機)
                        delayMicroseconds(200);
                        
                        // 次ノードへバトンパス送出
                        transmitBatonFrame(NEXT_TARGET_ID, MY_SLOT_ID, myTxData, 4);
                    } else {
                        // 他ノード宛てバトン (傍受のみ)
                        pcSerial.print(F("[SUBSCRIBE ONLY] Topic Sender=0x0"));
                        pcSerial.print(sender_id, HEX);
                        pcSerial.print(F(" -> Target=0x0"));
                        pcSerial.println(target_id, HEX);
                    }
                } else {
                    // CRC エラー
                    PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
                    pcSerial.println(F("[CRC ERROR] Corrupted Baton Frame Discarded!"));
                }
            }
        }
        
        // 次の Break 待機へ再アーム
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
}
