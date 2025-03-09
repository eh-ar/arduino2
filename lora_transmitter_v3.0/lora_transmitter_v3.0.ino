#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <ModbusMaster.h>

// LoRa pins and settings
#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

#define RX 3
#define TX 2
// SoftwareSerial pins for RS485
SoftwareSerial mySerial(RX, TX);  // RX, TX
ModbusMaster node;

// Analog read pins for battery and solar
const int batteryPin = A0;
const int solarPin = A1;

// Sensor command pins
const int sensorPin1 = 6;
const int sensorPin2 = 7;
const int sensorPin3 = 8;
int sensorData[6] = { 0, 0, 0, 0, 0, 0 };

// Unique ID
String id = "AA12-2503-0001";  // Example ID

// Structure to store sensor data
struct SensorData {
  unsigned long timestamp;
  float battVoltage;
  float solarVoltage;
  int s1Id;
  int s1Rh;
  int s1T;
  int s1Ec;
  int s1Ph;
  int s1Sa;
  int s2Id;
  int s2Rh;
  int s2T;
  int s2Ec;
  int s2Ph;
  int s2Sa;
  int s3Id;
  int s3Rh;
  int s3T;
  int s3Ec;
  int s3Ph;
  int s3Sa;
};

// Maximum number of data entries to store
const int maxDataEntries = 100;

// EEPROM start address for data storage
const int eepromStartAddress = 10;


// Watchdog timer setup
volatile bool is_awake = true;

// Function prototypes
void enterSleep();
void wakeUp();
void setupWatchdogTimer(int interval);
bool waitForAckMessage(String expectedMessageType, unsigned long timeout);
void collectData();
String readRS485();
void checkAndPerformTimingOperations();


//-----   SETUP ----------------

void setup() {
  // Initialize pins
  pinInit();

  // Initialize LoRa module
  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    while (1) {};
  }

  // Initialize SoftwareSerial
  mySerial.begin(9600);

  // Initialize watchdog timer
  setupWatchdogTimer(8);  // Set interval to 8 seconds

  // Load counter from EEPROM
  int counter = EEPROM.read(0) | (EEPROM.read(1) << 8);
}

void loop() {
  // Empty loop as the main logic is within the setup()
  if (is_awake) {

    // Check and perform timing operations
    checkAndPerformTimingOperations();

    // Enter sleep mode
    enterSleep();
  }
}


void sendRegisterMessage() {
  String message = id + ",Re";
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

void sendData() {
  SensorData dataEntries[maxDataEntries];
  int dataSize = 0;

  // Read stored data from EEPROM
  for (int i = 0; i < maxDataEntries; i++) {
    dataEntries[i] = readDataFromEEPROM(eepromStartAddress + i * sizeof(SensorData));
    if (dataEntries[i].timestamp != 0) {
      dataSize++;
    }
  }

  // Calculate statistics
  calculateStatistics(dataEntries, dataSize);

  // Prepare message to send to receiver
  // Example: Sending statistics of Sensor 1 RH
  float s1RhMin = calculateMin(s1RhData, dataSize);
  float s1RhMax = calculateMax(s1RhData, dataSize);
  float s1RhAvg = calculateAvg(s1RhData, dataSize);
  float s1RhStdDev = calculateStdDev(s1RhData, dataSize);

  String message = id + ",Da," +
                   String(s1RhMin) + "," + String(s1RhMax) + "," + String(s1RhAvg) + "," + String(s1RhStdDev);

  // Send the message
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}


void sendSyncMessage() {
  String message = id + ",Sy";
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

void sendScheduleMessage() {
  String message = id + ",Sh";
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

bool waitForAckMessage(String expectedMessageType, unsigned long timeout) {
  unsigned long startTime = millis();
  while (millis() - startTime < timeout) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String incoming = LoRa.readString();
      String receivedID = incoming.substring(0, 15);
      String messageType = incoming.substring(16, 18);
      if (receivedID == id && messageType == expectedMessageType) {
        return true;
      } else if (receivedID == id && messageType == "Rs") {
        resetArduino();
      }
    }
  }
  return false;  // Timeout without receiving the expected acknowledgment
}

void collectData() {
  SensorData data;
  data.timestamp = millis();

  float battVoltage = analogRead(batteryPin) * (5.0 / 1023.0);
  float solarVoltage = analogRead(solarPin) * (5.0 / 1023.0);

  data.battVoltage = battVoltage;
  data.solarVoltage = solarVoltage;
  // Turn on sensor 1, wait for stabilization, and read RS485
  turnSensor(sensorPin1, "on");
  sensorData[6] = { 0, 0, 0, 0, 0, 0 };
  readRS485(30, 0, 5);
  turnSensor(sensorPin1, "off");

  data.s1Id = sensorData[0];
  data.s1Rh = sensorData[1];
  data.s1T = sensorData[2];
  data.s1Ec = sensorData[3];
  data.s1Ph = sensorData[4];
  data.s1Sa = sensorData[5];

  // Turn on sensor 2, wait for stabilization, and read RS485
  turnSensor(sensorPin2, "on");
  sensorData[6] = { 0, 0, 0, 0, 0, 0 };
  readRS485(30, 0, 5);
  turnSensor(sensorPin2, "off");

  data.s2Id = sensorData[0];
  data.s2Rh = sensorData[1];
  data.s2T = sensorData[2];
  data.s2Ec = sensorData[3];
  data.s2Ph = sensorData[4];
  data.s2Sa = sensorData[5];

  // Turn on sensor 3, wait for stabilization, and read RS485
  turnSensor(sensorPin3, "on");
  sensorData[6] = { 0, 0, 0, 0, 0, 0 };
  readRS485(30, 0, 5);
  turnSensor(sensorPin3, "off");


  data.s3Id = sensorData[0];
  data.s3Rh = sensorData[1];
  data.s3T = sensorData[2];
  data.s3Ec = sensorData[3];
  data.s3Ph = sensorData[4];
  data.s3Sa = sensorData[5];

  // Calculate the EEPROM address to store the data
  int address = eepromStartAddress + (wakeupCounter % maxDataEntries) * sizeof(SensorData);

  // Save the collected data to EEPROM
  saveDataToEEPROM(data, address);
}



void checkAndPerformTimingOperations() {
  // Determine which operations need to be performed without executing them
  bool collectRequired = (wakeupCounter % dataCollectionCycle == 0);
  bool sendRequired = (wakeupCounter % dataTransmitCycle == 0);
  bool syncRequired = (wakeupCounter % syncCycle == 0);

  // Perform the operations based on priority: collecting, sending, then synchronizing
  if (collectRequired) {
    collectData();
  }
  if (sendRequired) {
    sendData();
  }
  if (syncRequired) {
    syncTime();
  }
}

void resetArduino() {
  wdt_enable(WDTO_15MS);  // Enable watchdog timer for 15 ms to trigger a system reset
  while (1) {};
  ;  // Wait for the reset to occur
}