#ifndef ENCRYPTDECRYPT_H
#define ENCRYPTDECRYPT_H

#include <AESLib.h>
#include <Arduino.h>

class EncryptDecrypt {
public:
    // Constructor
    EncryptDecrypt();

    // Variables
    byte key[16];
    byte iv[16];
    String message;
    byte encrypted[240]; // Maximum length for encrypted data considering LoRa package size
    byte decrypted[240]; // Maximum length for decrypted data considering LoRa package size

    // Methods
    void setKey(byte newKey[16]);
    void setIV(byte newIV[16]);
    void setMessage(String newMessage);
    String encrypt();
    String decrypt(int packetSize);
    String toString(byte* data, size_t length);
    void fromString(String str, byte* data, size_t length);

private:
    AESLib aesLib;
};

#endif