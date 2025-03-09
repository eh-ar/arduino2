#include <SPI.h>
#include <LoRa.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <EEPROM.h>


unsigned long secCount = 0;
int secCountAddress = 0;

String dataMessageType = "Data";
String acceptMessageType = "Acpt";
String regsiterMessageType = "Regs";
String schduleMessageType = "Sche";
String syncMessageType = "Sync";
String resetMessageType = "Rest"

const String TRANSMITTER_NAME = "FAB0000000";  // Change for each transmitter

unsigned long sendCycle;  //in sec cycle of sending data
unsigned long syncCycle;  // in sec cycle of syncing with receiver
const unsigned long ACK_TIMEOUT = 2000;

//---------------------------------------
void serialEnable() {
  Serial.begin(57600);
  while (!Serial) {};
  Serial.println("serial enables");
}

void setup() {
  // enable serial;
  serialEnable();
  // enable lora;
  loraEnable();
  // register transmitter
  registerWithReceiver();
  // Initialize watchdog timer
  watchdogSetup();
  //read timer saved in eeprom
  readTimer();
}
//--------------------------------------
void loop() {
  Serial.println("------ Start ---------");
  delay(1);
  Serial.println(String(secCount) + " s");
  delay(1);
  sendData();
  Serial.println("------- SLEEP --------");
  delay(50);
  sleepForSeconds(cycleTime);
}
