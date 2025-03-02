
#include <SPI.h>
#include <LoRa.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <ModbusMaster.h>

const int EEPROM_ADDRESS1 = 50;
unsigned long timerValue = 0;

const int EEPROM_ADDRESS2 = 0;
String ID = "";

#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

#define Rx 2
#define Tx 3

SoftwareSerial mySerial(Rx, Tx);
int vin = A0;
int sensorV = A1;
float vin_m;
float vin_measure;

ModbusMaster node;

volatile bool f_wdt = true;      // Flag for Watchdog Timer
volatile int wakeUpCounter = 0;  // Counter for wake-ups
String message = "";

int cc = 1;
static uint32_t i;
uint8_t j, result;
uint16_t data[5];
String d;
uint16_t val;

int loopDelay = 20;        //seconds
float commandDelay = 0.5;  //seonds
bool debugFlag = false;

//------------------------------------------
String readRS485Device(uint8_t deviceAddress, uint8_t st, uint8_t n) {
  mySerial.begin(4800);
  delay(commandDelay * 1000);
  Serial.print(", start reading");
  String d = ",,,,,";
  node.begin(deviceAddress, mySerial);  // Set the Modbus address and use the SoftwareSerial connection
  //Serial.println("Check RS485");
  result = node.readHoldingRegisters(st, n);
  Serial.print(", parsing results");
  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    d = String(deviceAddress) + "," + String(val);
    for (j = 1; j < n; j++) {
      val = node.getResponseBuffer(j);
      //Serial.println(val);
      d = d + "," + String(val);
    }
  }
  Serial.print(", data: " + d + " ");
  mySerial.flush();
  return d;
}
//-------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Setup");

  pinMode(NSS, OUTPUT);
  pinMode(NRESET, OUTPUT);
  pinMode(DIO0, INPUT);
  pinMode(vin, INPUT);
  pinMode(sensorV, OUTPUT);

  //digitalWrite(sensorV, LOW);

  Serial.println("Starting Module");
  while (!Serial) {};

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1) {};
  }
  delay(commandDelay * 1000);
  Serial.println("LoRa Transmitter");
  //sendMessage("Lora Start");
  //LoRa.sleep();  // Put the LoRa module to sleep
  delay(50);
  //EEPROM.put(EEPROM_ADDRESS, timerValue);
  EEPROM.get(EEPROM_ADDRESS1, timerValue);
  //EEPROM.put(EEPROM_ADDRESS, ID);
  ID = readStringFromEEPROM(EEPROM_ADDRESS2);
  message = ID;
  delay(commandDelay * 1000);

  Serial.println(ID);
  Serial.flush();
  delay(commandDelay * 1000);

  //node.begin(50, mySerial);
  // Set up the watchdog timer to wake up every 8 seconds
}

//-------------------------------------------
int repeat8 = 15;
int sensorIDs[3] = { 30, 60, 90 };
int sensorId = 0;
int sensorCounter = 0;
int delaySensor = 5;  ///sec

