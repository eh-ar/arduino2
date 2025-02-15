#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

// Set the pins for LoRa module
#define LORA_SCK 18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_RST 5
#define LORA_DIO0 26

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial monitor to open

  // Initialize LoRa
  LoRa.begin(433E6); // Set frequency to 433 MHz (adjust as needed)
  LoRa.setTxPower(20);
}

int cc=0;
void loop() {
  cc++;
  // Send a simple message every 100 milliseconds
  String message = "Hello, lora";
  LoRa.beginPacket();
  LoRa.print(message + " " + cc);
  LoRa.endPacket();
  Serial.print(message + " " + cc);
  delay(100); // Delay for 100 milliseconds
}
