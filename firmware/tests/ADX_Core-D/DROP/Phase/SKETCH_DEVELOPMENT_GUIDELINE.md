<!--
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# ADX Core-D DROP-Bus スケッチ開発ガイドライン ＆ 注意事項
(DROP-Bus Sketch Development Guidelines & Precautions)

本ドキュメントは、**ADX Core-D**（MCU: ATtiny1616, RS-485: SP485EEN）上で DROP-Bus（自律分散Pub/Sub ＆ SYSTEM `0xFF` 調停モデル）のテストファームウェアおよびドライバを実装する際の**必須コーディングルール、レジスタ制約、および推奨テンプレート**をまとめたガイドラインです。実機検証（`TC-D1-01` 等）を通じて確立された必須要件を反映しています。

---

## 1. DROP-Bus スケッチ開発の 10大鉄則 (Critical Rules)

```text
========================================================================================
                     DROP-Bus スケッチ開発 10大鉄則
========================================================================================
 1. 単一スケッチ構成 & デバッグラインの完全分離
    - #define ROLE_TX 等で同一ソースからバイナリ生成。PCモニタは SoftwareSerial (PB4/PB5) を使用
 2. 受信機側における Serial.begin() の完全排除 & 割り込み遮断
    - 受信機で Serial.begin() を呼ぶと Arduino コアの RX 割り込みが RXCIF を横取りするため厳禁
    - PORTMUX / PORTA 直接設定、PORTA.PIN2CTRL = PORT_PULLUPEN_bm、USART0.CTRLA = 0 を徹底
 3. 受信レジスタ読み出し順序の厳守
    - 必ず先に RXDATAH (ステータス/エラー) を読み、次に RXDATAL (データ) を読む (逆順は FIFO 破壊)
 4. 受信ポーリングループ内の低速 I/O 完全排除 (バッファリング ＆ 一括ダンプ)
    - 高速 UART 受信ループ内で SoftwareSerial 出力を行わず、rxBuf[] 格納後に無音検知で一括出力
 5. ボーレート設定の整数丸め誤差回避 ＆ 送信直前の名目ボーレート強制リセット
    - USART0.BAUD = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE)); (除算より先に乗算)
    - LINAUTO 受信で動的に微調整された BAUD レジスタを、フレーム送信直前に必ず名目（公称）値へリセットし、正帰還累積ドリフトを完全防止
 6. STATUS レジスタ操作における |= の絶対禁止 & ISFIF 常時監視
    - USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm; (直接代入で一括クリア)
    - ISFIF (同期エラー) は while (STATUS & RXCIF) の【外側】で常時監視・復帰
 7. 一体型 Break 送出シーケンス & Serial.flush() の徹底
    - 直前送信完了待機 (Serial.flush) -> TX 無効化 -> PA1 LOW (14 Tbit) -> PA1 HIGH (2 Tbit) -> TX 再有効化 -> 0x55 送出
    - 送信末尾バイト (CRC-16) 送出完了を Serial.flush() / TXCIF で待機してから DE=0 (受信モード) へ戻す
 8. LEN 不正値ガード (バッファオーバーラン防御)
    - LEN > 64 を受信した瞬間、即座に受信をアボートしてバトンを落とし、パッシブ STO へ誘導
 9. CRC-16-CCITT による高速整合性検証
    - 多項式 0x1021 (初期値: 0xFFFF) を用い、LEN 〜 PAYLOAD 末尾までを確実に検証
 10. TCB0 タイマー連動のパッシブ STO (心中) & SYSTEM 0xFF 調停
    - 正常フレーム受信ごとにタイマーをリセット。途絶時は TCB0 満了で全出力を即時遮断 (STO)
    - SYSTEM (0xFF) からの Ping 受信時は即座に Target: 0xFF で Pong 応答
========================================================================================
```

---

## 2. 各鉄則の詳細解説

### 2.1 単一スケッチ構成とデバッグライン分離（鉄則 1）
* 同一のスケッチファイル内で `#define ROLE_TX` の有無により送信側・受信側の動作を切り替えます。
* PC へのログ出力には `SoftwareSerial` (PB5: RX, PB4: TX / 9600 bps) を使用し、RS-485 バス（USART0: PA1/PA2）と通信ラインを完全に分離します。

### 2.2 受信機での `Serial.begin()` 完全排除とレジスタ直接設定（鉄則 2, 3）
* 受信側で `Serial.begin()` を呼び出すと、megaTinyCore の RX 割り込みハンドラ（ISR）が登録され、ハードウェア `RXDATAL` を横取りして `STATUS.RXCIF` を勝手にクリアしてしまいます。
* 受信機では `Serial.begin()` を一切呼ばず、以下のレジスタ直接設定を行います。
  ```cpp
  // USART0 ピンリマップ (PA1: TX, PA2: RX)
  PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
  PORTA.DIRSET = PIN1_bm; // PA1 (TX) = OUTPUT
  PORTA.DIRCLR = PIN2_bm; // PA2 (RX) = INPUT
  PORTA.PIN2CTRL = PORT_PULLUPEN_bm; // PA2 の入力バッファ有効化・プルアップ

  // USART0 レジスタ初期化
  USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
  USART0.CTRLA = 0; // 割り込み完全無効化
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  USART0.BAUD  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));
  USART0.CTRLB = USART_RXEN_bm; // 受信有効 (LINAUTO の場合は USART_RXMODE_LINAUTO_gc)
  ```
* **FIFO 読み出し順序:** 必ず `RXDATAH`（ステータス）を取得した後に `RXDATAL`（データ）を読み出します。

