#include <SPI.h>
#include <LoRa.h>



void setup() {
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(10, 4, 5);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1) {};
  }
  Serial.println("LoRa transmitter");
  delay(20);
}

void loop() {
  String message = "Hello from LoRa!";
  Serial.print("Sending: ");
  Serial.println(message);

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  delay(5000); // Wait 10 seconds
}
