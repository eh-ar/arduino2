<<<<<<< HEAD

#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

#define Rx 2
#define Tx 3

#define NSS 10
#define NRESET 4
#define DIO0 5
#define BAND 433E6  // Set frequency to 433 MHz

#define VOLTAGE_DIVIDER_RATIO 2.0
#define ADC_REFERENCE 5.0
#define BAT_PIN A0

SoftwareSerial mySerial(Rx, Tx);
=======
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
>>>>>>> a9f705a84c1112b37c61ee4b1db313c363b88b0b

void setup() {

  Serial.begin(9600);
<<<<<<< HEAD
  mySerial.begin(9600);

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
=======
  //mySerial.begin(9600);

  LoRa.setPins(10, 4, 5);
  if (!LoRa.begin(433E6)) {
>>>>>>> a9f705a84c1112b37c61ee4b1db313c363b88b0b
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
<<<<<<< HEAD
    float batVoltage = readVoltage(BAT_PIN);
    if (trueMessage) {
      Serial.println(receivedMessage + "," + rssi + "," + batVoltage);
      mySerial.print(receivedMessage);
    }
  }
}

float readVoltage(int pin) {
  int rawValue = analogRead(pin);
  return (rawValue * ADC_REFERENCE) / 1023.0 * VOLTAGE_DIVIDER_RATIO;
}
=======
    if (trueMessage) {
      Serial.println(receivedMessage + "," + rssi);
      //mySerial.print(receivedMessage);
    }
  }
}
>>>>>>> a9f705a84c1112b37c61ee4b1db313c363b88b0b
