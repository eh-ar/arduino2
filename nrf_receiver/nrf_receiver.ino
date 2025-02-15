#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10, 4000000); // CE, CSN pins

void setup() {
  Serial.begin(9600);
  Serial.println("Receiver Initializing...");
  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!!");
    while (1);
  }
  radio.setChannel(76); // Set the channel
  radio.setPALevel(RF24_PA_MAX); // Set the power level
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL); // Set the address
  radio.startListening();
  Serial.println("Receiver Ready");
}

void loop() {
  if (radio.available()) {
    char text[32] = {0}; // Create a buffer to hold the received message
    radio.read(&text, sizeof(text));
    Serial.println("Received message: " + String(text));
  }
}
