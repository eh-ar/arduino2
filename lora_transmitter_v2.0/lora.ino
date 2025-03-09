
#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

String delimiter = ",";
String messageEnd = "%;";

char recivedMessage[256];
int receivedMessageLength = 0;
char transmittMessage[256];
int transmittMessageLength = 0;
//--------------------------------------
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

//-----------------------------------
bool registerWithReceiver() {
  LoRa.beginPacket();
  LoRa.print(TRANSMITTER_NAME);
  LoRa.print(",");
  LoRa.print(regsiterMessageType);
  LoRa.endPacket();

  Serial.println("Registration sent");

  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {  // 2 second timeout
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String message = getMessage();
      String type = messageType(message);
      if (type == acceptMessageType) {
        Serial.println("Transmitter registered");
        delay(1);
        return true;
      } else {
        Serial.println("Transmitter Not registered");
        delay(1);
        return false;
      }
    }
  }
  Serial.println("Registration timeout");
  delay(1);
  return false;
}

void getMessage() {
  String message;
  receivedMessageLength = 0;
  while (LoRa.available()) {
    receivedMessage[receivedMessageLength++] = (char)LoRa.read();
  }
  message = String(recivedMessage).substring(0, receivedMessageLength - 1);
  return message;
}

//----------------------------------------
void sendData() {
  LoRa.beginPacket();
  LoRa.write(TRANSMITTER_ID);
  LoRa.print("Data");
  LoRa.print(",");
  LoRa.print(TRANSMITTER_NAME);
  LoRa.print(",");
  LoRa.print(random(0, 15));
  LoRa.endPacket();
  Serial.println("Data Sent, Waiting for ACK");
  delay(1);
  if (!waitForAck()) {
    Serial.println("ACK Timeout, Retrying");
    delay(1);
    sendData();
  } else {
    Serial.println("Cycle time: " + String(cycleTime));
    delay(1);
  }
}

//---------------------------------------
bool waitForAck() {
  unsigned long startTime = millis();
  while (millis() - startTime < ACK_TIMEOUT) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      int id = LoRa.read();
      char com = LoRa.read();
      Serial.println(String(id) + " " + String(com));
      if (id == TRANSMITTER_ID && com == 'A') {
        cycleTime = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();

        Serial.println("ACK");
        delay(1);
        return true;
      } else if (id == TRANSMITTER_ID && com == 'R') {
        Serial.println("Register");
        delay(1);
        registerWithReceiver();
      }
    } else {
      //Serial.println("no packet");
      //Serial.println("Cycle: " + String(cycleTime) + " sec");
      //delay(5);
    }
  }
  return false;
}
//------------------------------------------
bool transmitterNameCheck(String message) {
  if (message.substring(0, 9) == TRANSMITTER_NAME) {
    return true;
  } else {
    return false;
  }
}

String messageType(String message) {
  return message.substring(11, 14);
}

unsigned long getDataCycle(String message) {
  int ind = message.lastIndexOf(delimiter);
  return message.substring(ind + 1);
}