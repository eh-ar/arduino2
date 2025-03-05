#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial mySerial(2, 3);  // RX, TX
#define NSS 10
#define NRESET 4
#define DIO0 5
#define BAND 433E6  // Set frequency to 433 MHz


void setup() {
  mySerial.begin(9600);
  Serial.begin(115200);
  Wire.begin(3);  // Join I2C bus as master
  Wire.setWireTimeout(30000 /* us */, true /* reset_on_timeout */);
  while (!Serial)
    ;

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Receiver");
}

void loop() {
  // Try to parse a packet
  int packetSize = LoRa.parsePacket();


  if (packetSize > 0) {
    // Received a packet
    String receivedMessage = "";

    while (LoRa.available()) {
      char c = LoRa.read();
      receivedMessage += c;
    }
    int rssi = (LoRa.packetRssi());
    Serial.print("Received: ");
    Serial.println(receivedMessage + "- rssi: " + rssi);
    Serial.println("send to OPI: ");
    mySerial.println(receivedMessage);
    //String dataToSend = receivedMessage;
    //char charArray[dataToSend.length() + 1];               // Create a character array
    //dataToSend.toCharArray(charArray, sizeof(charArray));  // Convert String to character array

    //Wire.beginTransmission(8);  // Address of the slave
    //Wire.write(charArray);      // Send the character array
    //Wire.endTransmission();
  }
}
