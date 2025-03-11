#ifndef MYCRYPTO_H
#define MYCRYPTO_H

#include <Arduino.h>
#include <AESLib.h>
#include <base64.h>  // Include Base64 encoding/decoding library

enum AESMode { AES_128, AES_256 };

class MyCrypto {
public:
    MyCrypto(AESMode mode = AES_128);
    void setKey(const char* key);
    String encrypt(String plainText);
    String decrypt(String encryptedText);

private:
    AESLib aesLib;
    AESMode aesMode;
    byte aesKey[32];
    byte aesIV[16];
    uint8_t keySize;
    void generateRandomIV();
    
    String base64Encode(byte* data, size_t length);  // Base64 encoding
    String base64Decode(String encoded);  // Base64 decoding
};

#endif