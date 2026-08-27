// =============================================================================
// ADX Core-D LN-485 Test Firmware - Phase 4 (TC-P4)
// Type B (Slave Pub -> Master Sub) Verification & Master Broker MVP
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
// マスター機 (Master Broker) ビルド時 : 有効化 (#define ROLE_MASTER)
// スレーブ機 (Slave Node)    ビルド時 : コメントアウト (//#define ROLE_MASTER)
#define ROLE_MASTER

// =============================================================================
// ピン定義 (Hardware Pin Configuration)
// =============================================================================
const int PIN_TXD = PIN_PA1; // USART0 TXD (Alternate Pin)
const int PIN_RXD = PIN_PA2; // USART0 RXD (Alternate Pin)
const int PIN_DE  = PIN_PA4; // RS-485 Driver Enable (HIGH = 送信有効)
const int PIN_RE  = PIN_PA7; // RS-485 Receiver Enable (LOW = 受信有効)
const int LED_R   = PIN_PB2; // 赤色 LED (Master: 送信中 / Slave: Type A コマンド受信)
const int LED_W   = PIN_PB3; // 白色 LED (Master: 受信完了 / Slave: Type B 応答送信)

// PCシリアルモニタ用 SoftwareSerial (RX: PB5, TX: PB4, 9600 bps)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// =============================================================================
// 通信パラメータ & プロトコル定数
// =============================================================================
const uint32_t BAUDRATE = 9600;
#define BIT_TIME_US(baud) (1000000UL / (baud))

// トピック ID 定義
const uint8_t TOPIC_TYPE_A_LED    = 0x02; // Type A: Master Pub -> Slave Sub (LED制御 1B)
const uint8_t TOPIC_TYPE_B_UPTIME = 0x03; // Type B: Slave Pub -> Master Sub (スレーブ稼働時間 4B)
const uint8_t TOPIC_TYPE_B_UNCONN = 0x05; // Type B: 未接続ノード用 (タイムアウト検証用)

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
// スレーブ用: レジスタ直接送信 & フラッシュ関数
// =============================================================================
#ifndef ROLE_MASTER
void slaveTxByte(uint8_t data) {
  while (!(USART0.STATUS & USART_DREIF_bm)); // 送信データレジスタ空き待ち
  USART0.TXDATAL = data;
}

void slaveTxFlush() {
  while (!(USART0.STATUS & USART_TXCIF_bm)); // シフトレジスタ送出完了待ち
  USART0.STATUS |= USART_TXCIF_bm;          // TXCIF クリア
}
#endif

// =============================================================================
// スレーブ用: Double Buffer Mailbox 構造体 & 実装
// =============================================================================
#ifndef ROLE_MASTER
struct DoubleBufferMailbox {
  uint8_t  topicId;
  uint8_t  length;
  uint8_t  payload[2][8];     // 2面バッファ ([0] と [1])
  uint8_t  checksum[2];       // 各面の Classic Checksum
  volatile uint8_t activeIdx; // 通信層が読み出すアクティブ面 (0 または 1)
};

// スレーブ側のパブリッシュ用メールボックス (ID=0x03: 稼働時間 4B)
DoubleBufferMailbox uptimeMailbox;

