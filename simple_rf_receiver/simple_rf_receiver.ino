#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 4); // RX, TX

byte buffer[64];
int bufferIndex = 0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("Receiver Ready - Byte Array!");
}

void loop() {
  while (mySerial.available() > 0) {
    byte inByte = mySerial.read();
    
    if (inByte == '\n') {
      // End of message
      if (bufferIndex > 0) {
        Serial.write("Received: ");
        Serial.write(buffer, bufferIndex);
        Serial.println();
        bufferIndex = 0;
      }
    } else if (bufferIndex < 63) {
      // Add to buffer if there's space
      buffer[bufferIndex++] = inByte;
    }
  }
}