### 2.3 受信ポーリングループ内の低速 I/O 完全排除（鉄則 4）
* `SoftwareSerial` による PC 出力はビットバンギング処理のため非常に低速です。
* `while (USART0.STATUS & USART_RXCIF_bm)` 内で `pcSerial.print()` を呼ぶと受信取りこぼしが発生するため、ループ内では `rxBuf[]` への格納のみを行い、最後の受信から一定時間（50ms 等）無音になった時点で一括ダンプ出力します。

### 2.4 送信機の実装と `Serial.flush()` の徹底（鉄則 7）
* 送信側は実績のある Arduino `Serial` API を利用可能（`Serial.swap(1); Serial.begin(BAUDRATE);`）。
* RS-485 トランシーバー（SP485EEN）の `DE` ピンを LOW（受信モード）に戻す前には、**必ず `Serial.flush()` を実行** してシフトレジスタ末尾バイトの送出完了を待ちます。

---

## 3. コア・アルゴリズム実装スニペット

### 3.1 CRC-16-CCITT 高速計算ルーチン

```cpp
/**
 * @brief CRC-16-CCITT (Poly: 0x1021, Init: 0xFFFF)
 */
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
```

---

### 3.2 TCB0 タイマーによる生存監視 ＆ パッシブ STO

```cpp
#include <avr/io.h>
#include <avr/interrupt.h>

volatile bool isSafeTorqueOff = false;

void initFailSafeTimer(uint16_t timeout_ticks) {
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;       // Periodic Interrupt モード
    TCB0.CCMP  = timeout_ticks;           // タイムアウト値
    TCB0.INTCTRL = TCB_CAPT_bm;            // 割り込み許可
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm; // 起動
}

inline void resetFailSafeTimer() {
    TCB0.CNT = 0; // カウンタクリア
}

ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm;
    
    // 【パッシブ・フェイルセーフ発動】
    isSafeTorqueOff = true;
    PORTB.OUTSET = PIN2_bm;      // 赤LED 点灯 (STO警告)
    PORTB.OUTCLR = PIN3_bm;      // 白LED 消灯
    // モータ / PWM 出力をすべてハードウェア LOW 遮断
}
```

---

## 4. DROP-Bus スケッチ基本テンプレート

```cpp
// =============================================================================
// ADX Core-D DROP-Bus Node Firmware Standard Template
// =============================================================================
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// --- 定数定義 ---
#define DROP_ID_SYSTEM       (0xFF)
#define MAX_PAYLOAD_SIZE     (64)
const uint32_t BAUDRATE    = 9600;
#define BIT_TIME_US(baud)    (1000000UL / (baud))

// --- ハードウェアピン定義 (ADX Core-D 仕様準拠) ---
const int PIN_TXD   = PIN_PA1; // USART0 TXD
const int PIN_RXD   = PIN_PA2; // USART0 RXD
const int PIN_DE    = PIN_PA4; // RS-485 Driver Enable (HIGH=送信)
const int PIN_RE    = PIN_PA7; // RS-485 Receiver Enable (LOW=受信)
const int PIN_LED_R = PIN_PB2; // 赤色 LED (STO 警告)
const int PIN_LED_W = PIN_PB3; // 白色 LED (通信アクティブ)

SoftwareSerial pcSerial(PIN_PB5, PIN_PB4);

// =============================================================================
// RS-485 送受信切り替え
// =============================================================================
inline void setTxMode() {
    PORTA.OUTSET = PIN7_bm; // /RE = HIGH
    PORTA.OUTSET = PIN4_bm; // DE  = HIGH
    delayMicroseconds(10);
}

inline void setRxMode() {
    PORTA.OUTCLR = PIN4_bm; // DE  = LOW
    PORTA.OUTCLR = PIN7_bm; // /RE = LOW
    delayMicroseconds(10);
}

// 一体型 Break 送出関数 (14 Tbit LOW + 2 Tbit HIGH + 0x55)
void sendDropBreak(uint32_t baud) {
    uint16_t tBit = BIT_TIME_US(baud);
    Serial.flush();
    setTxMode();
    
    USART0.CTRLB &= ~USART_TXEN_bm;
    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm;         // Break: LOW (14 Tbit)
    delayMicroseconds(tBit * 14);
    
    PORTA.OUTSET = PIN1_bm;         // Delimiter: HIGH (2 Tbit)
    delayMicroseconds(tBit * 2);
    
    USART0.CTRLB |= USART_TXEN_bm;
    Serial.write(0x55);             // Sync Byte
}

// =============================================================================
// 初期化ルーチン (setup)
// =============================================================================
void setup() {
    PORTA.DIRSET = PIN4_bm | PIN7_bm;
    PORTB.DIRSET = PIN2_bm | PIN3_bm;
    PORTB.OUTCLR = PIN2_bm | PIN3_bm;
    setRxMode();
    
    pcSerial.begin(9600);
    delay(100);
    pcSerial.println(F("=== ADX Core-D DROP-Bus Node Initialized ==="));
    
    // 受信機側はレジスタ直接初期化 (Serial.begin は呼ばない)
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
    PORTA.DIRSET = PIN1_bm;
    PORTA.DIRCLR = PIN2_bm;
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    
    USART0.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
    USART0.CTRLA = 0; // 割り込み無効化
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
    USART0.BAUD  = (uint16_t)((64UL * F_CPU) / (16UL * BAUDRATE));
    USART0.CTRLB = USART_RXMODE_LINAUTO_gc | USART_RXEN_bm | USART_TXEN_bm;
    USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
}

// =============================================================================
// メインループ (loop)
// =============================================================================
void loop() {
    // 1. ISFIF デッドロック防止監視
    if (USART0.STATUS & USART_ISFIF_bm) {
        USART0.STATUS = USART_WFB_bm | USART_ISFIF_bm | USART_BDF_bm;
    }
    
    // 2. 高速受信バッファリング処理 (SoftwareSerial 出力は無音時に分離)
}
```
