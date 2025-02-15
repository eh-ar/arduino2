/* Wi-Fi STA Connect and Disconnect Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

*/
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


const char *ssid = "Farhan2.4";
const char *password = "Farhan123456.";

#define MYPORT_TX 2
#define MYPORT_RX 4


int btnGPIO = 0;
int btnState = false;

SoftwareSerial mySerial(MYPORT_RX, MYPORT_TX);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 12600, 15);

#define pin 3
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  //pinMode(pin, OUTPUT);
  mySerial.begin(9600);
  delay(10);

  // Set GPIO0 Boot button as input
  pinMode(btnGPIO, INPUT);

  // We start by connecting to a WiFi network
  // To debug, please enable Core Debug Level to Verbose

  Serial.println();
  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  // Auto reconnect is set true as default
  // To set auto connect off, use the following function
  //    WiFi.setAutoReconnect(false);

  // Will try for about 10 seconds (20x 500ms)
  int tryDelay = 500;
  int numberOfTries = 20;

  // Wait for the WiFi event
  while (true) {

    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: Serial.println("[WiFi] SSID not found"); break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return;
        break;
      case WL_CONNECTION_LOST: Serial.println("[WiFi] Connection was lost"); break;
      case WL_SCAN_COMPLETED: Serial.println("[WiFi] Scan is completed"); break;
      case WL_DISCONNECTED: Serial.println("[WiFi] WiFi is disconnected"); break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        return;
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;

        timeClient.begin();
    }
    delay(tryDelay);

    if (numberOfTries <= 0) {
      Serial.print("[WiFi] Failed to connect to WiFi!");
      // Use disconnect function to force stop trying to connect
      WiFi.disconnect();
      return;
    } else {
      numberOfTries--;
    }
    
  }
}

void sendDataToAPI(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("sending data");
    HTTPClient http;
    http.begin("http://185.142.158.194:54016/your_api_endpoint");  // Replace <YOUR_SERVER_IP> with the actual IP address of your server
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(data);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("[HTTP] Response: " + response);
    } else {
      Serial.println("[HTTP] Error on sending POST: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("[HTTP] WiFi not connected");
  }
}
int cc = 0;
void loop() {
 
 //Serial.println("Current time: ");
 //Serial.println();
  
  if (mySerial.available() > 0) {
    timeClient.update();
    cc++;
    // Create a JSON object
    String data = mySerial.readString();
    //String data = "this is a test";
    Serial.println("received: " + data);
    Serial.println("time: " + timeClient.getFormattedTime());
    Serial.println("timeStmp: " + String(timeClient.getEpochTime()));
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["id"] = cc;
    jsonDoc["message"] = String(timeClient.getEpochTime()) + ":" + data ;

    // Convert JSON object to a string
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    sendDataToAPI(jsonString);
    //Serial.println("data is sent");
    // Read the button state
  }
  btnState = digitalRead(btnGPIO);

  if (btnState == LOW) {
    // Disconnect from WiFi
    Serial.println("[WiFi] Disconnecting from WiFi!");
    // This function will disconnect and turn off the WiFi (NVS WiFi data is kept)
    if (WiFi.disconnect(true, false)) {
      Serial.println("[WiFi] Disconnected from WiFi!");
    }
    delay(1000);
  }
  delay(50);
}
