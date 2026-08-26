#include <Arduino.h>
#include <SoftwareSerial.h>

// =============================================================================
// ADX Core-D LN-485 テストスケッチ: TC-P3
// (Phase 3: Type A [Master Pub -> Slave Sub] 実証 ＆ Slave Subscriber 実装)
// =============================================================================
// 【役割設定】
// マスター機（スケジューラ ＆ Master-Pub 送信側）: #define ROLE_MASTER を有効化
// スレーブ機（Subscriber 受信 ＆ LED 制御側）     : // #define ROLE_MASTER のようにコメントアウト
// =============================================================================
#define ROLE_MASTER

// --- ピン定義 (ADX Core-D ハードウェア仕様準拠) ---
const int PIN_TXD = PIN_PA1; // USART0 TXD (RS-485 DI)
const int PIN_RXD = PIN_PA2; // USART0 RXD (RS-485 RO)
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH: 送信有効)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW: 受信有効)
const int LED_R   = PIN_PB2; // 赤色 LED (Master: 送信中 / Slave: ID=0x02 コマンド連動)
const int LED_W   = PIN_PB3; // 白色 LED (Master: パターン完了 / Slave: ID=0x03 コマンド連動)

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
// LIN Classic Checksum 計算・照合ヘルパー
// (データバイトの総和に桁上がりを加算し、ビット反転: sum + CS == 0xFF)
// =============================================================================
uint8_t calculateClassicChecksum(const uint8_t *data, uint8_t len) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += data[i];
    if (sum > 0xFF) {
      sum -= 0xFF; // キャリーの加算 (8bitラップアラウンド)
    }
  }
  return (uint8_t)(~sum);
}

bool verifyClassicChecksum(const uint8_t *data, uint8_t len, uint8_t rxChecksum) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += data[i];
    if (sum > 0xFF) {
      sum -= 0xFF;
    }
  }
  sum += rxChecksum;
  if (sum > 0xFF) {
    sum -= 0xFF;
  }
  return (sum == 0xFF);
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
// 【マスター専用】 Type A (Master-Publish) フレーム送出ルーチン
// =============================================================================
#ifdef ROLE_MASTER

