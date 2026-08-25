#include <Arduino.h>
#include <SoftwareSerial.h>

// =============================================================================
// ADX Core-D LN-485 テストスケッチ: TC-P2
// (スレーブ LINAUTO ハードウェア自動同期 & PID パリティ・エラー検証)
// =============================================================================
// 【役割設定】
// マスター機（Header/テストパターン送出側）: #define ROLE_MASTER を有効化
// スレーブ機（LINAUTO ハードウェア同期側）  : // #define ROLE_MASTER のようにコメントアウト
// =============================================================================
#define ROLE_MASTER

// --- ピン定義 (ADX Core-D ハードウェア仕様準拠) ---
const int PIN_TXD = PIN_PA1; // USART0 TXD (RS-485 DI)
const int PIN_RXD = PIN_PA2; // USART0 RXD (RS-485 RO)
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH: 送信有効)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW: 受信有効)
const int LED_R   = PIN_PB2; // 赤色 LED (Master: 送信中 / Slave: エラー検知)
const int LED_W   = PIN_PB3; // 白色 LED (Slave: LINAUTO 同期・PID 正常検知)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信パラメータ ---
const uint32_t BAUDRATE = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud)) // 9600bps時: 約104.17 µs

// =============================================================================
// LIN PID (Protected Identifier) 計算ヘルパー
// =============================================================================
uint8_t calculatePID(uint8_t id) {
  uint8_t p0 = ((id >> 0) ^ (id >> 1) ^ (id >> 2) ^ (id >> 4)) & 0x01;
  uint8_t p1 = !(((id >> 1) ^ (id >> 3) ^ (id >> 4) ^ (id >> 5)) & 0x01);
  return (id & 0x3F) | (p0 << 6) | (p1 << 7);
}

// =============================================================================
// RS-485 半二重方向制御ヘルパー
// =============================================================================
void setTxMode() {
  digitalWrite(PIN_RE, HIGH); // 受信ディスエーブル
  digitalWrite(PIN_DE, HIGH); // 送信イネーブル
  delayMicroseconds(10);      // SP485EEN 切り替わり安定化待ち
}

void setRxMode() {
  digitalWrite(PIN_DE, LOW);  // 送信ディスエーブル
  digitalWrite(PIN_RE, LOW);  // 受信イネーブル
  delayMicroseconds(10);      // SP485EEN 切り替わり安定化待ち
}

// =============================================================================
// GPIO トグルによる LIN Break 信号送出関数
// =============================================================================
void sendLinBreak(uint32_t baud) {
  uint16_t tBit = BIT_TIME_US(baud);

  Serial.flush();
  setTxMode();

  USART0.CTRLB &= ~USART_TXEN_bm; // TX 一時無効化

  PORTA.DIRSET = PIN1_bm;
  PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
  delayMicroseconds(tBit * 14);

  PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (1 Tbit)
  delayMicroseconds(tBit * 1);

  USART0.CTRLB |= USART_TXEN_bm; // TX 再有効化
}

// =============================================================================
// 【マスター専用】 テストパターン別 ヘッダ送出関数
// =============================================================================
#ifdef ROLE_MASTER

// パターン 1: 正常ヘッダ (Break + Sync 0x55 + 有効PID 0xC1 [ID:0x01])
void sendNormalHeader(uint8_t id, uint32_t baud) {
  sendLinBreak(baud);
  Serial.write(0x55);
  Serial.write(calculatePID(id));
  Serial.flush();
  setRxMode();
}

// パターン 2: 不正 Sync ヘッダ (Break + 不正Sync 0xAA + 有効PID 0xC1) -> ISFIF テスト
void sendInvalidSyncHeader(uint8_t id, uint32_t baud) {
  sendLinBreak(baud);
  Serial.write(0xAA); // 不正 Sync
  Serial.write(calculatePID(id));
  Serial.flush();
  setRxMode();
}

// パターン 3: 不正パリティ PID ヘッダ (Break + Sync 0x55 + パリティ反転PID 0x01) -> PERR テスト
void sendInvalidParityHeader(uint8_t id, uint32_t baud) {
  sendLinBreak(baud);
  Serial.write(0x55);
  Serial.write((id & 0x3F)); // パリティビット P0, P1 を付加しない不正 PID
  Serial.flush();
  setRxMode();
}

