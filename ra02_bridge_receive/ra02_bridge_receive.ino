#include <SPI.h>
#include <RH_RF22.h>

#define RFM_CS   10
#define RFM_INT  2
#define RFM_RST  9

RH_RF22 rf22(RFM_CS, RFM_INT);

void setup() {
  Serial.begin(9600);

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

  rf22.setFrequency(433.0);

  Serial.println("FSK receiver ready");
}

void loop() {
  if (rf22.available()) {
    uint8_t buf[1];
    uint8_t len = sizeof(buf);
    if (rf22.recv(buf, &len)) {
      Serial.write(buf[0]);
    }
  }
}