void sendMasterPublishFrame(uint8_t id, const uint8_t *data, uint8_t len, bool corruptChecksum = false) {
  // 1. Break 信号送出
  sendLinBreak(BAUDRATE);

  // 2. Sync (0x55) & PID 送信
  Serial.write(0x55);
  Serial.write(calculatePID(id));

  // 3. ペイロードデータ送信
  for (uint8_t i = 0; i < len; i++) {
    Serial.write(data[i]);
  }

  // 4. チェックサム送信（意図的エラー時は反転）
  uint8_t cs = calculateClassicChecksum(data, len);
  if (corruptChecksum) {
    cs = ~cs; // 不正なチェックサムを生成
  }
  Serial.write(cs);

  // 5. 送信バッファの完全送出を待ってから受信モード (DE=0) へ解放
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
  pinMode(LED_R, OUTPUT);
  pinMode(LED_W, OUTPUT);

  setRxMode(); // 初期状態は受信モード

  // PC デバッグ用 SoftwareSerial 初期化 (9600 bps)
  pcSerial.begin(9600);

#ifdef ROLE_MASTER
  // -------------------------------------------------------------------------
  // 【マスター側】 Arduino の Serial オブジェクトを利用
  // -------------------------------------------------------------------------
  pinMode(PIN_RXD, INPUT);
  
  // ハードウェアシリアル(USART0)のピンを PA1(TXD) / PA2(RXD) にリマップ
  Serial.swap(1);
  Serial.begin(BAUDRATE);

  pcSerial.println(F("=================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P3              ==="));
  pcSerial.println(F("=== [MASTER] Master-Publish (Type A) Scheduler ==="));
  pcSerial.println(F("=================================================="));
  pcSerial.println(F("[Test Sequence]"));
  pcSerial.println(F("  Cycle 1: ID=0x02 Data=[0x01] -> Red LED ON"));
  pcSerial.println(F("  Cycle 2: ID=0x02 Data=[0x00] -> Red LED OFF"));
  pcSerial.println(F("  Cycle 3: ID=0x03 Data=[0x01] -> White LED ON"));
  pcSerial.println(F("  Cycle 4: ID=0x03 Data=[0x00] -> White LED OFF"));
  pcSerial.println(F("  Cycle 5: ID=0x02 Data=[0x01] (Corrupted CS) -> [TEST TC-P3-03]"));
  pcSerial.println(F("  Cycle 6: ID=0x04 Data=[0x12, 0x34, 0x56, 0x78] -> Multi-byte Test"));
  pcSerial.println(F("[Status] Scheduler running every 1500ms...\n"));

#else
  // -------------------------------------------------------------------------
  // 【スレーブ側】 ATtiny1616 ハードウェア LINAUTO モード直接レジスタ設定
  // -------------------------------------------------------------------------
  // Arduinoコアの割り込みとの競合を防ぐため、Serial.begin() は呼ばずに手動設定する

  // 1. ピンリマップ (USART0 を PA1: TXD, PA2: RXD に変更)
  PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
  
  // 2. ピンの入出力方向を手動設定
  PORTA.DIRSET = PIN1_bm; // PA1 (TX) = OUTPUT
  PORTA.DIRCLR = PIN2_bm; // PA2 (RX) = INPUT

  // 3. レジスタを安全に上書きするため、USART0 を一時無効化
  USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);

  // 4. USART0 CTRLC: 非同期, パリティ無効, 8データビット
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;

  // 5. USART0 CTRLB: LINAUTO (LIN制約付き自動ボーレート) モード, 送受信有効
  USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;

  // 6. 初期 BAUD 値を設定 (9600 bps 想定の目安値)
  USART0.BAUD = (uint16_t)(64 * (F_CPU / (16 * BAUDRATE)));

  // 7. STATUS.WFB をセット (直接代入により WFB セット & 残留フラグクリア)
  USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;

  pcSerial.println(F("=================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P3              ==="));
  pcSerial.println(F("=== [SLAVE] Subscriber Engine Active (Type A)  ==="));
  pcSerial.println(F("=================================================="));
  pcSerial.print(F("[Config] Initial USART0.BAUD: 0x"));
  pcSerial.println(USART0.BAUD, HEX);
  pcSerial.println(F("[Supported Topics / Commands]"));
  pcSerial.println(F("  - ID: 0x02 (Red LED Control: 0x01=ON, 0x00=OFF)"));
  pcSerial.println(F("  - ID: 0x03 (White LED Control: 0x01=ON, 0x00=OFF)"));
  pcSerial.println(F("  - ID: 0x04 (Multi-byte Telemetry: 4 Bytes)"));
  pcSerial.println(F("[Status] WFB=1. Listening for Type A frames on RS-485 bus...\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
#ifdef ROLE_MASTER
  // -------------------------------------------------------------------------
  // 【マスター側】 ミニマム・スケジューラによる Type A フレーム定期巡回送出
  // -------------------------------------------------------------------------
  static unsigned long lastSendTime = 0;
  static uint32_t patternCycle = 0;

  if (millis() - lastSendTime >= 1500) {
    lastSendTime = millis();
    patternCycle++;

    digitalWrite(LED_R, HIGH);

    uint8_t cycleType = (patternCycle % 6);

    if (cycleType == 1) {
      // 1. 赤 LED 点灯コマンド (ID=0x02, Data=0x01)
      uint8_t payload[1] = {0x01};
      sendMasterPublishFrame(0x02, payload, 1, false);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] Sent Type A: ID=0x02, Data=[0x01] (Red LED ON)"));

    } else if (cycleType == 2) {
      // 2. 赤 LED 消灯コマンド (ID=0x02, Data=0x00)
      uint8_t payload[1] = {0x00};
      sendMasterPublishFrame(0x02, payload, 1, false);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] Sent Type A: ID=0x02, Data=[0x00] (Red LED OFF)"));

    } else if (cycleType == 3) {
      // 3. 白 LED 点灯コマンド (ID=0x03, Data=0x01)
      uint8_t payload[1] = {0x01};
      sendMasterPublishFrame(0x03, payload, 1, false);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] Sent Type A: ID=0x03, Data=[0x01] (White LED ON)"));

    } else if (cycleType == 4) {
      // 4. 白 LED 消灯コマンド (ID=0x03, Data=0x00)
      uint8_t payload[1] = {0x00};
      sendMasterPublishFrame(0x03, payload, 1, false);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] Sent Type A: ID=0x03, Data=[0x00] (White LED OFF)"));

    } else if (cycleType == 5) {
      // 5. 【不正チェックサム試験 (TC-P3-03)】 赤 LED 点灯コマンド + 破損CS
      uint8_t payload[1] = {0x01};
      sendMasterPublishFrame(0x02, payload, 1, true); // 不正チェックサム送信
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] [TEST: CORRUPTED CS] Sent Type A: ID=0x02, Data=[0x01] + Inverted Checksum"));

    } else {
      // 6. 複数バイトデータ送信テスト (ID=0x04, 4 Bytes)
      uint8_t payload[4] = {0x12, 0x34, 0x56, 0x78};
      sendMasterPublishFrame(0x04, payload, 4, false);
      pcSerial.print(F("[Master Pattern #"));
      pcSerial.print(patternCycle);
      pcSerial.println(F("] Sent Type A: ID=0x04, Data=[0x12, 0x34, 0x56, 0x78] (Multi-byte Test)"));
    }

    digitalWrite(LED_R, LOW);
  }