#endif

// =============================================================================
// 初期化ルーチン (setup)
// =============================================================================
void setup() {
  pinMode(PIN_DE, OUTPUT);
  pinMode(PIN_RE, OUTPUT);
  pinMode(PIN_RXD, INPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_W, OUTPUT);

  setRxMode(); // 初期状態は受信モード

  // PC デバッグ用 SoftwareSerial 初期化 (9600 bps)
  pcSerial.begin(9600);

#ifdef ROLE_MASTER
  // ハードウェアシリアル(USART0)のピンを PA1(TXD) / PA2(RXD) にリマップ
  Serial.swap(1);
  Serial.begin(BAUDRATE);

  pcSerial.println(F("=================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P2              ==="));
  pcSerial.println(F("=== [MASTER] LIN Header Test Pattern Suite     ==="));
  pcSerial.println(F("=================================================="));
  pcSerial.println(F("[Test Sequence]"));
  pcSerial.println(F("  Cycle 1..3 : Valid Header (Break + 0x55 + PID 0xC1)"));
  pcSerial.println(F("  Cycle 4    : Invalid Sync Header (Break + 0xAA + PID 0xC1) [ISFIF Test]"));
  pcSerial.println(F("  Cycle 5    : Invalid Parity PID (Break + 0x55 + PID 0x01)  [PERR Test]"));
  pcSerial.println(F("  Cycle 6..  : Valid Header (Recovery Confirmation)"));
  pcSerial.println(F("[Status] Sending test patterns every 1200ms...\n"));

#else
  // -------------------------------------------------------------------------
  // 【スレーブ側】 ATtiny1616 ハードウェア LINAUTO モード直接レジスタ設定
  // -------------------------------------------------------------------------
  // 1. ピンリマップ (PA1: TXD, PA2: RXD) を確定させるため、一度beginを呼ぶ
  Serial.swap(1);
  Serial.begin(BAUDRATE);
  Serial.flush();

  // 2. レジスタを安全に上書きするため、USART0 を一時無効化
  USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);

  // 3. USART0 CTRLC: 非同期, パリティ無効, 8データビット
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;

  // 4. USART0 CTRLB: LINAUTO (LIN制約付き自動ボーレート) モード, 送受信有効
  USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;

  // 5. 初期 BAUD 値を設定 (9600 bps 想定の目安値)
  USART0.BAUD = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));

  // 6. STATUS.WFB をセット (|= ではなく = を使い、念のため他のエラーフラグもクリアする)
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  pcSerial.println(F("=================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P2              ==="));
  pcSerial.println(F("=== [SLAVE] Hardware LINAUTO Engine Active     ==="));
  pcSerial.println(F("=================================================="));
  pcSerial.print(F("[Config] Initial USART0.BAUD: 0x"));
  pcSerial.println(USART0.BAUD, HEX);
  pcSerial.println(F("[Status] WFB=1. Listening for Break on RS-485 bus...\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
#ifdef ROLE_MASTER
  // -------------------------------------------------------------------------
  // 【マスター側】 テストパターンの定期送出 (正常/異常シーケンス)
  // -------------------------------------------------------------------------
  static unsigned long lastSendTime = 0;
  static uint32_t patternCycle = 0;

  if (millis() - lastSendTime >= 1200) {
    lastSendTime = millis();
    patternCycle++;

    digitalWrite(LED_R, HIGH);

    uint8_t testId = 0x01; // ID=0x01 -> 正当PID=0xC1

    // パターンサイクル分岐
    uint8_t patternType = (patternCycle % 6);

    if (patternType == 4) {
      // 不正 Sync テスト (0xAA)
      sendInvalidSyncHeader(testId, BAUDRATE);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] [TEST: ISFIF] Sent Corrupted Sync (0xAA) + PID(0xC1)"));

    } else if (patternType == 5) {
      // 不正パリティ PID テスト (0x01)
      sendInvalidParityHeader(testId, BAUDRATE);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] [TEST: PERR]  Sent Corrupted Parity PID (0x01)"));

    } else {
      // 正常ヘッダ (Break + 0x55 + 0xC1)
      sendNormalHeader(testId, BAUDRATE);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] [TEST: NORMAL] Sent Valid Header: [Break] + [0x55] + [PID: 0xC1]"));
    }

    digitalWrite(LED_R, LOW);
  }

