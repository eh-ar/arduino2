#include <SoftwareSerial.h>

// Define software serial pins
SoftwareSerial gpsSerial(3, 2); // RX, TX
String gpsData;
void setup() {
  // Start the hardware serial for the Serial Monitor
  Serial.begin(115200);
  // Start the software serial for the GPS module
  gpsSerial.begin(115200); // Set to 115200 baud rate

  Serial.println("Type a command and press Enter to send it to the GPS module:");
  //sendGPSCommand("gpgga 1");
  delay(1000);
}

void loop() {
  // Check if there's data from the Serial Monitor
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    sendGPSCommand(command.c_str());
  }

  // Check if data is available from the GPS module
  if (gpsSerial.available()) {
     gpsData = gpsSerial.readStringUntil('\n');
     //Serial.println(gpsData);
    if (gpsData.startsWith("$GNGGA")) {
      parseGPGGA(gpsData);
    } else{
      Serial.println(gpsData);
    }
  }
}

void sendGPSCommand(const char* command) {
  gpsSerial.println(command);
  Serial.print("Command sent: ");
  Serial.println(command);
}
void parseGPGGA(String gpgga) {
  int commaIndex = 0;
  int nextCommaIndex = -1;
  String data[15];

  for (int i = 0; i < 15; i++) {
    nextCommaIndex = gpgga.indexOf(',', commaIndex + 1);
    if (nextCommaIndex == -1) {
      data[i] = gpgga.substring(commaIndex + 1);
      break;
    }
    data[i] = gpgga.substring(commaIndex + 1, nextCommaIndex);
    commaIndex = nextCommaIndex;
  }
String time = data[1];
  String lat = data[2];
  String latDir = data[3];
  String lon = data[4];
  String lonDir = data[5];
  String fixQuality = data[6];
  String satellites = data[7];
  String hdop = data[8];
  String altitude = data[9];

  // Convert latitude and longitude to decimal degrees
  float latitude = convertToDecimalDegrees(lat, latDir, "loat");
  float longitude = convertToDecimalDegrees(lon, lonDir, "lon");
/*
  Serial.println("Time: " + time);
  Serial.print("Latitude: ");
  Serial.println(latitude, 8);
  Serial.print("Longitude: ");
  Serial.println(longitude, 8);
  Serial.println("Fix Quality: " + fixQuality);
  Serial.println("Satellites: " + satellites);
  Serial.println("HDOP: " + hdop);
  Serial.println("Altitude: " + altitude + " M");
*/
   Serial.print(latitude, 10);
  Serial.print(",");
   Serial.print(longitude, 10);
   Serial.print(",");
   Serial.println(altitude);
}

float convertToDecimalDegrees(String coordinate, String direction, String type) {
  float decimal;
  if (type == "lon"){
    //Serial.println(coordinate.substring(0, 3));
 decimal = coordinate.substring(0, 3).toFloat() + (coordinate.substring(3).toFloat() / 60.0);
  } else {
    decimal = coordinate.substring(0, 2).toFloat() + (coordinate.substring(2).toFloat() / 60.0);
  }
  if (direction == "S" || direction == "W") {
    decimal *= -1;
  }
  return decimal;
}