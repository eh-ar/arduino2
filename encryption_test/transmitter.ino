#include <SPI.h>
#include <LoRa.h>
#include <TinyAES.h>

#define NSS 10    // LoRa Chip Select
#define RST 9     // LoRa Reset
#define DIO0 2    // LoRa Interrupt

// AES Key (128-bit)
byte aes_key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// Example message
char message[] = "Secure LoRa Transmission!";

int original_length = strlen(message);
int padded_length = (original_length + 2 + 15) & ~15; // Ensure 16-byte alignment (2 bytes for length)
byte plaintext[256];  // Buffer for plaintext (including length)
byte encrypted_message[256];  // Buffer for encryption

TinyAES aes;

void prepareMessage() {
    // Store the message length in the first 2 bytes (big-endian format)
    plaintext[0] = (original_length >> 8) & 0xFF;
    plaintext[1] = original_length & 0xFF;

    // Copy message into the buffer
    memcpy(plaintext + 2, message, original_length);

    // PKCS#7 Padding
    byte padding = padded_length - (original_length + 2);
    for (int i = original_length + 2; i < padded_length; i++) {
        plaintext[i] = padding;
    }
}

void setup() {
    Serial.begin(9600);

    // Initialize LoRa
    LoRa.setPins(NSS, RST, DIO0);
    if (!LoRa.begin(915E6)) {
        Serial.println("LoRa initialization failed!");
        while (1);
    }
    Serial.println("LoRa Initialized");

    // Prepare and encrypt the message
    prepareMessage();
    aes.setKey(aes_key);
    aes.encrypt(plaintext, encrypted_message, padded_length);

    // Send encrypted message
    LoRa.beginPacket();
    LoRa.write(encrypted_message, padded_length);
    LoRa.endPacket();

    Serial.println("Encrypted message sent.");
}

void loop() {
    // Nothing to do here
}