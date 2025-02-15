#include <SPI.h>
#include <Ethernet.h>
#include <NTPClient.h>  // Install NTPClient library
#include <WiFiUdp.h>    // Required for NTPClient
#include <TimeLib.h>

// Network configuration (using DHCP - recommended)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Unique MAC address
IPAddress ip(192, 168, 1, 17);                        // Arduino's IP (or use DHCP)
IPAddress gateway(192, 168, 1, 1);                    // Your router's IP
IPAddress subnet(255, 255, 255, 0);                   // Your subnet mask
// NTP Server configuration
//const char* ntpServer = "pool.ntp.org"; // Use a public NTP server
const char* ntpServer = "time.nist.gov";  // Try NIST
// or
//const char* ntpServer = "time.google.com"; // Try Google

const long ntpUpdateTime = 3600;    // Update NTP time every hour (in seconds)
unsigned long previousNTPTime = 0;  // To keep track of last NTP update

// Timezone offset (in seconds) - adjust for your timezone
const long timezoneOffset = 3600 + 3600 + 3600 + 1800;  // Example: GMT+1 (Central European Time)

// Ethernet and NTP setup
EthernetClient ethClient;
WiFiUDP udp;

NTPClient timeClient(udp, ntpServer, timezoneOffset, ntpUpdateTime);
//NTPClient timeClient(udp);

//IPAddress ntpServerIP(129, 6, 15, 28); // IP address for time.nist.gov (example, check for current IP)
//NTPClient timeClient(udp, ntpServerIP, timezoneOffset, ntpUpdateTime); // Use IP address



void setup() {
  Serial.begin(9600);

  // Initialize Ethernet with DHCP
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    while (true)
      ;
  } else {
    //Ethernet.begin(mac, ip, gateway, subnet);
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  }
  delay(2000);
  timeClient.begin();                        // Initialize NTPClient
  timeClient.setTimeOffset(timezoneOffset);  // Set the timezone offset
  Serial.println("NTPClient initialized");
}

void loop() {
  // Update time from NTP server if needed
  if (millis() - previousNTPTime >= ntpUpdateTime * 1000) {
    Serial.println("Updating time from NTP server...");
    if (timeClient.update()) {
      previousNTPTime = millis();
      Serial.println("Time updated successfully");
    } else {
      Serial.println("Failed to update time from NTP server");
    }
  }

  // Now you can use timeClient.getEpochTime() to get the current Unix epoch time
  // or timeClient.getFormattedTime() to get a formatted time string.

  if (timeClient.update()) {

    unsigned long epochTime = timeClient.getEpochTime();
    Serial.print("Epoch Time: ");
    Serial.println(epochTime);
    time_t currentTime = (time_t)epochTime;
    Serial.println(currentTime);
    setTime(currentTime);

    // Now use TimeLib's functions for formatted time
    Serial.print(year());
    Serial.print("-");
    Serial.print(month());
    Serial.print("-");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());
  } else {
    Serial.println("NTP update failed");
  }
  // Example: Timer based on network time
  if (timeClient.getHours() == 10 && timeClient.getMinutes() == 00) {  // Example: Trigger at 10:00 AM
    Serial.println("It's 10:00 AM!");
    // Perform your desired action here
  }

  delay(1000);  // Check time every second
}
