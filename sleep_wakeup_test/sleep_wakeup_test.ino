#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

volatile unsigned int seconds = 0;
const unsigned int sleepInterval = 6;  // Set desired sleep interval in seconds

void setup() {
  Serial.begin(57600);

  // Set all pins as inputs with pull-ups to reduce power consumption
  for (int x = 1; x < 20; x++) {
    pinMode(x, INPUT);
    digitalWrite(x, HIGH);
  }

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
  Serial.println("Woke up and processing...");
  delay(5);  // Simulate processing with a delay

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
