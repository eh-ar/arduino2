#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <EEPROM.h>
#include <CRC32.h>

// ================= PIN DEFINITIONS =================
#define NSS 10
#define NRESET 9
#define DIO0 8
#define BAND 433E6

#define RS485 A0
#define SENSOR_1 A1
#define SENSOR_2 A2
#define SENSOR_3 A3
#define SOLAR_PIN A6
#define BATTERY_PIN A7

#define RX 7
#define TX 6
#define LED 2  // PROGRAM MODE SWITCH

#define VOLTAGE_DIVIDER_RATIO 2.0
#define ADC_REFERENCE 5.0
#define WDT_MAX_SLEEP 8

// ================= EEPROM MARKER =================
#define EEPROM_MARKER_ADDR 500
#define EEPROM_MARKER_VALUE 0x42


//==============  Exit Timeout =======
#define PROGRAM_WARN_TIME 30000UL  // 30 seconds
#define PROGRAM_EXIT_TIME 60000UL  // 60 seconds

// ================= CONFIG STRUCT =================
struct Config {
  int deviceAddress;
  int sensorWarmUp;
  int rs485WarmUp;
  unsigned int measurementIntervalMinutes;
  char secretKey[20];
  char deviceID[20];
  // Add future parameters here
};

Config cfg;

// ================= GLOBALS =================
unsigned int counter = 0;
unsigned int wdt_cycle_count = 0;
volatile bool f_wdt = true;

SoftwareSerial mySerial(RX, TX);
ModbusMaster node;

// ================= DEFAULT CONFIG =================
void loadDefaults() {
  cfg.deviceAddress = 1;
  cfg.sensorWarmUp = 5000;
  cfg.rs485WarmUp = 2000;               // RS485 warmup
  cfg.measurementIntervalMinutes = 10;  // measurement interval
  strcpy(cfg.secretKey, "LoraKey7742");
  strcpy(cfg.deviceID, "Faraz00003");
}

// ================= EEPROM FUNCTIONS =================
void saveConfig() {
  EEPROM.put(0, cfg);
  EEPROM.write(EEPROM_MARKER_ADDR, EEPROM_MARKER_VALUE);  // mark EEPROM as valid
  Serial.println("CONFIG SAVED");
}

void loadConfig() {
  byte marker = EEPROM.read(EEPROM_MARKER_ADDR);
  if (marker != EEPROM_MARKER_VALUE) {
    loadDefaults();
    saveConfig();  // save defaults to EEPROM
  } else {
    EEPROM.get(0, cfg);
  }
}

// ================= PROGRAM MODE =================
void programMode() {
  Serial.println("=== PROGRAM MODE ===");
  Serial.println("EXIT AFTER 1 Minut OF INACTIVITY");
  Serial.println("FORMAT  ->  (KEY VALUE)");
  Serial.println("ADDR x | WARM x | RS485 x | INT x | KEY xxxx | ID xxxx");
  Serial.println("SHOW | SAVE | EXIT");

  unsigned long lastInputTime = millis();
  bool warningShown = false;


  while (true) {
    unsigned long now = millis();

    // ✅ 30s WARNING
    if (!warningShown && (now - lastInputTime >= PROGRAM_WARN_TIME)) {
      Serial.println("⚠ WARNING: No activity. Auto-exit in 30 seconds...");
      warningShown = true;
    }

    // ✅ 60s AUTO EXIT
    if (now - lastInputTime >= PROGRAM_EXIT_TIME) {
      Serial.println("⛔ PROGRAM MODE AUTO-EXIT (timeout)");
      return;
    }

    // ✅ SERIAL INPUT
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      lastInputTime = millis();  // ✅ RESET TIMER
      warningShown = false;      // ✅ CLEAR WARNING

      if (cmd.startsWith("ADDR")) cfg.deviceAddress = cmd.substring(5).toInt();
      else if (cmd.startsWith("WARM")) cfg.sensorWarmUp = cmd.substring(5).toInt();
      else if (cmd.startsWith("RS485")) cfg.rs485WarmUp = cmd.substring(6).toInt();
      else if (cmd.startsWith("INT")) cfg.measurementIntervalMinutes = cmd.substring(4).toInt();
      else if (cmd.startsWith("KEY")) cmd.substring(4).toCharArray(cfg.secretKey, 20);
      else if (cmd.startsWith("ID")) cmd.substring(3).toCharArray(cfg.deviceID, 20);

      else if (cmd == "SHOW") {
        Serial.print("ADDR=");
        Serial.println(cfg.deviceAddress);
        Serial.print("WARM=");
        Serial.println(cfg.sensorWarmUp);
        Serial.print("RS485=");
        Serial.println(cfg.rs485WarmUp);
        Serial.print("INT=");
        Serial.println(cfg.measurementIntervalMinutes);
        Serial.print("KEY=");
        Serial.println(cfg.secretKey);
        Serial.print("ID=");
        Serial.println(cfg.deviceID);
      }

      else if (cmd == "SAVE") saveConfig();

      else if (cmd == "EXIT") {
        Serial.println("✅ Program mode exited by user.");
        return;
      }

      Serial.println("OK");
    }
  }
}
// ================= SETUP =================
void setup() {

  Serial.begin(9600);

  loadConfig();
  programMode();

  Serial.println("=== LOGGER MODE ===");
  power_adc_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();

  mySerial.begin(4800);
  node.begin(cfg.deviceAddress, mySerial);

  LoRa.setPins(NSS, NRESET, DIO0);
  LoRa.begin(BAND);
  LoRa.sleep();

  pinMode(SENSOR_1, OUTPUT);
  pinMode(SENSOR_2, OUTPUT);
  pinMode(SENSOR_3, OUTPUT);
  pinMode(RS485, OUTPUT);
  pinMode(LED, OUTPUT);


  digitalWrite(LED, HIGHT);
  delay(1000);
  takeMeasurementsAndTransmit();
  setup_watchdog(WDT_MAX_SLEEP);
}

