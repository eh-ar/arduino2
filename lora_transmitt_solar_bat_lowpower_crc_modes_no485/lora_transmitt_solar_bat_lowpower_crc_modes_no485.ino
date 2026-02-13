#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
//#include <ModbusMaster.h>
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
#define SENSOR_1 2
#define SENSOR_2 3
#define SENSOR_3 4
#define SOLAR_PIN A6
#define BATTERY_PIN A7

#define RX A1
#define TX A3
#define RE_DE_PIN A2  // RE and DE control pin for MAX485

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

//SoftwareSerial mySerial(RX, TX);
//ModbusMaster node;

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

// ================= CRC16 CALCULATION (MODBUS) =================
uint16_t calculateCRC16(byte *buf, int len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// ================= CRC32 CALCULATION (LORA DATA) =================
uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return ~crc;
}

// ================= SINGLE SENSOR READING FUNCTION =================
bool readSingleSensor(int sensorNum, int enablePin, uint16_t *results, int numRegisters) {
  bool success = false;

  // Turn ON this specific sensor
  digitalWrite(enablePin, HIGH);
  Serial.print("Sensor ");
  Serial.print(sensorNum);
  Serial.print(": ON (Warming up ");
  Serial.print(cfg.sensorWarmUp);
  Serial.println(" ms)");

  // Wait for sensor warmup
  delay(cfg.sensorWarmUp);

  // Initialize software serial for RS485 communication
  SoftwareSerial rs485(RX, TX);
  rs485.begin(4800);

  // Set up RE/DE pin
  pinMode(RE_DE_PIN, OUTPUT);

  // Create Modbus request
  byte request[8];

  // Device address (using configured address from cfg)
  request[0] = cfg.deviceAddress;

  // Function code (0x03 = read holding registers)
  request[1] = 0x03;

  // Starting address (0 = register 0)
  request[2] = highByte(0);
  request[3] = lowByte(0);

  // Number of registers to read
  request[4] = highByte(numRegisters);
  request[5] = lowByte(numRegisters);

  // Calculate CRC16 for Modbus frame
  uint16_t crc = calculateCRC16(request, 6);
  request[6] = lowByte(crc);
  request[7] = highByte(crc);

  // Enable RS485 transmitter
  digitalWrite(RE_DE_PIN, HIGH);  // Enable transmit mode

  // Send Modbus request
  rs485.write(request, sizeof(request));
  rs485.flush();

  // Switch to receive mode
  digitalWrite(RE_DE_PIN, LOW);  // Enable receive mode

  // Wait for response with timeout
  unsigned long startTime = millis();

  while (millis() - startTime < 2000) {  // 2 second timeout
    if (rs485.available() >= 5 + numRegisters * 2) {
      // Read response
      byte response[5 + numRegisters * 2];
      rs485.readBytes(response, sizeof(response));

      // Verify CRC16 of response
      uint16_t receivedCRC = (response[sizeof(response) - 1] << 8) | response[sizeof(response) - 2];
      uint16_t calculatedCRC = calculateCRC16(response, sizeof(response) - 2);

      if (receivedCRC == calculatedCRC) {
        // Extract register values
        for (int i = 0; i < numRegisters; i++) {
          results[i] = (response[3 + i * 2] << 8) | response[4 + i * 2];
        }
        success = true;
        Serial.print("Sensor ");
        Serial.print(sensorNum);
        Serial.println(": Read successful");
      } else {
        Serial.print("Sensor ");
        Serial.print(sensorNum);
        Serial.println(": CRC error");
        // Initialize results to zero on error
        for (int i = 0; i < numRegisters; i++) {
          results[i] = 0;
        }
      }
      break;
    }
  }

  // If timeout occurred
  if (!success && (millis() - startTime >= 2000)) {
    Serial.print("Sensor ");
    Serial.print(sensorNum);
    Serial.println(": Timeout");
    // Initialize results to zero on timeout
    for (int i = 0; i < numRegisters; i++) {
      results[i] = 0;
    }
  }

  // Turn OFF this sensor
  digitalWrite(enablePin, LOW);
  Serial.print("Sensor ");
  Serial.print(sensorNum);
  Serial.println(": OFF");

  // Clean up
  rs485.end();

  return success;
}

// ================= BUILD SENSOR STRING =================
String buildSensorString(int sensorNum, uint16_t *data, int numRegisters) {
  String result = String(sensorNum);
  for (int i = 0; i < numRegisters; i++) {
    result += "," + String(data[i]);
  }
  return result;
}

// ================= READ ALL SENSORS =================
String readAllSensors() {
  // Define how many registers to read from each sensor
  const int NUM_REGISTERS = 5;

  // Arrays to store sensor results
  uint16_t sensor1Results[NUM_REGISTERS];
  uint16_t sensor2Results[NUM_REGISTERS];
  uint16_t sensor3Results[NUM_REGISTERS];

  // Enable RS485 transceiver
  pinMode(RS485, OUTPUT);
  digitalWrite(RS485, HIGH);
  Serial.println("RS485: Enabled");

  // Small delay for RS485 stabilization
  delay(cfg.rs485WarmUp);

  // Read Sensor 1
  Serial.println("=== Reading Sensor 1 ===");
  bool sensor1Success = readSingleSensor(1, SENSOR_1, sensor1Results, NUM_REGISTERS);

  // Small delay between sensors
  delay(100);

  // Read Sensor 2
  Serial.println("=== Reading Sensor 2 ===");
  bool sensor2Success = readSingleSensor(2, SENSOR_2, sensor2Results, NUM_REGISTERS);

  // Small delay between sensors
  delay(100);

  // Read Sensor 3
  Serial.println("=== Reading Sensor 3 ===");
  bool sensor3Success = readSingleSensor(3, SENSOR_3, sensor3Results, NUM_REGISTERS);

  // Disable RS485 transceiver
  digitalWrite(RS485, LOW);
  Serial.println("RS485: Disabled");

  // Build the result string
  String result = buildSensorString(1, sensor1Results, NUM_REGISTERS) + "," + buildSensorString(2, sensor2Results, NUM_REGISTERS) + "," + buildSensorString(3, sensor3Results, NUM_REGISTERS);
  //Serial.println("Result: " + result);
  return result;
}

// ================= MAIN FUNCTION =================
void takeMeasurementsAndTransmit() {
  power_all_enable();
  ADCSRA |= (1 << ADEN);

  float solarVoltage = readVoltage(SOLAR_PIN);
  float batteryVoltage = readVoltage(BATTERY_PIN);
  String sensorData = readAllSensors();

  // ----------------- RANDOM JITTER BEFORE TX -----------------
  randomSeed(analogRead(A0) + micros());
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

//--------------- 
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


  LoRa.setPins(NSS, NRESET, DIO0);
  LoRa.begin(BAND);
  LoRa.sleep();

  pinMode(SENSOR_1, OUTPUT);
  pinMode(SENSOR_2, OUTPUT);
  pinMode(SENSOR_3, OUTPUT);
  pinMode(RS485, OUTPUT);
  pinMode(RE_DE_PIN, OUTPUT);
  
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


