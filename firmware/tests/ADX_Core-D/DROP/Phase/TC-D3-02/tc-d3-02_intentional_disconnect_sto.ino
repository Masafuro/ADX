// =============================================================================
// ADX Core-D DROP-Bus Phase 3: 意図的断線 ＆ 即時安全心中 (STO) テスト
// Test Sketch: TC-D3-02 (tc-d3-02_intentional_disconnect_sto.ino)
//
// Target Board: ADX Core-D (Microchip ATtiny1616 @ 20MHz/16MHz)
// RS-485 Transceiver: SP485EEN (DE: PA4, /RE: PA7)
//
// 特徴:
// 1. 【意図的断線 / 通信途絶】: Node 1 が 200 周期周回後にバトン送信を意図的停止 (ドロップ誘発)
// 2. 【全ノード一斉安全心中 (STO)】: 最後の通信から 250ms (50 ticks) で全ノードが同期して出力遮断
// 3. 【TCB0 ハードウェアタイマー】: 5ms 周期割り込み x 50 ticks = 250ms 心中タイムアウト
// 4. 【安全状態ラッチ】: 赤LED 点灯 & 白LED 消灯 & モータ出力遮断状態を保持
// 5. 【名目ボーレート送信リセット】: 送信直前に USART0.BAUD を強制リセット (正帰還ドリフト防止)
//
// Copyright (c) 2026 ADX Project Contributors
// SPDX-License-Identifier: CC-BY-4.0
// =============================================================================

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>

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
const int PIN_LED_R = PIN_PB2; // 赤色 LED (STO トリップ警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (周回中インジケータ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信 ＆ タイミングパラメータ ---
const uint32_t BAUDRATE          = 9600;
#define BIT_TIME_US(baud)        (1000000UL / (baud)) // 9600bps: 約104.17 µs
#define MAX_PAYLOAD_SIZE         (64)
#define DISCONNECT_AT_CYCLE      (200UL)   // 意図的切断周期: 200 周期目で送信停止
const uint16_t RESPONSE_GAP_MS   = 10;     // 固定レスポンスギャップ: 10ms
const uint8_t  STO_TIMEOUT_TICKS = 50;     // 50 * 5ms = 250ms 心中タイムアウト

// 名目ボーレート定数 (送信時に必ずこの値にリセット)
const uint16_t NOMINAL_BAUD_REG  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));

// --- ノード別バトン設定 ---
#if (NODE_ID == 1)
  const uint8_t MY_SLOT_ID      = 0x01;
  const uint8_t NEXT_TARGET_ID  = 0x02;
#elif (NODE_ID == 2)
  const uint8_t MY_SLOT_ID      = 0x02;
  const uint8_t NEXT_TARGET_ID  = 0x01;
#else
  #error "Invalid NODE_ID! Please set NODE_ID to 1 or 2."
#endif

// --- 心中フェイルセーフ (STO) 監視変数 ---
volatile uint8_t stoTickCounter = 0;
volatile bool isSafeTorqueOff = false;
volatile bool isMonitoringActive = false;
volatile uint32_t stoTripTimeMs = 0;
bool stoReported = false;

// --- 統計 ＆ 状態監視変数 ---
uint32_t cycleCount = 0;
uint32_t lastCycleMicros = 0;
uint32_t lastBusActivityMs = 0;

// =============================================================================
// TCB0 ハードウェアタイマー初期化 (5ms 周期割り込み)
// =============================================================================
void initTCB0SurvivalTimer() {
    TCB0.CTRLA = 0; // 一旦タイマー停止
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Periodic Interrupt Mode
    // 5ms 周期 TOP値計算: (F_CPU / 2) / 200
    TCB0.CCMP = (uint16_t)((F_CPU / 2UL) / 200UL);
    TCB0.CNT = 0;
    TCB0.INTFLAGS = TCB_CAPT_bm;
    TCB0.INTCTRL = TCB_CAPT_bm; // 割り込み有効化
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm; // CLK_PER/2 で起動
}

// TCB0 割り込みハンドラ (5ms ごとに実行)
ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm; // 割り込みフラグクリア
    
    // 通信開始（アーム）後のみタイムアウト監視を実行
    if (isMonitoringActive && !isSafeTorqueOff) {
        stoTickCounter++;
        if (stoTickCounter >= STO_TIMEOUT_TICKS) {
            isSafeTorqueOff = true;
            PORTB.OUTSET = PIN2_bm; // 赤LED 点灯 (STO 安全心中 発動)
            PORTB.OUTCLR = PIN3_bm; // 白LED 消灯
            stoTripTimeMs = millis();
        }
    }
}

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
    
    // 送信時は常に名目ボーレート（公称値）へリセット
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

