/*
 * Copyright (c) 2026 ADX Project Contributors
 * SPDX-License-Identifier: MIT
 *
 * ADX Core-D Bring-up / Hardware Verification Test
 * Target MCU : ATtiny1616 (megaTinyCore)
 * Description: Tests PA3 12MHz External Oscillator via RTC EXTCLK without disturbing main system clock.
 */

#include <SoftwareSerial.h>
#include <util/delay.h>

// ピン定義（sample.txtのソフトウェアシリアル設定を継承）
const uint8_t RX_PIN = PIN_PB5; 
const uint8_t TX_PIN = PIN_PB4; 

SoftwareSerial mySerial(RX_PIN, TX_PIN);

// コマンド一覧を表示する関数
void printHelp() {
  mySerial.println(F("\n--- ATtiny1616 PA3 Oscillator Test ---"));
  mySerial.println(F("  0: Show Help (This Menu)"));
  mySerial.println(F("  1: Start Oscillator Test (PA3)"));
  mySerial.println(F("--------------------------------------"));
}

// PA3オシレーターの動作を確認する安全なチェック関数
void testOscillator() {
  mySerial.println(F("\n--- PA3 オシレーター動作テスト開始 ---"));

  // 1. RTCの設定を一時的に退避し、無効化する
  while (RTC.STATUS & RTC_CTRLABUSY_bm); // 同期完了待ち
  uint8_t old_ctrla = RTC.CTRLA;
  RTC.CTRLA &= ~RTC_RTCEN_bm; // RTCを一時停止

  // 2. RTCのクロック源を EXTCLK (PA3) に切り替える
  uint8_t old_clksel = RTC.CLKSEL;
  RTC.CLKSEL = 0x03; // 0x03 = EXTCLK (PA3ピン)

  // 3. RTCを再度有効にして、PA3のクロック入力を要求させる
  while (RTC.STATUS & RTC_CTRLABUSY_bm);
  RTC.CTRLA |= RTC_RTCEN_bm;

  // 4. クロックが起動し、安定するのを少し待つ
  _delay_ms(100);

  // 5. CLKCTRL.MCLKSTATUS レジスタの EXTS ビット (Bit 7) を確認する
  // 1 であれば外部クロックエッジが正常に検出され、安定しています
  bool extclk_stable = (CLKCTRL.MCLKSTATUS & 0x80) != 0; 

  // 6. 実際にカウンタ値が変化しているか、複数回読み取って確認する
  uint16_t counts[5];
  for (int i = 0; i < 5; i++) {
    counts[i] = RTC.CNT;
    _delay_ms(10);
  }

  // カウンタ値が変動しているかチェック（カウントアップしていればクロックが供給されています）
  bool counter_changing = false;
  for (int i = 1; i < 5; i++) {
    if (counts[i] != counts[0]) {
      counter_changing = true;
      break;
    }
  }

  // 7. テスト結果をソフトウェアシリアルで出力
  mySerial.print(F("1. クロック検出ステータス (EXTS): "));
  if (extclk_stable) {
    mySerial.println(F("検出成功 (STABLE)"));
  } else {
    mySerial.println(F("未検出 (NO CLOCK)"));
  }

  mySerial.print(F("2. RTCカウンタ遷移: "));
  for (int i = 0; i < 5; i++) {
    mySerial.print(counts[i]);
    if (i < 4) mySerial.print(F(" -> "));
  }
  mySerial.println();

  mySerial.print(F("3. 発振確認結果: "));
  if (counter_changing) {
    mySerial.println(F("OK (パルスをカウントしています)"));
  } else {
    mySerial.println(F("NG (カウントが停止しています)"));
  }

  if (extclk_stable && counter_changing) {
    mySerial.println(F(">>> 判定: PA3のオシレーターは正常に動作しています！ <<<"));
  } else {
    mySerial.println(F(">>> 判定: オシレーターが動作していない、または配線不良です。 <<<"));
  }

  // 8. 元のRTC設定に復元し、Arduinoのミリ秒計測等への影響を元に戻す
  while (RTC.STATUS & RTC_CTRLABUSY_bm);
  RTC.CTRLA &= ~RTC_RTCEN_bm;
  while (RTC.STATUS & RTC_CTRLABUSY_bm);
  RTC.CLKSEL = old_clksel;
  while (RTC.STATUS & RTC_CTRLABUSY_bm);
  RTC.CTRLA = old_ctrla;

  mySerial.println(F("-------------------------------------\n"));

  // 送信時のノイズ混入をクリア
  delay(10);
  while (mySerial.available() > 0) {
    mySerial.read();
  }
}

void setup() {
  // RXピンのノイズ防止用プルアップ（sample.txtより継承）
  pinMode(RX_PIN, INPUT_PULLUP);

  mySerial.begin(9600);
  delay(100);

  // 起動時にメニュー表示
  printHelp();
}

void loop() {
  if (mySerial.available() > 0) {
    int raw = mySerial.read();

    switch (raw) {
      case '0':
        printHelp();
        break;
      case '1':
        testOscillator(); // 安全にPA3をテスト
        break;
      default:
        // 改行コード以外の未知のコマンドはエラー表示
        if (raw != '\r' && raw != '\n') {
          mySerial.print(F("Unknown Command: "));
          mySerial.println((char)raw);
        }
        break;
    }
  }
}