#include <SoftwareSerial.h>

SoftwareSerial gpsSerial(4, 5); // D4 = RX, D5 = TX

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(115200);
  
  delay(3000);
  Serial.println("Configuring GPS as Rover...");
  
  configureRoverSimple();
}

void loop() {
  // Forward all GPS data to Serial Monitor
  if (gpsSerial.available()) {
    Serial.write(gpsSerial.read());
  }
  
  // Send commands from Serial Monitor to GPS
  if (Serial.available()) {
    gpsSerial.write(Serial.read());
  }
}

void configureRoverSimple() {
  // Basic rover configuration with GPGGA output
  //gpsSerial.println("MODE ROVER");
  delay(1000);
    gpsSerial.println("unlog com1");
    gpsSerial.println("unlog com2");
  //gpsSerial.println("GPGGA COM2 0.1");
    gpsSerial.println("GPGGA COM1 1");
   gpsSerial.println("GPGGA COM2 0.5");
   
  delay(1000);

  
  Serial.println("Rover configuration complete!");
  Serial.println("GPGGA messages will be output on COM1 at 1Hz");
}