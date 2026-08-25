#include <Arduino.h>
#include <SoftwareSerial.h>

// =============================================================================
// ADX Core-D LN-485 テストスケッチ: TC-P1-02 (Sync 0x55 & PID 送出・受信検証)
// =============================================================================
// 【役割設定】
// マスター機（Header送出側）に書き込む場合 : #define ROLE_MASTER を有効化
// スレーブ機（Header受信側）に書き込む場合 : // #define ROLE_MASTER のようにコメントアウト
// =============================================================================
#define ROLE_MASTER

// --- ピン定義 (ADX Core-D ハードウェア仕様準拠) ---
const int PIN_TXD = PIN_PA1; // USART0 TXD (RS-485 DI)
const int PIN_RXD = PIN_PA2; // USART0 RXD (RS-485 RO)
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH: 送信有効)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW: 受信有効)
const int LED_R   = PIN_PB2; // 赤色 LED (マスター: 送信中インジケータ)
const int LED_W   = PIN_PB3; // 白色 LED (スレーブ: 受信検知インジケータ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信パラメータ ---
const uint32_t BAUDRATE = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud)) // 9600bps時: 約104.17 µs

// テスト対象 ID
const uint8_t TEST_ID = 0x01; // 例: ID = 0x01 -> PID = 0x81

// =============================================================================
// LIN PID (Protected Identifier) 計算・検証ヘルパー
// =============================================================================
// LIN 規格パリティ方程式:
//   P0 = ID0 ^ ID1 ^ ID2 ^ ID4
//   P1 = !(ID1 ^ ID3 ^ ID4 ^ ID5)
//   PID = (ID & 0x3F) | (P0 << 6) | (P1 << 7)
// =============================================================================
uint8_t calculatePID(uint8_t id) {
  uint8_t p0 = ((id >> 0) ^ (id >> 1) ^ (id >> 2) ^ (id >> 4)) & 0x01;
  uint8_t p1 = !(((id >> 1) ^ (id >> 3) ^ (id >> 4) ^ (id >> 5)) & 0x01);
  return (id & 0x3F) | (p0 << 6) | (p1 << 7);
}

bool verifyPID(uint8_t pid, uint8_t &extractedId) {
  extractedId = pid & 0x3F;
  uint8_t expectedPid = calculatePID(extractedId);
  return (pid == expectedPid);
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

  // 1. 直前の送信が完全に完了するまで待機
  Serial.flush();

  // 2. RS-485 送信モードへ設定
  setTxMode();

  // 3. USART TX 機能を一時的に無効化 (PA1 ピンを汎用 GPIO 出力へ開放)
  USART0.CTRLB &= ~USART_TXEN_bm;

  // 4. PA1 を LOW に駆動 (Break Field: 14 Tbit)
  PORTA.DIRSET = PIN1_bm;
  PORTA.OUTCLR = PIN1_bm;
  delayMicroseconds(tBit * 14);

  // 5. PA1 を HIGH に駆動 (Break Delimiter: 1 Tbit)
  PORTA.OUTSET = PIN1_bm;
  delayMicroseconds(tBit * 1);

  // 6. USART TX 機能を再有効化
  USART0.CTRLB |= USART_TXEN_bm;
}

// =============================================================================
// 完全な LIN Header 送出関数 (Master専用: Break + Sync 0x55 + PID)
// =============================================================================
void sendLinHeader(uint8_t id, uint32_t baud) {
  // 1. Break 送出 (setTxMode 含む)
  sendLinBreak(baud);

  // 2. Sync キャラクタ (0x55) 送信
  Serial.write(0x55);

  // 3. Protected Identifier (PID) 送信
  uint8_t pid = calculatePID(id);
  Serial.write(pid);

  // 4. 送信完了を待機し、バスを解放 (受信モードへ戻す)
  Serial.flush();
  setRxMode();
}

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

  // ハードウェアシリアル(USART0)のピンを PA1(TXD) / PA2(RXD) にリマップ
  Serial.swap(1);
  Serial.begin(BAUDRATE);

#ifdef ROLE_MASTER
  pcSerial.println(F("=============================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P1-02       ==="));
  pcSerial.println(F("=== [MASTER] LIN Header Generator          ==="));
  pcSerial.println(F("=============================================="));
  pcSerial.print(F("[Config] BaudRate: "));
  pcSerial.print(BAUDRATE);
  pcSerial.println(F(" bps"));
  pcSerial.print(F("[Config] Header Target: Break + Sync(0x55) + PID(0x"));
  uint8_t pid = calculatePID(TEST_ID);
  if (pid < 0x10) pcSerial.print(F("0"));
  pcSerial.print(pid, HEX);
  pcSerial.print(F(" [ID:0x0"));
  pcSerial.print(TEST_ID, HEX);
  pcSerial.println(F("])"));
  pcSerial.println(F("[Status] Sending Header every 1000ms...\n"));