// ================= LOOP =================
void loop() {
  unsigned int WDT_CYCLES_NEEDED =
    (cfg.measurementIntervalMinutes * 60) / WDT_MAX_SLEEP;

  if (f_wdt) {
    f_wdt = false;
    wdt_cycle_count++;

    if (wdt_cycle_count >= WDT_CYCLES_NEEDED) {
      wdt_cycle_count = 0;
      takeMeasurementsAndTransmit();
    }

    prepareForSleep();
    sleepNow();
  }
}

// ================= MAIN FUNCTION =================
void takeMeasurementsAndTransmit() {
  power_all_enable();
  ADCSRA |= (1 << ADEN);

  float solarVoltage = readVoltage(SOLAR_PIN);
  float batteryVoltage = readVoltage(BATTERY_PIN);
  String sensorData = readAllSensors();

  // ----------------- RANDOM JITTER BEFORE TX -----------------
  delay(random(0, 10000));  // 0–10 seconds

  String data = String(solarVoltage, 2) + "," + String(batteryVoltage, 2) + "," + sensorData;

  uint32_t checksum = calculateChecksum(cfg.deviceID, data, counter);
  String message = String(cfg.deviceID) + "|" + String(counter) + "|" + data + "|" + String(checksum, HEX);

  LoRa.idle();
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  LoRa.sleep();

  Serial.println("Sent: " + message);
  delay(100);
  counter++;
}

// ================= SENSORS =================
String readAllSensors() {
  String data1 = "1,0,0,0,0,0";
  String data2 = "2,0,0,0,0,0";
  String data3 = "3,0,0,0,0,0";

  digitalWrite(RS485, HIGH);
  delay(cfg.rs485WarmUp);

  digitalWrite(SENSOR_1, HIGH);
  delay(cfg.sensorWarmUp);
  if (node.readHoldingRegisters(0, 5) == node.ku8MBSuccess)
    data1 = buildSensorString(1, node);
  digitalWrite(SENSOR_1, LOW);

  digitalWrite(SENSOR_2, HIGH);
  delay(cfg.sensorWarmUp);
  if (node.readHoldingRegisters(0, 5) == node.ku8MBSuccess)
    data2 = buildSensorString(2, node);
  digitalWrite(SENSOR_2, LOW);

  digitalWrite(SENSOR_3, HIGH);
  delay(cfg.sensorWarmUp);
  if (node.readHoldingRegisters(0, 5) == node.ku8MBSuccess)
    data3 = buildSensorString(3, node);
  digitalWrite(SENSOR_3, LOW);

  digitalWrite(RS485, LOW);
  return data1 + "," + data2 + "," + data3;
}

String buildSensorString(int sensorNum, ModbusMaster &node) {
  String result = String(sensorNum);
  for (int i = 0; i < 5; i++)
    result += "," + String(node.getResponseBuffer(i));
  return result;
}

float readVoltage(int pin) {
  int rawValue = analogRead(pin);
  return (rawValue * ADC_REFERENCE) / 1023.0 * VOLTAGE_DIVIDER_RATIO;
}

// ================= POWER =================
void prepareForSleep() {
  digitalWrite(SENSOR_1, LOW);
  digitalWrite(SENSOR_2, LOW);
  digitalWrite(SENSOR_3, LOW);
  digitalWrite(RS485, LOW);
  ADCSRA &= ~(1 << ADEN);
  power_all_disable();
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
}

// ================= WATCHDOG =================
void setup_watchdog(int timerPrescaler) {
  byte bb = timerPrescaler & 7;
  if (timerPrescaler > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

ISR(WDT_vect) {
  f_wdt = true;
}

// ================= CRC =================
uint32_t calculateChecksum(const String &code, const String &data, uint32_t counter) {
  CRC32 crc;
  crc.update((const uint8_t *)cfg.secretKey, strlen(cfg.secretKey));
  crc.update((const uint8_t *)code.c_str(), code.length());
  crc.update((const uint8_t *)data.c_str(), data.length());

  uint8_t counterBytes[4] = {
    (counter >> 0) & 0xFF,
    (counter >> 8) & 0xFF,
    (counter >> 16) & 0xFF,
    (counter >> 24) & 0xFF
  };

  crc.update(counterBytes, 4);
  return crc.finalize();
}
