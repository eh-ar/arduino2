#include <SPI.h>
#include <LoRa.h>

#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

const int NUM_TRANSMITTERS = 8;
const unsigned long CYCLE_TIME = 30;         // s (adjustable)
const unsigned long ACK_TIMEOUT = 2 * 1000;  // ms seconds timeout for ACK

struct TransmitterSchedule {
  int id;
  bool registered;
  unsigned long currentTransmitTime;
  unsigned long nextTransmitTime;
};

TransmitterSchedule schedules[NUM_TRANSMITTERS];

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println(CYCLE_TIME);
  delay(10);
  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {  // Adjust frequency
    Serial.println("LoRa init failed");
    while (1)
      ;
  }
  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);

  for (int i = 0; i < NUM_TRANSMITTERS; i++) {
    schedules[i].registered = false;
  }

  Serial.println("LoRa receiver ready");
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
    if (message[0] == 'R') {  // Registration request
      handleRegistration(transmitterId);
    } else {
      Serial.print("Received from ID ");
      Serial.print(transmitterId);
      Serial.print(": ");
      Serial.println(message);
      sendAck(transmitterId);
    }
  }
}

void handleRegistration(int transmitterId) {
  bool found = false;
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

  for (int i = 0; i < NUM_TRANSMITTERS; i++) {
    if (schedules[i].id == transmitterId) {
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
}
