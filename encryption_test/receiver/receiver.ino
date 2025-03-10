#include <SPI.h>
#include <LoRa.h>
#include "AESLib.h"  // AES library

#define NSS 10  // LoRa Chip Select
#define RST 5   // LoRa Reset
#define DIO0 4  // LoRa Interrupt

#define BAUD 57600

AESLib aesLib;

#define INPUT_BUFFER_LIMIT (128 + 1)
unsigned char cleartext[INPUT_BUFFER_LIMIT] = { 0 };       // Input buffer for text
unsigned char ciphertext[2 * INPUT_BUFFER_LIMIT] = { 0 };  // Output buffer for encrypted data

// AES Encryption Key
byte aes_key[] = { 57, 36, 24, 25, 28, 86, 32, 41, 31, 36, 91, 36, 51, 74, 63, 89 };

// Initialization Vector (IV)
byte aes_iv[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };

void setup() {
  Serial.begin(BAUD);
  Serial.setTimeout(60000);

  // Initialize AES
  aesLib.gen_iv(aes_iv);
  aesLib.set_paddingmode((paddingMode)0);

  // Initialize LoRa
  LoRa.setPins(NSS, RST, DIO0);
  if (!LoRa.begin(433E6)) {  // Set frequency to 915 MHz
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Receiver Initialized");
LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);
  // Set the receive callback function
  //LoRa.onReceive(onReceive);
  //LoRa.receive();
}

void loop() {
//Serial.println("Recived Signal");

int packetSize = LoRa.parsePacket();
  delay(10);
  if (packetSize == 0) return;  // If no packet received, return
Serial.println("--------------------");
delay(5);
Serial.println("RSSI: "+ LoRa.packetRssi());
delay(5);
  // Read packet into ciphertext buffer
  for (int i = 0; i < packetSize; i++) {
    if (i < sizeof(ciphertext)) {
      ciphertext[i] = LoRa.read();
    } else {
      LoRa.read();  // Discard any extra bytes
    }
  }
  
  Serial.println("Encrypted: ");
  delay(5);
  Serial.println((char *)ciphertext);
  delay(5);
  // Decrypt the message
  uint16_t decLen = aesLib.decrypt(ciphertext, packetSize, (char *)cleartext, aes_key, sizeof(aes_key), aes_iv);
  Serial.println("Decrypted Message:");
  delay(5);
  Serial.println((char *)cleartext);
  delay(10);
}
// Callback function to handle received packets
void onReceive(int packetSize) {
  Serial.println("Recived Signal");
  delay(10);
  if (packetSize == 0) return;  // If no packet received, return

  // Read packet into ciphertext buffer
  for (int i = 0; i < packetSize; i++) {
    if (i < sizeof(ciphertext)) {
      ciphertext[i] = LoRa.read();
    } else {
      LoRa.read();  // Discard any extra bytes
    }
  }
  Serial.println("Encrypted: ");
  Serial.println((char *)ciphertext);
  // Decrypt the message
  uint16_t decLen = aesLib.decrypt(ciphertext, packetSize, (char *)cleartext, aes_key, sizeof(aes_key), aes_iv);
  Serial.println("Decrypted Message:");
  Serial.println((char *)cleartext);
}