#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN pins

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(76); // Set the channel
  radio.setPALevel(RF24_PA_MAX); // Set the power level
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL); // Set the address
  radio.printDetails();
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    String message;
    radio.read(&message, sizeof(message));
    Serial.println(message);
  }
}
