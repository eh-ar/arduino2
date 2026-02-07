#include <SoftwareSerial.h>

SoftwareSerial gpsSerial(4, 5); // RX = D4, TX = D5

const double BASE_LATITUDE = 30.2225356;
const double BASE_LONGITUDE = 57.09026540;
const double BASE_HEIGHT = 1782.5;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(115200);
  
  delay(3000);
  Serial.println("Starting GPS Base Configuration...");
  
  configureBaseStation();
}

void loop() {
  // Monitor GPS output
  if (gpsSerial.available()) {
    Serial.write(gpsSerial.read());
  }
  
  // Send manual commands from Serial Monitor
  if (Serial.available()) {
    gpsSerial.write(Serial.read());
  }
}

void configureBaseStation() {
  String commands[] = {
    "MODE BASE " + String(BASE_LATITUDE, 11) + " " + 
                    String(BASE_LONGITUDE, 11) + " " + 
                    String(BASE_HEIGHT, 2),    
    "unlog",  
    "CONFIG COM2 9600",
    "rtcm1006 com2 10",
    "rtcm1033 com2 10",
    "rtcm1074 com2 2",
    "rtcm1084 com2 2",
    "rtcm1094 com2 2",
    "rtcm1124 com2 2", 
    "rtcm1230 com2 10",
    "SAVECONFIG",
  
  };
  
  for (int i = 0; i < 9; i++) {
    Serial.print("Sending: ");
    Serial.println(commands[i]);
    gpsSerial.println(commands[i]);
    delay(500); // Wait for processing
  }
  
  Serial.println("Configuration complete!");
}