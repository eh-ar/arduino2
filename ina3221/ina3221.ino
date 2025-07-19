#include <Wire.h>
#include <Adafruit_INA3221.h>

Adafruit_INA3221 ina3221 = Adafruit_INA3221();

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Wait for serial monitor (for debugging)

  if (!ina3221.begin()) {
    Serial.println("Failed to find INA3221 chip!");
    while (1);
  }
}

void loop() {
  // Read shunt voltage (mV) and current (mA) for Channel 1
  float shuntVoltage_mV = ina3221.getShuntVoltage(0);
  float busVoltage_V = ina3221.getBusVoltage(0);
  float current_mA = ina3221.getCurrentAmps(0);

  // Print results
  Serial.print("Bus Voltage: "); Serial.print(busVoltage_V); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntVoltage_mV); Serial.println(" mV");
  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  Serial.println("------------------");

  delay(1000);
}