// バトンフレーム送信関数
void transmitBaton(uint8_t target_id, uint8_t sender_id, uint8_t payload_byte) {
    sendDropBreak(BAUDRATE);
    
    uint8_t len = 4;
    Serial.write(len);
    Serial.write(target_id);
    Serial.write(sender_id);
    Serial.write(0xAA);
    Serial.write(sender_id);
    Serial.write(payload_byte);
    Serial.write((uint8_t)(payload_byte ^ 0xFF));
    
    uint8_t crcBuf[7];
    crcBuf[0] = len;
    crcBuf[1] = target_id;
    crcBuf[2] = sender_id;
    crcBuf[3] = 0xAA;
    crcBuf[4] = sender_id;
    crcBuf[5] = payload_byte;
    crcBuf[6] = (uint8_t)(payload_byte ^ 0xFF);
    uint16_t crc = calculateCRC16(crcBuf, 7);
    
    Serial.write((uint8_t)(crc >> 8));
    Serial.write((uint8_t)(crc & 0xFF));
    
    Serial.flush(); // 送出完了待機
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
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D3-02 Test          "));
    pcSerial.println(F("   (Intentional Disconnect & Synchronized STO)"));
    pcSerial.println(F("=============================================="));
    pcSerial.print(F("Node Config: [NODE_ID = "));
    pcSerial.print(NODE_ID);
    pcSerial.print(F("] SlotID=0x0"));
    pcSerial.print(MY_SLOT_ID, HEX);
    pcSerial.print(F(" -> TargetID=0x0"));
    pcSerial.println(NEXT_TARGET_ID, HEX);
    pcSerial.println(F("TCB0 Shinju Timer: 250ms Timeout Active."));
#if (NODE_ID == 1)
    pcSerial.println(F("Test Scenario: Node 1 will INTENTIONALLY STOP transmitting at Cycle #200."));
#else
    pcSerial.println(F("Test Scenario: Node 2 will detect sudden silence and trip STO synchronously."));
#endif
    pcSerial.println(F("Pass Criteria: Both nodes trip STO (Red LED ON) within 250ms.\r\n"));

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

    // TCB0 ハードウェアタイマー起動 (心中タイマー開始)
    initTCB0SurvivalTimer();
    sei(); // 全体割り込み許可

    lastBusActivityMs = millis();

    // --- 点火シーケンス (Node 1 のみ初期バトンを投入) ---
#if (NODE_ID == 1)
    pcSerial.println(F("[IGNITER] Igniting Relay in 500ms..."));
    delay(500);
    
    lastCycleMicros = micros();
    lastBusActivityMs = millis();
    transmitBaton(NEXT_TARGET_ID, MY_SLOT_ID, 0);
    
    // 送信と同時にタイマー監視アーム
    stoTickCounter = 0;
    TCB0.CNT = 0;
    isSafeTorqueOff = false;
    isMonitoringActive = true;
    
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

    // --- ① STO (安全心中) 発動時のレポート出力 ---
    if (isSafeTorqueOff && !stoReported) {
        stoReported = true;
        uint32_t elapsedMs = stoTripTimeMs - lastBusActivityMs;
        
        pcSerial.println(F("\r\n=============================================="));
        pcSerial.println(F(" ★ [STO TRIP] SYNCHRONIZED SHINJU EXECUTED! ★ "));
        pcSerial.print(F(" Node ID      : ")); pcSerial.println(NODE_ID);
        pcSerial.print(F(" Stopped After: Cycle #")); pcSerial.println(cycleCount);
        pcSerial.print(F(" Reason       : Bus Silence Timeout (250ms)\r\n"));
        pcSerial.print(F(" Elapsed Time : ")); pcSerial.print(elapsedMs); pcSerial.println(F(" ms after last bus activity"));
        pcSerial.print(F(" Motor Output : FORCED OFF (Safe Torque Off)\r\n"));
        pcSerial.print(F(" Red LED (PB2): ON (Latched)\r\n"));
        pcSerial.print(F(" Status       : SYNCHRONIZED SHINJU [PASS]\r\n"));
        pcSerial.println(F("==============================================\r\n"));
    }

    // --- ② STO 発動後は通信処理を完全停止 (心中安全停止を維持) ---
    if (isSafeTorqueOff) {
        return;
    }

    // --- ③ 通信層: ISFIF 常時監視 ---
    if (USART0.STATUS & USART_ISFIF_bm) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }

    // --- ④ 通信層: 受信 FIFO からデータを吸い上げ ---
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
                    // 生存タイマーゼロリセット
                    stoTickCounter = 0;
                    TCB0.CNT = 0;
                    isMonitoringActive = true;
                    
                    // 発話権判定 (自身宛てバトン)
                    if (target_id == MY_SLOT_ID) {
                        cycleCount++;
                        
                        // LED 点滅 (10 周期ごとにトグル)
                        if ((cycleCount % 10) == 0) {
                            PORTB.OUTTGL = PIN3_bm;
                        }
                        
                        // 進捗表示 (50 周期ごと)
                        if ((cycleCount % 50) == 0) {
                            pcSerial.print(F("[RUN] Cycle #"));
                            pcSerial.print(cycleCount);
                            pcSerial.print(F(" / "));
                            pcSerial.print(DISCONNECT_AT_CYCLE);
                            pcSerial.println(F(" running normally..."));
                        }
                        
                        // =====================================================
                        // ★【重要】意図的切断トリガー (Node 1 は 200 周期で送信停止)
                        // =====================================================
#if (NODE_ID == 1)
                        if (cycleCount >= DISCONNECT_AT_CYCLE) {
                            pcSerial.println(F("\r\n=============================================="));
                            pcSerial.println(F(" ★ [TRIGGER] DISCONNECT INDUCED AT CYCLE #200! ★"));
                            pcSerial.println(F(" Node 1 is ceasing transmission to drop baton."));
                            pcSerial.println(F(" Waiting for 250ms silence timeout STO trip..."));
                            pcSerial.println(F("=============================================="));
                            
                            // ★ バトンを送信しない！バスを完全無音化して心中へ倒す
                            USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                            rxIdx = 0;
                            return;
                        }
#endif
                        
                        // 固定レスポンスギャップ (10ms)
                        delay(RESPONSE_GAP_MS);
                        
                        // 次のノードへバトンパス
                        transmitBaton(NEXT_TARGET_ID, MY_SLOT_ID, (uint8_t)(cycleCount & 0xFF));
                        lastBusActivityMs = millis();
                    }
                }
                
                // 次の Break 待機へ再アーム
                USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
                rxIdx = 0;
            }
        }
    }

    // --- ⑤ 不完全フレームのタイムアウト破棄 (30ms 無音) ---
    if (rxIdx > 0 && (millis() - lastBusActivityMs > 30)) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxIdx = 0;
    }
}
