#include <Arduino.h>

#define DECODE_HASH
#include <IRremote.hpp>

const uint8_t IR_RECEIVE_PIN = 2;

void setup() {
    Serial.begin(115200);
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    Serial.println(F("Press remote button to see MICROSECOND values..."));
}

void loop() {
    if (IrReceiver.decode()) {
        // Print in microseconds instead of ticks
        IrReceiver.printIRResultRawFormatted(&Serial, true); // true = use microseconds
        
        Serial.println(F("\n💡 Copy the MICROSECOND values above for transmitter"));
        Serial.println();
        IrReceiver.resume();
    }
}