// アプリ層からの非同期更新 (ロックフリー・ゼロジッター)
void updateUptimeMailbox(uint32_t uptimeMs) {
  uint8_t nextIdx = uptimeMailbox.activeIdx ^ 1; // 裏バッファを計算
  
  uint8_t data[4];
  data[0] = (uptimeMs >> 0)  & 0xFF;
  data[1] = (uptimeMs >> 8)  & 0xFF;
  data[2] = (uptimeMs >> 16) & 0xFF;
  data[3] = (uptimeMs >> 24) & 0xFF;

  memcpy(uptimeMailbox.payload[nextIdx], data, 4);
  uptimeMailbox.checksum[nextIdx] = calculateClassicChecksum(data, 4);
  uptimeMailbox.length = 4;

  // 1クロックでアトミックに切り替え
  uint8_t sreg = SREG;
  cli();
  uptimeMailbox.activeIdx = nextIdx;
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
  
  PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (1 Tbit)
  delayMicroseconds(tBit * 1);
  
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
  // 【マスター機初期化】
  pinMode(PIN_RXD, INPUT);
  Serial.swap(1); // PA1/PA2 にリマップ
  Serial.begin(BAUDRATE);

  pcSerial.println(F("=================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Master Broker MVP (TC-P4)==="));
  pcSerial.println(F("=================================================="));
  pcSerial.println(F("[Ready] Scheduler started (Cycle: 1.5s)"));
  pcSerial.println(F("--------------------------------------------------"));
#else
  // 【スレーブ機初期化】
  // 割り込み競合を防ぐため Serial.begin() は使わずレジスタ直接設定
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

  pcSerial.println(F("=================================================="));
  pcSerial.println(F("=== ADX Core-D LN-485 Slave Node (TC-P4)       ==="));
  pcSerial.println(F("=================================================="));
  pcSerial.println(F("[Ready] LINAUTO + Double Buffer Mailbox active"));
  pcSerial.println(F("--------------------------------------------------"));
#endif
}

// =============================================================================
// メインループ (loop)
// =============================================================================
#ifdef ROLE_MASTER
// -----------------------------------------------------------------------------
// マスター側: スケジューラ & プロミスキャス傍受 ＆ タイムアウト管理 (Master Broker MVP)
// -----------------------------------------------------------------------------
uint32_t lastCycleTime = 0;
uint8_t  schedulerSlot = 0;
bool     ledStateCmd   = false;

