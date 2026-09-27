/*
 * ADX Core-D (ATtiny1616) RS-485 Serial Monitor & Echo Test Sketch
 * 
 * Target Board : ADX Core-D (Microchip ATtiny1616-MNR)
 * Core / BSP   : megaTinyCore by SpenceKonde
 * Baud Rate    : 115,200 bps (8N1)
 *
 * Pin Mapping:
 *   - PIN_PB3 (Pin 9)  : White LED (Heartbeat output)
 *   - PIN_PB2 (Pin 8)  : Red LED   (Status indicator, held OFF)
 *   - PIN_PA1 (Pin 15) : USART0 TX (Connected to RS-485 transceiver DI)
 *   - PIN_PA2 (Pin 16) : USART0 RX (Connected to RS-485 transceiver RO)
 *   - PIN_PA7 (Pin 14) : RS-485 RE/DE Direction Control
 *                        HIGH = Transmit Mode (Driver Enabled)
 *                        LOW  = Receive Mode (Receiver Enabled)
 *
 * Description:
 *   1. Sends a startup announcement packet upon boot.
 *   2. Sends a periodic heartbeat packet every 1000ms while toggling the White LED.
 *   3. Listens for incoming characters from Web Serial Monitor and echoes them back.
 */

#define PIN_LED_WHITE PIN_PB3
#define PIN_LED_RED   PIN_PB2
#define PIN_RS485_DIR PIN_PA7

uint32_t lastHeartbeat = 0;
uint16_t packetCount = 0;

// Helper function to send RS-485 packet with strict half-duplex direction switching
void sendRS485(const String &msg) {
  // 1. Switch transceiver to Transmit mode
  digitalWrite(PIN_RS485_DIR, HIGH);
  delayMicroseconds(10); // Transceiver setup time

  // 2. Transmit data over USART0
  Serial.print(msg);

  // 3. Crucial: Wait until shift register is completely empty before switching back
  Serial.flush();
  delayMicroseconds(10); // Hold time for stop bit to propagate

  // 4. Return transceiver to Receive mode
  digitalWrite(PIN_RS485_DIR, LOW);
}

void setup() {
  // Configure LED pins
  pinMode(PIN_LED_WHITE, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);    // Keep Red LED OFF
  digitalWrite(PIN_LED_WHITE, HIGH); // Start White LED ON

  // Configure RS-485 Direction pin
  pinMode(PIN_RS485_DIR, OUTPUT);
  digitalWrite(PIN_RS485_DIR, LOW);  // Default to Receive mode

  // Initialize USART0 at 115200 bps
  Serial.begin(115200);

  // Allow voltage and bus levels to settle
  delay(100);

  // Send initial startup banner
  sendRS485("\r\n=========================================\r\n");
  sendRS485("  ADX Core-D RS-485 Serial Monitor Ready!\r\n");
  sendRS485("  Baudrate: 115200 bps | LED: PB3 (White)\r\n");
  sendRS485("=========================================\r\n");
}

void loop() {
  // 1. Check for incoming characters from PC / Web Serial Monitor
  if (Serial.available()) {
    char c = Serial.read();
    
    // Echo back the received character
    digitalWrite(PIN_RS485_DIR, HIGH);
    delayMicroseconds(10);
    Serial.print(">>> Echo received: '");
    Serial.write(c);
    Serial.print("'\r\n");
    Serial.flush();
    delayMicroseconds(10);
    digitalWrite(PIN_RS485_DIR, LOW);
  }

  // 2. Periodic Heartbeat Packet (1000ms interval)
  uint32_t currentMillis = millis();
  if (currentMillis - lastHeartbeat >= 1000) {
    lastHeartbeat = currentMillis;
    packetCount++;

    // Toggle onboard White LED (PB3)
    digitalWrite(PIN_LED_WHITE, !digitalRead(PIN_LED_WHITE));

    // Construct heartbeat status message
    String packet = "[ADX Core-D] Heartbeat packet #" + String(packetCount);
    packet += " | White LED: ";
    packet += (digitalRead(PIN_LED_WHITE) == HIGH) ? "ON\r\n" : "OFF\r\n";

    sendRS485(packet);
  }
}
