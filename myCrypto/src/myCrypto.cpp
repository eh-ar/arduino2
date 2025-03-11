#include "MyCrypto.h"

// Constructor
MyCrypto::MyCrypto(AESMode mode) {
    aesMode = mode;
    keySize = (aesMode == AES_128) ? 16 : 32;
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
        aesIV[i] = random(0, 256);
    }
}

// Base64 Encoding
String MyCrypto::base64Encode(byte* data, size_t length) {
    return base64::encode(data, length);
}

// Base64 Decoding
String MyCrypto::base64Decode(String encoded) {
    int decodedLength = base64::decodedLength(encoded.c_str(), encoded.length());
    char decoded[decodedLength];
    base64::decode(encoded.c_str(), encoded.length(), (byte*)decoded);
    return String(decoded);
}

// Encrypt: Generate IV, prepend to ciphertext, encode in Base64
String MyCrypto::encrypt(String plainText) {
    generateRandomIV();

    int len = plainText.length() + 1;
    byte encrypted[len];
    aesLib.encrypt64((byte*)plainText.c_str(), encrypted, aesKey, keySize * 8, aesIV);

    // Combine IV + Ciphertext
    byte combined[16 + len];  // IV (16) + Ciphertext
    memcpy(combined, aesIV, 16);
    memcpy(combined + 16, encrypted, len);

    // Encode in Base64
    return base64Encode(combined, sizeof(combined));
}

// Decrypt: Decode Base64, extract IV, decrypt ciphertext
String MyCrypto::decrypt(String encryptedText) {
    // Decode Base64
    String decoded = base64Decode(encryptedText);
    if (decoded.length() < 16) return "";

    // Extract IV (first 16 bytes)
    for (int i = 0; i < 16; i++) aesIV[i] = decoded[i];

    // Extract Ciphertext (after IV)
    String cipher = decoded.substring(16);
    int len = cipher.length() + 1;
    char decrypted[len];
    aesLib.decrypt64((byte*)cipher.c_str(), (byte*)decrypted, aesKey, keySize * 8, aesIV);

    return String(decrypted);
}