
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
  while (!Serial);
 
  mySerial.begin(9600);

 
}


void loop() {
  Serial.println("loop");
  

    // Increment wake-up counter
    wakeUpCounter++;
    timerValue += 8;
    
   
      cc++;
      Serial.println("analog read");
      int light_value = analogRead(temt6000Pin);
      light = light_value * 0.00978;  //* (8/4);

      String d1 = "";
      String d2 = "";


      Serial.println("rs485 1");
      d2 = readRS485Device(51, 0, 5);
      delay(100);
    
    
      if (d2 == "") {
        delay(1000);
        d2 = readRS485Device(51, 0, 5);
      }
      //delay(5000);


      String mmsg = String(timerValue);

      Serial.println("-----------------------");
      Serial.println("Sent: " + mmsg + " " + cc);
      Serial.println("light: " + String(light));
      Serial.println(d1);
      Serial.println(d2);
      delay(8000);

      

}
