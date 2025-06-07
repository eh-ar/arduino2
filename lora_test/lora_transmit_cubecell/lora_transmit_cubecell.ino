#include <LoRa.h>

void setup() {
  Serial.begin(9600);
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
  uint16_t voltage = getBatteryVoltage();
  LoRa.print("Battery: " + String(voltage));
  LoRa.endPacket();
  delay(1000); // Send a packet every 10 seconds
}
