#include "myCrypto.h"

// Constructor
MyCrypto::MyCrypto(AESMode mode) {
    aesMode = mode;
    keySize = (aesMode == AES_128) ? 16 : 32;  // Select key size
    memset(aesKey, 0, sizeof(aesKey));
    memset(aesIV, 0, sizeof(aesIV));
}

// Set AES Key
void MyCrypto::setKey(const char* key) {
    strncpy((char*)aesKey, key, keySize);
}

// Generate a Random IV (16 Bytes)
void MyCrypto::generateRandomIV() {
    for (int i = 0; i < 16; i++) {
        aesIV[i] = random(0, 256);  // Generate a random byte
    }
}

// Encrypt: Generate IV, prepend to ciphertext
String MyCrypto::encrypt(String plainText) {
    generateRandomIV();  // Create a new random IV

    int len = plainText.length() + 1;
    char encrypted[len];
    aesLib.encrypt64((byte*)plainText.c_str(), (byte*)encrypted, aesKey, keySize * 8, aesIV);

    // Combine IV + Ciphertext
    String output;
    for (int i = 0; i < 16; i++) output += (char)aesIV[i];  // Append IV
    output += encrypted;  // Append Ciphertext

    return output;
}

// Decrypt: Extract IV, then decrypt the rest
String MyCrypto::decrypt(String encryptedText) {
    if (encryptedText.length() < 16) return "";  // Invalid data

    // Extract IV (first 16 bytes)
    for (int i = 0; i < 16; i++) aesIV[i] = encryptedText[i];

    // Extract Ciphertext (after IV)
    String cipher = encryptedText.substring(16);
    int len = cipher.length() + 1;
    char decrypted[len];
    aesLib.decrypt64((byte*)cipher.c_str(), (byte*)decrypted, aesKey, keySize * 8, aesIV);

    return String(decrypted);
}