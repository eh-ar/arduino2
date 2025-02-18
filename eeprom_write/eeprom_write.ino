#include <EEPROM.h>

// Address location in EEPROM
const int stringAddress = 0;
const int intAddress = 50;


void writeStringToEEPROM(int address, const String &str) {
  int len = str.length(); // Get string length
  EEPROM.put(address, len); // Write string length
  for (int i = 0; i < len; i++) {
    EEPROM.write(address + sizeof(len) + i, str[i]); // Write string characters
  }
  EEPROM.write(address + sizeof(len) + len, '\0'); // Write null-terminator
}

String readStringFromEEPROM(int address) {
  int len;
  EEPROM.get(address, len); // Read string length
  char data[len + 1]; // Create a char array to hold the string and null-terminator
  for (int i = 0; i < len; i++) {
    data[i] = EEPROM.read(address + sizeof(len) + i); // Read string characters
  }
  data[len] = '\0'; // Ensure null-termination
  return String(data); // Convert char array to String
}

void setup() {
  Serial.begin(115200);

  // Write a String to EEPROM
  String myString = "FASO00002";
  writeStringToEEPROM(stringAddress, myString);
  Serial.println("String written to EEPROM");
  EEPROM.put(    intAddress, 0);
  // Read the String from EEPROM
  String storedString = readStringFromEEPROM(stringAddress);
  Serial.println("Stored String: " + storedString);
}

void loop() {
  // Your main code here
}
