

// AES Encryption Key
byte aes_key[] = { 57, 36, 24, 25, 28, 86, 32, 41, 31, 36, 91, 36, 51, 74, 63, 89 };

// Initialization Vector (IV)


void encryptString(String message) {
  message.toCharArray((char*)cleartext, INPUT_BUFFER_LIMIT);
  Serial.print("Message: ");
  Serial.println(message);
  // Encrypt the message
  encLen = aesLib.encrypt((byte*)cleartext, sizeof(cleartext), (char*)ciphertext, aes_key, sizeof(aes_key), aes_iv);
  Serial.print("Encrypted Message:");
  Serial.println((char*)ciphertext);

}