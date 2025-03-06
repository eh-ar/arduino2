#include <SPI.h>
#include <LoRa.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

#define NSS 10
#define NRESET 5
#define DIO0 4
#define BAND 433E6

volatile unsigned int seconds = 0;
volatile unsigned int sleepInterval = 30;  // Set desired sleep interval in seconds

const int NUM_TRANSMITTERS = 5;

int cc = 0;
const int TRANSMITTER_ID = 1;  // Change for each transmitter
unsigned long firstTransmitDelay = 0;
unsigned long cycleTime = 3 * 1000;  // ms  (adjustable)
unsigned long nextTransmitTime = 0;
const unsigned long ACK_TIMEOUT = 500;

void setup() {
  Serial.begin(57600);

  // Set all pins as inputs with pull-ups to reduce power consumption
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
  // Setup Timer2
  setupTimer2();

  // Enable global interrupts
  sei();
}

void loop() {
  // Sleep for the desired interval
  while (seconds < sleepInterval) {
    goToSleep();
  }

  // Perform your processing here
  sendData();
  Serial.println("set Timer");
  delay(500);

  setupTimer2();  // Reconfigure for next cycle

  // Reset the seconds counter
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    seconds = 0;
  }

  // Re-enable Timer2 interrupt for the next sleep cycle
  TIMSK2 |= (1 << TOIE2);
}

void setupTimer2() {
  // Set up Timer2 to overflow approximately every 1 second
  TCCR2A = 0;                                        // Normal mode
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 1024
  TCNT2 = 0;
  TIMSK2 = (1 << TOIE2);  // Enable Timer2 overflow interrupt
}

void goToSleep() {
  // Enable sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();

  // Go to sleep
  sleep_mode();

  // MCU will continue from here after waking up
  sleep_disable();
}

// Timer2 overflow interrupt service routine
ISR(TIMER2_OVF_vect) {
  static unsigned int overflowCount = 0;
  overflowCount++;

  if (overflowCount >= 61) {  // Approximately 1 second (16 MHz / 1024 / 256)
    overflowCount = 0;
    seconds++;
  }

  if (seconds >= sleepInterval) {
    // Disable Timer2 interrupt to stop waking up
    TIMSK2 &= ~(1 << TOIE2);
  }
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

        Serial.println("ACK: Schedueled to run in " + String(nextTransmitTime));

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
