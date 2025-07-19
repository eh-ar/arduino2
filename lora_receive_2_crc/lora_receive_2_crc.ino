
#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <CRC32.h>

#define Rx 2
#define Tx 3

#define NSS 10
#define NRESET 4
#define DIO0 5
#define BAND 433E6  // Set frequency to 433 MHz

#define VOLTAGE_DIVIDER_RATIO 2.0
#define ADC_REFERENCE 5.0
#define BAT_PIN A0

// Shared secret (minimum 8 bytes recommended)
const char SECRET_KEY[] = "LoraKey7742";

SoftwareSerial mySerial(Rx, Tx);

void setup() {

  Serial.begin(9600);
  mySerial.begin(9600);

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
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
    // Parse message: "counter|code|data|checksum"
    int pos1 = receivedMessage.indexOf('|');
    int pos2 = receivedMessage.indexOf('|', pos1 + 1);
    int pos3 = receivedMessage.indexOf('|', pos2 + 1);

    if (pos1 == -1 || pos2 == -1 || pos3 == -1) {
      
      Serial.println("Invalid format");
      Serial.println("message:" + receivedMessage);
      return;
    }

    String code = receivedMessage.substring(0, pos1);
    unsigned int  counter1 = receivedMessage.substring(pos1 + 1, pos2).toInt();;
    String data = receivedMessage.substring(pos2 + 1, pos3);
    //Serial.println(code);
    //Serial.println(counter1);
    //Serial.println(data);
  
    //uint32_t receivedChecksum = strtoul(receivedMessage.substring(pos3 + 1).c_str(), NULL, 16);
    String receivedChecksum = receivedMessage.substring(pos3 + 1);
    //Serial.println(receivedChecksum);

    // Verify checksum
    uint32_t calculatedChecksum = calculateChecksum(code, data, counter1);

    int rssi = (LoRa.packetRssi());
    float batVoltage = readVoltage(BAT_PIN);
    if (trueMessage && (String(calculatedChecksum, HEX) == receivedChecksum)) {
      //Serial.println("Valid mesage"); 
      //Serial.println(receivedMessage + "," + rssi + "," + batVoltage);
      Serial.println(code + "," + counter1 + "," + data + "," + rssi + "," + batVoltage);
      delay(5);
    } else{
       //Serial.println("inValid mesage"); 
       //Serial.println(trueMessage);
       //Serial.println(String(calculatedChecksum, HEX) + "-" + receivedChecksum);
      Serial.println("Message: " + receivedMessage);
    }
  }
}

float readVoltage(int pin) {
  int rawValue = analogRead(pin);
  return (rawValue * ADC_REFERENCE) / 1023.0 * VOLTAGE_DIVIDER_RATIO;
}

uint32_t calculateChecksum(const String &code, const String &data, uint32_t counter) {
  CRC32 crc;
  
  // 1. Add secret key (as bytes)
  crc.update((const uint8_t*)SECRET_KEY, strlen(SECRET_KEY));
  
  // 2. Add code (as bytes)
  crc.update((const uint8_t*)code.c_str(), code.length());
  
  // 3. Add data (as bytes)
  crc.update((const uint8_t*)data.c_str(), data.length());
  
  // 4. Add counter (as 4 bytes, little-endian)
  uint8_t counterBytes[4];
  counterBytes[0] = (counter >> 0) & 0xFF;
  counterBytes[1] = (counter >> 8) & 0xFF;
  counterBytes[2] = (counter >> 16) & 0xFF;
  counterBytes[3] = (counter >> 24) & 0xFF;
  crc.update(counterBytes, 4);
  
  uint32_t result = crc.finalize();
  
  // Debug output
  /*
  Serial.print("TX Checksum Input: ");
  Serial.print(SECRET_KEY); Serial.print("|");
  Serial.print(code); Serial.print("|");
  Serial.print(data); Serial.print("|");
  Serial.println(counter);
  
  Serial.print("TX Checksum Result: 0x");
  Serial.println(result, HEX);
  */
  return result;
}