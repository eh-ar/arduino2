
volatile boolean f_wdt = 1;
int watchDogSec = 8;  //sec timer for sleep

void watchdogSetup() {

  // Clear watchdog reset flag
  MCUSR &= ~(1 << WDRF);
  // Set up WDT interrupt
  WDTCSR = (1 << WDCE) | (1 << WDE);

  if (watchDogSec == 8) {
    // Set up watchdog timer for 8 seconds timeout
    WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0);
  } else if (watchDogSec == 4) {
    // Set up watchdog timer for 4 seconds timeout
    WDTCSR = (1 << WDIE) | (1 << WDP3);  // | (1 << WDP0);
  } else if (watchDogSec == 2) {
    // Set WDIE (interrupt enable) and WDP2 & WDP1 (1s timeout)
    WDTCSR = (1 << WDIE) | (1 << WDP2) | (1 << WDP1);
  }
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

  int iterations = totalSeconds / (watchDogSec);
  int calcSeconds = iterations * watchDogSec;

  Serial.println("sleep for: " + String(calcSeconds) + " sec");
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

