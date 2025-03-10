#include "AESLib.h"  //biblotheek import

#define BAUD 9600

char ID[] = "HY08V21234567865";  //ID of container

AESLib aesLib;

#define INPUT_BUFFER_LIMIT (128 + 1)  // designed for Arduino UNO, not stress-tested anymore (this works with readBuffer[129])
#define trigPin 8
#define echoPin 10

long duration;
float longitude;
float latitude;

unsigned char cleartext[INPUT_BUFFER_LIMIT] = { 0 };       // THIS IS INPUT BUFFER (FOR TEXT)
unsigned char ciphertext[2 * INPUT_BUFFER_LIMIT] = { 0 };  // THIS IS OUTPUT BUFFER (FOR BASE64-ENCODED ENCRYPTED DATA)
unsigned char readBuffer[128] = { 0 };                      //Waarde 0 er achter gezet omdat het nog toegekend moet worden.

// AES Encryption Key (same as in node-js example)
byte aes_key[] = { 57, 36, 24, 25, 28, 86, 32, 41, 31, 36, 91, 36, 51, 74, 63, 89 };

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte aes_iv[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };

// Generate IV (once)
void aes_init() {
  aesLib.gen_iv(aes_iv);
  aesLib.set_paddingmode((paddingMode)0);
}

uint16_t encrypt_to_ciphertext(char* msg, uint16_t msgLen, byte iv[]) {
  Serial.println("Calling encrypt (string)...");
  // aesLib.get_cipher64_length(msgLen);
  int cipherlength = aesLib.encrypt((byte*)msg, msgLen, (char*)ciphertext, aes_key, sizeof(aes_key), iv);
  // uint16_t encrypt(byte input[], uint16_t input_length, char * output, byte key[],int bits, byte my_iv[]);
  return cipherlength;
}

uint16_t decrypt_to_cleartext(byte msg[], uint16_t msgLen, byte iv[]) {
  Serial.print("Calling decrypt...; ");
  uint16_t dec_bytes = aesLib.decrypt(msg, msgLen, (char*)cleartext, aes_key, sizeof(aes_key), iv);
  Serial.print("Decrypted bytes: ");
  Serial.println(dec_bytes);
  return dec_bytes;
}

void setup() {
  // Define inputs and outputs:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(BAUD);
  Serial.setTimeout(60000);
  delay(2000);

  aes_init();  // generate random IV, should be called only once? causes crash if repeated...
}

/* non-blocking wait function */
void wait(unsigned long milliseconds) {
  unsigned long timeout = millis() + milliseconds;
  while (millis() < timeout) {
    yield();
  }
}

unsigned long loopcount = 0;

// Working IV buffer: Will be updated after encryption to follow up on next block.
// But we don't want/need that in this test, so we'll copy this over with enc_iv_to/enc_iv_from
// in each loop to keep the test at IV iteration 1. We could go further, but we'll get back to that later when needed.

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte enc_iv[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };
byte enc_iv_to[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };
byte enc_iv_from[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };

void loop() {
  int st = millis();
  Serial.println("----------------------");
  // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  Serial.println("--- process in " + String(millis() - st));
  // Read the echoPin, pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance:
  Serial.println("--- process in " + String(millis() - st));
  longitude = (duration * 0.034 / 2) + 40.36549000080095475375537699983898;
  latitude = (duration * 0.034 / 2) + 50.3654;
  Serial.println((longitude), 6);
  dtostrf(longitude, 3, 15, readBuffer);  //6 is number after decimal, daarvoor is aantal daarvor inclusief de punt



  //Serial.print("readBuffer length: "); Serial.println(sizeof(readBuffer)); wel weer activeren!

  // must not exceed INPUT_BUFFER_LIMIT bytes; may contain a newline
  sprintf((char*)cleartext, "%s", readBuffer);

  // Encrypt
  // iv_block gets written to, provide own fresh copy... so each iteration of encryption will be the same.
  uint16_t msgLen = sizeof(readBuffer);
  memcpy(enc_iv, enc_iv_to, sizeof(enc_iv_to));
  uint16_t encLen = encrypt_to_ciphertext((char*)cleartext, msgLen, enc_iv);
  //Serial.print("Encrypted length = "); Serial.println(encLen); NOG WEL WEER AANZETTEN
  Serial.println((char*)ciphertext);  //Nu laat hij de ciphertekst zien!! Dit is wat je naar Raspberry PI stuurt
  Serial.println("");
  Serial.println("--- process in " + String(millis() - st));
  Serial.println("Encrypted. Decrypting...");  //Serial.println(encLen ); Serial.flush();
  memcpy(enc_iv, enc_iv_from, sizeof(enc_iv_from));
  uint16_t decLen = decrypt_to_cleartext(ciphertext, encLen, enc_iv);
  //Serial.print("Decrypted cleartext of length: "); Serial.println(decLen);
  Serial.print("Decrypted cleartext:\n");
  Serial.println((char*)cleartext);

  if (strcmp((char*)readBuffer, (char*)cleartext) == 0) {
    Serial.println("Decrypted correctly.");
  } else {
    Serial.println("Decryption test failed.");
  }

  Serial.println("--- process in " + String(millis() - st));
  delay(500);
}