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

volatile unsigned int interruptCounter = 0;
volatile unsigned int inerruptsPerSecond = 60;

volatile bool timeToWakeUp = false;
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
  //setupTimer2();

  // Enable global interrupts
  sei();
}

void loop() {
  Serial.println("LOOP");
  delay(5);
  // Slee!p for the desired interval
  while (!timeToWakeUp) {
    //Serial.println("Sleep");
    //delay(5);
    goToSleep();
  }

  // Perform your processing here
  sendData();

  setupTimer2();  // Reconfigure for next cycle

  // Reset the seconds counter
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    interruptCounter = 0;
    timeToWakeUp = false;
  }

  // Re-enable Timer2 interrupt for the next sleep cycle
  TIMSK2 |= (1 << TOIE2);
}

void setupTimer2() {
  Serial.println("TIMER");
  delay(5);
  TCCR2A = (1 << WGM21);                             // CTC mode
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 1024
  OCR2A = 249;                                       // Corrected value for 1s interval
  TCNT2 = 0;
  TIMSK2 = (1 << OCIE2A);  // Enable Timer2 compare match interrupt
  Serial.println("TIMER Done");
  delay(5);
}

void goToSleep() {
  //Serial.println("SLEEP MODE");
  //delay(5);
  // Enable sleep mode
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  // Go to sleep
  sleep_mode();

  // MCU will continue from here after waking up
  sleep_disable();
}

// Timer2 overflow interrupt service routine
ISR(TIMER2_OVF_vect) {
  interruptCounter++;
  Serial.println(interruptCounter);
  delay(5);
  if (interruptCounter >= (inerruptsPerSecond * sleepInterval)) {
    timeToWakeUp = true;
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
        sleepInterval = nextTransmitTime;
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
      sleepInterval = cycleTime;
      Serial.print("Schedule received. First Delay: ");
      delay(5);
      Serial.print(firstTransmitDelay);
      delay(5);
      Serial.print(" Cycle time: ");
      delay(5);
      Serial.println(sleepInterval);
      delay(5);
      return;
    }
  }
  Serial.println("Registration timeout");
}
