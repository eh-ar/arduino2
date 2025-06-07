#include <SPI.h>
#include <LoRa.h>




#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

   LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    while (1) {};
  }
  
  // Optional settings
  LoRa.setSpreadingFactor(7);      // Range: 6-12, higher = longer range but slower
  LoRa.setSignalBandwidth(125E3);  // Bandwidth options: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
  LoRa.setCodingRate4(8);          // Denominator of coding rate (5-8)
  
  Serial.println("LoRa receiver initialized!");
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    Serial.print("Received packet '");

    // Read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // Print RSSI (Received Signal Strength Indicator)
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}