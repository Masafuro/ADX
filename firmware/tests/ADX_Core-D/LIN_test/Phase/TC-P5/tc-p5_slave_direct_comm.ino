// =============================================================================
// ADX Core-D LN-485 Test Firmware - Phase 5 (TC-P5)
// Type C (Slave A Pub -> Slave B Sub) Direct Slave-to-Slave Communication & LN-485 UP/CS
// 
// Target MCU: ATtiny1616 (Internal 20MHz/16MHz OSC or 12MHz Ext-OSC)
// Transceiver: SP485EEN (Half-Duplex RS-485)
// 
// Copyright (c) 2026 ADX Project Contributors
// SPDX-License-Identifier: MIT
// =============================================================================

#include <Arduino.h>
#include <SoftwareSerial.h>

// =============================================================================
// 役割設定 (Role Configuration)
// =============================================================================
// ビルド対象のノードに合わせて、以下の 3 つのいずれか 1 つを有効化してください。
// -----------------------------------------------------------------------------
#define ROLE_MASTER     // Master Broker (Node #1) : スケジューラ場作り ＆ バス全傍受ログ出力
//#define ROLE_SLAVE_A  // Slave A: Publisher (Node #2) : Double Buffer Mailbox ＆ データパブリッシュ
//#define ROLE_SLAVE_B  // Slave B: Subscriber (Node #3) : LINAUTO 受信 ＆ LED/アクチュエータ直接制御

// ロール定義の整合性チェック
#if (defined(ROLE_MASTER) && defined(ROLE_SLAVE_A)) || \
    (defined(ROLE_MASTER) && defined(ROLE_SLAVE_B)) || \
    (defined(ROLE_SLAVE_A) && defined(ROLE_SLAVE_B))
  #error "Only one ROLE should be defined! (ROLE_MASTER, ROLE_SLAVE_A, or ROLE_SLAVE_B)"
#endif
#if !defined(ROLE_MASTER) && !defined(ROLE_SLAVE_A) && !defined(ROLE_SLAVE_B)
  #error "Please define exactly one ROLE! (ROLE_MASTER, ROLE_SLAVE_A, or ROLE_SLAVE_B)"
#endif

// =============================================================================
// ピン定義 (Hardware Pin Configuration)
// =============================================================================
const int PIN_TXD = PIN_PA1; // USART0 TXD (Alternate Pin)
const int PIN_RXD = PIN_PA2; // USART0 RXD (Alternate Pin)
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH = 送信有効)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW = 受信有効)
const int LED_R   = PIN_PB2; // 赤色 LED
const int LED_W   = PIN_PB3; // 白色 LED

// PCシリアルモニタ用 SoftwareSerial (RX: PB5, TX: PB4, 9600 bps)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// =============================================================================
// 通信パラメータ & プロトコル定数
// =============================================================================
const uint32_t BAUDRATE = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud))

// トピック ID 定義
const uint8_t TOPIC_TYPE_A_LED     = 0x02; // Type A: Master Pub -> Slave B Sub (LED制御 1B)
const uint8_t TOPIC_TYPE_B_UPTIME  = 0x03; // Type B: Slave A Pub -> Master Sub (稼働時間 4B)
const uint8_t TOPIC_TYPE_C_TRIGGER = 0x04; // Type C: Slave A Pub -> Slave B Sub (ダイレクトトリガー 1B)
const uint8_t TOPIC_TYPE_B_UNCONN  = 0x05; // Type B: 未接続ノード用 (タイムアウト検証用)

// =============================================================================
// ヘルパー関数: RS-485 半二重方向制御
// =============================================================================
void setTxMode() {
  digitalWrite(PIN_RE, HIGH); // 受信ディスエーブル
  digitalWrite(PIN_DE, HIGH); // 送信イネーブル
  delayMicroseconds(10);      // トランシーバーIC安定化待ち
}

void setRxMode() {
  digitalWrite(PIN_DE, LOW);  // 送信ディスエーブル
  digitalWrite(PIN_RE, LOW);  // 受信イネーブル
  delayMicroseconds(10);      // トランシーバーIC安定化待ち
}

