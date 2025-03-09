
void enterSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
  is_awake = true;
}

void wakeUp() {
  is_awake = true;
}

void setupWatchdogTimer(int interval) {
  // Setup watchdog timer with specified interval (4 or 8 seconds)
  if (interval == 4) {
    wdt_enable(WDTO_4S);
  } else if (interval == 8) {
    wdt_enable(WDTO_8S);
  }
  WDTCSR |= (1 << WDIE);  // Enable watchdog interrupt
}

ISR(WDT_vect) {
  wakeUp();
  wakeupCounter += interval;
  EEPROM.write(0, wakeupCounter & 0xFF);
  EEPROM.write(1, (wakeupCounter >> 8) & 0xFF);
}