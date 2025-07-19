#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <EEPROM.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

// Pin definitions
#define NSS 10
#define NRESET 9  // Changed to 9 as requested
#define DIO0 8    // Changed to 8 as requested
#define BAND 433E6

// Analog pins for voltage measurements

#define RS485 A0
#define SENSOR_1 A1
#define SENSOR_2 A2
#define SENSOR_3 A3

#define RX 7
#define TX 6

#define SOLAR_PIN A6
#define BATTERY_PIN A7

// Voltage divider ratio (since you mentioned the measured voltage is half)
#define VOLTAGE_DIVIDER_RATIO 2.0

// Reference voltage (assuming Arduino's default 5V)
// Change to 3.3 if your board uses 3.3V
#define ADC_REFERENCE 5.0

// Counter variable
unsigned int counter = 0;
int deviceAddress = 1;
int result;
int val;
int j;
int repeat = 15;
volatile bool f_wdt = true;      // Flag for Watchdog Timer
volatile int wakeUpCounter = 0;  // Counter for wake-ups

SoftwareSerial mySerial(RX, TX);
ModbusMaster node;


void setup() {
  Serial.begin(9600);
  while (!Serial) {};
  mySerial.begin(4800);
  node.begin(deviceAddress, mySerial);
  Serial.println("LoRa Transmitter");

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1) {};
  }

  // Optional settings (should match receiver settings)
  LoRa.setSpreadingFactor(7);      // Range: 6-12, higher = longer range but slower
  LoRa.setSignalBandwidth(125E3);  // Bandwidth options: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
  LoRa.setCodingRate4(8);          // Denominator of coding rate (5-8)

  Serial.println("LoRa transmitter initialized!");

  pinMode(SENSOR_1, OUTPUT);
  pinMode(SENSOR_2, OUTPUT);
  pinMode(SENSOR_3, OUTPUT);
  pinMode(RS485, OUTPUT);

  digitalWrite(SENSOR_1, LOW);
  digitalWrite(SENSOR_2, LOW);
  digitalWrite(SENSOR_3, LOW);
  digitalWrite(RS485, LOW);
}


void loop() {


  // Read voltages
  float solarVoltage = readVoltage(SOLAR_PIN);
  float batteryVoltage = readVoltage(BATTERY_PIN);

  String data1 = "1,0,0,0,0,0";
  String data2 = "2,0,0,0,0,0";
  String data3 = "3,0,0,0,0,0";
  digitalWrite(RS485, HIGH);
  digitalWrite(SENSOR_1, HIGH);
  delay(2000);

  result = node.readHoldingRegisters(0, 5);
  //Serial.println(result);
  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    data1 = "1," + String(val);
    for (j = 1; j < 5; j++) {
      val = node.getResponseBuffer(j);
      data1 = data1 + "," + String(val);
    }
  }

  digitalWrite(SENSOR_1, LOW);

  digitalWrite(SENSOR_2, HIGH);
  delay(2000);
  result = node.readHoldingRegisters(0, 5);
  //Serial.println(result);
  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    data2 = "2," + String(val);
    for (j = 1; j < 5; j++) {
      val = node.getResponseBuffer(j);
      data2 = data2 + "," + String(val);
    }
  }
  digitalWrite(SENSOR_2, LOW);

  digitalWrite(SENSOR_3, HIGH);
  delay(2000);

  result = node.readHoldingRegisters(0, 5);
  //Serial.println(result);
  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    data3 = "3," + String(val);
    for (j = 1; j < 5; j++) {
      val = node.getResponseBuffer(j);
      data3 = data3 + "," + String(val);
    }
  }

  digitalWrite(SENSOR_3, LOW);

  digitalWrite(RS485, LOW);

  delay(2000);
  // Prepare the message
  String message = "Faraz00001," + String(counter) + "," + String(solarVoltage, 2) + "" + "," + String(batteryVoltage, 2) + "";

  message = message + "," + data1 + "," + data2 + "," + data3;
  // Send the message
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  // Print to serial for debugging
  Serial.println("Sent: " + message);

  // Increment counter
  counter++;

  // Wait 5 seconds before next transmission
  delay(5000);
}
//-------------------------------------
// Function to read and calculate actual voltage considering the divider
float readVoltage(int pin) {
  // Read the analog value (0-1023)
  int rawValue = analogRead(pin);

  // Convert to voltage at the pin (before divider)
  float voltageAtPin = (rawValue * ADC_REFERENCE) / 1023.0;

  // Calculate actual voltage (after divider)
  float actualVoltage = voltageAtPin * VOLTAGE_DIVIDER_RATIO;

  return actualVoltage;
}

//----------------------------------------
void turnOffADC() {
  ADCSRA &= ~(1 << ADEN);  // Disable ADC
}

void turnOnADC() {
  ADCSRA |= (1 << ADEN);  // Enable ADC
}
//-----------------------------------------
void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();  // Disable interrupts
  sleep_enable();  // Enable sleep mode
  // Enable watchdog timer interrupt
  WDTCSR |= (1 << WDIE);
  interrupts();  // Enable interrupts
  sleep_cpu();   // Enter sleep mode
  // Disable sleep mode after wake up
  //delay(10);
  //Serial.println("Waking up");
  sleep_disable();
}
//------------------------------------------
ISR(WDT_vect) {
  f_wdt = true;  // Set the watchdog timer flag
}