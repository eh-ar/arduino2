

void decryption(int packetSize){
  decLen = aesLib.decrypt(ciphertext, packetSize, (char *)cleartext, aes_key, sizeof(aes_key), aes_iv);
}