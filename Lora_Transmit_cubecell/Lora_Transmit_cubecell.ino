/* Heltec Automation send communication test example
 *
 * Function:
 * 1. Send data from a esp32 device over hardware 
 *  
 * Description:
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
 * */

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <softSerial.h>
#include <ModbusMaster.h>

softSerial softwareSerial(GPIO5 /*TX pin*/, GPIO0 /*RX pin*/);
ModbusMaster node;

#define RF_FREQUENCY 433000000  // Hz

#define TX_OUTPUT_POWER 23  // dBm

#define LORA_BANDWIDTH 0         // [0: 125 kHz, \
                                 //  1: 250 kHz, \
                                 //  2: 500 kHz, \
                                 //  3: Reserved]
#define LORA_SPREADING_FACTOR 7  // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5, \
                                 //  2: 4/6, \
                                 //  3: 4/7, \
                                 //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false


#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30  // Define the payload size here

#define vext GPIO6
char txpacket[90];
char txpacket1[BUFFER_SIZE];
char txpacket2[BUFFER_SIZE];
char txpacket3[BUFFER_SIZE];

char rxpacket[BUFFER_SIZE];

int txNumber;

bool lora_idle = true;


static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

#define timetillsleep 6000
#define timetillwakeup 10000
#define sensorDelay 2000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower = 1;


void onSleep() {
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n", timetillwakeup);
  lowpower = 1;
  //timetillwakeup ms later wake up;
  TimerSetValue(&wakeUp, timetillwakeup);
  TimerStart(&wakeUp);
}
void onWakeUp() {
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n", timetillsleep);
  lowpower = 0;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue(&sleep, timetillsleep);
  TimerStart(&sleep);
}
//-------------------------------------------------------
void setup() {
  Serial.begin(9600);
  //Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  pinMode(vext, OUTPUT);
  digitalWrite(vext, HIGH);

  pinMode(GPIO1, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  pinMode(GPIO3, OUTPUT);

  digitalWrite(GPIO1, LOW);
  digitalWrite(GPIO2, LOW);
  digitalWrite(GPIO3, LOW);


  uint16_t voltage = getBatteryVoltage();
  Serial.println(voltage);
  softwareSerial.begin(4800);
  node.begin(1, softwareSerial);
  txNumber = 0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);


  TimerInit(&sleep, onSleep);
  TimerInit(&wakeUp, onWakeUp);
  onSleep();
}



void loop() {
  if (lowpower) {
    lowPowerHandler();
  } else {
    if (lora_idle == true) {
      txNumber += timetillwakeup;
      //delay(4000);

      uint8_t j, result;
      uint16_t data1[5], data2[5], data3[5];  // = { 0, 0, 0, 0, 0 };
      String Out = "";

      uint16_t voltage = getBatteryVoltage();
      Serial.println(voltage);
      digitalWrite(vext, LOW);

      digitalWrite(GPIO1, HIGH);
      delay(sensorDelay);
      txNumber += sensorDelay;
      data1[0] = 0;
      data1[1] = 0;
      data1[2] = 0;
      data1[3] = 0;
      data1[4] = 0;

      result = node.readHoldingRegisters(0, 5);
      if (result == node.ku8MBSuccess) {
        for (j = 0; j < 5; j++) {
          data1[j] = node.getResponseBuffer(j);
        }
      }
      digitalWrite(GPIO1, LOW);

      digitalWrite(GPIO2, HIGH);
      delay(sensorDelay);
      txNumber += sensorDelay;
      data2[0] = 0;
      data2[1] = 0;
      data2[2] = 0;
      data2[3] = 0;
      data2[4] = 0;
      result = node.readHoldingRegisters(0, 5);
      if (result == node.ku8MBSuccess) {
        for (j = 0; j < 5; j++) {
          data2[j] = node.getResponseBuffer(j);
        }
      }
      digitalWrite(GPIO2, LOW);

      digitalWrite(GPIO3, HIGH);
      delay(sensorDelay);
      txNumber += sensorDelay;
      data3[0] = 0;
      data3[1] = 0;
      data3[2] = 0;
      data3[3] = 0;
      data3[4] = 0;
      result = node.readHoldingRegisters(0, 5);
      if (result == node.ku8MBSuccess) {
        for (j = 0; j < 5; j++) {
          data3[j] = node.getResponseBuffer(j);
        }
      }
      digitalWrite(GPIO3, LOW);

      digitalWrite(vext, HIGH);

      sprintf(txpacket1, "%d,%d,%d,%d,%d,%d,%d", txNumber, voltage, data1[0], data1[1], data1[2], data1[3], data1[4]);  //start a package
      sprintf(txpacket2, "%d,%d,%d,%d,%d", data2[0], data2[1], data2[2], data2[3], data2[4]);                           //start a package
      sprintf(txpacket3, "%d,%d,%d,%d,%d", data3[0], data3[1], data3[2], data3[3], data3[4]);                           //start a package

      Serial.printf("\r\nsending packet \"%s,%s,%s\" , length %d\r\n", txpacket1, txpacket2, txpacket3, strlen(txpacket1) + strlen(txpacket2) + strlen(txpacket3));


      sprintf(txpacket, "%s,%s,%s", txpacket1, txpacket2, txpacket3);
      Radio.Send((uint8_t *)txpacket, strlen(txpacket));  //send the package out
      lora_idle = false;
    }
    Radio.IrqProcess();
  }
}

void OnTxDone(void) {

  Serial.println("TX done......");
  lora_idle = true;
}

void OnTxTimeout(void) {
  Radio.Sleep();
  Serial.println("TX Timeout......");
  lora_idle = true;
}