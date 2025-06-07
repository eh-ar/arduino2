#include <SPI.h>
#include <LoRa.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>  // Watchdog timer if needed

#define NSS 10
#define NRESET 9
#define DIO0 8
#define BAND 433E6

int cc = 0;
const int TRANSMITTER_ID = 1;  // Change for each transmitter
unsigned long firstTransmitDelay = 0;
unsigned long cycleTime = 3 * 1000;  // ms  (adjustable)
unsigned long nextTransmitTime = 0;
const unsigned long ACK_TIMEOUT = 500;

volatile bool wakeUpFlag = false;

void setup() {
  Serial.begin(57600);
  while (!Serial)
    ;

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {  // Adjust frequency
    Serial.println("LoRa init failed");
    while (1)
      ;
  }
  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);

  registerWithReceiver();
  configureTimer1();
}


int cc2 = 0;
void loop() {
  Serial.println(cc2);
  delay(10);
  cc2++;
  if (wakeUpFlag) {
    wakeUpFlag = false;
    sendData();
    Serial.println("set Timer");
    delay(500);

    configureTimer1();  // Reconfigure for next cycle
  }
  sleepNow();
}

void configureTimer1() {
  Serial.println("timer");
  delay(5);


  unsigned long timeToSleep = nextTransmitTime - millis();
  Serial.println("Sleep for " + String(timeToSleep) + " " + String(nextTransmitTime));
  delay(30);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  noInterrupts();

  if (timeToSleep > 0 && nextTransmitTime != 0) {
    Serial.println("enable timer");
    delay(50);
    /*
    OCR1A = 200;// timeToSleep * 0.01;  // 16 prescaler

    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS10);  //16 prescaler
    TIMSK1 |= (1 << OCIE1A);
    */
    TCCR1A = 0x00;  //this register contains WGM11, WGM10 bits
    TCCR1B = (1 << WGM12) | (1 << CS12);
    //OCR1A = 0xFFFF;
    OCR1A = 0xF424;
    TIMSK1 = (1 << OCIE1A);

    TCNT1 = 0;
  } else {
    Serial.println("No Sleep");
    delay(5);
    wakeUpFlag = true;
  }

  Serial.println("interrupt");
  delay(10);

  interrupts();

  Serial.println("run");
  delay(10);
}

ISR(TIMER1_COMPA_vect) {
  wakeUpFlag = true;
  TCCR1B &= ~(1 << CS12);  // Turn off timer
}

void sleepNow() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  power_all_disable();
  sleep_mode();
  sleep_disable();
  power_all_enable();

}

void sendData() {
  LoRa.beginPacket();
  LoRa.write(TRANSMITTER_ID);
  LoRa.print("Hello from ");
  LoRa.print(TRANSMITTER_ID);
  LoRa.print(" ");
  LoRa.print(cc);
  LoRa.endPacket();
  cc++;
  Serial.println("Data Sent, Waiting for ACK");
  delay(5);
  if (!waitForAck()) {
    Serial.println("ACK Timeout, Retrying");
    delay(10);
    sendData();
  } else {
    Serial.println("adjust");
    delay(10);
    nextTransmitTime = cycleTime;
  }
}

bool waitForAck() {
  unsigned long startTime = millis();
  while (millis() - startTime < ACK_TIMEOUT) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      Serial.println("packet: ");
      Serial.println(packetSize);
      delay(10);
      if (LoRa.read() == TRANSMITTER_ID && LoRa.read() == 'A') {
        nextTransmitTime = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();

        Serial.println("ACK: " + String(nextTransmitTime));
        delay(5);
        return true;
      }
    } else {
    }
  }
  return false;
}

void registerWithReceiver() {
  LoRa.beginPacket();
  LoRa.write(TRANSMITTER_ID);
  LoRa.print("R");  // Registration request
  LoRa.endPacket();

  Serial.println("Registration sent");

  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {  // 10 second timeout
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      firstTransmitDelay = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();
      cycleTime = (unsigned long)LoRa.read() << 24 | (unsigned long)LoRa.read() << 16 | (unsigned long)LoRa.read() << 8 | LoRa.read();
      nextTransmitTime = millis() + firstTransmitDelay;
      Serial.print("Schedule received. First Delay: ");
      Serial.print(firstTransmitDelay);
      Serial.print(" Cycle time: ");
      Serial.println(cycleTime);
      return;
    }
  }
  Serial.println("Registration timeout");
}
