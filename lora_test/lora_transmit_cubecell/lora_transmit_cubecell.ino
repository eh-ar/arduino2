#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect
  LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(23, false);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    

    while (1);
  }
}

void loop() {
  Serial.println("Sending packet...");
  LoRa.beginPacket();
  LoRa.print("Hello, RFM96!");
  LoRa.endPacket();
  delay(10000); // Send a packet every 10 seconds
}
