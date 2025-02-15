#include <SPI.h>
#include <LoRa.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <Preferences.h>



#include <Arduino.h>
#include <esp_sleep.h>
#include <esp32/rtc.h>

const int EEPROM_ADDRESS = 2;
unsigned long timerValue = 0;
#define NSS 7
#define NRESET 3
#define DIO0 2
#define BAND 433E6

#define MYPORT_TX 0
#define MYPORT_RX 1

#define TIME_TO_SLEEP 8
#define uS_TO_S_FACTOR 1000000

RTC_DATA_ATTR int timeCount = 0;
RTC_DATA_ATTR int bootCount = 0;
float light;
int light_value;

EspSoftwareSerial::UART mySerial;
ModbusMaster node;
Preferences preferences;


int cc = 0;
static uint32_t i;
uint8_t j, result;
uint16_t data[5];
String d;
uint16_t val;

String readRS485Device(uint8_t deviceAddress) {
  String d = "";
  node.begin(deviceAddress, mySerial);  // Set the Modbus address and use the SoftwareSerial connection
  //Serial.println("Check RS485");
  result = node.readHoldingRegisters(0, 5);

  if (result == node.ku8MBSuccess) {
    val = node.getResponseBuffer(0);
    d = String(deviceAddress) + " " + String(val);
    for (j = 1; j < 5; j++) {
      val = node.getResponseBuffer(j);
      //Serial.println(val);
      d = d + " " + String(val);
    }
  }
  return d;
}

void setup() {
  Serial.println("--------  setup  -----------");
  preferences.begin("lora", false);
  timerValue = preferences.getInt("timerValue", 0);

  pinMode(NSS, OUTPUT);
  pinMode(NRESET, OUTPUT);
  pinMode(DIO0, INPUT);

  Serial.begin(115200);
  Serial.println("Starting Module");
  while (!Serial)
    ;

  LoRa.setPins(NSS, NRESET, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Transmitter");

  //EEPROM.get(EEPROM_ADDRESS, timerValue);

  Serial.println("last timer: " + timerValue);
  mySerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  //node.begin(51, mySerial);
}


void loop() {
  Serial.println("----------  event loop  ------------");
  cc++;
  timerValue = timerValue + 8;
  bootCount = bootCount + 1;
  timeCount = timeCount + 8;
  String d1 = "";
  String d2 = "";


  Serial.println("rs485 1");
  d2 = readRS485Device(51);
  if (d2 == "") {
    delay(1000);
    d2 = readRS485Device(51);
  }

  String mmsg = String(timerValue);
  LoRa.beginPacket();
  LoRa.print(mmsg + " - " + bootCount + " - " + light + " - " + d1 + " - " + d2);
  LoRa.endPacket();
  Serial.println("Sent: " + mmsg + " " + cc + " - light: " + light);
  Serial.println(d2);

  //EEPROM.put(EEPROM_ADDRESS, timerValue);
  preferences.putInt("timerValue", timerValue);
  //EEPROM.commit();
  delay(8000);

  // Enable timer wake-up
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  // Enter deep sleep mode
  //Serial.println("Deep Sleep");
  //esp_deep_sleep_start();
  //Serial.println("wakeup");
}
