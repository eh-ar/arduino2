#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Tcl.h>
#include <Preferences.h>      // ← Modern ESP32 non-volatile storage
#include <ModbusRTU.h>

// ---------- IR ----------
const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);
IRTcl112Ac ac(kIrLed);
ModbusRTU mb;
Preferences prefs;            // For saving last state


// ---------- MAX485 Pins (manual control) ----------
#define RS485_TX_PIN    7   // DI → ESP32 TX
#define RS485_RX_PIN    5   // RO → ESP32 RX
#define RS485_DE_RE     6    // DE + RE tied together → GPIO5 (HIGH = transmit)

#define SLAVE_ID        32
#define MB_BAUD         9600

// ---------- DHT22 Sensor ----------
#define LDR 3

// Structure to save/restore all settings at once
struct Settings {
  uint16_t power;
  uint16_t mode;
  uint16_t temp;
  uint16_t fan;
  uint16_t swingH;
  uint16_t swingV;
} lastSettings;

// Register map (40001+)
#define REG_POWER      0   // R/W
#define REG_MODE       1   // R/W
#define REG_TEMP_SET   2   // R/W
#define REG_FAN        3   // R/W
#define REG_H_SWING    4   // R/W
#define REG_V_SWING    5   // R/W
#define REG_ROOM_TEMP  6   // Read-only (40010)

void setup() {
  Serial.begin(115200);

  // MAX485 direction control
  pinMode(RS485_DE_RE, OUTPUT);
  pinMode(LDR, INPUT);
  digitalWrite(RS485_DE_RE, LOW);  // Start in receive mode

  // UART1 for RS485
  Serial1.begin(MB_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

  // Modbus with manual DE/RE control
  mb.begin(&Serial1, RS485_DE_RE);   // ← This line tells the library to use GPIO5 for TX enable
  mb.slave(SLAVE_ID);

// --- Load last known state from flash ---
  prefs.begin("tcl-ac", false);   // false = read/write mode
  size_t len = prefs.getBytes("state", &lastSettings, sizeof(lastSettings));

  ool valid = (len == sizeof(lastSettings) &&
                lastSettings.temp >= 16 && lastSettings.temp <= 30 &&
                lastSettings.mode <= 4);

  if (!valid) {
    // First boot or corrupted → use safe defaults
    lastSettings = {1, 2, 22, 0, 0, 0};  // OFF, Cool, 24°C, Auto, swings OFF
    Serial.println("No valid saved state → using defaults");
  } else {
    Serial.println("Loaded last state from flash");
  }

// Apply loaded values to Modbus registers
  mb.addHreg(ADDR_POWER,     lastSettings.power);
  mb.addHreg(ADDR_MODE,      lastSettings.mode);
  mb.addHreg(ADDR_TEMP_SET,  lastSettings.temp);
  mb.addHreg(ADDR_FAN,       lastSettings.fan);
  mb.addHreg(ADDR_SWING_H,   lastSettings.swingH);
  mb.addHreg(ADDR_SWING_V,   lastSettings.swingV);
  mb.addHreg(ADDR_ROOM_TEMP, 0);

  // IR + sensor
  irsend.begin();
  ac.begin();
  //dht.begin();

  Serial.println("TCL AC Modbus Slave (MAX485 manual) READY");
  Serial.println("DE/RE on GPIO5 | Slave ID:1 | 19200 8N1");
}

void loop() {
  mb.task();  // Handles Modbus + automatically toggles DE/RE

  // Update room temperature every 5 seconds
  static uint32_t lastTempRead = 0;
  if (millis() - lastTempRead >= 5000) {
    lastTempRead = millis();
    float t = analogRead(LDR) / 10;// dht.readTemperature();
    if (!isnan(t)) {
      mb.Hreg(REG_ROOM_TEMP, (uint16_t)(t * 10));  // send as 24.5 → 245 (one decimal)
      Serial.print("Room temp: "); Serial.println(t, 1);
    }
    else Serial.println("DHT read error");
  }

  // Detect changes in control registers
  static uint16_t last[6] = {99,99,99,99,99,99};
  bool changed = false;
  for (int i = 0; i < 6; i++) {
    if (mb.Hreg(i) != last[i]) { changed = true; last[i] = mb.Hreg(i); }
  }

  if (changed) {
    Serial.println("PLC command → Sending to AC");

    mb.Hreg(REG_POWER) ? ac.on() : ac.off();

    switch (mb.Hreg(REG_MODE)) {
      case 0: ac.setMode(kTcl112AcAuto); break;
      case 1: ac.setMode(kTcl112AcCool); break;
      case 2: ac.setMode(kTcl112AcHeat); break;
      case 3: ac.setMode(kTcl112AcDry);  break;
      case 4: ac.setMode(kTcl112AcFan);  break;
      default: ac.setMode(kTcl112AcCool); break;
    }

    ac.setTemp(constrain(mb.Hreg(REG_TEMP_SET), 16, 30));

    switch (mb.Hreg(REG_FAN)) {
      case 0: ac.setFan(kTcl112AcFanAuto); break;
      case 1: ac.setFan(kTcl112AcFanLow);  break;
      case 2: ac.setFan(kTcl112AcFanMed);  break;
      case 3: ac.setFan(kTcl112AcFanHigh); break;
      default: ac.setFan(kTcl112AcFanAuto); break;
    }

    ac.setSwingHorizontal(mb.Hreg(REG_H_SWING));
    ac.setSwingVertical(mb.Hreg(REG_V_SWING));

    ac.send(); delay(80);
    ac.send(); delay(80);
    ac.send();

    Serial.printf("→ P:%d M:%d T:%d F:%d HS:%d VS:%d\n",
                  mb.Hreg(REG_POWER), mb.Hreg(REG_MODE), mb.Hreg(REG_TEMP_SET),
                  mb.Hreg(REG_FAN), mb.Hreg(REG_H_SWING), mb.Hreg(REG_V_SWING));
  }
}