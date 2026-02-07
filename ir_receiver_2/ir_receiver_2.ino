#include <IRremote.hpp>

const uint8_t RECV_PIN = 2;  // Your IR receiver pin

void setup() {
  Serial.begin(115200);
  while (!Serial);  // Wait for Serial Monitor

  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);  // Standard start

  Serial.println(F("=== TCL/Baumann AC Capture (Handles Overflow) ==="));
  Serial.println(F("Point ORIGINAL remote at receiver and PRESS/HOLD a button."));
  Serial.println(F("Copy the 'Clean First Burst' uint16_t rawAC[...] line!"));
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println();  // Blank line
    IrReceiver.printIRResultShort(&Serial);  // Shows UNKNOWN + bits + Overflow

    // Official full raw print (even with overflow)
    Serial.println(F("--- Full Raw Timings ---"));
    IrReceiver.printIRSendUsage(&Serial);

    // Clean first burst (cut at long gap – for your AC protocol)
    Serial.println(F("--- Clean First Burst (copy this for sending) ---"));
    uint16_t len = IrReceiver.irparams.rawlen;  // Correct access for v4.5+
    uint16_t cutLen = len;
    for (uint16_t i = 1; i < len; i++) {
      uint32_t us = IrReceiver.irparams.rawbuf[i] * MICROS_PER_TICK;  // Correct access
      if (us > 5000) {  // End of first burst (long gap before repeat)
        cutLen = i + 1;
        break;
      }
    }
    //if (cutLen == len) cutLen = 110;  // Fallback for no gap: limit to ~110 for AC

    Serial.print(F("uint16_t rawAC["));
    Serial.print(cutLen);
    Serial.println(F("] = {"));
    for (uint16_t i = 1; i < cutLen; i++) {
      uint32_t us = IrReceiver.irparams.rawbuf[i] * MICROS_PER_TICK;  // Correct access
      Serial.print(us);
      if (i < cutLen - 1) Serial.print(F(", "));
      if (i % 12 == 0) Serial.println();  // Line breaks
    }
    Serial.println(F("};"));
    Serial.print(F("IrSender.sendRaw(rawAC, "));
    Serial.print(cutLen);
    Serial.println(F(", 38);  // For TCL/Baumann AC"));
 Serial.println("-----------------------");
    IrReceiver.resume();  // Ready for next
  }
}