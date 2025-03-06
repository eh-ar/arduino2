#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

volatile unsigned int seconds = 0;
const unsigned int sleepInterval = 10;  // Set desired sleep interval in seconds

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
  delay(1000);  // Simulate processing with a delay

  // Reset the seconds counter
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    seconds = 0;
  }

  // Re-enable Timer2 interrupt for the next sleep cycle
  TIMSK2 |= (1 << OCIE2A);
}

void setupTimer2() {
  // Set up Timer2 to generate an interrupt every 1 second
  TCCR2A = (1 << WGM21);  // CTC mode
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 1024
  OCR2A = 15624 / 256;  // (16 MHz / (1024 * 256)) - 1
  TCNT2 = 0;
  TIMSK2 = (1 << OCIE2A);  // Enable Timer2 compare match interrupt
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

// Timer2 compare match interrupt service routine
ISR(TIMER2_COMPA_vect) {
  seconds++;
  if (seconds >= sleepInterval) {
    // Disable Timer2 interrupt to stop waking up
    TIMSK2 &= ~(1 << OCIE2A);
  }
}
