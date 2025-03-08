#include <SPI.h>
#include <LoRa.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>


#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

volatile boolean f_wdt = 1;

const int TRANSMITTER_ID = 1;                  // Change for each transmitter
const String TRANSMITTER_NAME = "FAB0000000";  // Change for each transmitter
unsigned long firstTransmitDelay = 0;
volatile unsigned int cycleTime = 32;  // ms  (adjustable)
unsigned long nextTransmitTime = 0;
const unsigned long ACK_TIMEOUT = 2000;

void watchdogSetup() {
  // Clear watchdog reset flag
  MCUSR &= ~(1 << WDRF);
  // Set up WDT interrupt
  WDTCSR = (1 << WDCE) | (1 << WDE);
  // Set up watchdog timer for 8 seconds timeout
  WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0);
}

ISR(WDT_vect) {
  f_wdt = 1;  // Set watchdog timer flag
}

void disableComponents() {
  power_all_disable();  // Disable all components for power saving
  // Add any additional components you need to disable here
}

void enableComponents() {
  power_all_enable();  // Enable all components
  // Add any additional components you need to enable here
}

void sleepForSeconds(int totalSeconds) {

  int iterations = totalSeconds / (8);
  Serial.println("sleep for: " + String(iterations * 8) + " sec");
  delay(5);
  for (int i = 0; i < iterations; i++) {
    disableComponents();  // Disable components before sleep

    // Enable sleep mode and enter sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();

    // Wait for watchdog timer interrupt
    while (!f_wdt) {};
    f_wdt = 0;

    enableComponents();  // Enable components after wakeup
  }
}

void serialEnable() {

  Serial.begin(57600);
  while (!Serial) {};
  Serial.println("serial enables");
}

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


void registerWithReceiver() {
  LoRa.beginPacket();
  LoRa.write(TRANSMITTER_ID);
  LoRa.print("R");  // Registration request
  LoRa.endPacket();

  Serial.println("Registration sent");

  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {  // 2 second timeout
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      firstTransmitDelay = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();
      cycleTime = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();
      //nextTransmitTime = millis() + firstTransmitDelay;

      Serial.print("Schedule received. Delay: ");
      delay(5);
      Serial.print(" Cycle time: ");
      delay(5);
      Serial.println(cycleTime);
      delay(5);
      return;
    }
  }
  Serial.println("Registration timeout");
}

void sendData() {
  LoRa.beginPacket();
  LoRa.write(TRANSMITTER_ID);
  LoRa.print("Hello from ");
  LoRa.print(TRANSMITTER_ID);
  LoRa.endPacket();
  Serial.println("Data Sent, Waiting for ACK");
  delay(5);
  if (!waitForAck()) {
    Serial.println("ACK Timeout, Retrying");
    delay(5);
    sendData();
  } else {
    Serial.println("Cycle time: " + String(cycleTime));
    delay(5);
  }
}


bool waitForAck() {
  unsigned long startTime = millis();
  while (millis() - startTime < ACK_TIMEOUT) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      if (LoRa.read() == TRANSMITTER_ID && LoRa.read() == 'A') {
        nextTransmitTime = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();

        Serial.println("ACK: Schedueled to run in " + String(nextTransmitTime) + " sec");
        delay(5);
        cycleTime = int(nextTransmitTime);
        Serial.println("Cycle: " + String(cycleTime) + " sec");
        delay(5);
        return true;
      }
    } else {
      Serial.println("no packet");
      //Serial.println("Cycle: " + String(cycleTime) + " sec");
      //delay(5);
    }
  }
  return false;
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
  // Put the microcontroller to sleep for a total of 24 seconds
  //sleepForSeconds(cycleTime);
}

void loop() {

  Serial.println("------ Start ---------");
  delay(5);
  sendData();
  Serial.println("------- END --------");
  delay(5);
  delay(500);
  Serial.println("Sleep: " + String(cycleTime) + " s");
  delay(5);
  sleepForSeconds(cycleTime);
}