void loop() {
  uint32_t now = millis();
  
  // 1.5秒ごとに次のスロットを実行
  if (now - lastCycleTime >= 1500) {
    lastCycleTime = now;
    
    switch (schedulerSlot) {
      // -----------------------------------------------------------------------
      // Slot 0: Type A (Master Pub -> Slave Sub: LED コマンド送出)
      // -----------------------------------------------------------------------
      case 0: {
        ledStateCmd = !ledStateCmd;
        uint8_t payload[1] = { (uint8_t)(ledStateCmd ? 0x01 : 0x00) };
        uint8_t cs = calculateClassicChecksum(payload, 1);
        uint8_t pid = calculatePID(TOPIC_TYPE_A_LED);
        
        pcSerial.print(F("[Master Broker] [Slot 0] [Type A Pub] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.print(F(" (ID=0x02) CMD=0x"));
        pcSerial.print(payload[0], HEX);
        pcSerial.print(F(" (LED: "));
        pcSerial.print(ledStateCmd ? F("ON") : F("OFF"));
        pcSerial.println(F(")"));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.write(payload[0]);
        Serial.write(cs);
        Serial.flush();
        setRxMode(); // 送信後即座に受信モードへ
        digitalWrite(LED_R, LOW);
        break;
      }

      // -----------------------------------------------------------------------
      // Slot 1: Type B (Slave Pub -> Master Sub: スレーブ稼働時間要求 4B)
      // -----------------------------------------------------------------------
      case 1: {
        uint8_t pid = calculatePID(TOPIC_TYPE_B_UPTIME);
        
        pcSerial.print(F("[Master Broker] [Slot 1] [Type B Poll] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.println(F(" (ID=0x03: Slave Uptime Request) -> Promiscuous RX"));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.flush();
        setRxMode(); // 送信完了直後に受信モードへ移行 (DE=0)
        digitalWrite(LED_R, LOW);
        
        // 非ブロッキング・プロミスキャス受信 (15ms タイムアウト)
        uint32_t rxStart = millis();
        uint8_t  rxBuf[8];
        uint8_t  rxCount = 0;
        const uint8_t EXPECTED_LEN = 4; // 4B Uptime + 1B CS = 5B
        
        while (millis() - rxStart < 15) { // 15ms タイムアウト監視
          if (Serial.available()) {
            rxBuf[rxCount++] = Serial.read();
            if (rxCount >= EXPECTED_LEN + 1) break; // 5B 受信完了
          }
        }
        
        // 受信結果の検証と出力
        if (rxCount >= EXPECTED_LEN + 1) {
          uint8_t calcCs = calculateClassicChecksum(rxBuf, EXPECTED_LEN);
          uint8_t recvCs = rxBuf[EXPECTED_LEN];
          
          if (calcCs == recvCs) {
            uint32_t uptimeVal = ((uint32_t)rxBuf[0]) |
                                 ((uint32_t)rxBuf[1] << 8) |
                                 ((uint32_t)rxBuf[2] << 16) |
                                 ((uint32_t)rxBuf[3] << 24);
            
            digitalWrite(LED_W, HIGH);
            pcSerial.print(F("  --> [RECV PROMISCUOUS OK] Slave Uptime: "));
            pcSerial.print(uptimeVal);
            pcSerial.print(F(" ms (Hex: ["));
            for (uint8_t i = 0; i < EXPECTED_LEN; i++) {
              if (rxBuf[i] < 0x10) pcSerial.print(F("0"));
              pcSerial.print(rxBuf[i], HEX);
              pcSerial.print(F(" "));
            }
            pcSerial.print(F("] CS: 0x"));
            if (recvCs < 0x10) pcSerial.print(F("0"));
            pcSerial.print(recvCs, HEX);
            pcSerial.println(F(")"));
            delay(20);
            digitalWrite(LED_W, LOW);
          } else {
            pcSerial.print(F("  --> [FAIL / CS MISMATCH] Calc: 0x"));
            pcSerial.print(calcCs, HEX);
            pcSerial.print(F(", Recv: 0x"));
            pcSerial.println(recvCs, HEX);
          }
        } else if (rxCount == 0) {
          pcSerial.println(F("  --> [TIMEOUT / NO RESPONSE (15ms)] Slave offline or not answering."));
        } else {
          pcSerial.print(F("  --> [TIMEOUT / INCOMPLETE FRAME] Received bytes: "));
          pcSerial.println(rxCount);
        }
        break;
      }

      // -----------------------------------------------------------------------
      // Slot 2: Type B タイムアウト検証用 (未接続ノード ID=0x05 要求)
      // -----------------------------------------------------------------------
      case 2: {
        uint8_t pid = calculatePID(TOPIC_TYPE_B_UNCONN);
        
        pcSerial.print(F("[Master Broker] [Slot 2] [Type B Poll - Unconnected Node] PID=0x"));
        if (pid < 0x10) pcSerial.print(F("0"));
        pcSerial.print(pid, HEX);
        pcSerial.println(F(" (ID=0x05: Timeout Test)"));
        
        digitalWrite(LED_R, HIGH);
        sendLinBreak(BAUDRATE);
        Serial.write(0x55);
        Serial.write(pid);
        Serial.flush();
        setRxMode(); // 受信モードへ
        digitalWrite(LED_R, LOW);
        
        // 15ms タイムアウト待機
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
    
    // スロット切り替え (0 -> 1 -> 2 -> 0)
    schedulerSlot = (schedulerSlot + 1) % 3;
  }
}

#else
// -----------------------------------------------------------------------------
// スレーブ側: Double Buffer Mailbox 更新 & LINAUTO 受信・応答ステートマシン
// -----------------------------------------------------------------------------
enum SlaveRxState {
  STATE_WAIT_HEADER,
  STATE_RECV_TYPE_A_PAYLOAD
};

SlaveRxState slaveState = STATE_WAIT_HEADER;
uint8_t currentPID = 0;
uint8_t currentID  = 0;
uint8_t payloadBuf[8];
uint8_t payloadIdx = 0;
uint8_t expectedPayloadLen = 0;

uint32_t lastMailboxUpdate = 0;

void loop() {
  uint32_t now = millis();

  // 1. 【アプリ層の非同期処理】 100msごとに Uptime を裏バッファへ更新
  if (now - lastMailboxUpdate >= 100) {
    lastMailboxUpdate = now;
    updateUptimeMailbox(now); // ロックフリー・ダブルバッファ更新
  }

  // 2. 【通信層の高速ポーリング処理】
  while (USART0.STATUS & USART_RXCIF_bm) {
    uint8_t rxHigh = USART0.RXDATAH; // ① 先にステータス取得
    uint8_t rxLow  = USART0.RXDATAL; // ② 次にデータ取得

    // エラー検知 (ISFIF, BDF 等のフラグチェック)
    if (USART0.STATUS & USART_ISFIF_bm) {
      pcSerial.println(F("[Slave Error] ISFIF Sync Error!"));
      USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
      slaveState = STATE_WAIT_HEADER;
      continue;
    }

    switch (slaveState) {
      // -----------------------------------------------------------------------
      // ヘッダ (PID) 待機状態
      // -----------------------------------------------------------------------
      case STATE_WAIT_HEADER: {
        // パリティエラー・フレーミングエラーのチェック
        if (rxHigh & (USART_PERR_bm | USART_FERR_bm | USART_BUFOVF_bm)) {
          pcSerial.println(F("[Slave Error] Header PERR/FERR/OVF detected. Discarding."));
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          break;
        }

        currentPID = rxLow;
        currentID  = currentPID & 0x3F;

        // パリティ合致確認
        if (calculatePID(currentID) != currentPID) {
          pcSerial.println(F("[Slave Error] PID Parity Mismatch. Discarding."));
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          break;
        }

        // ---------------------------------------------------------------------
        // 【Type A 受信】 ID=0x02 (Master Pub -> Slave Sub: LED コマンド)
        // ---------------------------------------------------------------------
        if (currentID == TOPIC_TYPE_A_LED) {
          slaveState = STATE_RECV_TYPE_A_PAYLOAD;
          expectedPayloadLen = 1; // 1バイトデータ + 1バイトCS = 2バイト
          payloadIdx = 0;
        }
        // ---------------------------------------------------------------------
        // 【Type B 応答】 ID=0x03 (Slave Pub -> Master Sub: Uptime 4B 返信)
        // ---------------------------------------------------------------------
        else if (currentID == TOPIC_TYPE_B_UPTIME) {
          // レスポンススペース待機 (50〜60µs)
          delayMicroseconds(60);

          // 表バッファのインデックスを取得 (Zero-Copy)
          uint8_t curIdx = uptimeMailbox.activeIdx;

          // RS-485 送信モードへ
          setTxMode();
          digitalWrite(LED_W, HIGH); // 白色 LED 点灯 (送信中)

          // 4バイトデータ + チェックサム 送出
          slaveTxByte(uptimeMailbox.payload[curIdx][0]);
          slaveTxByte(uptimeMailbox.payload[curIdx][1]);
          slaveTxByte(uptimeMailbox.payload[curIdx][2]);
          slaveTxByte(uptimeMailbox.payload[curIdx][3]);
          slaveTxByte(uptimeMailbox.checksum[curIdx]);

          // 送信完了待機 & 受信モードへ復帰
          slaveTxFlush();
          setRxMode();
          digitalWrite(LED_W, LOW);

          uint32_t sentUptime = ((uint32_t)uptimeMailbox.payload[curIdx][0]) |
                                ((uint32_t)uptimeMailbox.payload[curIdx][1] << 8) |
                                ((uint32_t)uptimeMailbox.payload[curIdx][2] << 16) |
                                ((uint32_t)uptimeMailbox.payload[curIdx][3] << 24);

          pcSerial.print(F("[Slave Pub Sent] Type B (ID=0x03) Uptime: "));
          pcSerial.print(sentUptime);
          pcSerial.print(F(" ms, CS: 0x"));
          pcSerial.println(uptimeMailbox.checksum[curIdx], HEX);

          // 次フレームのブレーク待機へ
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }
        // ---------------------------------------------------------------------
        // 自ノードに無関係な ID の場合 -> 即座に次フレーム待機へ
        // ---------------------------------------------------------------------
        else {
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
        }
        break;
      }

      // -----------------------------------------------------------------------
      // Type A ペイロード受信状態
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

            pcSerial.print(F("[Slave Sub Recv OK] Type A (ID=0x02) LED: "));
            pcSerial.println(ledCmd ? F("ON") : F("OFF"));
          } else {
            pcSerial.println(F("[Slave Sub Error] Checksum Mismatch!"));
          }

          // 次フレームのブレーク待機へ復帰
          USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
          slaveState = STATE_WAIT_HEADER;
        }
        break;
      }
    }
  }
}
#endif