#else
  pcSerial.println(F("=============================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P1-02       ==="));
  pcSerial.println(F("=== [SLAVE] Header Reception & Parser      ==="));
  pcSerial.println(F("=============================================="));
  pcSerial.print(F("[Config] BaudRate: "));
  pcSerial.print(BAUDRATE);
  pcSerial.println(F(" bps (Standard UART Mode)"));
  pcSerial.println(F("[Target] Expected Frame: [Break/0x00] + [0x55] + [PID: 0x81 (ID:0x01)]"));
  pcSerial.println(F("[Status] Listening on RS-485 bus...\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
#ifdef ROLE_MASTER
  // -------------------------------------------------------------------------
  // 【マスター側】 1秒周期で LIN Header (Break + 0x55 + PID) を送出
  // -------------------------------------------------------------------------
  static unsigned long lastSendTime = 0;
  static uint32_t headerCount = 0;

  if (millis() - lastSendTime >= 1000) {
    lastSendTime = millis();
    headerCount++;

    digitalWrite(LED_R, HIGH); // 送信中 LED 点灯

    // LIN Header 一括送出
    sendLinHeader(TEST_ID, BAUDRATE);

    digitalWrite(LED_R, LOW);  // 送信完了 LED 消灯

    uint8_t pid = calculatePID(TEST_ID);

    // PC シリアルモニタへログ出力
    pcSerial.print(F("[Master Uptime: "));
    pcSerial.print(millis());
    pcSerial.print(F(" ms] #"));
    pcSerial.print(headerCount);
    pcSerial.print(F(" Sent Header: [Break] + [0x55] + [PID: 0x"));
    if (pid < 0x10) pcSerial.print(F("0"));
    pcSerial.print(pid, HEX);
    pcSerial.println(F("]"));
  }

#else
  // -------------------------------------------------------------------------
  // 【スレーブ側】 標準 UART による Header バイト列の受信・パース・パリティ検証
  // -------------------------------------------------------------------------
  static uint8_t rxBuf[8];
  static uint8_t rxIndex = 0;
  static unsigned long lastByteTime = 0;
  static uint32_t recvCount = 0;

  // データ受信
  while (Serial.available() > 0) {
    uint8_t b = Serial.read();
    lastByteTime = millis();

    if (rxIndex < sizeof(rxBuf)) {
      rxBuf[rxIndex++] = b;
    }
  }

  // フレーム間タイムアウト判定 (最後のバイト受信から 30ms 経過で 1フレーム確定)
  if (rxIndex > 0 && (millis() - lastByteTime >= 30)) {
    recvCount++;
    digitalWrite(LED_W, HIGH); // 受信検知 LED 点灯

    pcSerial.print(F("[Slave Recv #"));
    pcSerial.print(recvCount);
    pcSerial.print(F("] Raw Bytes ("));
    pcSerial.print(rxIndex);
    pcSerial.print(F("B): ["));

    for (uint8_t i = 0; i < rxIndex; i++) {
      pcSerial.print(F("0x"));
      if (rxBuf[i] < 0x10) pcSerial.print(F("0"));
      pcSerial.print(rxBuf[i], HEX);
      if (i < rxIndex - 1) pcSerial.print(F(", "));
    }
    pcSerial.print(F("] -> "));

    // ヘッダ解析
    // 期待パターン: [0x00(Break)] + [0x55(Sync)] + [PID] (または [0x55] + [PID])
    bool syncOk = false;
    bool pidOk = false;
    uint8_t receivedPid = 0;
    uint8_t extractedId = 0;

    for (uint8_t i = 0; i < rxIndex; i++) {
      if (rxBuf[i] == 0x55 && (i + 1 < rxIndex)) {
        syncOk = true;
        receivedPid = rxBuf[i + 1];
        pidOk = verifyPID(receivedPid, extractedId);
        break;
      }
    }

    if (syncOk && pidOk) {
      pcSerial.print(F("Sync(0x55): OK, PID(0x"));
      if (receivedPid < 0x10) pcSerial.print(F("0"));
      pcSerial.print(receivedPid, HEX);
      pcSerial.print(F(" / ID:0x0"));
      pcSerial.print(extractedId, HEX);
      pcSerial.println(F(") Parity: OK -> [ PASS / OK ]"));
    } else {
      if (!syncOk) pcSerial.print(F("Sync: MISSING, "));
      if (syncOk && !pidOk) pcSerial.print(F("PID Parity: FAIL, "));
      pcSerial.println(F("-> [ FAIL / NG ]"));
    }

    // バッファリセット
    rxIndex = 0;
    delay(20);
    digitalWrite(LED_W, LOW);
  }
#endif
}
