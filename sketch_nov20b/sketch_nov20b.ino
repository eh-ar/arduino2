// E61-433T17D - simple Arduino UNO example (transparent serial)
// Assumes module powered at 3.3V and RX pin has level shifting from Arduino TX -> module RX.
// Uses SoftwareSerial so USB serial (Serial) remains available for monitor.

#include <SoftwareSerial.h>

const uint8_t MOD_M0 = 6;
const uint8_t MOD_M1 = 7;
const uint8_t MOD_AUX = 8;

// SoftwareSerial pins: RX is Arduino pin that reads module TX.
// TX is Arduino pin that writes to module RX (use level shifter / divider)
SoftwareSerial rfSerial(5, 4);  // rfSerial(RX, TX)

void setMode(uint8_t m1, uint8_t m0) {
  // m1, m0 are 0 or 1
  digitalWrite(MOD_M1, m1 ? HIGH : LOW);
  digitalWrite(MOD_M0, m0 ? HIGH : LOW);
  // Wait for AUX to indicate ready; datasheet suggests waiting for AUX HIGH then 2ms
  unsigned long start = millis();
  while (digitalRead(MOD_AUX) == LOW) {
    if (millis() - start > 1000) break;  // safety timeout
  }
  delay(5);
}

void enterNormalMode() {
  setMode(0, 0);  // Mode0: transparent / normal
}

void enterCommandMode() {
  // Mode2: M1=1, M0=0 is command mode (UART open, RF disabled)
  // Datasheet notes config happens on power-on or exiting sleep in some cases.
  setMode(1, 0);
  delay(50);
}

void setup() {
  Serial.begin(115200);
  pinMode(MOD_M0, OUTPUT);
  pinMode(MOD_M1, OUTPUT);
  pinMode(MOD_AUX, INPUT);

  // Start with normal mode
  digitalWrite(MOD_M0, LOW);
  digitalWrite(MOD_M1, LOW);
  delay(50);

  // Start RF serial at module's UART baud (match your module; default often 9600)
  rfSerial.begin(9600);  // set to module baud (check datasheet/config)
  Serial.println("E61 demo: enter text to send via RF; received RF prints here.");
  enterNormalMode();
}

void loop() {
  // Forward USB serial -> RF module (user types in Serial Monitor)
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s += "\n";  // keep newline if you want
    rfSerial.print(s);
    Serial.print(F("Sent to RF: "));
    Serial.println(s);
  }

  // Forward RF -> USB Serial
  while (rfSerial.available()) {
    int c = rfSerial.read();
    Serial.write(c);
  }

  // Example: press 'r' in Serial Monitor to read module parameters (enter command mode)
  if (Serial.available()) {
    char ch = Serial.peek();
    if (ch == 'r') {
      Serial.read();  // consume

      Serial.println("Entering command mode...");
      enterCommandMode();
      delay(20);

      // READ PARAMETERS COMMAND = C1 C1 C1
      const byte readCmd[3] = { 0xC1, 0xC1, 0xC1 };
      Serial.println("Sending READ PARAMETERS command...");
      sendCommand(readCmd, 3);

      Serial.println("Returning to normal mode...");
      enterNormalMode();
    }
    if (ch == 'w') {
      Serial.print("w");
      writeSettings2();
    }
    if (ch == 'u') {
      Serial.print("u");
      writeSettings();
    }
  }
}

void sendCommand(const byte *cmd, uint8_t len) {
  // Send command bytes
  rfSerial.write(cmd, len);

  // Wait a bit for module to reply
  delay(150);

  // Print reply in hex
  Serial.print("Reply: ");
  while (rfSerial.available()) {
    byte b = rfSerial.read();
    Serial.print("0x");
    if (b < 0x10) Serial.print("0");
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void writeSettings() {
  enterCommandMode();
  delay(80);

  // C0 00 00 1B 50 01
  uint8_t cmd[] = { 0xC0, 0x00, 0x00, 0x18, 0x50, 0x50 };
  rfSerial.write(cmd, sizeof(cmd));

  delay(200);
  enterNormalMode();
  Serial.println("Settings written.");
}

void writeSettings2() {
  enterCommandMode();
  delay(80);

  // C0 00 00 1B 50 01
  uint8_t cmd[] = { 0xC0, 0x00, 0x00, 0x1B, 0x50, 0x01 };
  rfSerial.write(cmd, sizeof(cmd));

  delay(200);
  enterNormalMode();
  Serial.println("Settings written.");
}
