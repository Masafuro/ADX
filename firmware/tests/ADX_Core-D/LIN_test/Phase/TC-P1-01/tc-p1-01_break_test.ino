#include <Arduino.h>
#include <SoftwareSerial.h>

// =============================================================================
// ADX Core-D LN-485 テストスケッチ: TC-P1-01 (GPIO Break 信号波形検証)
// =============================================================================
// 【役割設定】
// マスター機（Break送出側）に書き込む場合 : #define ROLE_MASTER を有効化
// スレーブ機（Break測定側）に書き込む場合 : // #define ROLE_MASTER のようにコメントアウト
// =============================================================================
#define ROLE_MASTER

// --- ピン定義 (ADX Core-D ハードウェア仕様準拠) ---
const int PIN_TXD = PIN_PA1; // USART0 TXD (RS-485 DI)
const int PIN_RXD = PIN_PA2; // USART0 RXD (RS-485 RO)
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH: 送信有効)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW: 受信有効)
const int LED_R   = PIN_PB2; // 赤色 LED (マスター: 送信中インジケータ)
const int LED_W   = PIN_PB3; // 白色 LED (スレーブ: Break検知インジケータ)

// PCデバッグモニタ用 SoftwareSerial (RX: PB5, TX: PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// --- 通信速度およびブレーク設計値 ---
const uint32_t BAUDRATE = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud)) // 9600bps時: 約104.17 µs
const uint32_t TARGET_BREAK_US = BIT_TIME_US(BAUDRATE) * 14; // 14 Tbit = 1458 µs
const uint32_t MIN_PASS_US    = BIT_TIME_US(BAUDRATE) * 11; // 11 Tbit (LIN規格下限: 1146 µs)
const uint32_t MAX_PASS_US    = BIT_TIME_US(BAUDRATE) * 18; // 18 Tbit (許容上限: 1875 µs)

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
// GPIO トグルによる LIN Break 信号送出関数 (マスター専用)
// =============================================================================
// 仕様: 14 Tbit Dominant (LOW) + 1 Tbit Recessive (HIGH / Delimiter)
// 9600 bps 時: LOW = 約1458 µs, HIGH = 約104 µs
// =============================================================================
void sendLinBreak(uint32_t baud) {
  uint16_t tBit = BIT_TIME_US(baud);

  // 1. 直前の送信が完全に完了するまで待機
  Serial.flush();

  // 2. RS-485 トランシーバーを送信モードへ設定
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

  // 7. RS-485 トランシーバーを受信モードへ戻す (バス解放)
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

#ifdef ROLE_MASTER
  // ハードウェアシリアル(USART0)のピンを PA1(TXD) / PA2(RXD) にリマップ
  Serial.swap(1);
  Serial.begin(BAUDRATE);

  pcSerial.println(F("=============================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P1-01       ==="));
  pcSerial.println(F("=== [MASTER] GPIO LIN Break Generator      ==="));
  pcSerial.println(F("=============================================="));
  pcSerial.print(F("[Config] BaudRate: "));
  pcSerial.print(BAUDRATE);
  pcSerial.print(F(" bps (1 Tbit = "));
  pcSerial.print(BIT_TIME_US(BAUDRATE));
  pcSerial.println(F(" us)"));
  pcSerial.print(F("[Config] Target Break LOW: "));
  pcSerial.print(TARGET_BREAK_US);
  pcSerial.println(F(" us (14 Tbit)"));
  pcSerial.println(F("[Status] Sending Break pulses every 1000ms...\n"));

#else
  // スレーブ側：自己測定アナライザモード初期化
  pcSerial.println(F("=============================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Test: TC-P1-01       ==="));
  pcSerial.println(F("=== [SLAVE] Break Pulse Width Analyzer     ==="));
  pcSerial.println(F("=============================================="));
  pcSerial.print(F("[Target] Expected Break LOW: "));
  pcSerial.print(TARGET_BREAK_US);
  pcSerial.println(F(" us (14 Tbit @ 9600 bps)"));
  pcSerial.print(F("[Pass Range] "));
  pcSerial.print(MIN_PASS_US);
  pcSerial.print(F(" us (11 Tbit) <= Width <= "));
  pcSerial.print(MAX_PASS_US);
  pcSerial.println(F(" us (18 Tbit)"));
  pcSerial.println(F("[Status] Waiting for Master Break pulses on PA2 (RXD)...\n"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
#ifdef ROLE_MASTER
  // -------------------------------------------------------------------------
  // 【マスター側】 1秒周期で GPIO LIN Break 信号を送出
  // -------------------------------------------------------------------------
  static unsigned long lastSendTime = 0;
  static uint32_t breakCount = 0;

  if (millis() - lastSendTime >= 1000) {
    lastSendTime = millis();
    breakCount++;

    digitalWrite(LED_R, HIGH); // 送信中 LED 点灯

    // GPIO Break 信号送出
    sendLinBreak(BAUDRATE);

    digitalWrite(LED_R, LOW);  // 送信完了 LED 消灯

    // PC シリアルモニタへログ出力
    pcSerial.print(F("[Master Uptime: "));
    pcSerial.print(millis());
    pcSerial.print(F(" ms] #"));
    pcSerial.print(breakCount);
    pcSerial.println(F(" Sent LIN Break (14 Tbit LOW + 1 Tbit HIGH)"));
  }

#else
  // -------------------------------------------------------------------------
  // 【スレーブ側】 pulseIn による Break 信号 LOW 時間の自己高精度測定
  // -------------------------------------------------------------------------
  static uint32_t measCount = 0;

  // PA2(RXD) ピンが LOW になっている時間（パルス幅）をマイクロ秒単位で測定 (タイムアウト: 2.5秒)
  unsigned long duration = pulseIn(PIN_RXD, LOW, 2500000UL);

  if (duration > 0) {
    measCount++;
    digitalWrite(LED_W, HIGH); // 受信パルス検知 LED 点灯

    // 誤差率計算 ((実測値 - 理論値) / 理論値 * 100)
    long diffUs = (long)duration - (long)TARGET_BREAK_US;
    float errorPercent = ((float)diffUs / (float)TARGET_BREAK_US) * 100.0;

    // 合否判定: 11 Tbit (1146 µs) 以上 かつ 18 Tbit (1875 µs) 以下であれば合格
    bool isPass = (duration >= MIN_PASS_US && duration <= MAX_PASS_US);

    // PC シリアルモニタへ測定結果を出力
    pcSerial.print(F("[Slave Meas #"));
    pcSerial.print(measCount);
    pcSerial.print(F("] Width: "));
    pcSerial.print(duration);
    pcSerial.print(F(" us (Diff: "));
    if (diffUs >= 0) pcSerial.print(F("+"));
    pcSerial.print(diffUs);
    pcSerial.print(F(" us, Error: "));
    if (errorPercent >= 0) pcSerial.print(F("+"));
    pcSerial.print(errorPercent, 1);
    pcSerial.print(F("%) -> "));

    if (isPass) {
      pcSerial.println(F("[ PASS / OK ]"));
    } else {
      pcSerial.println(F("[ FAIL / NG ]"));
    }

    delay(20);
    digitalWrite(LED_W, LOW);
  }
#endif
}
