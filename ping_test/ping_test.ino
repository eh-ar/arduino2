#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> // Install ArduinoJson library

// Network configuration (adjust to your network)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Unique MAC address
IPAddress ip(192, 168, 1, 17); // Arduino's IP (or use DHCP)
IPAddress gateway(192, 168, 1, 1); // Your router's IP
IPAddress subnet(255, 255, 255, 0); // Your subnet mask

// API configuration
const char* apiHost = "192.168.1.179"; // Replace with your API's domain or IP
const char* apiPath = "/your_api_endpoint"; // Replace with your API's path
int apiPort = 3000; // Or 443 for HTTPS (if using HTTPS, you'll need more setup)

EthernetClient client;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip, gateway, subnet); // Initialize Ethernet (or use DHCP)
  delay(2000);
  Serial.println("Ethernet Initialized");
}

int c = 0;
void loop() {
  // Data to send to the API (example)
  float temperature = 25.5;
  int humidity = 60;
  c++;

  // Create JSON object
  StaticJsonDocument<200> doc;  // Adjust size as needed
  doc["id"] = c;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  // Serialize JSON to a string
  String jsonData;
  serializeJson(doc, jsonData);

  // Connect to the API
  if (client.connect(apiHost, apiPort)) {
    Serial.println("Connected to API");

    // Send HTTP POST request
    client.print("POST ");
    client.print(apiPath);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(apiHost);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println("Connection: close"); // Important for Arduino
    client.println(); // Empty line to signal end of headers
    client.print(jsonData); // Send the JSON data

    // Read the response from the API (optional)
    while (client.available()) {
      String line = client.readString();
      Serial.println(line);
    }

    client.stop(); // Close the connection
    Serial.println("Disconnected from API");

  } else {
    Serial.println("Connection to API failed");
  }

  delay(5000); // Send data every 5 seconds (adjust as needed)
}
