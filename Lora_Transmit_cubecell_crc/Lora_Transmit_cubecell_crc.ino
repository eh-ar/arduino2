/* Heltec Automation send communication test example
 * 
 * Fixed version that:
 * 1. Runs exactly once after startup
 * 2. Then enters regular wakeup-sleep cycle
 */

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <softSerial.h>
#include <ModbusMaster.h>
#include <CRC32.h>

// Shared secret (minimum 8 bytes recommended)
const char SECRET_KEY[] = "LoraKey7742";

softSerial softwareSerial(GPIO5 /*TX pin*/, GPIO0 /*RX pin*/);
ModbusMaster node;

#define RF_FREQUENCY 433000000  // Hz
#define TX_OUTPUT_POWER 23      // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30  // Define the payload size here

#define vext GPIO6
char txpacket[90];
char txpacket0[BUFFER_SIZE];
char txpacket1[BUFFER_SIZE];
char txpacket2[BUFFER_SIZE];
char txpacket3[BUFFER_SIZE];

char rxpacket[BUFFER_SIZE];

int counter;
bool lora_idle = true;
bool firstRunComplete = false;  // Tracks if initial transmission is done

static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

#define timetillsleep 5000      // Time active after wakeup
#define timetillwakeup 400000    // Time sleeping between transmissions
#define sensorDelay 2000        // Delay between sensor readings
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower = 0;           // Start in active mode (0 = awake)

String deviceID = "Faraz00004";

void onSleep() {
  //Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n", timetillwakeup);
  //delay(5);
  lowpower = 1;
  TimerSetValue(&wakeUp, timetillwakeup);
  TimerStart(&wakeUp);
}

void onWakeUp() {
  //Serial.printf("Woke up, %d ms later into lowpower mode.\r\n", timetillsleep);
  //delay(5);
  lowpower = 0;
  TimerSetValue(&sleep, timetillsleep);
  TimerStart(&sleep);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Lora Transmitter");
  delay(10);
  
  pinMode(vext, OUTPUT);
  digitalWrite(vext, HIGH);

  pinMode(GPIO1, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  pinMode(GPIO3, OUTPUT);

  digitalWrite(GPIO1, LOW);
  digitalWrite(GPIO2, LOW);
  digitalWrite(GPIO3, LOW);

  softwareSerial.begin(4800);
  node.begin(1, softwareSerial);
  counter = 0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  Serial.println("Lora Transmitter Initialized");
  delay(5);
  
  TimerInit(&sleep, onSleep);
  TimerInit(&wakeUp, onWakeUp);
  
  // Perform initial transmission right in setup()
  //sendSensorData();
  firstRunComplete = true;
  
  // Start sleep timer (will go to sleep after timetillsleep)
  TimerSetValue(&sleep, timetillsleep);
  TimerStart(&sleep);
}

void loop() {
  if (lowpower) {
    lowPowerHandler();
  } else {
    if (lora_idle == true && firstRunComplete) {
      sendSensorData();
    }
    Radio.IrqProcess();
  }
}

void sendSensorData() {
  counter++;

  uint8_t j, result;
  uint16_t data1[5], data2[5], data3[5];
  String Out = "";

  double voltage = getBatteryVoltage();
  //Serial.println(voltage);
  //Serial.println(voltage/1000);
  digitalWrite(vext, LOW);

  // Read sensor 1
  digitalWrite(GPIO1, HIGH);
  //Serial.print("Sensor 1");
  delay(sensorDelay);

  data1[0] = 0;
  data1[1] = 0;
  data1[2] = 0;
  data1[3] = 0;
  data1[4] = 0;

  result = node.readHoldingRegisters(0, 5);
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 5; j++) {
      data1[j] = node.getResponseBuffer(j);
    }
  }
  digitalWrite(GPIO1, LOW);

  // Read sensor 2
  digitalWrite(GPIO2, HIGH);
  //Serial.print(",Sensor 2");
  delay(sensorDelay);

  data2[0] = 0;
  data2[1] = 0;
  data2[2] = 0;
  data2[3] = 0;
  data2[4] = 0;
  result = node.readHoldingRegisters(0, 5);
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 5; j++) {
      data2[j] = node.getResponseBuffer(j);
    }
  }
  digitalWrite(GPIO2, LOW);

  // Read sensor 3
  digitalWrite(GPIO3, HIGH);
  //Serial.print(",Sensor 3");
  delay(sensorDelay);

  data3[0] = 0;
  data3[1] = 0;
  data3[2] = 0;
  data3[3] = 0;
  data3[4] = 0;
  result = node.readHoldingRegisters(0, 5);
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 5; j++) {
      data3[j] = node.getResponseBuffer(j);
    }
  }
  digitalWrite(GPIO3, LOW);

  digitalWrite(vext, HIGH);

  // Create String objects for each part
  String packet0 = String(counter) + "," + String(0) + "," + String(voltage);
  String packet1 = String(1) + "," + String(data1[0]) + "," + String(data1[1]) + "," + 
                   String(data1[2]) + "," + String(data1[3]) + "," + String(data1[4]);
  String packet2 = String(2) + "," + String(data2[0]) + "," + String(data2[1]) + "," + 
                   String(data2[2]) + "," + String(data2[3]) + "," + String(data2[4]);
  String packet3 = String(3) + "," + String(data3[0]) + "," + String(data3[1]) + "," + 
                   String(data3[2]) + "," + String(data3[3]) + "," + String(data3[4]);

  // Combine sensor data
  String sensorData = packet1 + "," + packet2 + "," + packet3;
  
  // Prepare and send LoRa message
  String data = String(0, 2) + "," + String(voltage/1000) + "," + sensorData;
  uint32_t checksum = calculateChecksum(deviceID, data, counter);
  String loraMessage = deviceID + "|" + String(counter) + "|" + data + "|" + String(checksum, HEX);

  Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", loraMessage.c_str(), loraMessage.length());
  delay(100);

  // Send the String
  Radio.Send((uint8_t *)loraMessage.c_str(), loraMessage.length());
  lora_idle = false;
}

void OnTxDone(void) {
  Serial.println("TX done......");
  delay(5);
  Radio.Sleep();
  lora_idle = true;
}

void OnTxTimeout(void) {
  Radio.Sleep();
  Serial.println("TX Timeout......");
  delay(5);
  lora_idle = true;
}

uint32_t calculateChecksum(const String &code, const String &data, uint32_t counter) {
  CRC32 crc;

  // 1. Add secret key (as bytes)
  crc.update((const uint8_t *)SECRET_KEY, strlen(SECRET_KEY));

  // 2. Add code (as bytes)
  crc.update((const uint8_t *)code.c_str(), code.length());

  // 3. Add data (as bytes)
  crc.update((const uint8_t *)data.c_str(), data.length());

  // 4. Add counter (as 4 bytes, little-endian)
  uint8_t counterBytes[4];
  counterBytes[0] = (counter >> 0) & 0xFF;
  counterBytes[1] = (counter >> 8) & 0xFF;
  counterBytes[2] = (counter >> 16) & 0xFF;
  counterBytes[3] = (counter >> 24) & 0xFF;
  crc.update(counterBytes, 4);

  return crc.finalize();
}