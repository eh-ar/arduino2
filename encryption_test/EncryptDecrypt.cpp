#include "EncryptDecrypt.h"

// Constructor
EncryptDecrypt::EncryptDecrypt() {
    // Initialize with default values
    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    message = "";
    memset(encrypted, 0, sizeof(encrypted));
    memset(decrypted, 0, sizeof(decrypted));
}

// Set the encryption key
void EncryptDecrypt::setKey(byte newKey[16]) {
    memcpy(key, newKey, sizeof(key));
}

// Set the initialization vector (IV)
void EncryptDecrypt::setIV(byte newIV[16]) {
    memcpy(iv, newIV, sizeof(iv));
}

// Set the message to be encrypted
void EncryptDecrypt::setMessage(String newMessage) {
    message = newMessage;
}

// Encrypt the message and return the encrypted string
String EncryptDecrypt::encrypt() {
    if (message.length() > 0) {
        aesLib.set_paddingmode((paddingMode)0);
        aesLib.gen_iv(iv);

        size_t messageLength = message.length();
        char messageArray[240];
        message.toCharArray(messageArray, 240);

        uint16_t encLen = aesLib.encrypt((byte*)messageArray, messageLength, (char*)encrypted, key, sizeof(key), iv);
        return toString(encrypted, encLen);
    }
    return "";
}

// Decrypt the message and return the decrypted string
String EncryptDecrypt::decrypt(int packetSize) {
    if (packetSize > 0) {
        aesLib.set_paddingmode((paddingMode)0);

        uint16_t decLen = aesLib.decrypt(encrypted, packetSize, (char*)decrypted, key, sizeof(key), iv);
        return String((char*)decrypted);
    }
    return "";
}

// Convert byte array to string
String EncryptDecrypt::toString(byte* data, size_t length) {
    String result = "";
    for (size_t i = 0; i < length; i++) {
        char hex[3];
        sprintf(hex, "%02X", data[i]);
        result += hex;
    }
    return result;
}

// Convert string to byte array
void EncryptDecrypt::fromString(String str, byte* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        char hex[3];
        hex[0] = str[2 * i];
        hex[1] = str[2 * i + 1];
        hex[2] = '\0';
        data[i] = (byte)strtoul(hex, NULL, 16);
    }
}