#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define NSS 10
#define NRESET 9    // Changed to 9 as requested
#define DIO0 8      // Changed to 8 as requested
#define BAND 433E6

// Analog pins for voltage measurements
#define SOLAR_PIN A1
#define BATTERY_PIN A2

// Voltage divider ratio (since you mentioned the measured voltage is half)
#define VOLTAGE_DIVIDER_RATIO 2.0

// Reference voltage (assuming Arduino's default 5V)
// Change to 3.3 if your board uses 3.3V
#define ADC_REFERENCE 5.0

// Counter variable
unsigned int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Transmitter");

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Optional settings (should match receiver settings)
  LoRa.setSpreadingFactor(7);      // Range: 6-12, higher = longer range but slower
  LoRa.setSignalBandwidth(125E3);  // Bandwidth options: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
  LoRa.setCodingRate4(8);          // Denominator of coding rate (5-8)
  
  Serial.println("LoRa transmitter initialized!");
}

void loop() {
  // Read voltages
  float solarVoltage = readVoltage(SOLAR_PIN);
  float batteryVoltage = readVoltage(BATTERY_PIN);
  
  // Prepare the message
  String message = "Count:" + String(counter) + 
                   ", Solar:" + String(solarVoltage, 2) + "V" +
                   ", Battery:" + String(batteryVoltage, 2) + "V";
  
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