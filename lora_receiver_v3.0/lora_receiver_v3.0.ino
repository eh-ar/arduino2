#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Crypto.h>
#include <AES.h>
#include <Base64.h>

// LoRa pins and settings
#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

// SoftwareSerial pins for RS485
SoftwareSerial mySerial(2, 3);  // RX, TX

// Define AES key (16 bytes)
byte key[16] = {
  0x2b, 0x7e, 0x15, 0x16,
  0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x1f, 0xb2,
  0x3c, 0x4f, 0xea, 0xc8
};

// Define initialization vector (IV) (16 bytes)
byte iv[16] = {
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,
  0x0c, 0x0d, 0x0e, 0x0f
};


// Maximum number of registered transmitters
const int MAX_TRANSMITTERS = 5;

// Time delay between transmitters (in seconds)
const int TIME_DELAY = 10;

// Transmitter ID structure
struct Transmitter {
  String id;
  String name;
  int dataCollectionCycle;  // In minutes
  int dataTransmitCycle;    // In minutes
  int syncCycle;            // In minutes
  DateTime startTime;       // Start time for the transmitter
  DateTime dataCollectionStart;
  DateTime dataTransmitStart;
  DateTime syncStart;
};


// Registered transmitters array
Transmitter transmitters[MAX_TRANSMITTERS];
int transmitterCount = 0;

// Create RTC object
RTC_DS3231 rtc;

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Update interval 1 minute

// Function prototypes
void processMessage(String message);
bool isValidID(String id);
void handleRegisterMessage(String id);
void handleDataMessage(String id);
void handleSyncMessage(String id);
void handleScheduleMessage(String id);
void sendAcceptMessage(String id);
void sendAcknowledgeMessage(String id);
void sendTimingMessage(String id);
void sendResetMessage(String id);
void scheduleTransmitters();
void updateRTCWithNTP();

void setup() {
  // Initialize LoRa module
  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    while (1)
      ;
  }

  // Initialize SoftwareSerial
  mySerial.begin(9600);

  // Initialize RTC
  if (!rtc.begin()) {
    while (1)
      ;  // Halt if RTC initialization fails
  }

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // Initialize NTP client
  timeClient.begin();
  updateRTCWithNTP();  // Initial RTC update

  // Initialize registered transmitters
  for (int i = 0; i < MAX_TRANSMITTERS; i++) {
    transmitters[i] = { "", "", 0, 0, 0, DateTime() };
  }
}

void loop() {
  // Update RTC with NTP time at regular intervals
  timeClient.update();
  if (timeClient.updated()) {
    updateRTCWithNTP();
  }

  // Check for incoming messages
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = LoRa.readString();
    processMessage(incoming);
  }
}

void processMessage(String message) {
  // Extract ID and message type from incoming message
  int commaIndex1 = message.indexOf(',');
  int commaIndex2 = message.indexOf(',', commaIndex1 + 1);

  String id = message.substring(0, commaIndex1);                         // ID
  String messageType = message.substring(commaIndex1 + 1, commaIndex2);  // Message Type

  // Check if ID is valid
  if (isValidID(id)) {
    if (messageType == "Re") {
      handleRegisterMessage(id);
    } else if (messageType == "Da") {
      handleDataMessage(id);
    } else if (messageType == "Sy") {
      handleSyncMessage(id);
    } else if (messageType == "Sh") {
      handleScheduleMessage(id);
    }
  }
}

bool isValidID(String id) {
  for (int i = 0; i < transmitterCount; i++) {
    if (transmitters[i].id == id) {
      return true;
    }
  }
  return false;
}

void handleRegisterMessage(String id) {
  if (transmitterCount < MAX_TRANSMITTERS) {
    Transmitter& tx = transmitters[transmitterCount];
    tx.id = id;
    tx.name = id.substring(0, 4);  // Example name extraction

    // Set specific schedule bases (timing cycles)
    tx.dataCollectionCycle = 5;  // 5 minutes
    tx.dataTransmitCycle = 15;   // 15 minutes
    tx.syncCycle = 5;            // 30 minutes

    transmitterCount++;
    sendAcceptMessage(id);
  }
}


void handleDataMessage(String id) {
  // Handle data message logic
  sendAcknowledgeMessage(id);
}

void handleSyncMessage(String id) {
  // Handle sync message logic
  sendTimingMessage(id);
}

void handleScheduleMessage(String id) {
  // Handle schedule message logic
  scheduleTransmitters();
  sendAcknowledgeMessage(id);
}

void sendAcceptMessage(String id) {
  String message = id + ",Ac";
  loraSend(message);
}

void sendAcknowledgeMessage(String id) {
  String message = id + ",Ak";
 loraSend(message);
}

void sendTimingMessage(String id) {
  String message = id + ",Ti,";
  // Add timing values to message
  for (int i = 0; i < transmitterCount; i++) {
    if (transmitters[i].id == id) {
      message += String(transmitters[i].dataCollectionCycle) + ",";
      message += String(transmitters[i].dataTransmitCycle) + ",";
      message += String(transmitters[i].syncCycle);
    }
  }
 loraSend(message);
}

void sendResetMessage(String id) {
  String message = id + ",Rs";
  loraSend(message);
}

void scheduleTransmitters() {
  DateTime currentTime = rtc.now();
  DateTime baseTime = DateTime(currentTime.year(), currentTime.month(), currentTime.day(), currentTime.hour(), currentTime.minute(), 0);  // Round down to the nearest minute

  for (int i = 0; i < transmitterCount; i++) {
    Transmitter& tx = transmitters[i];

    // Calculate the time for the first schedule
    int minutesUntilNextSchedule = tx.dataCollectionCycle - (baseTime.minute() % tx.dataCollectionCycle);
    tx.startTime = baseTime + TimeSpan(0, 0, minutesUntilNextSchedule, 0);

    // Calculate the start times for data collection, transmission, and synchronization
    tx.dataCollectionStart = tx.startTime;
    tx.dataTransmitStart = tx.dataCollectionStart + TimeSpan(0, 0, tx.dataTransmitCycle, 0);
    tx.syncStart = tx.dataTransmitStart + TimeSpan(0, 0, tx.syncCycle, 0);
  }
}

void loraSend(String message) {
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

void updateRTCWithNTP() {
  // Get NTP time and set RTC
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  DateTime ntpTime = DateTime(epochTime);
  rtc.adjust(ntpTime);
}
