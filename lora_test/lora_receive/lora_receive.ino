#include <LiquidCrystal.h>
#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial mySerial(2, 3);  // RX, TX
#define NSS 10
#define NRESET 6
#define DIO0 7
#define BAND 433E6  // Set frequency to 433 MHz

const int rs = 8, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  mySerial.begin(9600);
  Serial.begin(115200);
  Wire.begin(3);  // Join I2C bus as master
  Wire.setWireTimeout(30000 /* us */, true /* reset_on_timeout */);
  while (!Serial) {};

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1) {};
  }
  Serial.println("LoRa Receiver");
  delay(20);
  //lcdInit();
  //lcdPrint("HELLO, WORLD!");
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("!");
}

void loop() {
  // Try to parse a packet
  int packetSize = LoRa.parsePacket();


  if (packetSize > 0) {
    // Received a packet
    String receivedMessage = "";

    while (LoRa.available()) {
      char c = LoRa.read();
      receivedMessage += c;
    }
    int rssi = (LoRa.packetRssi());
    Serial.print("Received: ");
    Serial.println(receivedMessage + "- rssi: " + rssi);
    Serial.println("send to OPI: ");
    mySerial.println(receivedMessage);
    //String dataToSend = receivedMessage;
    //char charArray[dataToSend.length() + 1];               // Create a character array
    //dataToSend.toCharArray(charArray, sizeof(charArray));  // Convert String to character array

    //Wire.beginTransmission(8);  // Address of the slave
    //Wire.write(charArray);      // Send the character array
    //Wire.endTransmission();
    int ind1 = receivedMessage.indexOf(" ");
    String msg = receivedMessage.substring(ind1);
    ind1 = msg.indexOf(" ");
    msg = msg.substring(ind1 + 1);
    ind1 = msg.indexOf(" ");
    String counter = msg.substring(0, ind1);
    msg = msg.substring(ind1 + 1);

    ind1 = msg.indexOf(" ");
    String panel = msg.substring(0, ind1);
    msg = msg.substring(ind1 + 1);

    ind1 = msg.indexOf(" ");
    String batt = msg.substring(0, ind1);

    msg = msg.substring(ind1 + 1);

    lcd.setCursor(6, 0);
    lcd.print(counter);
    delay(5);


    lcd.setCursor(12, 0);
    lcd.print(panel.toFloat() * 10 / 1024);
    delay(5);


    lcd.setCursor(12, 1);
    lcd.print(msg.toFloat() * 10 / 1024);
    delay(5);


    lcd.setCursor(0, 1);
    lcd.print(String(rssi));
    delay(5);
  }
  lcd.setCursor(0, 0);
  lcd.print(int(millis() / 1000));
  delay(5);
}
