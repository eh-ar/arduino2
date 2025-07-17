//libs
#include <SPI.h>
#include <LoRa.h>
//#include <SoftwareSerial.h>

/*
#define Rx 2
#define Tx 3
#define NSS 10
#define NRESET 4
#define DIO0 5
#define BAND 433E6
*/
// Set frequency to 433 MHz

//SoftwareSerial mySerial(Rx, Tx);

void setup() {

  Serial.begin(9600);
  //mySerial.begin(9600);

  LoRa.setPins(10, 4, 5);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1) {};
  }
  Serial.println("LoRa Receiver");
  delay(20);
  //lcdInit();
  //lcdPrint("HELLO, WORLD!");
}


void loop() {
  // Try to parse a packet
  int packetSize = LoRa.parsePacket();

  if (packetSize > 0) {
    // Received a packet
    String receivedMessage = "";

    int counter = 0;
    String pretext = "";
    int trueMessage = true;

    while (LoRa.available()) {
      char c = LoRa.read();

      receivedMessage += c;
      pretext += c;
      counter++;
      if (counter == 5 && pretext != "Faraz") {
        trueMessage = false;
        break;
      }
    }
    int rssi = (LoRa.packetRssi());
    if (trueMessage) {
      Serial.println(receivedMessage + "," + rssi);
      //mySerial.print(receivedMessage);
    }
  }
}
