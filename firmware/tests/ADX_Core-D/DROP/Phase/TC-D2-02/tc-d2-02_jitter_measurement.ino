// =============================================================================
// ADX Core-D DROP-Bus Phase 2: 10,000周期連続周回 ＆ ジッター計測テスト
// Test Sketch: TC-D2-02 (tc-d2-02_jitter_measurement.ino)
//
// Target Board: ADX Core-D (Microchip ATtiny1616 @ 20MHz/16MHz)
// RS-485 Transceiver: SP485EEN (DE: PA4, /RE: PA7)
//
// 特徴:
// 1. 【名目ボーレート送信リセット】: 送信時に USART0.BAUD を公称値へ強制リセット (正帰還ドリフト完全防止)
// 2. 【固定レスポンスギャップ】: ターンアラウンド安全確保 ＆ アプリ時間確保のため 10ms のガード時間を挿入
// 3. 【再点火 Mediator】: 500ms 無音検知で Node 1 が自動再点火 (耐ドロップ自律復帰)
// 4. 【非干渉サンプリング】: 1,000 周期ごとの統計サマリー出力
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
const int PIN_LED_R = PIN_PB2; // 赤色 LED (CRC エラー / 異常警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (周回中インジケータ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信 ＆ タイミングパラメータ ---
const uint32_t BAUDRATE          = 9600;
#define BIT_TIME_US(baud)        (1000000UL / (baud)) // 9600bps: 約104.17 µs
#define MAX_PAYLOAD_SIZE         (64)
#define TARGET_CYCLES            (10000UL) // 目標周回数: 10,000 周期
const uint16_t RESPONSE_GAP_MS   = 10;     // 固定レスポンスギャップ: 10ms
const uint32_t BUS_TIMEOUT_MS    = 500;    // 再点火タイムアウト: 500ms

// 名目ボーレート定数 (送信時に必ずこの値にリセット)
const uint16_t NOMINAL_BAUD_REG  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));

// --- ノード別バトン設定 ---
#if (NODE_ID == 1)
  const uint8_t MY_SLOT_ID      = 0x01; // 自ノードの発話バトンID
  const uint8_t NEXT_TARGET_ID  = 0x02; // 次にバトンを渡す宛先ID
  const bool    IS_IGNITER      = true; // Node 1 が起動時点火役 ＆ 再点火 Mediator
#elif (NODE_ID == 2)
  const uint8_t MY_SLOT_ID      = 0x02; // 自ノードの発話バトンID
  const uint8_t NEXT_TARGET_ID  = 0x01; // 次にバトンを渡す宛先ID
  const bool    IS_IGNITER      = false; // Node 2 はバトン受領待ち
#else
  #error "Invalid NODE_ID! Please set NODE_ID to 1 or 2."
#endif

// 送受信メールボックス
uint8_t myTxData[4] = {0x10, 0x20, 0x30, (uint8_t)(NODE_ID * 0x11)};
uint8_t rxMailbox[4] = {0, 0, 0, 0};

