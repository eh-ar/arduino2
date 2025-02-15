
#include <SPI.h>
#include <LoRa.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <ModbusMaster.h>

const int EEPROM_ADDRESS = 0;
unsigned long timerValue = 0;
#define NSS 10
#define NRESET 7
#define DIO0 6
#define BAND 433E6

#define Rx 3
#define Tx 2

int temt6000Pin = A1;
float light;
int light_value;

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

String readRS485Device(uint8_t deviceAddress, uint8_t st, uint8_t n) {
  String d = "";
  node.begin(deviceAddress, mySerial);  // Set the Modbus address and use the SoftwareSerial connection
  //Serial.println("Check RS485");
  result = node.readHoldingRegisters(st, n);

  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    d = String(deviceAddress) + " " + String(val);
    for (j = 1; j < n; j++) {
      val = node.getResponseBuffer(j);
      //Serial.println(val);
      d = d + " " + String(val);
    }
  }
  return d;
}

void setup() {
  pinMode(NSS, OUTPUT);
  pinMode(NRESET, OUTPUT);
  pinMode(DIO0, INPUT);
  pinMode(temt6000Pin, INPUT);

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
  Serial.println("LoRa Transmitter");

  EEPROM.get(EEPROM_ADDRESS, timerValue);
  mySerial.begin(9600);
  //node.begin(50, mySerial);
  // Set up the watchdog timer to wake up every 8 seconds
  wdt_enable(WDTO_8S);
  WDTCSR |= (1 << WDIE);  // Enable interrupt mode
}


void loop() {
  Serial.println("loop");
  if (f_wdt) {
    f_wdt = false;  // Clear the watchdog timer flag

    // Increment wake-up counter
    wakeUpCounter++;
    timerValue += 8;
    // Check if 8 wake-ups (1 minute) have passed
    if (wakeUpCounter >= 1) {
      wakeUpCounter = 0;  // Reset wake-up counter

      cc++;
      Serial.println("analog read");
      int light_value = analogRead(temt6000Pin);
      light = light_value * 0.00978;  //* (8/4);

      String d1 = "";
      String d2 = "";


      Serial.println("rs485 1");
      d2 = readRS485Device(1, 0, 5);
      delay(100);
      //Serial.println(readRS485Device(1, 35, 3));
      //delay(100);
      //Serial.println(readRS485Device(1, 80, 4));
      //delay(100);

      /*
      Serial.println( readRS485Device(70));
      delay(100);
      Serial.println( readRS485Device(71));
      delay(100);
      Serial.println( readRS485Device(72));
      delay(100);
      Serial.println( readRS485Device(73));
      delay(100);
      Serial.println( readRS485Device(74));
      delay(100);
      */
      if (d2 == "") {
        delay(1000);
        d2 = readRS485Device(1, 0, 6);
      }
      //delay(5000);

      /*
      Serial.println("rs485 2");
      d2 = readRS485Device(51);
      if (d2 == ""){
        delay(5000);
        d2 = readRS485Device(51);
      }
      */

      String mmsg = String(timerValue);

      Serial.println("-----------------------");
      Serial.println("Sent: " + mmsg + " " + cc);
      Serial.println("light: " + String(light));
      Serial.println(d1);
      Serial.println(d2);

      LoRa.beginPacket();
      LoRa.print(mmsg + " - " + cc + " - " + light + " - " + d1 + " - " + d2);
      LoRa.endPacket();


      //delay(3000); // Let serial communication finish

      EEPROM.put(EEPROM_ADDRESS, timerValue);
    }

    // Enter sleep mode
    sleepNow();
  }
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
  sleep_disable();
}

ISR(WDT_vect) {
  f_wdt = true;  // Set the watchdog timer flag
}
