#include <SPI.h>
#include <LoRa.h>

#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

const int NUM_TRANSMITTERS = 10;
const unsigned long CYCLE_TIME = 30;         // s (adjustable)
const unsigned long ACK_TIMEOUT = 2 * 1000;  // ms seconds timeout for ACK

// Possible daa collection cycle times in seconds
const unsigned long collectionCycle[] = { 5 * 60, 10 * 60, 15 * 60, 30 * 60, 60 * 60 };
// Possible sync cycle times in seconds
const unsigned long syncCycle[] = { 5 * 60, 10 * 60, 15 * 60, 30 * 60, 60 * 60 };
// Possible data transmitt cycle times in seconds
const unsigned long transmittCycle[] = { 5 * 60, 10 * 60, 15 * 60, 30 * 60, 60 * 60 };

struct TransmitterSchedule {
  int id;
  String name;
  bool registered;
  unsigned long currentTransmitTime;
  unsigned long nextTransmitTime;
};

TransmitterSchedule schedules[NUM_TRANSMITTERS];

//-------------------------
void serialEnable() {
  Serial.begin(57600);
  while (!Serial) {};
  Serial.println("------ SETUP ---------" );
  Serial.println("serial enables");
  delay(10);
}
//--------------------------
void loraEnable() {
  Serial.println("Activating Lora");
  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {  // Adjust frequency
    Serial.println("LoRa init failed");
    while (1) {};
  }
  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);
  Serial.println("Lora enabled");
}

//----------------------
void unregisterTransmitters() {
  for (int i = 0; i < NUM_TRANSMITTERS; i++) {
    schedules[i].registered = false;
  }
}
//--------------------------

void setup() {

  // enable serial;
  serialEnable();
  // enable lora;
  loraEnable();
  // unregister transmitter
  unregisterTransmitters();
  // Ready
  Serial.println("LoRa receiver ready");
  delay(10);
  randomSeed(analogRead(0));
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    int transmitterId = LoRa.read();
    char message[256];
    int messageLength = 0;
    while (LoRa.available()) {
      message[messageLength++] = (char)LoRa.read();
    }
    message[messageLength] = '\0';
    Serial.println(message);
    delay(5);
    if (message[0] == 'R' && message[1] == 'e' && message[2] == 'g') {  // Registration request
      handleRegistration(transmitterId, message);
    } else if (message[0] == 'S' && message[1] == 'c' && message[2] == 'h') {
      handleSchedule(transmitterId, message);
    } else {
      Serial.print("Received from ID ");
      Serial.print(transmitterId);
      Serial.print(": ");
      Serial.println(message);
      sendAck(transmitterId);
    }
  }
}

void handleSchedule(int transmitterId, String message) {

  for (int i = 0; i < NUM_TRANSMITTERS; i++) {
    if (schedules[i].id == transmitterId && schedules[i].registered) {

      Serial.println("Schedule Transmitter");
      if (i == 0) {
        schedules[i].nextTransmitTime = CYCLE_TIME;
      } else {
        schedules[i].nextTransmitTime = schedules[i - 1].nextTransmitTime + 5;
      }

      LoRa.beginPacket();
      LoRa.write(schedules[i].currentTransmitTime >> 24);
      LoRa.write(schedules[i].currentTransmitTime >> 16);
      LoRa.write(schedules[i].currentTransmitTime >> 8);
      LoRa.write(schedules[i].currentTransmitTime);

      LoRa.write(schedules[i].nextTransmitTime >> 24);
      LoRa.write(schedules[i].nextTransmitTime >> 16);
      LoRa.write(schedules[i].nextTransmitTime >> 8);
      LoRa.write(schedules[i].nextTransmitTime);
      LoRa.endPacket();

      Serial.print("Registered ID ");
      Serial.print(transmitterId);
      Serial.print(", delay: ");
      Serial.print(schedules[i].nextTransmitTime);
      Serial.println("ms");
      break;
    }
  }
}

