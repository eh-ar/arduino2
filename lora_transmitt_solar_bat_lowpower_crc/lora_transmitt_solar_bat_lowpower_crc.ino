#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <CRC32.h>

// Pin definitions
#define NSS 10
#define NRESET 9
#define DIO0 8
#define BAND 433E6

// Analog pins
#define RS485 A0
#define SENSOR_1 A1
#define SENSOR_2 A2
#define SENSOR_3 A3
#define SOLAR_PIN A6
#define BATTERY_PIN A7

#define RX 7
#define TX 6

// Constants
#define VOLTAGE_DIVIDER_RATIO 2.0
#define ADC_REFERENCE 5.0
#define WDT_MAX_SLEEP 8  // Max WDT sleep duration in seconds

// Shared secret (minimum 8 bytes recommended)
const char SECRET_KEY[] = "LoraKey7742";

// Configuration - Set your desired interval here (in minutes)
const unsigned int MEASUREMENT_INTERVAL_MINUTES = 10;  // Adjust this value (1-255 minutes)
const unsigned int WDT_CYCLES_NEEDED = (MEASUREMENT_INTERVAL_MINUTES * 60) / WDT_MAX_SLEEP;

// Global variables
unsigned int counter = 0;
unsigned int wdt_cycle_count = 0;
int deviceAddress = 1;
int sensorWarmUp = 2000;
String deviceID = "Faraz00003";
volatile bool f_wdt = true;
SoftwareSerial mySerial(RX, TX);
ModbusMaster node;

void setup() {
  Serial.begin(9600);
  while (!Serial) {};

  // Initialize with power saving
  power_adc_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();

  mySerial.begin(4800);
  node.begin(deviceAddress, mySerial);

  // Configure LoRa with minimal settings
  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.sleep();  // Put LoRa module to sleep initially

  Serial.println("Lora Transmitter");
  delay(100);
  // Configure sensor pins
  pinMode(SENSOR_1, OUTPUT);
  pinMode(SENSOR_2, OUTPUT);
  pinMode(SENSOR_3, OUTPUT);
  pinMode(RS485, OUTPUT);
  digitalWrite(SENSOR_1, LOW);
  digitalWrite(SENSOR_2, LOW);
  digitalWrite(SENSOR_3, LOW);
  digitalWrite(RS485, LOW);

  //
  takeMeasurementsAndTransmit();
  delay(100);
  // Configure watchdog timer for maximum sleep duration
  setup_watchdog(WDT_MAX_SLEEP);
}

void loop() {
  if (f_wdt) {
    f_wdt = false;
    wdt_cycle_count++;

    // Only take measurements after the full interval has elapsed
    if (wdt_cycle_count >= WDT_CYCLES_NEEDED) {
      wdt_cycle_count = 0;
      takeMeasurementsAndTransmit();
      delay(100);
    }

    // Return to sleep
    prepareForSleep();
    sleepNow();
  }
}

void takeMeasurementsAndTransmit() {
  // Enable necessary peripherals
  power_all_enable();
  turnOnADC();
  delay(100);
  // Read voltages
  float solarVoltage = readVoltage(SOLAR_PIN);
  float batteryVoltage = readVoltage(BATTERY_PIN);

  // Read sensor data
  String sensorData = readAllSensors();

  // Prepare and send LoRa message
  String data = String(solarVoltage, 2) + "," + String(batteryVoltage, 2) + "," + sensorData;
  uint32_t checksum = calculateChecksum(deviceID, data, counter);
  String message = deviceID + "|" + String(counter) + "|" + data + "|" + String(checksum, HEX);

  delay(100);
  LoRa.idle();  // Wake LoRa module
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  LoRa.sleep();  // Put LoRa back to sleep

  // Debug output
  Serial.println("Sent: " + message);
  counter++;
}

String readAllSensors() {
  String data1 = "1,0,0,0,0,0";
  String data2 = "2,0,0,0,0,0";
  String data3 = "3,0,0,0,0,0";

  digitalWrite(RS485, HIGH);
  delay(100);
  // Sensor 1
  digitalWrite(SENSOR_1, HIGH);
  delay(sensorWarmUp);  // Reduced stabilization time
  if (node.readHoldingRegisters(0, 5) == node.ku8MBSuccess) {
    data1 = buildSensorString(1, node);
  }
  digitalWrite(SENSOR_1, LOW);

  // Sensor 2
  digitalWrite(SENSOR_2, HIGH);
  delay(sensorWarmUp);
  if (node.readHoldingRegisters(0, 5) == node.ku8MBSuccess) {
    data2 = buildSensorString(2, node);
  }
  digitalWrite(SENSOR_2, LOW);

  // Sensor 3
  digitalWrite(SENSOR_3, HIGH);
  delay(sensorWarmUp);
  if (node.readHoldingRegisters(0, 5) == node.ku8MBSuccess) {
    data3 = buildSensorString(3, node);
  }
  digitalWrite(SENSOR_3, LOW);

  digitalWrite(RS485, LOW);

  return data1 + "," + data2 + "," + data3;
}

String buildSensorString(int sensorNum, ModbusMaster &node) {
  String result = String(sensorNum) + "," + String(node.getResponseBuffer(0));
  for (int j = 1; j < 5; j++) {
    result += "," + String(node.getResponseBuffer(j));
  }
  return result;
}

float readVoltage(int pin) {
  int rawValue = analogRead(pin);
  return (rawValue * ADC_REFERENCE) / 1023.0 * VOLTAGE_DIVIDER_RATIO;
}

void prepareForSleep() {
  // Power down all sensors
  digitalWrite(SENSOR_1, LOW);
  digitalWrite(SENSOR_2, LOW);
  digitalWrite(SENSOR_3, LOW);
  digitalWrite(RS485, LOW);

  // Disable peripherals
  turnOffADC();
  power_all_disable();
}

void turnOffADC() {
  ADCSRA &= ~(1 << ADEN);
}

void turnOnADC() {
  ADCSRA |= (1 << ADEN);
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
}

void setup_watchdog(int timerPrescaler) {
  byte bb;
  if (timerPrescaler > 9) timerPrescaler = 9;
  bb = timerPrescaler & 7;
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

uint32_t calculateChecksum(const String &code, const String &data, uint32_t counter) {
  CRC32 crc;
  
  // 1. Add secret key (as bytes)
  crc.update((const uint8_t*)SECRET_KEY, strlen(SECRET_KEY));
  
  // 2. Add code (as bytes)
  crc.update((const uint8_t*)code.c_str(), code.length());
  
  // 3. Add data (as bytes)
  crc.update((const uint8_t*)data.c_str(), data.length());
  
  // 4. Add counter (as 4 bytes, little-endian)
  uint8_t counterBytes[4];
  counterBytes[0] = (counter >> 0) & 0xFF;
  counterBytes[1] = (counter >> 8) & 0xFF;
  counterBytes[2] = (counter >> 16) & 0xFF;
  counterBytes[3] = (counter >> 24) & 0xFF;
  crc.update(counterBytes, 4);
  
  uint32_t result = crc.finalize();
  
  // Debug output
  /*
  Serial.print("TX Checksum Input: ");
  Serial.print(SECRET_KEY); Serial.print("|");
  Serial.print(code); Serial.print("|");
  Serial.print(data); Serial.print("|");
  Serial.println(counter);
  
  Serial.print("TX Checksum Result: 0x");
  Serial.println(result, HEX);
  */
  return result;
}