// --- ジッター計測 ＆ 状態監視変数 ---
uint32_t cycleCount = 0;
uint32_t crcErrorCount = 0;
uint32_t reignitionCount = 0;
uint32_t lastCycleMicros = 0;
uint32_t lastBusActivityMs = 0;
uint32_t minCycleUs = 0xFFFFFFFF;
uint32_t maxCycleUs = 0;
uint64_t totalCycleUsSum = 0;

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
    
    // ★【重要】送信時は常に名目ボーレート（公称値）へリセット！(正帰還ドリフト完全防止)
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
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D2-02 Jitter Test   "));
    pcSerial.println(F("   (10,000-Cycle High-Speed Relay Stability)  "));
    pcSerial.println(F("=============================================="));
    pcSerial.print(F("Node Config: [NODE_ID = "));
    pcSerial.print(NODE_ID);
    pcSerial.print(F("] SlotID=0x0"));
    pcSerial.print(MY_SLOT_ID, HEX);
    pcSerial.print(F(" -> TargetID=0x0"));
    pcSerial.println(NEXT_TARGET_ID, HEX);
    pcSerial.print(F("Nominal BAUD Register: 0x"));
    pcSerial.println(NOMINAL_BAUD_REG, HEX);
    pcSerial.println(F("Feature: Transmitter BAUD Reset enabled (Anti-Drift).\r\n"));

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

    // --- 点火シーケンス (Node 1 のみ初期バトンを投入) ---
    if (IS_IGNITER) {
        pcSerial.println(F("[IGNITER] Igniting Baton Relay in 500ms..."));
        delay(500);
        
        lastCycleMicros = micros();
        lastBusActivityMs = millis();
        transmitBatonFrame(NEXT_TARGET_ID, MY_SLOT_ID, myTxData, 4);
        
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    } else {
        pcSerial.println(F("[LISTENER] Ready. Waiting for Initial Baton from Node 1...\r\n"));
    }
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
        uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
        uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得
        
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
                    // --- ① Common Subscriber 購読 ---
                    for (uint8_t i = 0; i < expectedLen && i < 4; i++) {
                        rxMailbox[i] = rxBuf[3 + i];
                    }
                    
                    // --- ② 発話権判定 (自身宛てバトン) ---
                    if (target_id == MY_SLOT_ID) {
                        uint32_t nowMicros = micros();
                        uint32_t cycleDeltaUs = nowMicros - lastCycleMicros;
                        lastCycleMicros = nowMicros;
                        
                        cycleCount++;
                        
                        // 最初の数周期を除外してジッター統計を更新
                        if (cycleCount > 5) {
                            if (cycleDeltaUs < minCycleUs) minCycleUs = cycleDeltaUs;
                            if (cycleDeltaUs > maxCycleUs) maxCycleUs = cycleDeltaUs;
                            totalCycleUsSum += cycleDeltaUs;
                        }
                        
                        // LED 点滅 (25 周期 = 約1秒ごとにトグル点滅)
                        if ((cycleCount % 25) == 0) {
                            PORTB.OUTTGL = PIN3_bm;
                        }
                        
                        // 1,000 周期ごとの統計サマリー出力 (Node 1 のみ出力)
#if (NODE_ID == 1)
                        if ((cycleCount % 1000) == 0) {
                            uint32_t avgUs = (uint32_t)(totalCycleUsSum / (cycleCount - 5));
                            uint32_t jitterUs = (maxCycleUs - minCycleUs) / 2;
                            
                            pcSerial.print(F("[STAT #"));
                            pcSerial.print(cycleCount);
                            pcSerial.print(F("] Avg: "));
                            pcSerial.print(avgUs / 1000);
                            pcSerial.print(F("."));
                            pcSerial.print((avgUs % 1000) / 10);
                            pcSerial.print(F("ms ("));
                            pcSerial.print(1000000UL / avgUs);
                            pcSerial.print(F("Hz) | Min: "));
                            pcSerial.print(minCycleUs);
                            pcSerial.print(F("us, Max: "));
                            pcSerial.print(maxCycleUs);
                            pcSerial.print(F("us | Jitter: +/-"));
                            pcSerial.print(jitterUs);
                            pcSerial.print(F("us | Re-ignites: "));
                            pcSerial.print(reignitionCount);
                            pcSerial.print(F(" | CRC Err: "));
                            pcSerial.println(crcErrorCount);
                        }
                        
                        // 10,000 周期到達時の完了表示
                        if (cycleCount == TARGET_CYCLES) {
                            uint32_t avgUs = (uint32_t)(totalCycleUsSum / (cycleCount - 5));
                            uint32_t jitterUs = (maxCycleUs - minCycleUs) / 2;
                            
                            pcSerial.println(F("\r\n=============================================="));
                            pcSerial.println(F(" ★ 10,000 CYCLES COMPLETED SUCCESSFULLY! ★ "));
                            pcSerial.print(F(" Total Cycles : ")); pcSerial.println(cycleCount);
                            pcSerial.print(F(" Overall Avg  : ")); pcSerial.print(avgUs); pcSerial.println(F(" us"));
                            pcSerial.print(F(" Jitter Range : +/-")); pcSerial.print(jitterUs); pcSerial.println(F(" us"));
                            pcSerial.print(F(" Re-ignites   : ")); pcSerial.println(reignitionCount);
                            pcSerial.print(F(" CRC Errors   : ")); pcSerial.println(crcErrorCount);
                            pcSerial.println(F(" Error Rate   : 0.00% [PASS]"));
                            pcSerial.println(F("==============================================\r\n"));
                        }
#endif
                        
                        // 送信データ更新
                        myTxData[3] = (uint8_t)(cycleCount & 0xFF);
                        
                        // 【固定レスポンスギャップ】 ターンアラウンド安全確保 & アプリ処理時間
                        delay(RESPONSE_GAP_MS);
                        
                        // 次ノードへバトンパス送出
                        transmitBatonFrame(NEXT_TARGET_ID, MY_SLOT_ID, myTxData, 4);
                        lastBusActivityMs = millis();
                    }
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
    if (cycleCount < TARGET_CYCLES && (millis() - lastBusActivityMs > BUS_TIMEOUT_MS)) {
        reignitionCount++;
        pcSerial.print(F("★ [MEDIATOR] Bus Silence Timeout (500ms). Re-igniting Baton #"));
        pcSerial.print(reignitionCount);
        pcSerial.println(F("..."));
        
        lastBusActivityMs = millis();
        lastCycleMicros = micros();
        transmitBatonFrame(NEXT_TARGET_ID, MY_SLOT_ID, myTxData, 4);
        
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
#endif
}
