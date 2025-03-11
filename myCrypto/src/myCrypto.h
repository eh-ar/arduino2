#ifndef MYCRYPTO_H
#define MYCRYPTO_H

#include <Arduino.h>
#include <AESLib.h>

enum AESMode { AES_128, AES_256 };

class MyCrypto {
public:
    MyCrypto(AESMode mode = AES_128);  // Mode selection
    void setKey(const char* key);  // Set AES key
    String encrypt(String plainText);  // Encrypt (IV + Cipher)
    String decrypt(String encryptedText);  // Decrypt (Extract IV)
    
private:
    AESLib aesLib;
    AESMode aesMode;
    byte aesKey[32]; // 32-byte max for AES-256
    byte aesIV[16];  // IV (Always 16 bytes)
    uint8_t keySize; // 16 for AES-128, 32 for AES-256
    void generateRandomIV();  // Function to generate a random IV
};

#endif