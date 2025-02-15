


#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <softSerial.h>


softSerial mySerial(2, 4);  // rx / tx

#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

// Set MAC Address (must be unique on the network)
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set static IP (or use DHCP)
//IPAddress ip(192, 168, 1, 177);
//EthernetClient client;

// Define W5500 Chip Select (CS) Pin
#define W5500_CS 15

#define RF_FREQUENCY 433000000   // Hz
#define TX_OUTPUT_POWER 14       // dBm
#define LORA_BANDWIDTH 0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7  // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 2000
#define BUFFER_SIZE 60  // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
int16_t txNumber;
int16_t rssi, rxSize;

bool lora_idle = true;

void setup() {
  Serial.begin(9600);
  txNumber = 0;
  rssi = 0;

  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;  // Handle RX Timeout
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
  // Start receiving mode
  Radio.Rx(0);
  /*
    Serial.println("Initializing Ethernet...");
    Ethernet.init(W5500_CS);  // Set CS pin
    Ethernet.begin(mac, ip);

    Serial.print("Local IP: ");
    Serial.println(Ethernet.localIP());
    */
  mySerial.begin(9600);

  //pinMode(0, INPUT);
  //pinMode(2, INPUT);
  //pinMode(GPIO1, INPUT);
  //pinMode(4, INPUT);
  
}

int c = 0;
void loop() {
  c++;
  //Serial.println("loop "+ String(c));
  Radio.IrqProcess();  // Process IRQs
                       /*
// Test: Print Local IP Every 5 seconds
    Serial.print("Current IP: ");
    Serial.println(Ethernet.localIP());
    */
  delay(2000);


/*

Serial.println("----");
  Serial.println(analogRead(2));
  
  Serial.println(digitalRead(GPIO1));
  
  Serial.println(digitalRead(3));
  Serial.println(digitalRead(4));
  Serial.println("----");
  */
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  //Serial.println("[OnRxDone] Packet Received!");

  rxSize = size;
  memcpy(rxpacket, payload, min(size, BUFFER_SIZE - 1));
  rxpacket[min(size, BUFFER_SIZE - 1)] = '\0';

  Serial.printf("[OnRxDone] Received: \"%s\" | RSSI: %d | Length: %d\r\n", rxpacket, rssi, rxSize);
  

  mySerial.printf("%s%d\n", rxpacket,rssi);  // Send the received data to the hardware serial port
  //mySerial.printf("%s", "\n");
  
  memset(rxpacket, 0, sizeof(rxpacket));

  //Serial.println("[OnRxDone] Restarting Radio...");
  Radio.Sleep();  // Put radio to sleep before re-init
  delay(50);
  Radio.Rx(0);

  //Serial.println("[OnRxDone] Waiting for next packet...");
}

void OnRxTimeout(void) {
  Serial.println("RX Timeout!");
  //Radio.Sleep();
  Radio.IrqProcess();
  delay(500);
  Radio.Rx(0);
  Serial.println("Entering RX mode again...");
  return;
}

void OnTxTimeout(void) {
  Serial.println("TX Timeout!");
  //Radio.Sleep();
  Radio.IrqProcess();
  delay(500);
  Radio.Rx(0);
  Serial.println("Entering RX mode again...");
  return;
}
