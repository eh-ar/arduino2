#include <SPI.h>
#include <LoRa.h>
#include "AESLib.h"  // AES library
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#define NSS 10  // LoRa Chip Select
#define RST 5   // LoRa Reset
#define DIO0 4  // LoRa Interrupt

#define BAUD 57600

char ID[] = "HY08V21234567865";  // ID of container

// Watchdog timer setup
volatile bool is_awake = true;

AESLib aesLib;

#define INPUT_BUFFER_LIMIT (160 + 1)
#define trigPin 8
#define echoPin 10

unsigned char cleartext[INPUT_BUFFER_LIMIT] = { 0 };       // Input buffer for text
unsigned char ciphertext[2 * INPUT_BUFFER_LIMIT] = { 0 };  // Output buffer for encrypted data
uint16_t encLen;
byte aes_iv[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };


void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(BAUD);
  Serial.setTimeout(600);

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
  Serial.println("LoRa Transmitter Initialized");
  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);
  watchdogSetup();
  delay(1000);
}
int cc = 0;
void loop() {

  // Trigger the sensor
  cc++;
  // Read the echoPin

  String message = "This is Encryption test " + String(cc) + " This is a long Text to be encrypted in Receiver,This is a long Text to be encrypted in Receiver, This is a long Text to be encrypted in Receiver, This is a long Text to be encrypted in Receiver";
  Serial.println("Length:" + String(message.length()));
  delay(20);
  encryptString( message);
  
  // Send the encrypted message via LoRa
  
  LoRa.beginPacket();
  LoRa.write(ciphertext, encLen);
  LoRa.endPacket();

  //delay(500);  // Send message every 5 seconds
  sleepForSeconds(8);
}