#else
  // -------------------------------------------------------------------------
  // 【スレーブ側】 ハードウェア LINAUTO エンジン・ステータス監視 & PID 判定
  // -------------------------------------------------------------------------
  static uint32_t recvEventCount = 0;

  // 1. 同期エラー (ISFIF) の検出と自動復帰
  if (USART0.STATUS & USART_ISFIF_bm) {
    recvEventCount++;
    digitalWrite(LED_R, HIGH); // エラー LED 点灯

    pcSerial.print(F("[Slave Event #"));
    pcSerial.print(recvEventCount);
    pcSerial.println(F("] STATUS.ISFIF (Inconsistent Sync) DETECTED! -> Cleared & WFB Rearmed"));

    // 次フレームの待機状態へ復帰（WFBセット ＆ BDF/ISFIFクリア）
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

    delay(30);
    digitalWrite(LED_R, LOW);
  }

  // 2. 受信データ (RXCIF) の検出
  if (USART0.STATUS & USART_RXCIF_bm) {
    recvEventCount++;

    // ⚠️【最重要】必ず RXDATAH を先に読み出してから RXDATAL を読み出す
    uint8_t rxHigh = USART0.RXDATAH;
    uint8_t rxLow  = USART0.RXDATAL;

    bool isParityErr  = (rxHigh & USART_PERR_bm);   // 1: Parity Error
    bool isFrameErr   = (rxHigh & USART_FERR_bm);   // 1: Framing Error
    bool isBreakDet   = (USART0.STATUS & USART_BDF_bm); // Break Detected Flag

    uint16_t autoBaud = USART0.BAUD; // ハードウェア自動補正後の BAUD レジスタ値

    pcSerial.print(F("[Slave Event #"));
    pcSerial.print(recvEventCount);
    pcSerial.print(F("] "));

    if (!isParityErr && !isFrameErr) {
      // 【正常 PID 受信 (TC-P2-01, TC-P2-02, TC-P2-03)】
      digitalWrite(LED_W, HIGH);

      pcSerial.print(F("PID Received: 0x"));
      if (rxLow < 0x10) pcSerial.print(F("0"));
      pcSerial.print(rxLow, HEX);
      pcSerial.print(F(" (ID: 0x0"));
      pcSerial.print(rxLow & 0x3F, HEX);
      pcSerial.print(F(") | AutoBAUD: 0x"));
      pcSerial.print(autoBaud, HEX);
      pcSerial.print(F(" | BDF: "));
      pcSerial.print(isBreakDet ? F("1") : F("0"));
      pcSerial.println(F(" -> [ PASS / OK ]"));

      delay(30);
      digitalWrite(LED_W, LOW);

    } else if (isParityErr) {
      // 【PID パリティエラー検出 (TC-P2-05)】
      digitalWrite(LED_R, HIGH);

      pcSerial.print(F("Corrupted PID: 0x"));
      if (rxLow < 0x10) pcSerial.print(F("0"));
      pcSerial.print(rxLow, HEX);
      pcSerial.println(F(" | PERR: 1 (Parity Error Detected!) -> Discarded"));

      delay(30);
      digitalWrite(LED_R, LOW);
      
    } else if (isFrameErr) {
      // 【フレーミングエラー検出】
      digitalWrite(LED_R, HIGH);
      pcSerial.println(F("Framing Error Detected! -> Discarded"));
      delay(30);
      digitalWrite(LED_R, LOW);
    }

    // 次フレームの待機状態へ復帰（WFBセット ＆ BDF/ISFIFクリア）
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
  }

  // -------------------------------------------------------------------------
  // デバッグ用ハートビート：2秒ごとにレジスタとピンの物理状態を報告
  // -------------------------------------------------------------------------
  //static unsigned long lastDebugTime = 0;
  //if (millis() - lastDebugTime >= 2000) {
  //  lastDebugTime = millis();
    
    // RXピン(PA2)の物理的な論理レベルを読み取る
  //  int rxPinState = digitalRead(PIN_RXD);
    
  //  pcSerial.print(F("[Debug] Alive | RXD Pin: "));
  //  pcSerial.print(rxPinState);
  //  pcSerial.print(F(" | USART0.STATUS: 0b"));
  //  pcSerial.println(USART0.STATUS, BIN);
  //}
#endif
}