#else
  // -------------------------------------------------------------------------
  // 【スレーブ側】 Subscriber 受信ステートマシン (Type A ペイロード ＆ CS 照合)
  // -------------------------------------------------------------------------
  enum SlaveRxState {
    STATE_WAIT_HEADER,     // ヘッダ (Break + Sync + PID) 待機
    STATE_RECEIVE_PAYLOAD  // ペイロード ＆ チェックサム受信待機
  };

  static SlaveRxState rxState = STATE_WAIT_HEADER;
  static uint8_t  currentRawId = 0;
  static uint8_t  currentPid   = 0;
  static uint8_t  expectedLen  = 0;
  static uint8_t  rxBuffer[10];
  static uint8_t  rxIndex      = 0;
  static unsigned long frameStartTime = 0;
  static uint32_t frameCount   = 0;

  // 1. 同期エラー (ISFIF) の検出と自動復帰
  if (USART0.STATUS & USART_ISFIF_bm) {
    pcSerial.println(F("[Slave Event] STATUS.ISFIF (Sync Error) DETECTED! -> Reset to WFB=1"));
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    rxState = STATE_WAIT_HEADER;
  }

  // 2. ペイロード受信中のタイムアウト監視 (100ms で強制復帰)
  if (rxState == STATE_RECEIVE_PAYLOAD) {
    if (millis() - frameStartTime > 100) {
      pcSerial.println(F("[Slave Timeout] Incomplete payload received -> Reset to WFB=1"));
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
      rxState = STATE_WAIT_HEADER;
    }
  }

  // 3. 受信割り込みフラグ (RXCIF) の処理（FIFO バッファ内の全データを順次取得）
  while (USART0.STATUS & USART_RXCIF_bm) {
    // ⚠️【最重要】必ず RXDATAH を先に読み出してから RXDATAL を読み出す
    uint8_t rxHigh = USART0.RXDATAH;
    uint8_t rxLow  = USART0.RXDATAL;

    bool isParityErr = (rxHigh & USART_PERR_bm);
    bool isFrameErr  = (rxHigh & USART_FERR_bm);

    if (isParityErr || isFrameErr) {
      // 受信エラー時: 直ちに待機状態へ復帰
      pcSerial.print(F("[Slave Error] RX Error (PERR: "));
      pcSerial.print(isParityErr ? F("1") : F("0"));
      pcSerial.print(F(", FERR: "));
      pcSerial.print(isFrameErr ? F("1") : F("0"));
      pcSerial.println(F(") -> Discarded & WFB=1"));

      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
      rxState = STATE_WAIT_HEADER;
      break;
    }

    if (rxState == STATE_WAIT_HEADER) {
      // ---------------------------------------------------------------------
      // ヘッダ (PID) 受信処理 (Break + 0x55 同期直後の第1バイト)
      // ---------------------------------------------------------------------
      currentPid   = rxLow;
      currentRawId = rxLow & 0x3F;

      // トピック ID に応じたペイロード長の決定
      if (currentRawId == 0x02 || currentRawId == 0x03) {
        expectedLen = 1; // 1バイトコマンド
      } else if (currentRawId == 0x04) {
        expectedLen = 4; // 4バイトデータ
      } else {
        expectedLen = 1; // デフォルト 1バイト
      }

      rxIndex = 0;
      frameStartTime = millis();
      rxState = STATE_RECEIVE_PAYLOAD; // ペイロード受信状態へ移行（※WFBはセットしない）

    } else if (rxState == STATE_RECEIVE_PAYLOAD) {
      // ---------------------------------------------------------------------
      // ペイロード / チェックサム受信処理 (PID 以降に到来する後続データバイト)
      // ---------------------------------------------------------------------
      if (rxIndex < sizeof(rxBuffer)) {
        rxBuffer[rxIndex++] = rxLow;
      }

      // ペイロード(expectedLen バイト) + チェックサム(1バイト) 受信完了判定
      if (rxIndex >= (expectedLen + 1)) {
        frameCount++;
        uint8_t receivedCs = rxBuffer[expectedLen];
        bool csValid = verifyClassicChecksum(rxBuffer, expectedLen, receivedCs);

        pcSerial.print(F("[Slave Frame #"));
        pcSerial.print(frameCount);
        pcSerial.print(F("] PID: 0x"));
        if (currentPid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(currentPid, HEX);
        pcSerial.print(F(" (ID: 0x0"));
        pcSerial.print(currentRawId, HEX);
        pcSerial.print(F(") | Payload: ["));
        for (uint8_t i = 0; i < expectedLen; i++) {
          pcSerial.print(F("0x"));
          if (rxBuffer[i] < 0x10) pcSerial.print(F("0"));
          pcSerial.print(rxBuffer[i], HEX);
          if (i < expectedLen - 1) pcSerial.print(F(", "));
        }
        pcSerial.print(F("] | CS: 0x"));
        if (receivedCs < 0x10) pcSerial.print(F("0"));
        pcSerial.print(receivedCs, HEX);

        if (csValid) {
          // 【チェックサム一致 (TC-P3-01, TC-P3-02, TC-P3-03)】
          pcSerial.println(F(" -> [ PASS / OK ]"));

          // コマンド実行 (アクチュエータ / LED 制御)
          if (currentRawId == 0x02) {
            // 赤 LED 制御
            bool turnOn = (rxBuffer[0] == 0x01);
            digitalWrite(LED_R, turnOn ? HIGH : LOW);
            pcSerial.print(F("  └─> [Action] Red LED turned "));
            pcSerial.println(turnOn ? F("ON") : F("OFF"));

          } else if (currentRawId == 0x03) {
            // 白 LED 制御
            bool turnOn = (rxBuffer[0] == 0x01);
            digitalWrite(LED_W, turnOn ? HIGH : LOW);
            pcSerial.print(F("  └─> [Action] White LED turned "));
            pcSerial.println(turnOn ? F("ON") : F("OFF"));
          }

        } else {
          // 【チェックサム不一致・エラー検出 (TC-P3-03)】
          pcSerial.println(F(" -> [ FAIL / CS MISMATCH ] (Discarded)"));
          pcSerial.println(F("  └─> [Safety] Frame discarded, no LED change."));
        }

        // 次フレームの待機状態へ復帰（WFBセット ＆ BDF/ISFIFクリア）
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        rxState = STATE_WAIT_HEADER;
        break; // このフレームの受信完了
      }
    }
  }
#endif
}