// =============================================================================
// ヘルパー関数: LIN PID & Classic Checksum
// =============================================================================
uint8_t calculatePID(uint8_t id) {
  uint8_t p0 = ((id >> 0) ^ (id >> 1) ^ (id >> 2) ^ (id >> 4)) & 0x01;
  uint8_t p1 = !(((id >> 1) ^ (id >> 3) ^ (id >> 4) ^ (id >> 5)) & 0x01);
  return (id & 0x3F) | (p0 << 6) | (p1 << 7);
}

uint8_t calculateClassicChecksum(const uint8_t *data, uint8_t len) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += data[i];
    if (sum > 0xFF) sum -= 0xFF;
  }
  return (uint8_t)(~sum);
}

// =============================================================================
// スレーブ用: レジスタ直接送信 & フラッシュ関数 (Slave A Publisher用)
// =============================================================================
#ifdef ROLE_SLAVE_A
void slaveTxByte(uint8_t data) {
  while (!(USART0.STATUS & USART_DREIF_bm)); // 送信データレジスタ空き待ち
  USART0.TXDATAL = data;
}

void slaveTxFlush() {
  while (!(USART0.STATUS & USART_TXCIF_bm)); // シフトレジスタ送出完了待ち
  USART0.STATUS = USART_TXCIF_bm;           // TXCIF クリア (= で直接代入)
}

// Double Buffer Mailbox 構造体
struct DoubleBufferMailbox {
  uint8_t  topicId;
  uint8_t  length;
  uint8_t  payload[2][8];     // 2面バッファ ([0] と [1])
  uint8_t  checksum[2];       // 各面の Classic Checksum
  volatile uint8_t activeIdx; // 通信層が読み出すアクティブ面 (0 または 1)
};

// スレーブ A 側のメールボックス (Type B Uptime 4B / Type C Trigger 1B)
DoubleBufferMailbox uptimeMailbox;
DoubleBufferMailbox triggerMailbox;

// アプリ層からの非同期更新関数
void updateUptimeMailbox(uint32_t uptimeMs) {
  uint8_t nextIdx = uptimeMailbox.activeIdx ^ 1;
  uint8_t data[4];
  data[0] = (uptimeMs >> 0)  & 0xFF;
  data[1] = (uptimeMs >> 8)  & 0xFF;
  data[2] = (uptimeMs >> 16) & 0xFF;
  data[3] = (uptimeMs >> 24) & 0xFF;

  memcpy(uptimeMailbox.payload[nextIdx], data, 4);
  uptimeMailbox.checksum[nextIdx] = calculateClassicChecksum(data, 4);
  uptimeMailbox.length = 4;

  uint8_t sreg = SREG;
  cli();
  uptimeMailbox.activeIdx = nextIdx;
  SREG = sreg;
}

void updateTriggerMailbox(uint8_t triggerState) {
  uint8_t nextIdx = triggerMailbox.activeIdx ^ 1;
  uint8_t data[1] = { triggerState };

  memcpy(triggerMailbox.payload[nextIdx], data, 1);
  triggerMailbox.checksum[nextIdx] = calculateClassicChecksum(data, 1);
  triggerMailbox.length = 1;

  uint8_t sreg = SREG;
  cli();
  triggerMailbox.activeIdx = nextIdx;
  SREG = sreg;
}
#endif

// =============================================================================
// マスター用: Break 信号送出 (GPIO トグル法)
// =============================================================================
#ifdef ROLE_MASTER
void sendLinBreak(uint32_t baud) {
  uint16_t tBit = BIT_TIME_US(baud);
  
  Serial.flush();
  setTxMode();
  
  USART0.CTRLB &= ~USART_TXEN_bm; // USART TX 一時無効化
  
  PORTA.DIRSET = PIN1_bm;
  PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
  delayMicroseconds(tBit * 14);
  
  PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit - 安定同期のため延長)
  delayMicroseconds(tBit * 2);
  
  USART0.CTRLB |= USART_TXEN_bm;  // USART TX 再有効化
}
#endif

// =============================================================================
// 初期化ルーチン (setup)
// =============================================================================
void setup() {
  pinMode(PIN_DE, OUTPUT);
  pinMode(PIN_RE, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_W, OUTPUT);
  
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_W, LOW);
  
  setRxMode(); // 初期状態は受信モード
  pcSerial.begin(9600);

