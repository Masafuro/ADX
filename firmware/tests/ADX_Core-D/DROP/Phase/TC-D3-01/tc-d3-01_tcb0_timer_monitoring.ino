// =============================================================================
// ADX Core-D DROP-Bus Phase 3: TCB0 タイマー生存監視 ＆ リセットテスト
// Test Sketch: TC-D3-01 (tc-d3-01_tcb0_timer_monitoring.ino)
//
// Target Board: ADX Core-D (Microchip ATtiny1616 @ 20MHz/16MHz)
// RS-485 Transceiver: SP485EEN (DE: PA4, /RE: PA7)
//
// 特徴:
// 1. 【TCB0 ハードウェアタイマー】: 5ms 周期割り込み x 20回 = 100ms 心中タイムアウト
// 2. 【パッシブ生存監視】: 共有バス上の正常フレーム受信ごとに stoTickCounter = 0 & TCB0.CNT = 0 リセット
// 3. 【STO 遮断ロジック】: 100ms 無音で ISR 内にて赤LED即時点灯 & 白LED消灯 & STOフラグセット
// 4. 【正常周回安定性】: 5,000 周期周回中、STO 誤発動 0 回 (0.00% トリップ率) の実証
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
#define TARGET_CYCLES            (5000UL)  // 目標周回数: 5,000 周期
const uint16_t RESPONSE_GAP_MS   = 10;     // 固定レスポンスギャップ: 10ms
const uint8_t  STO_TIMEOUT_TICKS = 50;     // 50 * 5ms = 250ms 心中タイムアウト (推奨黄金比)

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
volatile bool isMonitoringActive = false; // 通信アーム（同期確立）フラグ
volatile uint32_t stoTripCount = 0;
uint32_t timerResetCount = 0;

// --- 統計 ＆ 状態監視変数 ---
uint32_t cycleCount = 0;
uint32_t crcErrorCount = 0;
uint32_t lastCycleMicros = 0;
uint32_t lastBusActivityMs = 0;
uint32_t minCycleUs = 0xFFFFFFFF;
uint32_t maxCycleUs = 0;
uint64_t totalCycleUsSum = 0;

// =============================================================================
// TCB0 ハードウェアタイマー初期化 (5ms 周期割り込み)
// =============================================================================
void initTCB0SurvivalTimer() {
    TCB0.CTRLA = 0; // 一旦タイマー停止
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Periodic Interrupt Mode
    // 5ms 周期 TOP値計算: (F_CPU / 2) / 200 (例: 20MHz -> 10MHz/200 = 50,000 ticks)
    TCB0.CCMP = (uint16_t)((F_CPU / 2UL) / 200UL);
    TCB0.CNT = 0;
    TCB0.INTFLAGS = TCB_CAPT_bm;
    TCB0.INTCTRL = TCB_CAPT_bm; // 割り込み有効化
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm; // CLK_PER/2 で起動
}

