/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * ADX Core-D Bring-up / Hardware Verification Test
 * Target MCU : ATtiny1616 (megaTinyCore)
 * Description: Tests GPIO output (PB2, PB3 LEDs) and SoftwareSerial (PB4 TX, PB5 RX).
 */

#include <SoftwareSerial.h>

const uint8_t RX_PIN = PIN_PB5;
const uint8_t TX_PIN = PIN_PB4;
const uint8_t LED_PB2 = PIN_PB2;
const uint8_t LED_PB3 = PIN_PB3;

SoftwareSerial mySerial(RX_PIN, TX_PIN);

// コマンド一覧を表示する関数
void printHelp() {
  mySerial.println(F("\n--- ATtiny1616 LED Controller ---"));
  mySerial.println(F("  0: Show Help (This Menu)"));
  mySerial.println(F("  1: PB2 ON   / 2: PB2 OFF"));
  mySerial.println(F("  3: PB3 ON   / 4: PB3 OFF"));
  mySerial.println(F("  5: Both ON  / 6: Both OFF"));
  mySerial.println(F("---------------------------------"));
  
  // 送信時のノイズ混入をクリア
  delay(10);
  while (mySerial.available() > 0) {
    mySerial.read();
  }
}

// 応答送信用のヘルパー関数
void sendResponse(const __FlashStringHelper* msg) {
  mySerial.println(msg);
  
  delay(10); 
  while (mySerial.available() > 0) {
    mySerial.read();
  }
}

void setup() {
  pinMode(LED_PB2, OUTPUT);
  pinMode(LED_PB3, OUTPUT);
  digitalWrite(LED_PB2, LOW);
  digitalWrite(LED_PB3, LOW);

  // RXピンのノイズ防止用プルアップ
  pinMode(RX_PIN, INPUT_PULLUP);

  mySerial.begin(9600);
  delay(100);

  // 起動時にコマンド一覧を表示
  printHelp();
}

void loop() {
  if (mySerial.available() > 0) {
    int raw = mySerial.read();

    // 制御文字・非表示文字（スペース、改行、ノイズ等）を除外
    if (raw <= 32 || raw > 126) {
      return;
    }

    char cmd = (char)raw;

    switch (cmd) {
      case '0':
        printHelp();
        break;

      case '1':
        digitalWrite(LED_PB2, HIGH);
        sendResponse(F("[OK] PB2: ON"));
        break;

      case '2':
        digitalWrite(LED_PB2, LOW);
        sendResponse(F("[OK] PB2: OFF"));
        break;

      case '3':
        digitalWrite(LED_PB3, HIGH);
        sendResponse(F("[OK] PB3: ON"));
        break;

      case '4':
        digitalWrite(LED_PB3, LOW);
        sendResponse(F("[OK] PB3: OFF"));
        break;

      case '5':
        digitalWrite(LED_PB2, HIGH);
        digitalWrite(LED_PB3, HIGH);
        sendResponse(F("[OK] Both (PB2, PB3): ON"));
        break;

      case '6':
        digitalWrite(LED_PB2, LOW);
        digitalWrite(LED_PB3, LOW);
        sendResponse(F("[OK] Both (PB2, PB3): OFF"));
        break;

      default:
        mySerial.print(F("[ERR] Unknown command: "));
        mySerial.println(cmd);
        delay(10);
        while (mySerial.available() > 0) {
          mySerial.read();
        }
        break;
    }
  }
}
