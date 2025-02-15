#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN pins

void setup() {
  Serial.begin(9600);
  Serial.println("Transmitter Initializing...");
  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!!");
    while (1);
  }
  radio.setChannel(76); // Set the channel
  radio.setPALevel(RF24_PA_MAX); // Set the power level
  radio.openWritingPipe(0xF0F0F0F0E1LL); // Set the address
  radio.stopListening();
  Serial.println("Transmitter Ready");
}

void loop() {
  const char text[] = "Hello, World!";
  Serial.println("Sending message: " + String(text));
  if (!radio.write(&text, sizeof(text))) {
    Serial.println("Transmission failed or timed out");
  }
  delay(1000);
}
