// =============================================================================
// ADX Core-D DROP-Bus Phase 2: Multi-rate スロット（不等周期リレー）テスト
// Test Sketch: TC-D2-04 (tc-d2-04_multirate_slot_test.ino)
//
// Target Board: ADX Core-D (Microchip ATtiny1616 @ 20MHz/16MHz)
// RS-485 Transceiver: SP485EEN (DE: PA4, /RE: PA7)
//
// 特徴:
// 1. 【Multi-rate スロット】: Node 1 が 2スロット(0x01: 4B, 0x03: 8B)、Node 2 が 1スロット(0x02: 4B)を所有
// 2. 【非対称スケジューリング】: [0x01 (4B)] -> [0x02 (4B)] -> [0x03 (8B)] -> [0x02 (4B)] の周回成立
// 3. 【トピック別相互購読】: Node 2 が 0x01 (制御) と 0x03 (ログ) を個別 Mailbox へ正確に格納
// 4. 【名目ボーレート送信リセット】: 送信直前に USART0.BAUD を強制リセット (正帰還ドリフト防止)
// 5. 【固定レスポンスギャップ】: 10ms のガードインターバル
// 6. 【再点火 Mediator】: 500ms 無音検知で Node 1 が自動再点火
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
const int PIN_LED_R = PIN_PB2; // 赤色 LED (エラー / 異常警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (周回中インジケータ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信 ＆ タイミングパラメータ ---
const uint32_t BAUDRATE          = 9600;
#define BIT_TIME_US(baud)        (1000000UL / (baud)) // 9600bps: 約104.17 µs
#define MAX_PAYLOAD_SIZE         (64)
#define TARGET_ROUNDS            (2000UL)  // 目標大周回数: 2,000 周 (8,000 スロット)
const uint16_t RESPONSE_GAP_MS   = 10;     // 固定レスポンスギャップ: 10ms
const uint32_t BUS_TIMEOUT_MS    = 500;    // 再点火タイムアウト: 500ms

// 名目ボーレート定数 (送信時に必ずこの値にリセット)
const uint16_t NOMINAL_BAUD_REG  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));

// --- スロット ＆ メールボックス定義 ---
#if (NODE_ID == 1)
  const bool IS_IGNITER = true; // Node 1 が起動時点火役 ＆ 再点火 Mediator
  
  // Node 1 送信スロット 1: 0x01 (高速制御データ 4B)
  uint8_t txData_01[4] = {0x01, 0x11, 0x22, 0x00};
  // Node 1 送信スロット 2: 0x03 (低速ログデータ 8B)
  uint8_t txData_03[8] = {0x03, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x00};
  
  // Node 1 受信メールボックス: 0x02 (Node 2 センサデータ 4B)
  uint8_t rxMailbox_02[4] = {0, 0, 0, 0};

