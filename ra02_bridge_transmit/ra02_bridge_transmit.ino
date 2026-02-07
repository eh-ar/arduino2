#include <SPI.h>
#include <RH_RF22.h>

#define RFM_CS   10
#define RFM_INT  8
#define RFM_RST  9

RH_RF22 rf22(RFM_CS, RFM_INT);

void setup() {
  Serial.begin(9600);

  // Reset module
  pinMode(RFM_RST, OUTPUT);
  digitalWrite(RFM_RST, HIGH);
  delay(10);
  digitalWrite(RFM_RST, LOW);
  delay(10);
  digitalWrite(RFM_RST, HIGH);
  delay(10);

  if (!rf22.init()) {
    Serial.println("RF22 init failed");
    while (1);
  }

  rf22.setFrequency(433.0);   // Set frequency
  rf22.setTxPower(31);        // Max power (0–31)

  Serial.println("FSK transmitter ready");
}

void loop() {
  if (Serial.available()) {
    char data = Serial.read();
    rf22.send((uint8_t*)&data, 1);
    rf22.waitPacketSent();
  }
}
