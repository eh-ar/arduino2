#include <SoftwareSerial.h>

// Pins configuration
#define RX_PIN 2
#define TX_PIN 4
#define RE_DE_PIN 3  // RE and DE connected together to this pin

// Modbus parameters
#define SENSOR_ADDRESS 1
#define FIRST_REGISTER 0
#define REGISTER_COUNT 5
#define BAUDRATE 4800

SoftwareSerial rs485(RX_PIN, TX_PIN);

// CRC calculation function
uint16_t calculateCRC(byte *buf, int len) {
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

void setup() {
  Serial.begin(9600);
  rs485.begin(BAUDRATE);
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW); // Start in receive mode
  Serial.println("RS485 Modbus Reader Ready");
}

void loop() {
  // Create Modbus request
  Serial.println("Sensor Reading");
  byte request[8];
  
  // Device address
  request[0] = SENSOR_ADDRESS;
  
  // Function code (0x03 = read holding registers)
  request[1] = 0x03;
  
  // Starting address (0 = register 0)
  request[2] = highByte(FIRST_REGISTER);
  request[3] = lowByte(FIRST_REGISTER);
  
  // Number of registers to read (5)
  request[4] = highByte(REGISTER_COUNT);
  request[5] = lowByte(REGISTER_COUNT);
  
  // Calculate CRC
  uint16_t crc = calculateCRC(request, 6);
  request[6] = lowByte(crc);
  request[7] = highByte(crc);

  // Send request
  digitalWrite(RE_DE_PIN, HIGH); // Enable transmit
  rs485.write(request, sizeof(request));
  rs485.flush();
  digitalWrite(RE_DE_PIN, LOW); // Return to receive mode

  // Wait for response (adjust timeout as needed)
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
    if (rs485.available() >= 5 + REGISTER_COUNT * 2) {
      // Read response
      byte response[5 + REGISTER_COUNT * 2];
      rs485.readBytes(response, sizeof(response));
      
      // Verify CRC
      uint16_t receivedCRC = (response[sizeof(response)-1] << 8) | response[sizeof(response)-2];
      uint16_t calculatedCRC = calculateCRC(response, sizeof(response)-2);
      
      if (receivedCRC == calculatedCRC) {
        // Print register values
        Serial.println("Received valid response:");
        for (int i = 0; i < REGISTER_COUNT; i++) {
          uint16_t value = (response[3 + i*2] << 8) | response[4 + i*2];
          Serial.print("Register ");
          Serial.print(i);
          Serial.print(": ");
          Serial.println(value);
        }
      } else {
        Serial.println("CRC error in response");
      }
      break;
    }
  }
  
  delay(2000); // Wait 2 seconds before next read
}