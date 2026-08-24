#include <SoftwareSerial.h>

// ==========================================
// 役割設定：書き込むボードに合わせて切り替えてください
// ==========================================
#define ROLE_MASTER    // マスターの場合は有効化、スレーブの場合は // を付けてコメントアウト

// --- ピン定義 ---
const int PIN_RE  = PIN_PA7; // LOWで受信有効
const int PIN_DE  = PIN_PA4; // HIGHで送信有効
const int LED_W   = PIN_PB3; // 白LED
const int LED_R   = PIN_PB2; // 赤LED

#ifdef ROLE_MASTER
// PCデバッグ用のソフトウェアシリアル (RX:PB5, TX:PB4)
SoftwareSerial pcSerial(PIN_PB5, PIN_PB4); 
#endif

// ==========================================
// RS-485 モード切替ヘルパー関数
// ==========================================
void setTxMode() {
  digitalWrite(PIN_RE, HIGH); // 受信無効
  digitalWrite(PIN_DE, HIGH); // 送信有効
  delay(2);                   // ICの切り替わり待ち
}

void setRxMode() {
  digitalWrite(PIN_DE, LOW);  // 送信無効
  digitalWrite(PIN_RE, LOW);  // 受信有効
  delay(2);                   // ICの切り替わり待ち
}

// ==========================================
// 初期設定
// ==========================================
void setup() {
  pinMode(PIN_RE, OUTPUT);
  pinMode(PIN_DE, OUTPUT);
  pinMode(LED_W, OUTPUT);
  pinMode(LED_R, OUTPUT);

  setRxMode(); // 初期状態は受信モード

  // ハードウェアシリアル(USART0)のピンをPA1(TX) / PA2(RX)に割り当てる
  Serial.swap(1);  
  Serial.begin(9600); 

#ifdef ROLE_MASTER
  pcSerial.begin(9600);
  pcSerial.println(F("--- Master Ready ---"));
#endif
}

// ==========================================
// メインループ
// ==========================================
void loop() {
#ifdef ROLE_MASTER
  // ----------------------------------------
  // 【マスター側】 PC ⇔ RS-485 の中継
  // ----------------------------------------
  
  // 1. PCからデータを受信したら、RS-485へ送信
  if (pcSerial.available() > 0) {
    String msg = pcSerial.readStringUntil('\n'); 
    
    unsigned long masterTime = millis(); // ★ マスター自身のミリ秒を取得
    
    digitalWrite(LED_R, HIGH); 
    
    setTxMode();              
    Serial.print(msg);        
    Serial.print('\n');       
    Serial.flush();           
    setRxMode();              
    
    digitalWrite(LED_R, LOW); 
    
    // PCへマスターの送信時間を出力
    pcSerial.print("[Master Uptime: ");
    pcSerial.print(masterTime);
    pcSerial.print(" ms] Sent to RS-485: ");
    pcSerial.println(msg);
  }

  // 2. RS-485(スレーブ)からデータを受信したら、PCへ表示
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n'); 
    digitalWrite(LED_W, HIGH);
    
    pcSerial.print("Recv from RS-485: ");
    pcSerial.println(msg);
    
    delay(50); 
    digitalWrite(LED_W, LOW);
  }

#else
  // ----------------------------------------
  // 【スレーブ側】 自身の稼働時間を付与して返信
  // ----------------------------------------
  
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n'); // マスターからのデータを受信
    
    digitalWrite(LED_W, HIGH); // 返信中LED ON
    
    unsigned long slaveTime = millis(); // ★ スレーブ自身のミリ秒を取得
    
    setTxMode();               // 送信モードへ
    
    // スレーブの稼働時間と、受け取ったメッセージをまとめて送信
    Serial.print("Slave Uptime: ");
    Serial.print(slaveTime);
    Serial.print(" ms (Req: ");
    Serial.print(msg);
    Serial.print(")\n");       // 終端の改行
    
    Serial.flush();            // 送信完了まで確実に待機
    setRxMode();               // 受信モードへ戻す
    
    digitalWrite(LED_W, LOW);  // 返信中LED OFF
  }
#endif
}