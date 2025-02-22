
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


int vin = A0;
int sensorV = A1;
float vin_m;
float vin_measure;

SoftwareSerial mySerial(Rx, Tx);
ModbusMaster node;

volatile bool f_wdt = true;      // Flag for Watchdog Timer
volatile int wakeUpCounter = 0;  // Counter for wake-ups

int cc = 0;
static uint32_t i;
uint8_t j, result;
uint16_t data[5];
String d;
uint16_t val;

int loopDelay = 20;  //seconds

String readRS485Device(uint8_t deviceAddress, uint8_t st, uint8_t n) {
  String d = ",,,,,";
  node.begin(deviceAddress, mySerial);  // Set the Modbus address and use the SoftwareSerial connection
  //Serial.println("Check RS485");
  result = node.readHoldingRegisters(st, n);

  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    d = String(deviceAddress) + "," + String(val);
    for (j = 1; j < n; j++) {
      val = node.getResponseBuffer(j);
      //Serial.println(val);
      d = d + "," + String(val);
    }
  }
  return d;
}

void setup() {
  pinMode(NSS, OUTPUT);
  pinMode(NRESET, OUTPUT);
  pinMode(DIO0, INPUT);
  pinMode(vin, INPUT);
  pinMode(sensorV, OUTPUT);

  digitalWrite(sensorV, LOW);
  Serial.begin(115200);
  Serial.println("Starting Module");
  while (!Serial)
    ;

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  delay(100);
  Serial.println("LoRa Transmitter");
  LoRa.sleep();  // Put the LoRa module to sleep
  delay(500);
  //EEPROM.put(EEPROM_ADDRESS, timerValue);
  EEPROM.get(EEPROM_ADDRESS1, timerValue);
  //EEPROM.put(EEPROM_ADDRESS, ID);
  ID = readStringFromEEPROM(EEPROM_ADDRESS2);
  delay(100);
  mySerial.begin(4800);

  delay(100);
  Serial.println(ID);
  //node.begin(50, mySerial);
  // Set up the watchdog timer to wake up every 8 seconds
}


int repeat8 = 15;
int delaySensor = 5;  ///sec
void loop() {
  //Serial.println("-----------------------");

  if (f_wdt) {
    f_wdt = false;  // Clear the watchdog timer flag

    // Increment wake-up counter
    wakeUpCounter++;

    // Check if 8 wake-ups (1 minute) have passed
    if (wakeUpCounter >= repeat8) {
      wakeUpCounter = 0;  // Reset wake-up counter
      timerValue += (8 * repeat8 + delaySensor);
      cc++;
      //Serial.println(" Vin");
      turnOnADC();
      float vin_m = analogRead(vin);
      vin_measure = vin_m * 0.00978;  //* (8/4);
      delay(100);


      String d1 = "";
      String d2 = "";
      String d3 = "";

      delay(100);
      Serial.print("Turn on Sensor");
      digitalWrite(sensorV, HIGH);
      delay(delaySensor * 1000);

      //Serial.print(", rs485 1");
      d1 = readRS485Device(30, 0, 5);
      delay(2000);

      //Serial.print(", rs485 2");
      d2 = readRS485Device(60, 0, 5);
      delay(2000);
      //Serial.print(", rs485 3");
      d3 = readRS485Device(90, 0, 5);

      delay(500);


      //Serial.print(", Turn off Sensor");
      digitalWrite(sensorV, LOW);

      delay(1000);
      turnOffADC();
      delay(100);
      Serial.println(", prepare message");
      String mmsg = String(ID) + "," + String(timerValue);


      Serial.print("message: " + mmsg + " " + cc);
      Serial.println(", voltage: " + String(vin_measure));
      Serial.println(d1 + " " + d2 + " " + d3);



      Serial.println("Sending message");
      LoRa.begin(BAND);  // Wake up the LoRa module
      LoRa.beginPacket();
      LoRa.print(mmsg + "," + cc + "," + vin_measure + "," + d1 + "," + d2 + "," + d3);
      LoRa.endPacket();
      Serial.println("message sent");
      LoRa.sleep();  // Put the LoRa module to sleep

      //delay(3000); // Let serial communication finish

      EEPROM.put(EEPROM_ADDRESS1, timerValue);
      Serial.println("----- ------");
      wdt_enable(WDTO_8S);
      WDTCSR |= (1 << WDIE);  // Enable interrupt mode
      delay(100);
    }

    // Enter sleep mode
    sleepNow();
  }


  //delay(loopDelay * 1000);
}

void turnOffADC() {
  ADCSRA &= ~(1 << ADEN);  // Disable ADC
}

void turnOnADC() {
  ADCSRA |= (1 << ADEN);  // Enable ADC
}

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

ISR(WDT_vect) {
  f_wdt = true;  // Set the watchdog timer flag
}
