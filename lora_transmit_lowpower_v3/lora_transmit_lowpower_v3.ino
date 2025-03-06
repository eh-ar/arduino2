
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
int vin_pin = A0;
int sensorV = A1;
float vin_m;
float vin;
float vin_measure;

ModbusMaster node;

volatile bool f_wdt = true;      // Flag for Watchdog Timer
volatile int wakeUpCounter = 0;  // Counter for wake-ups
volatile int milliLoop = 0;
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

///----------------
int repeat8 = 20;
int sensorIDs[] = { 30 };
int sensorId = 0;
int sensorCounter = 0;
int delaySensor = 5;  ///sec

int miliStart = 0;
int miliEnd = 0;

int sensorNo = 0;
//------------------------------
void setPins() {

  Serial.print("settings pins");
  delay(10);

  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A7, OUTPUT);
  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A6, LOW);
  digitalWrite(A7, LOW);

  pinMode(SDA, INPUT);
  pinMode(SCL, INPUT);
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);

  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);

  pinMode(NSS, OUTPUT);
  pinMode(NRESET, OUTPUT);
  pinMode(DIO0, INPUT);
  pinMode(vin_pin, INPUT);
  pinMode(sensorV, OUTPUT);

  digitalWrite(sensorV, LOW);

  delay(10);
  Serial.println(", done");
}
//--------------------------------
void getEEPROM() {
  Serial.println("Read EEPROM");
  delay(10);
  EEPROM.get(EEPROM_ADDRESS1, timerValue);
  ID = readStringFromEEPROM(EEPROM_ADDRESS2);

  delay(10);
  Serial.println(ID);
  return;
}


//-------------------------------
void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println("--------- SETUP --------");
  Serial.println("Setup");
  sensorNo = sizeof(sensorIDs) / sizeof(sensorIDs[0]);
  Serial.println("No. of Sensors: " + String(sensorNo));
  setPins();

  getEEPROM();

  Serial.print("Lora Module");
  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println(", failed!");
    while (1) {};
  }
  Serial.println(" active");

  transmitInit(ID);
  message = ID;

  delay(1000);
  Serial.flush();
}

//
void transmitInit(String ID) {
  sendMessage(ID + ";;" + String(readVoltage()) + ";;register");
  delay(500);

  return;
}
//-------------------------------------------

void loop() {
  //Serial.println("-----------------------");

  if (f_wdt) {

    f_wdt = false;  // Clear the watchdog timer flag
    // Increment wake-up counter
    wakeUpCounter++;


    if (wakeUpCounter >= repeat8) {
      miliStart = millis();
      Serial.begin(57600);

      Serial.println("---------- Cycle " + String(sensorCounter + 1) + " ---------");
      delay(commandDelay * 1000);  // 1

      sensorId = sensorIDs[sensorCounter];
      if (sensorCounter < sensorNo - 1) {
        sensorCounter++;
      } else {
        wakeUpCounter = 0;  // Reset wake-up counter
        sensorCounter = 0;  // Reset wake-up counter
      }

      timerValue += (8 * repeat8 + delaySensor);

      message = message + ";;" + String(timerValue) + "," + String(cc);

      delay(commandDelay * 1000);  //2

      vin_measure = readVoltage();
      message = message + "," + String(vin_measure);
      delay(commandDelay * 1000);  //3

      sensor("on");
      delay(delaySensor * 1000);  //4
      String d = readRS485Device(sensorId, 0, 5);
      message = message + "," + d;
      sensor("off");

      Serial.println("msg: " + message);
      EEPROM.put(EEPROM_ADDRESS1, timerValue);

      wdt_enable(WDTO_8S);
      WDTCSR |= (1 << WDIE);  // Enable interrupt mode

      delay(10);

      if (wakeUpCounter == 0) {  // all the sensors are read and the message is ready to be sent
        sendMessage(message);
        message = ID;
        cc++;
      }
      Serial.println("Data collection Time: " + String(millis() - miliStart) + " ms");
      delay(10);
      Serial.flush();
    }
    // Enter sleep mode
    sleepNow();
    //delay(8000);
  }
}

void sensor(String stat) {
  if (stat = "on") {
    digitalWrite(sensorV, HIGH);
  } else {
    digitalWrite(sensorV, LOW);
  }
}
//---------------------
float readVoltage() {
  turnOnADC();
  float vin;
  float vin_m = analogRead(vin_pin);
  vin = vin_m * 0.00978;  //* (8/4);
  delay(10);
  Serial.println("vin: " + String(vin_m) + " " + String(vin) + " v");
  turnOffADC();
  return vin;
}

//--------------------------
void sendMessage(String loraMessage) {
  Serial.print("Sending message, ");
  LoRa.begin(BAND);  // Wake up the LoRa module
  LoRa.beginPacket();
  LoRa.print(loraMessage);
  LoRa.endPacket();
  Serial.println(" Done");
  delay(10);
  LoRa.sleep();  // Put the LoRa module to sleep
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

//------------------------------------------
String readRS485Device(uint8_t deviceAddress, uint8_t st, uint8_t n) {
  mySerial.begin(4800);
  Serial.print("rs485 " + String(sensorId) + ", ");
  delay(10);
  Serial.print(", reading ");
  String d = ",,,,,";
  node.begin(deviceAddress, mySerial);  // Set the Modbus address and use the SoftwareSerial connection
  //Serial.println("Check RS485");
  result = node.readHoldingRegisters(st, n);
  Serial.print(" parsing ");
  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    d = String(deviceAddress) + "," + String(val);
    for (j = 1; j < n; j++) {
      val = node.getResponseBuffer(j);
      //Serial.println(val);
      d = d + "," + String(val);
    }
  }
  Serial.println(", data: " + d + " ");
  mySerial.flush();
  return d;
}

//-----------------------------------------
void sleepNow() {
  miliStart = millis();
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