#ifdef ROLE_MASTER
  // ---------------------------------------------------------------------------
  // 【Master Broker 初期化】
  // ---------------------------------------------------------------------------
  pinMode(PIN_RXD, INPUT);
  Serial.swap(1); // PA1/PA2 にリマップ
  Serial.begin(BAUDRATE);

  pcSerial.println(F("========================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Master Broker (TC-P5)           ==="));
  pcSerial.println(F("========================================================="));
  pcSerial.println(F("[Ready] Scheduler started (Cycle: 1.5s, 4-Slots)"));
  pcSerial.println(F("  - Slot 0: Type A (Master Pub -> Slave B Sub: ID=0x02)"));
  pcSerial.println(F("  - Slot 1: Type B (Slave A Pub -> Master Sub: ID=0x03)"));
  pcSerial.println(F("  - Slot 2: Type C (Slave A Pub -> Slave B Sub: ID=0x04)"));
  pcSerial.println(F("  - Slot 3: Type B Timeout (Unconnected Node: ID=0x05)"));
  pcSerial.println(F("---------------------------------------------------------"));

#elif defined(ROLE_SLAVE_A)
  // ---------------------------------------------------------------------------
  // 【Slave A: Unique Publisher 初期化】
  // ---------------------------------------------------------------------------
  PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc; // PA1:TXD, PA2:RXD
  PORTA.DIRSET = PIN1_bm;                       // PA1 (TX) = OUTPUT
  PORTA.DIRCLR = PIN2_bm;                       // PA2 (RX) = INPUT

  USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
  USART0.BAUD  = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; // WFB アーム & フラグクリア

  // Double Buffer Mailbox 初期化
  uptimeMailbox.topicId = TOPIC_TYPE_B_UPTIME;
  uptimeMailbox.length = 4;
  uptimeMailbox.activeIdx = 0;
  updateUptimeMailbox(0);

  triggerMailbox.topicId = TOPIC_TYPE_C_TRIGGER;
  triggerMailbox.length = 1;
  triggerMailbox.activeIdx = 0;
  updateTriggerMailbox(0);

  pcSerial.println(F("========================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Slave A: Publisher (TC-P5)      ==="));
  pcSerial.println(F("========================================================="));
  pcSerial.println(F("[Ready] LINAUTO + Double Buffer Mailbox Active"));
  pcSerial.println(F("  - Responds to ID=0x03 (Type B Uptime: 4B)"));
  pcSerial.println(F("  - Publishes to ID=0x04 (Type C Trigger: 1B)"));
  pcSerial.println(F("---------------------------------------------------------"));

