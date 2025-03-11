#include <MyCrypto.h>

MyCrypto crypto(AES_256);  // Choose AES_128 or AES_256

void setup() {
    Serial.begin(115200);
    
    // Set AES key (16 bytes for AES-128, 32 for AES-256)
    crypto.setKey("12345678901234567890123456789012");

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
}