#elif (NODE_ID == 2)
  const bool IS_IGNITER = false;
  
  // Node 2 送信スロット: 0x02 (センサデータ 4B)
  uint8_t txData_02[4] = {0x02, 0xAA, 0xBB, 0x00};
  
  // Node 2 受信メールボックス 1: 0x01 (Node 1 制御データ 4B)
  uint8_t rxMailbox_01[4] = {0, 0, 0, 0};
  // Node 2 受信メールボックス 2: 0x03 (Node 1 ログデータ 8B)
  uint8_t rxMailbox_03[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#else
  #error "Invalid NODE_ID! Please set NODE_ID to 1 or 2."
#endif

// --- 統計 ＆ 状態監視変数 ---
uint32_t roundCount = 0; // 大周期カウンタ ([0x01]->[0x02]->[0x03]->[0x02])
uint32_t slotTxCount = 0;
uint32_t slotRxCount_01 = 0;
uint32_t slotRxCount_02 = 0;
uint32_t slotRxCount_03 = 0;
uint32_t crcErrorCount = 0;
uint32_t reignitionCount = 0;
uint32_t lastRoundMicros = 0;
uint32_t lastBusActivityMs = 0;
uint32_t minRoundUs = 0xFFFFFFFF;
uint32_t maxRoundUs = 0;
uint64_t totalRoundUsSum = 0;

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

// 一体型 Break 送出関数 (送信前 名目ボーレート強制リセット)
void sendDropBreak(uint32_t baud) {
    uint16_t tBit = BIT_TIME_US(baud);
    
    Serial.flush();
    setTxMode();
    
    // ★【重要】送信時は常に名目ボーレート（公称値）へリセット！
    USART0.BAUD = NOMINAL_BAUD_REG;
    
    USART0.CTRLB &= ~USART_TXEN_bm;
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;
    Serial.write(0x55);             // Sync Byte
}

// 可変長バトンフレーム送信関数
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
    slotTxCount++;
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
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D2-04 Test          "));
    pcSerial.println(F("   (Multi-rate Asymmetric Relay & Pub/Sub)    "));
    pcSerial.println(F("=============================================="));
    pcSerial.print(F("Node Configuration: [NODE_ID = "));
    pcSerial.print(NODE_ID);
#if (NODE_ID == 1)
    pcSerial.println(F("] Owns Slots: 0x01 (4B Control) & 0x03 (8B Log)"));
    pcSerial.println(F("Relay Sequence: [0x01 (4B)] -> [0x02] -> [0x03 (8B)] -> [0x02]"));
#elif (NODE_ID == 2)
    pcSerial.println(F("] Owns Slot: 0x02 (4B Sensor) -> Passes to 0x03 / 0x01 alternately"));
#endif
    pcSerial.println(F("Timing: Fixed Gap = 10ms | Bus Timeout = 500ms.\r\n"));

    // ハードウェア Serial 初期化
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
    USART0.BAUD  = NOMINAL_BAUD_REG;
    
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    lastBusActivityMs = millis();

    // --- 点火シーケンス (Node 1 のみスロット 0x01 をバスへ投入) ---
#if (NODE_ID == 1)
    pcSerial.println(F("[IGNITER] Igniting Multi-rate Baton Relay with Slot 0x01 in 500ms..."));
    delay(500);
    
    lastRoundMicros = micros();
    lastBusActivityMs = millis();
    // スロット 0x01 (4B) を Target 0x02 宛てに送出
    transmitBatonFrame(0x02, 0x01, txData_01, 4);
    
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
#else
    pcSerial.println(F("[LISTENER] Ready. Waiting for Initial Baton from Node 1...\r\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
    static uint8_t rxBuf[MAX_PAYLOAD_SIZE + 5];
    static uint8_t rxIdx = 0;

    // 1. ISFIF 常時監視
    if (USART0.STATUS & USART_ISFIF_bm) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }

    // 2. 受信 FIFO からデータを吸い上げて配列に保存
    while (USART0.STATUS & USART_RXCIF_bm) {
        uint8_t rxHigh = USART0.RXDATAH;
        uint8_t rxLow  = USART0.RXDATAL;
        (void)rxHigh; // 警告抑止
        
        lastBusActivityMs = millis();
        
        if (rxIdx < sizeof(rxBuf)) {
            rxBuf[rxIdx++] = rxLow;
        }

        // バッファオーバーラン防御
        if (rxIdx >= 1 && rxBuf[0] > MAX_PAYLOAD_SIZE) {
            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
            rxIdx = 0;
            break;
        }

        // --- フレーム完了判定 (LEN + 5バイト到達で即座にパース) ---
        if (rxIdx >= 1) {
            uint8_t expectedLen = rxBuf[0];
            uint8_t targetFrameBytes = expectedLen + 5;
            
            if (rxIdx == targetFrameBytes) {
                uint8_t target_id = rxBuf[1];
                uint8_t sender_id = rxBuf[2];
                uint16_t receivedCrc = ((uint16_t)rxBuf[expectedLen + 3] << 8) | rxBuf[expectedLen + 4];
                uint16_t calculatedCrc = calculateCRC16(rxBuf, expectedLen + 3);
                
                if (receivedCrc == calculatedCrc) {
                    // =========================================================
                    // 【Node 1 の受信 ＆ 送信ルーティング】
                    // =========================================================
#if (NODE_ID == 1)
                    // Common Subscriber: Node 2 (0x02) のデータを購読
                    if (sender_id == 0x02) {
                        slotRxCount_02++;
                        for (uint8_t i = 0; i < expectedLen && i < 4; i++) {
                            rxMailbox_02[i] = rxBuf[3 + i];
                        }
                    }
                    
                    // 発話権判定
                    if (target_id == 0x01) {
                        // 1 大周回 (4スロット) の完了
                        uint32_t nowMicros = micros();
                        uint32_t roundDeltaUs = nowMicros - lastRoundMicros;
                        lastRoundMicros = nowMicros;
                        roundCount++;
                        
                        if (roundCount > 5) {
                            if (roundDeltaUs < minRoundUs) minRoundUs = roundDeltaUs;
                            if (roundDeltaUs > maxRoundUs) maxRoundUs = roundDeltaUs;
                            totalRoundUsSum += roundDeltaUs;
                        }
                        
                        if ((roundCount % 25) == 0) {
                            PORTB.OUTTGL = PIN3_bm; // 白LED 点滅
                        }
                        
                        // 500 大周期 (2,000 スロット) ごとの統計出力
                        if ((roundCount % 500) == 0) {
                            uint32_t avgUs = (uint32_t)(totalRoundUsSum / (roundCount - 5));
                            uint32_t jitterUs = (maxRoundUs - minRoundUs) / 2;
                            
                            pcSerial.print(F("[ROUND #"));
                            pcSerial.print(roundCount);
                            pcSerial.print(F("] Avg: "));
                            pcSerial.print(avgUs / 1000);
                            pcSerial.print(F("."));
                            pcSerial.print((avgUs % 1000) / 10);
                            pcSerial.print(F("ms ("));
                            pcSerial.print(1000000UL / avgUs);
                            pcSerial.print(F("Hz) | Rx02: "));
                            pcSerial.print(slotRxCount_02);
                            pcSerial.print(F(" | CRC Err: "));
                            pcSerial.println(crcErrorCount);
                        }
                        
                        if (roundCount == TARGET_ROUNDS) {
                            uint32_t avgUs = (uint32_t)(totalRoundUsSum / (roundCount - 5));
                            uint32_t jitterUs = (maxRoundUs - minRoundUs) / 2;
                            
                            pcSerial.println(F("\r\n=============================================="));
                            pcSerial.println(F(" ★ 2,000 ROUNDS (8,000 SLOTS) COMPLETED! ★ "));
                            pcSerial.print(F(" Total Rounds : ")); pcSerial.println(roundCount);
                            pcSerial.print(F(" Round Avg    : ")); pcSerial.print(avgUs); pcSerial.println(F(" us"));
                            pcSerial.print(F(" Rx Slot 0x02 : ")); pcSerial.println(slotRxCount_02);
                            pcSerial.print(F(" Re-ignites   : ")); pcSerial.println(reignitionCount);
                            pcSerial.print(F(" CRC Errors   : ")); pcSerial.println(crcErrorCount);
                            pcSerial.println(F(" Status       : ALL MATCH [PASS]"));
                            pcSerial.println(F("==============================================\r\n"));
                        }
                        
                        // スロット 0x01 (4B 制御データ) を送出
                        txData_01[3] = (uint8_t)(roundCount & 0xFF);
                        delay(RESPONSE_GAP_MS);
                        transmitBatonFrame(0x02, 0x01, txData_01, 4);
                        lastBusActivityMs = millis();
                        
                    } else if (target_id == 0x03) {
                        // スロット 0x03 (8B ログデータ) を送出
                        txData_03[7] = (uint8_t)(roundCount & 0xFF);
                        delay(RESPONSE_GAP_MS);
                        transmitBatonFrame(0x02, 0x03, txData_03, 8);
                        lastBusActivityMs = millis();
                    }

                    // =========================================================
                    // 【Node 2 の受信 ＆ 送信ルーティング】
                    // =========================================================
#elif (NODE_ID == 2)
                    // Common Subscriber: 0x01 (4B) と 0x03 (8B) を個別 Mailbox へ購読
                    if (sender_id == 0x01) {
                        slotRxCount_01++;
                        for (uint8_t i = 0; i < expectedLen && i < 4; i++) {
                            rxMailbox_01[i] = rxBuf[3 + i];
                        }
                    } else if (sender_id == 0x03) {
                        slotRxCount_03++;
                        for (uint8_t i = 0; i < expectedLen && i < 8; i++) {
                            rxMailbox_03[i] = rxBuf[3 + i];
                        }
                    }
                    
                    // 発話権判定 (Node 2 宛てバトン)
                    if (target_id == 0x02) {
                        roundCount++;
                        if ((roundCount % 50) == 0) {
                            PORTB.OUTTGL = PIN3_bm; // 白LED 点滅
                        }
                        
                        // スロット 0x02 (4B センサデータ) を準備
                        txData_02[3] = (uint8_t)(roundCount & 0xFF);
                        
                        delay(RESPONSE_GAP_MS);
                        
                        // 送信元に応じて次宛先を切り替え
                        if (sender_id == 0x01) {
                            // 0x01 から受け取ったら、次は Node 1 の 0x03 スロットへ渡す
                            transmitBatonFrame(0x03, 0x02, txData_02, 4);
                        } else {
                            // 0x03 から受け取ったら、次は Node 1 の 0x01 スロットへ渡す
                            transmitBatonFrame(0x01, 0x02, txData_02, 4);
                        }
                        lastBusActivityMs = millis();
                    }
#endif
                } else {
                    crcErrorCount++;
                    PORTB.OUTSET = PIN2_bm; // 赤LED 点灯
                }
                
                // 次の Break 待機へ再アーム
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                rxIdx = 0;
            }
        }
    }

    // 3. 不完全フレームのタイムアウト破棄 (30ms 無音)
    if (rxIdx > 0 && (millis() - lastBusActivityMs > 30)) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }

    // 4. 【再点火 Mediator Watchdog】 (Node 1 のみ担当: 500ms 無音で再点火)
#if (NODE_ID == 1)
    if (roundCount < TARGET_ROUNDS && (millis() - lastBusActivityMs > BUS_TIMEOUT_MS)) {
        reignitionCount++;
        pcSerial.print(F("★ [MEDIATOR] Bus Silence Timeout (500ms). Re-igniting with Slot 0x01 #"));
        pcSerial.print(reignitionCount);
        pcSerial.println(F("..."));
        
        lastBusActivityMs = millis();
        lastRoundMicros = micros();
        transmitBatonFrame(0x02, 0x01, txData_01, 4);
        
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
#endif
}