void loop() {
  //Serial.println("-----------------------");

  if (f_wdt) {

    f_wdt = false;  // Clear the watchdog timer flag
    // Increment wake-up counter
    wakeUpCounter++;

    // Check if 8 wake-ups (1 minute) have passed
    if (wakeUpCounter >= repeat8) {
      Serial.begin(115200);
      Serial.println("Reading Cycle");
      delay(commandDelay * 1000);

      sensorId = sensorIDs[sensorCounter];
      if (sensorCounter < 2) {
        sensorCounter++;
      } else {
        wakeUpCounter = 0;  // Reset wake-up counter
        sensorCounter = 0;  // Reset wake-up counter
      }

      timerValue += (8 * repeat8 + delaySensor);

      //Serial.println(" Vin");
      //turnOnADC();
      delay(commandDelay * 1000);

      vin_measure = readVoltage();
      message = message + ","+ String(vin_measure);

      delay(commandDelay * 1000);
      Serial.println("Turn on Sensor");
      digitalWrite(sensorV, HIGH);
      delay(delaySensor * 1000);

      Serial.print(", rs485 " + String(sensorId) + " ");
      String d = readRS485Device(sensorId, 0, 5);
      Serial.println(", done");

      message = message + "," + d;

      delay(commandDelay * 1000);
      Serial.println(", Turn off Sensor");
      digitalWrite(sensorV, LOW);

      //turnOffADC();
      delay(commandDelay * 1000);
      Serial.println(", prepare message");
      message = message + "," + String(timerValue) + "," + String(cc);
      Serial.println("msg: " + message);
      EEPROM.put(EEPROM_ADDRESS1, timerValue);

      wdt_enable(WDTO_8S);
      WDTCSR |= (1 << WDIE);  // Enable interrupt mode
      delay(commandDelay * 1000);

      if (wakeUpCounter == 0) {  // all the sensors are read and the message is ready to be sent
        sendMessage(message);
        //sendMessageWithAck(message);
        
        message = ID;
        cc++;
      }

      Serial.flush();
    }
    // Enter sleep mode
    Serial.println("----- sleep  -----");
    sleepNow();
    //delay(8000);
  }
}
//---------------------
float readVoltage() {
  float vin;
  float vin_m = analogRead(vin);
  vin = vin_m * 0.00978;  //* (8/4);
  delay(10);
  Serial.println(", voltage: " + String(vin_measure));
  return vin;
}

//--------------------------
void sendMessage(String loraMessage) {
  Serial.println("Sending message");
  LoRa.begin(BAND);  // Wake up the LoRa module
  LoRa.beginPacket();
  LoRa.print(loraMessage);
  LoRa.endPacket();
  Serial.println("message sent");
  LoRa.sleep();  // Put the LoRa module to sleep
}

//----------------------------------
unsigned long ackTimeout = 5000; // 5 seconds timeout for ACK
unsigned long sendTime;
bool ackReceived = false;

void sendMessageWithAck(String message) {
  Serial.println("Sending message with ACK request: " + message);
  LoRa.begin(BAND);
  LoRa.beginPacket();
  LoRa.print(message + ",ACK_REQ"); // Append an ACK request flag
  LoRa.endPacket();
  LoRa.sleep();

  sendTime = millis();
  ackReceived = false;

  Serial.println("wait for Ack");
  while (millis() - sendTime < ackTimeout) {
    if (LoRa.parsePacket()) {
      String received = "";
      while (LoRa.available()) {
        received += (char)LoRa.read();
      }
      if (String(received) == ID) {
        Serial.println("ACK received!");
        ackReceived = true;
        break;
      }
    }
    delay(10); // Small delay to avoid busy-waiting
  }

  if (ackReceived) {
    Serial.println("Transmission successful!");
  } else {
    Serial.println("ACK timeout or not received. Resend or handle failure.");
    // Implement resend logic here if needed
  }
}
//----------------------------------------
void turnOffADC() {
  ADCSRA &= ~(1 << ADEN);  // Disable ADC
}

void turnOnADC() {
  ADCSRA |= (1 << ADEN);  // Enable ADC
}
//----------------------------------------
String readStringFromEEPROM(int address) {
  int len;
  EEPROM.get(address, len);  // Read string length
  char data[len + 1];        // Create a char array to hold the string and null-terminator
  for (int i = 0; i < len; i++) {
    data[i] = EEPROM.read(address + sizeof(len) + i);  // Read string characters
  }
  data[len] = '\0';     // Ensure null-termination
  return String(data);  // Convert char array to String
}

//-----------------------------------------
void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();  // Disable interrupts
  sleep_enable();  // Enable sleep mode
  // Enable watchdog timer interrupt
  WDTCSR |= (1 << WDIE);
  interrupts();  // Enable interrupts
  sleep_cpu();   // Enter sleep mode
  // Disable sleep mode after wake up
  //delay(10);
  //Serial.println("Waking up");
  sleep_disable();
}
//------------------------------------------
ISR(WDT_vect) {
  f_wdt = true;  // Set the watchdog timer flag
}
