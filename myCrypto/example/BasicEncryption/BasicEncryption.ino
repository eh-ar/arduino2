#include "myCrypto.h"
#include <AESLib.h>
#include <Arduino.h>

MyCrypto crypto(AES_128);  // Choose AES_128 or AES_256

void setup() {
  Serial.begin(115200);

  // Set AES key (16 bytes for AES-128, 32 for AES-256)
  crypto.setKey("1234567890123456");

  // Encrypt message (IV is automatically generated)
  String message = "Hello, Secure World!";
  String encrypted = crypto.encrypt(message);
  Serial.println("Encrypted (Base64): " + encrypted);

  // Decrypt message
  String decrypted = crypto.decrypt(encrypted);
  Serial.println("Decrypted: " + decrypted);
}

void loop() {
  // Nothing needed here
  for (int cc = 0; cc < 100; cc++) {
    String message = "Hello, Secure World!";// + String(cc);
    message = padString(message);
    Serial.println("Message: " + message);
    String encrypted = crypto.encrypt(message);
    Serial.println("Encrypted (Base64): " + encrypted);

    // Decrypt message
    String decrypted = crypto.decrypt(encrypted);
    Serial.println("Decrypted: " + decrypted);
    delay(1000);
  }
}

String padString(String input) {
  int padLen = 16 - (input.length() % 16);
  for (int i = 0; i < padLen; i++) {
    input += (char)padLen;
  }
  return input;
}