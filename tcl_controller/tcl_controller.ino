// Working code for IRremoteESP8266 v2.8+ / v4.x (2025)
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Tcl.h>        // This pulls in the TCL112AC class

const uint16_t kIrLed = 4;          // IR LED on GPIO4
IRsend irsend(kIrLed);              // Create the IR sender object

IRTcl112Ac ac(kIrLed);              // ← NEW correct constructor (pin only)

void setup() {
  Serial.begin(115200);
  irsend.begin();                   // Start the IR sender (required)
  ac.begin();                       // Initialise the AC object
  Serial.println("TCL112AC ready – ESP32");
}

void loop() {
  Serial.println("Sending: Cool 24°C, Power ON, Auto Fan, Swing OFF");

  ac.on();                          // Power ON
  ac.setMode(kTcl112AcCool);        // Cool mode
  ac.setTemp(24);                   // 24°C
  ac.setFan(kTcl112AcFanAuto);      // Auto fan
  ac.setSwingHorizontal(false);     // Horizontal swing OFF
  ac.setSwingVertical(false);       // Vertical swing OFF

  ac.send();                        // Send once
  ac.send();                        // Send second time → total 2 repeats (TCL needs this)

  delay(5000);                     // Wait 12 seconds before next command

  ac.on();                          // Power ON
  ac.setMode(kTcl112AcHeat);        // Cool mode
  ac.setTemp(27);                   // 24°C
  ac.setFan(kTcl112AcFanAuto);      // Auto fan
  ac.setSwingHorizontal(false);     // Horizontal swing OFF
  ac.setSwingVertical(false);       // Vertical swing OFF

  ac.send();                        // Send once
  ac.send();                        // Send second time → total 2 repeats (TCL needs this)

  delay(5000);                     // Wait 12 seconds before next command

}