#elif defined(ROLE_SLAVE_B)
  // ---------------------------------------------------------------------------
  // 【Slave B: Common Subscriber 初期化】
  // ---------------------------------------------------------------------------
  PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc; // PA1:TXD, PA2:RXD
  PORTA.DIRSET = PIN1_bm;                       // PA1 (TX) = OUTPUT
  PORTA.DIRCLR = PIN2_bm;                       // PA2 (RX) = INPUT

  USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
  USART0.BAUD  = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; // WFB アーム & フラグクリア

  pcSerial.println(F("========================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Slave B: Subscriber (TC-P5)     ==="));
  pcSerial.println(F("========================================================="));
  pcSerial.println(F("[Ready] LINAUTO Subscriber Active"));
  pcSerial.println(F("  - Subscribes to ID=0x02 (Type A Master -> Slave B: Red LED)"));
  pcSerial.println(F("  - Subscribes to ID=0x04 (Type C Slave A -> Slave B: White LED)"));
  pcSerial.println(F("---------------------------------------------------------"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================

#ifdef ROLE_MASTER
// -----------------------------------------------------------------------------
// 【Master Broker】 スケジューラ ＆ プロミスキャス傍受 ＆ トラフィック監視
// -----------------------------------------------------------------------------
uint32_t lastCycleTime = 0;
uint8_t  schedulerSlot = 0;
bool     typeALedState = false;

void loop() {
  uint32_t now = millis();
  
  // 1.5秒ごとに次のスロットを実行
  if (now - lastCycleTime >= 1500) {
    lastCycleTime = now;
    
    switch (schedulerSlot) {
      // -----------------------------------------------------------------------
      // Slot 0: Type A (Master Pub -> Slave B Sub: LED コマンド送出)
      // -----------------------------------------------------------------------
      case 0: {
        typeALedState = !typeALedState;
        uint8_t payload[1] = { (uint8_t)(typeALedState ? 0x01 : 0x00) };
        uint8_t cs = calculateClassicChecksum(payload, 1);
        uint8_t pid = calculatePID(TOPIC_TYPE_A_LED);
        
        pcSerial.print(F("[Master Broker] [Slot 0] [Type A Pub] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.print(F(" (ID=0x02) CMD=0x"));
        pcSerial.print(payload[0], HEX);
        pcSerial.print(F(" (Slave B Red LED: "));
        pcSerial.print(typeALedState ? F("ON") : F("OFF"));
        pcSerial.println(F(")"));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.write(payload[0]);
        Serial.write(cs);
        Serial.flush();
        setRxMode();
        digitalWrite(LED_R, LOW);
        break;
      }

      // -----------------------------------------------------------------------
      // Slot 1: Type B (Slave A Pub -> Master Sub: Uptime 4B 応答要求 & 傍受)
      // -----------------------------------------------------------------------
      case 1: {
        uint8_t pid = calculatePID(TOPIC_TYPE_B_UPTIME);
        
        pcSerial.print(F("[Master Broker] [Slot 1] [Type B Poll] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.println(F(" (ID=0x03) -> Requesting Slave A Uptime..."));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.flush();
        
        // ヘッダ送出後、直ちに RX モードへ移行してスレーブ応答を待機
        setRxMode();
        digitalWrite(LED_R, LOW);
        
        // 15ms 非ブロッキング受信待機 (4バイトデータ + 1バイトCS = 5バイト)
        uint32_t rxStart = millis();
        uint8_t  rxBuf[5];
        uint8_t  rxCount = 0;
        
        while (millis() - rxStart < 15) {
          if (Serial.available()) {
            rxBuf[rxCount++] = Serial.read();
            if (rxCount >= 5) break;
          }
        }
        
        if (rxCount == 5) {
          uint8_t calcCs = calculateClassicChecksum(rxBuf, 4);
          if (calcCs == rxBuf[4]) {
            uint32_t slaveUptime = ((uint32_t)rxBuf[0]) |
                                   ((uint32_t)rxBuf[1] << 8) |
                                   ((uint32_t)rxBuf[2] << 16) |
                                   ((uint32_t)rxBuf[3] << 24);
            
            digitalWrite(LED_W, HIGH);
            pcSerial.print(F("  --> [Broker Recv] Slave A Uptime: "));
            pcSerial.print(slaveUptime);
            pcSerial.print(F(" ms (Raw: ["));
            for (int i = 0; i < 4; i++) {
              if (rxBuf[i] < 0x10) pcSerial.print(F("0"));
              pcSerial.print(rxBuf[i], HEX);
              if (i < 3) pcSerial.print(F(" "));
            }
            pcSerial.print(F("] CS=0x"));
            if (rxBuf[4] < 0x10) pcSerial.print(F("0"));
            pcSerial.print(rxBuf[4], HEX);
            pcSerial.println(F(" [PASS])"));
            digitalWrite(LED_W, LOW);
          } else {
            pcSerial.print(F("  --> [Broker Recv] Checksum Mismatch! Calc=0x"));
            pcSerial.print(calcCs, HEX);
            pcSerial.print(F(" Recv=0x"));
            pcSerial.println(rxBuf[4], HEX);
          }
        } else {
          pcSerial.print(F("  --> [Broker Timeout] Incomplete response. Recv bytes: "));
          pcSerial.println(rxCount);
        }
        break;
      }

      // -----------------------------------------------------------------------
      // Slot 2: Type C (Slave A Pub -> Slave B Sub: ダイレクト通信場作り & 傍受)
      // -----------------------------------------------------------------------
      case 2: {
        uint8_t pid = calculatePID(TOPIC_TYPE_C_TRIGGER);
        
        pcSerial.print(F("[Master Broker] [Slot 2] [Type C Header] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.println(F(" (ID=0x04) -> Broadcasting Header for Slave A -> Slave B..."));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.flush();
        
        // ヘッダ送出後、直ちに RX モードへ移行してスレーブ間通信をプロミスキャス傍受
        setRxMode();
        digitalWrite(LED_R, LOW);
        
        // 15ms 非ブロッキング受信待機 (1バイトデータ + 1バイトCS = 2バイト)
        uint32_t rxStart = millis();
        uint8_t  rxBuf[2];
        uint8_t  rxCount = 0;
        
        while (millis() - rxStart < 15) {
          if (Serial.available()) {
            rxBuf[rxCount++] = Serial.read();
            if (rxCount >= 2) break;
          }
        }
        
        if (rxCount == 2) {
          uint8_t calcCs = calculateClassicChecksum(rxBuf, 1);
          if (calcCs == rxBuf[1]) {
            digitalWrite(LED_W, HIGH);
            pcSerial.print(F("  --> [Broker Monitor TC-P5-02] Type C (Slave A -> Slave B): Trigger=0x"));
            if (rxBuf[0] < 0x10) pcSerial.print(F("0"));
            pcSerial.print(rxBuf[0], HEX);
            pcSerial.print(F(" (Slave B White LED: "));
            pcSerial.print(rxBuf[0] == 0x01 ? F("ON") : F("OFF"));
            pcSerial.print(F(") CS=0x"));
            if (rxBuf[1] < 0x10) pcSerial.print(F("0"));
            pcSerial.print(rxBuf[1], HEX);
            pcSerial.println(F(" [PASS - Direct Comm Intercepted]"));
            digitalWrite(LED_W, LOW);
          } else {
            pcSerial.print(F("  --> [Broker Monitor Error] Checksum Mismatch! Calc=0x"));
            pcSerial.print(calcCs, HEX);
            pcSerial.print(F(" Recv=0x"));
            pcSerial.println(rxBuf[1], HEX);
          }
        } else {
          pcSerial.print(F("  --> [Broker Monitor Timeout] No Slave A Publish detected. Recv bytes: "));
          pcSerial.println(rxCount);
        }
        break;
      }

      // -----------------------------------------------------------------------
      // Slot 3: Type B タイムアウト耐性検証 (未接続ノード ID=0x05)
      // -----------------------------------------------------------------------
      case 3: {
        uint8_t pid = calculatePID(TOPIC_TYPE_B_UNCONN);
        
        pcSerial.print(F("[Master Broker] [Slot 3] [Timeout Test] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.println(F(" (ID=0x05) -> Polling Unconnected Node..."));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.flush();
        
        setRxMode();
        digitalWrite(LED_R, LOW);
        
        uint32_t rxStart = millis();
        uint8_t  rxCount = 0;
        while (millis() - rxStart < 15) {
          if (Serial.available()) {
            Serial.read();
            rxCount++;
          }
        }
        
        if (rxCount == 0) {
          pcSerial.println(F("  --> [PASS / EXPECTED TIMEOUT] Master Broker recovered safely in 15ms."));
        } else {
          pcSerial.print(F("  --> [UNEXPECTED RECV] Bytes: "));
          pcSerial.println(rxCount);
        }
        break;
      }
    }
    
    // スロット切り替え (0 -> 1 -> 2 -> 3 -> 0)
    schedulerSlot = (schedulerSlot + 1) % 4;
  }
}

#elif defined(ROLE_SLAVE_A)
// -----------------------------------------------------------------------------
// 【Slave A: Unique Publisher】 Double Buffer Mailbox 更新 ＆ パブリッシュ
// -----------------------------------------------------------------------------
enum SlaveARxState {
  STATE_WAIT_HEADER,
  STATE_IGNORE_PAYLOAD
};

SlaveARxState slaveAState = STATE_WAIT_HEADER;
uint32_t lastMailboxUpdate = 0;
uint32_t lastTriggerToggle = 0;
uint8_t  currentTriggerState = 0;
uint32_t rxIgnoreStartTime = 0;

void loop() {
  uint32_t now = millis();

  // 1. 【アプリ層】 100msごとに Uptime (Type B) を裏バッファへ更新
  if (now - lastMailboxUpdate >= 100) {
    lastMailboxUpdate = now;
    updateUptimeMailbox(now);
  }

  // 2. 【アプリ層】 1500msごとに Trigger (Type C) をトグル更新 (0x01 <-> 0x00)
  if (now - lastTriggerToggle >= 1500) {
    lastTriggerToggle = now;
    currentTriggerState ^= 0x01;
    updateTriggerMailbox(currentTriggerState);
    digitalWrite(LED_R, currentTriggerState ? HIGH : LOW); // 赤LEDでトリガー状態表示
  }

  // 無視モードのタイムアウト復帰処理（20msで安全に待機モードへ戻る）
  if (slaveAState == STATE_IGNORE_PAYLOAD && (now - rxIgnoreStartTime > 20)) {
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    slaveAState = STATE_WAIT_HEADER;
  }

  // 3. 【通信層】 高速ポーリング処理
  // RXCIF(受信完了)を待つ前に、ISFIFエラーでロックされていないか常に監視・復帰する
  if (USART0.STATUS & USART_ISFIF_bm) {
    pcSerial.println(F("[Slave A Warn] ISFIF Sync Error! Auto-recovered."));
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    slaveAState = STATE_WAIT_HEADER;
  }

  while (USART0.STATUS & USART_RXCIF_bm) {
    uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
    uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得

    switch (slaveAState) {
      case STATE_WAIT_HEADER: {
        // パリティエラー・フレーミングエラーのチェック
        if (rxHigh & (USART_PERR_bm | USART_FERR_bm | USART_BUFOVF_bm)) {
          pcSerial.println(F("[Slave A Error] Header PERR/FERR/OVF detected. Discarding."));
          slaveAState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
          break;
        }

        uint8_t currentPID = rxLow;
        uint8_t currentID  = currentPID & 0x3F;

        // パリティ合致確認
        if (calculatePID(currentID) != currentPID) {
          pcSerial.println(F("[Slave A Error] PID Parity Mismatch. Discarding."));
          slaveAState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
          break;
        }

        // ---------------------------------------------------------------------
        // 【Type B 応答】 ID=0x03 (Slave A Pub -> Master Sub: Uptime 4B 返信)
        // ---------------------------------------------------------------------
        if (currentID == TOPIC_TYPE_B_UPTIME) {
          delayMicroseconds(60); // レスポンススペース待機 (50〜60µs)

          uint8_t curIdx = uptimeMailbox.activeIdx; // 表バッファ取得 (Zero-Copy)

          setTxMode();
          digitalWrite(LED_W, HIGH); // 白色 LED 点灯 (送信中)

          slaveTxByte(uptimeMailbox.payload[curIdx][0]);
          slaveTxByte(uptimeMailbox.payload[curIdx][1]);
          slaveTxByte(uptimeMailbox.payload[curIdx][2]);
          slaveTxByte(uptimeMailbox.payload[curIdx][3]);
          slaveTxByte(uptimeMailbox.checksum[curIdx]);

          slaveTxFlush();
          setRxMode();
          digitalWrite(LED_W, LOW);

          uint32_t sentUptime = ((uint32_t)uptimeMailbox.payload[curIdx][0]) |
                                ((uint32_t)uptimeMailbox.payload[curIdx][1] << 8) |
                                ((uint32_t)uptimeMailbox.payload[curIdx][2] << 16) |
                                ((uint32_t)uptimeMailbox.payload[curIdx][3] << 24);

          pcSerial.print(F("[Slave A Pub Sent] Type B (ID=0x03) Uptime: "));
          pcSerial.print(sentUptime);
          pcSerial.print(F(" ms, CS: 0x"));
          pcSerial.println(uptimeMailbox.checksum[curIdx], HEX);

          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          slaveAState = STATE_WAIT_HEADER;
        }
        // ---------------------------------------------------------------------
        // 【Type C パブリッシュ】 ID=0x04 (Slave A Pub -> Slave B Sub: Trigger 1B)
        // ---------------------------------------------------------------------
        else if (currentID == TOPIC_TYPE_C_TRIGGER) {
          delayMicroseconds(60); // レスポンススペース待機 (50〜60µs)

          uint8_t curIdx = triggerMailbox.activeIdx; // 表バッファ取得 (Zero-Copy)

          setTxMode();
          digitalWrite(LED_W, HIGH); // 白色 LED 点灯 (送信中)

          slaveTxByte(triggerMailbox.payload[curIdx][0]);
          slaveTxByte(triggerMailbox.checksum[curIdx]);

          slaveTxFlush();
          setRxMode();
          digitalWrite(LED_W, LOW);

          pcSerial.print(F("[Slave A Pub Sent TC-P5-01] Type C (ID=0x04) Trigger: 0x"));
          if (triggerMailbox.payload[curIdx][0] < 0x10) pcSerial.print(F("0"));
          pcSerial.print(triggerMailbox.payload[curIdx][0], HEX);
          pcSerial.print(F(" ("));
          pcSerial.print(triggerMailbox.payload[curIdx][0] == 0x01 ? F("ON") : F("OFF"));
          pcSerial.print(F("), CS: 0x"));
          pcSerial.println(triggerMailbox.checksum[curIdx], HEX);

          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          slaveAState = STATE_WAIT_HEADER;
        }
        // ---------------------------------------------------------------------
        // 自ノードに無関係な ID の場合 (ID=0x02, 0x05 等) -> WFBをセットせず無視モードへ
        // ---------------------------------------------------------------------
        else {
          slaveAState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
        }
        break;
      }

      case STATE_IGNORE_PAYLOAD: {
        // 無視モード中は受信データを単に破棄（FIFOを進める）
        break;
      }
    }
  }
}

#elif defined(ROLE_SLAVE_B)
// -----------------------------------------------------------------------------
// 【Slave B: Common Subscriber】 LINAUTO 受信 ＆ アクチュエータ/LED 直接制御
// -----------------------------------------------------------------------------
enum SlaveBRxState {
  STATE_WAIT_HEADER,
  STATE_RECV_TYPE_A_PAYLOAD,
  STATE_RECV_TYPE_C_PAYLOAD,
  STATE_IGNORE_PAYLOAD
};

SlaveBRxState slaveBState = STATE_WAIT_HEADER;
uint8_t payloadBuf[4];
uint8_t payloadIdx = 0;
uint8_t expectedPayloadLen = 0;
uint32_t rxPayloadStartTime = 0;
uint32_t rxIgnoreStartTime = 0;

void loop() {
  uint32_t now = millis();

  // ペイロード受信タイムアウト監視 (20ms以上受信完了しなければ待機へ安全復帰)
  if ((slaveBState == STATE_RECV_TYPE_A_PAYLOAD || slaveBState == STATE_RECV_TYPE_C_PAYLOAD) && 
      (now - rxPayloadStartTime > 20)) {
    pcSerial.println(F("[Slave B Warn] Payload Rx Timeout. Returning to Wait Header."));
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    slaveBState = STATE_WAIT_HEADER;
  }

  // 無視モードのタイムアウト復帰処理（20msで安全に待機モードへ戻る）
  if (slaveBState == STATE_IGNORE_PAYLOAD && (now - rxIgnoreStartTime > 20)) {
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    slaveBState = STATE_WAIT_HEADER;
  }

  // 3. 【通信層】 高速ポーリング処理
  // RXCIF(受信完了)を待つ前に、ISFIFエラーでロックされていないか常に監視・復帰する
  if (USART0.STATUS & USART_ISFIF_bm) {
    pcSerial.println(F("[Slave B Warn] ISFIF Sync Error! Auto-recovered."));
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    slaveBState = STATE_WAIT_HEADER;
  }

  while (USART0.STATUS & USART_RXCIF_bm) {
    uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
    uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得

    switch (slaveBState) {
      // -----------------------------------------------------------------------
      // ヘッダ (PID) 待機状態
      // -----------------------------------------------------------------------
      case STATE_WAIT_HEADER: {
        // パリティエラー・フレーミングエラーのチェック
        if (rxHigh & (USART_PERR_bm | USART_FERR_bm | USART_BUFOVF_bm)) {
          pcSerial.println(F("[Slave B Error] Header PERR/FERR/OVF detected. Discarding."));
          slaveBState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
          break;
        }

        uint8_t currentPID = rxLow;
        uint8_t currentID  = currentPID & 0x3F;

        // パリティ合致確認
        if (calculatePID(currentID) != currentPID) {
          pcSerial.println(F("[Slave B Error] PID Parity Mismatch. Discarding."));
          slaveBState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
          break;
        }

        // ---------------------------------------------------------------------
        // 【Type A 受信】 ID=0x02 (Master Pub -> Slave B Sub: 赤LED制御 1B)
        // ---------------------------------------------------------------------
        if (currentID == TOPIC_TYPE_A_LED) {
          slaveBState = STATE_RECV_TYPE_A_PAYLOAD;
          expectedPayloadLen = 1; // 1バイトデータ + 1バイトCS = 2バイト
          payloadIdx = 0;
          rxPayloadStartTime = now;
        }
        // ---------------------------------------------------------------------
        // 【Type C 受信】 ID=0x04 (Slave A Pub -> Slave B Sub: 白LED直接制御 1B)
        // ---------------------------------------------------------------------
        else if (currentID == TOPIC_TYPE_C_TRIGGER) {
          slaveBState = STATE_RECV_TYPE_C_PAYLOAD;
          expectedPayloadLen = 1; // 1バイトデータ + 1バイトCS = 2バイト
          payloadIdx = 0;
          rxPayloadStartTime = now;
        }
        // ---------------------------------------------------------------------
        // 自ノードに無関係な ID の場合 (ID=0x03, 0x05 等) -> WFBをセットせず無視モードへ
        // ---------------------------------------------------------------------
        else {
          slaveBState = STATE_IGNORE_PAYLOAD;
          rxIgnoreStartTime = now;
        }
        break;
      }

      // -----------------------------------------------------------------------
      // Type A ペイロード受信状態 (Master -> Slave B: 赤LED)
      // -----------------------------------------------------------------------
      case STATE_RECV_TYPE_A_PAYLOAD: {
        payloadBuf[payloadIdx++] = rxLow;

        // ペイロード(1B) + チェックサム(1B) 受信完了
        if (payloadIdx >= expectedPayloadLen + 1) {
          uint8_t calcCs = calculateClassicChecksum(payloadBuf, expectedPayloadLen);
          uint8_t recvCs = payloadBuf[expectedPayloadLen];

          if (calcCs == recvCs) {
            bool ledCmd = (payloadBuf[0] == 0x01);
            digitalWrite(LED_R, ledCmd ? HIGH : LOW);

            pcSerial.print(F("[Slave B Sub Recv OK] Type A (ID=0x02) Master Cmd -> Red LED: "));
            pcSerial.println(ledCmd ? F("ON") : F("OFF"));
          } else {
            pcSerial.println(F("[Slave B Sub Error] Type A Checksum Mismatch!"));
          }

          // 次フレームのブレーク待機へ復帰
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          slaveBState = STATE_WAIT_HEADER;
        }
        break;
      }

      // -----------------------------------------------------------------------
      // Type C ペイロード受信状態 (Slave A -> Slave B 直接制御: 白LED)
      // -----------------------------------------------------------------------
      case STATE_RECV_TYPE_C_PAYLOAD: {
        payloadBuf[payloadIdx++] = rxLow;

        // ペイロード(1B) + チェックサム(1B) 受信完了
        if (payloadIdx >= expectedPayloadLen + 1) {
          uint8_t calcCs = calculateClassicChecksum(payloadBuf, expectedPayloadLen);
          uint8_t recvCs = payloadBuf[expectedPayloadLen];

          if (calcCs == recvCs) {
            bool triggerCmd = (payloadBuf[0] == 0x01);
            digitalWrite(LED_W, triggerCmd ? HIGH : LOW); // 白LEDを直接制御

            pcSerial.print(F("[Slave B Sub Recv OK TC-P5-01] Type C (ID=0x04) Slave A Direct -> White LED: "));
            pcSerial.println(triggerCmd ? F("ON") : F("OFF"));
          } else {
            pcSerial.println(F("[Slave B Sub Error] Type C Checksum Mismatch!"));
          }

          // 次フレームのブレーク待機へ復帰
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          slaveBState = STATE_WAIT_HEADER;
        }
        break;
      }

      // -----------------------------------------------------------------------
      // 無視モード (他ノード宛てペイロード通過待ち)
      // -----------------------------------------------------------------------
      case STATE_IGNORE_PAYLOAD: {
        // 無視モード中は受信データを単に破棄（FIFOを進める）
        break;
      }
    }
  }
}
#endif
