/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Modified from the examples of the Arduino LoRa library
  More resources: https://RandomNerdTutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
*********/

#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

SoftwareSerial gps(3, 2);  // RX, TX

//define the pins used by the transceiver module
#define ss 10
#define rst 9
#define dio0 2
String utcTime;
String latitude;
String latDirection;
String longitude;
String lonDirection;
String fixQuality;
String numSatellites;
String hdop;
String altitude;
String altitudeUnits;
String geoidSeparation;
String geoidUnits;
int rssi;
String LoRaData;
void setup() {
  //initialize Serial Monitor
  Serial.begin(9600);
  gps.begin(9600);
  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //868E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  //LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  parsemaker();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ");

    // read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
    }
   
    rssi = LoRa.packetRssi();
  Serial.print(LoRaData);
   Serial.print(" rssi: ");
    Serial.println(rssi);
    if (LoRaData == "gps") {
      printParsedData2();
    }
  }
}
void parseGNGGA(String gngga) {
  String fields[15];
  int fieldIndex = 0;
  int start = 0;
  int end = 0;

  while ((end = gngga.indexOf(',', start)) != -1 && fieldIndex < 15) {
    fields[fieldIndex++] = gngga.substring(start, end);
    start = end + 1;
  }
  fields[fieldIndex] = gngga.substring(start);

  // Assign to variables
  utcTime = fields[1];
  latitude = fields[2];
  latDirection = fields[3];
  longitude = fields[4];
  lonDirection = fields[5];
  fixQuality = fields[6];
  numSatellites = fields[7];
  hdop = fields[8];
  altitude = fields[9];
  altitudeUnits = fields[10];
  geoidSeparation = fields[11];
  geoidUnits = fields[12];
}

void printParsedData() {
  Serial.println("Parsed GNGGA Data:");
  Serial.print("UTC Time: ");
  Serial.println(utcTime);
  Serial.print("Latitude: ");
  Serial.print(latitude);
  Serial.print(" ");
  Serial.println(latDirection);
  Serial.print("Longitude: ");
  Serial.print(longitude);
  Serial.print(" ");
  Serial.println(lonDirection);
  Serial.print("Fix Quality: ");
  Serial.println(fixQuality);
  Serial.print("Satellites: ");
  Serial.println(numSatellites);
  Serial.print("HDOP: ");
  Serial.println(hdop);
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.print(" ");
  Serial.println(altitudeUnits);
  Serial.print("Geoid Separation: ");
  Serial.print(geoidSeparation);
  Serial.print(" ");
  Serial.println(geoidUnits);
  Serial.println("------------------------");
}
void printParsedData2() {
  Serial.print(utcTime);
  Serial.print(",");
  Serial.print(latitude);
  Serial.print(",");
  Serial.print(latDirection);
  Serial.print(",");
  Serial.print(longitude);
  Serial.print(",");
  Serial.print(lonDirection);
  Serial.print(",");
  Serial.print(fixQuality);
  Serial.print(",");
  Serial.print(numSatellites);
  Serial.print(",");
  Serial.print(hdop);
  Serial.print(",");
  Serial.print(altitude);
  Serial.print(",");
  Serial.print(altitudeUnits);
  Serial.print(",");
  Serial.print(geoidSeparation);
  Serial.print(",");
  Serial.print(geoidUnits);
  Serial.print(",");
  Serial.print(rssi);
  Serial.println(",");
}
void parsemaker() {
  static String sentence = "";

  while (gps.available()) {
    char c = gps.read();

    if (c == '\n') {
      if (sentence.startsWith("$GNGGA")) {
        
        parseGNGGA(sentence);
      }
      sentence = "";
    } else if (c != '\r') {
      sentence += c;
    }
  }
}