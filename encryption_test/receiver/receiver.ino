#include <SPI.h>
#include <LoRa.h>
#include <TinyAES.h>


#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

// AES Key (same as transmitter)
byte aes_key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// Buffers for received and decrypted message
byte received_message[256];
byte decrypted_message[256];

TinyAES aes;

void removePadding(byte *message, int &length) {
    byte padding = message[length - 1];
    if (padding > 0 && padding <= 16) {
        length -= padding;
    }
}

void setup() {
    Serial.begin(9600);

    // Initialize LoRa
    LoRa.setPins(NSS, RST, DIO0);
    if (!LoRa.begin(BAND)) {
        Serial.println("LoRa initialization failed!");
        while (1);
    }
    Serial.println("LoRa Initialized");

    LoRa.receive();
}

void loop() {
    if (LoRa.parsePacket()) {
        int received_length = LoRa.readBytes(received_message, sizeof(received_message));

        // Decrypt the received message
        aes.setKey(aes_key);
        aes.decrypt(received_message, decrypted_message, received_length);

        // Extract original message length
        int message_length = (decrypted_message[0] << 8) | decrypted_message[1];

        // Remove padding
        removePadding(decrypted_message, received_length);

        // Ensure valid message length
        if (message_length > received_length - 2) {
            Serial.println("Error: Message length mismatch!");
            return;
        }

        // Print the decrypted message
        decrypted_message[message_length + 2] = '\0'; // Null-terminate string
        Serial.print("Decrypted Message: ");
        Serial.println((char *)(decrypted_message + 2));
    }
}