void handleRegistration(int transmitterId, String message) {
  bool found = false;
  int ind = message.indexOf(",");
  String name = message.substring(ind + 1);
  for (int i = 0; i < NUM_TRANSMITTERS; i++) {
    if (schedules[i].id == transmitterId && schedules[i].registered) {
      found = true;
      Serial.println("Already registered");
      if (i == 0) {
        schedules[i].nextTransmitTime = CYCLE_TIME;
      } else {
        schedules[i].nextTransmitTime = schedules[i - 1].nextTransmitTime + 5;
      }

      LoRa.beginPacket();
      LoRa.write(schedules[i].currentTransmitTime >> 24);
      LoRa.write(schedules[i].currentTransmitTime >> 16);
      LoRa.write(schedules[i].currentTransmitTime >> 8);
      LoRa.write(schedules[i].currentTransmitTime);

      LoRa.write(schedules[i].nextTransmitTime >> 24);
      LoRa.write(schedules[i].nextTransmitTime >> 16);
      LoRa.write(schedules[i].nextTransmitTime >> 8);
      LoRa.write(schedules[i].nextTransmitTime);
      LoRa.endPacket();

      Serial.print("Registered ID ");
      Serial.print(transmitterId);
      Serial.print(", delay: ");
      Serial.print(schedules[i].nextTransmitTime);
      Serial.println("ms");
      break;
    }
  }

  if (!found) {
    for (int i = 0; i < NUM_TRANSMITTERS; i++) {
      if (!schedules[i].registered) {
        schedules[i].id = transmitterId;
        schedules[i].name = name;
        schedules[i].registered = true;

        schedules[i].currentTransmitTime = millis();
        if (i == 0) {
          schedules[i].nextTransmitTime = CYCLE_TIME;
        } else {
          schedules[i].nextTransmitTime = schedules[i - 1].nextTransmitTime + 10;
        }

        unsigned long time = schedules[i].currentTransmitTime;
        unsigned long nextTime = schedules[i].nextTransmitTime;

        Serial.println(String(CYCLE_TIME) + "  " + String(time) + "  " + String(nextTime));
        LoRa.beginPacket();

        LoRa.write(schedules[i].currentTransmitTime >> 24);
        LoRa.write(schedules[i].currentTransmitTime >> 16);
        LoRa.write(schedules[i].currentTransmitTime >> 8);
        LoRa.write(schedules[i].currentTransmitTime);

        LoRa.write(schedules[i].nextTransmitTime >> 24);
        LoRa.write(schedules[i].nextTransmitTime >> 16);
        LoRa.write(schedules[i].nextTransmitTime >> 8);
        LoRa.write(schedules[i].nextTransmitTime);
        LoRa.endPacket();

        Serial.print("Registered ID ");
        Serial.print(transmitterId);
        Serial.print(", delay: ");
        Serial.print(schedules[i].nextTransmitTime);
        Serial.println("ms");
        break;
      }
    }
  }
}

void sendAck(int transmitterId) {
  bool found = false;
  for (int i = 0; i < NUM_TRANSMITTERS; i++) {
    if (schedules[i].id == transmitterId) {
      found = true;
      schedules[i].nextTransmitTime = CYCLE_TIME + random(0, 15);
      LoRa.beginPacket();
      LoRa.write(transmitterId);
      LoRa.write('A');
      LoRa.write(schedules[i].nextTransmitTime >> 24);
      LoRa.write(schedules[i].nextTransmitTime >> 16);
      LoRa.write(schedules[i].nextTransmitTime >> 8);
      LoRa.write(schedules[i].nextTransmitTime);
      LoRa.endPacket();
      //delay(5);
      Serial.println("Ack, cycle: " + String(schedules[i].nextTransmitTime));
      delay(5);
      break;
    }
  }
  if (found == false) {
    Serial.println("transmitter not registered");
    LoRa.beginPacket();
    LoRa.write(transmitterId);
    LoRa.print("Reg");
    LoRa.endPacket();
  }
}
