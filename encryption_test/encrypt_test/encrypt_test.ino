#include "aes.h"

AES_ctx ctx;

byte aes_key[16] = {  // 128-bit AES key
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

byte iv[16] = {  // Initialization Vector (IV) for AES-CBC mode
    0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18,
    0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F, 0x80
};

char message[] = "Hello, AES on Arduino!";
byte encrypted[32];  // Buffer for encrypted message
byte decrypted[32];  // Buffer for decrypted message

void setup() {
    Serial.begin(9600);

    // Initialize AES context with the key
    AES_init_ctx_iv(&ctx, aes_key, iv);

    Serial.print("Original Message: ");
    Serial.println(message);

    // Ensure the message is 16-byte aligned
    int message_length = strlen(message);
    int padded_length = (message_length + 15) & ~15;  // Round up to nearest multiple of 16

    // Copy message into buffer and pad
    memcpy(encrypted, message, message_length);
    for (int i = message_length; i < padded_length; i++) {
        encrypted[i] = padded_length - message_length;  // PKCS#7 padding
    }

    // Encrypt the message
    AES_CBC_encrypt_buffer(&ctx, encrypted, padded_length);

    Serial.print("Encrypted: ");
    for (int i = 0; i < padded_length; i++) {
        Serial.print(encrypted[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Decrypt the message
    AES_init_ctx_iv(&ctx, aes_key, iv);  // Reset AES context
    AES_CBC_decrypt_buffer(&ctx, encrypted, padded_length);
    memcpy(decrypted, encrypted, padded_length);

    // Remove padding
    int padding = decrypted[padded_length - 1];
    decrypted[padded_length - padding] = '\0';

    Serial.print("Decrypted Message: ");
    Serial.println((char *)decrypted);
}

void loop() {
}