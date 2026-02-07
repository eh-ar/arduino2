#include <IRremote.hpp>

const int RECV_PIN = 2;        // ← change only if you connected to another pin

void setup() {
  Serial.begin(115200);
  while (!Serial);             // wait for serial monitor (needed on some boards)
  
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);  // starts the receiver + blinks LED on pin 13
  
  Serial.println(F("TCL AC IR capture ready!"));
  Serial.println(F("Point your ORIGINAL TCL AC remote at the 3-pin receiver"));
  Serial.println(F("and press any button. Codes will appear below:\n"));
}

void loop() {
  if (IrReceiver.decode()) {
    // Print everything in different useful formats
    Serial.println();
    IrReceiver.printIRResultShort(&Serial);             // Human readable
    Serial.print(F("Raw hex: 0x"));
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    if (IrReceiver.decodedIRData.protocol == NEC) {
      Serial.println(F("→ This is standard NEC → you can retransmit with sendNEC()"));
    }

    IrReceiver.resume();  // Prepare for the next signal
  }
}