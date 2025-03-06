#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

volatile unsigned int seconds = 0;
const unsigned int sleepInterval = 10;  // Sleep interval in seconds
volatile bool timeToWakeUp = false;

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
    // Sleep until the interval has passed
    while (!timeToWakeUp) {
        goToSleep();
    }

    // Perform processing
    Serial.println("Woke up and processing...");
    delay(1000);  // Simulate processing

    // Reset counters and re-enable Timer2
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        seconds = 0;
        timeToWakeUp = false;
    }

    // Re-enable Timer2 interrupt
    TIMSK2 |= (1 << OCIE2A);
}

void setupTimer2() {
    // Set up Timer2 to generate an interrupt every 1 second
    TCCR2A = (1 << WGM21);  // CTC mode
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 1024
    OCR2A = 249;  // Corrected value for 1s interval
    TCNT2 = 0;
    TIMSK2 = (1 << OCIE2A);  // Enable Timer2 compare match interrupt
}

void goToSleep() {
    // Set sleep mode to idle (since we need Timer2)
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    // Go to sleep (MCU continues from here after waking up)
    sleep_mode();

    // Disable sleep after wake-up
    sleep_disable();
}

// Timer2 compare match interrupt service routine
ISR(TIMER2_COMPA_vect) {
    seconds++;
    if (seconds >= sleepInterval) {
        timeToWakeUp = true;
    }
}