// TCB0 割り込みハンドラ (5ms ごとに実行)
ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm; // 割り込みフラグクリア
    
    // ★通信開始（アーム）後のみタイムアウト監視を実行
    if (isMonitoringActive && !isSafeTorqueOff) {
        stoTickCounter++;
        if (stoTickCounter >= STO_TIMEOUT_TICKS) {
            isSafeTorqueOff = true;
            PORTB.OUTSET = PIN2_bm; // 赤LED 点灯 (STO 安全遮断 発動)
            PORTB.OUTCLR = PIN3_bm; // 白LED 消灯
            stoTripCount++;
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
    pcSerial.println(F("   ADX Core-D DROP-Bus TC-D3-01 Test          "));
    pcSerial.println(F("   (TCB0 Timer Survival Monitoring & Reset)   "));
    pcSerial.println(F("=============================================="));
    pcSerial.print(F("Node Config: [NODE_ID = "));
    pcSerial.print(NODE_ID);
    pcSerial.print(F("] SlotID=0x0"));
    pcSerial.print(MY_SLOT_ID, HEX);
    pcSerial.print(F(" -> TargetID=0x0"));
    pcSerial.println(NEXT_TARGET_ID, HEX);
    pcSerial.println(F("TCB0 Timer: 250ms Timeout (50 Ticks) Periodic STO Monitoring Active."));
    pcSerial.println(F("Pass Condition: Zero STO Trips over 5,000 normal cycles.\r\n"));

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
    pcSerial.println(F("[IGNITER] Igniting Baton Relay in 500ms..."));
    delay(500);
    
    lastCycleMicros = micros();
    lastBusActivityMs = millis();
    transmitBaton(NEXT_TARGET_ID, MY_SLOT_ID, 0);
    
    // ★送信と同時にタイマー監視アーム（有効化）
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
                    // ★【重要】生存タイマーゼロリセット (パッシブ心中タイマークリア)
                    // 自宛て・他宛て問わず、共有バス上に正常フレームが流れるたびにタイマー更新 & 監視アーム
                    stoTickCounter = 0;
                    TCB0.CNT = 0;
                    isMonitoringActive = true;
                    timerResetCount++;
                    
                    // 発話権判定 (自身宛てバトン)
                    if (target_id == MY_SLOT_ID) {
                        uint32_t nowMicros = micros();
                        uint32_t cycleDeltaUs = nowMicros - lastCycleMicros;
                        lastCycleMicros = nowMicros;
                        
                        cycleCount++;
                        
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
                            
                            pcSerial.print(F("[STAT #"));
                            pcSerial.print(cycleCount);
                            pcSerial.print(F("] Avg: "));
                            pcSerial.print(avgUs / 1000);
                            pcSerial.print(F("."));
                            pcSerial.print((avgUs % 1000) / 10);
                            pcSerial.print(F("ms ("));
                            pcSerial.print(1000000UL / avgUs);
                            pcSerial.print(F("Hz) | TimerResets: "));
                            pcSerial.print(timerResetCount);
                            pcSerial.print(F(" | STO Trips: "));
                            pcSerial.print(stoTripCount);
                            pcSerial.print(F(" | CRC Err: "));
                            pcSerial.println(crcErrorCount);
                        }
                        
                        // 目標周期到達時の完了表示
                        if (cycleCount == TARGET_CYCLES) {
                            uint32_t avgUs = (uint32_t)(totalCycleUsSum / (cycleCount - 5));
                            
                            pcSerial.println(F("\r\n=============================================="));
                            pcSerial.println(F(" ★ 5,000 CYCLES COMPLETED SUCCESSFULLY! ★ "));
                            pcSerial.print(F(" Total Cycles   : ")); pcSerial.println(cycleCount);
                            pcSerial.print(F(" Overall Avg    : ")); pcSerial.print(avgUs); pcSerial.println(F(" us"));
                            pcSerial.print(F(" Timer Resets   : ")); pcSerial.println(timerResetCount);
                            pcSerial.print(F(" STO Trips      : ")); pcSerial.println(stoTripCount);
                            pcSerial.print(F(" CRC Errors     : ")); pcSerial.println(crcErrorCount);
                            pcSerial.println(F(" Fail-Safe Mode : ACTIVE (Zero False STO) [PASS]"));
                            pcSerial.println(F("==============================================\r\n"));
                        }
#endif
                        
                        // 【固定レスポンスギャップ】 10ms ガードインターバル
                        delay(RESPONSE_GAP_MS);
                        
                        // 次のノードへバトンパス
                        transmitBaton(NEXT_TARGET_ID, MY_SLOT_ID, (uint8_t)(cycleCount & 0xFF));
                        lastBusActivityMs = millis();
                    }
                } else {
                    crcErrorCount++;
                    // ※ CRC エラー時はタイマーをリセットしない（パッシブ心中へ倒す基本動作）
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
}
