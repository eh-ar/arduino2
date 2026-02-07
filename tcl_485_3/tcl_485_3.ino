#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Tcl.h>
#include <Preferences.h>  // ← Modern ESP32 non-volatile storage
#include <ModbusRTU.h>

// ---------- IR ----------
const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);
IRTcl112Ac ac(kIrLed);
ModbusRTU mb;
Preferences prefs;  // For saving last state


// ---------- MAX485 Pins (manual control) ----------
#define RS485_TX_PIN 7  // DI → ESP32 TX
#define RS485_RX_PIN 5  // RO → ESP32 RX
#define RS485_DE_RE 6   // DE + RE tied together → GPIO5 (HIGH = transmit)

#define SLAVE_ID 32
#define MB_BAUD 9600

// ---------- DHT22 Sensor ----------
#define LDR 3
#define A2 2
#define A1 1
#define A0 0

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
#define REG_POWER 0     // R/W
#define REG_MODE 1      // R/W
#define REG_TEMP_SET 2  // R/W
#define REG_FAN 3       // R/W
#define REG_H_SWING 4   // R/W
#define REG_V_SWING 5   // R/W
#define REG_COUNT 6     // R/W

void setup() {
  Serial.begin(115200);

  // MAX485 direction control
  pinMode(RS485_DE_RE, OUTPUT);
  digitalWrite(RS485_DE_RE, LOW);  // Start in receive mode

  // UART1 for RS485
  Serial1.begin(MB_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

  // Modbus with manual DE/RE control
  mb.begin(&Serial1, RS485_DE_RE);  // ← This line tells the library to use GPIO5 for TX enable
  mb.slave(SLAVE_ID);

  // --- Load last known state from flash ---
  prefs.begin("tcl-ac", false);  // false = read/write mode
  size_t len = prefs.getBytes("state", &lastSettings, sizeof(lastSettings));

  bool valid = (len == sizeof(lastSettings) && lastSettings.temp >= 160 && lastSettings.temp <= 300 && lastSettings.mode <= 4);

  if (!valid) {
    // First boot or corrupted → use safe defaults
    lastSettings = { 1, 2, 220, 0, 0, 0 };  // ON, GHeat, 22°C, Auto, swings OFF
    Serial.println("No valid saved state → using defaults");
  } else {
    Serial.println("Loaded last state from flash");
  }

  // Apply loaded values to Modbus registers
  mb.addHreg(REG_POWER, lastSettings.power);
  mb.addHreg(REG_MODE, lastSettings.mode);
  mb.addHreg(REG_TEMP_SET, lastSettings.temp);
  mb.addHreg(REG_FAN, lastSettings.fan);
  mb.addHreg(REG_H_SWING, lastSettings.swingH);
  mb.addHreg(REG_V_SWING, lastSettings.swingV);
  

  Serial.println("TCL AC Modbus Slave (MAX485 manual) READY");
  Serial.println("DE/RE on GPIO5 | Slave ID:32 | 9600 8N1");
  Serial.printf("Restored: P=%d M=%d T=%d F=%d HS=%d VS=%d\n",
                lastSettings.power, lastSettings.mode, lastSettings.temp,
                lastSettings.fan, lastSettings.swingH, lastSettings.swingV);
  applySettingsToAC();
}

void loop() {
  mb.task();

  // Update room temperature
  static uint32_t lastTemp = 0;
  if (millis() - lastTemp >= 20000) {
    lastTemp = millis();
    float t = analogRead(LDR);
    if (!isnan(t)) mb.Hreg(REG_LIGHT, (uint16_t)round(t * 10));
    Serial.print("Light: ");
    Serial.println(t);
  }

  // Check if any control register changed
  bool changed = false;
  if (mb.Hreg(REG_POWER) != lastSettings.power) {
    changed = true;
    lastSettings.power = mb.Hreg(REG_POWER);
  }
  if (mb.Hreg(REG_MODE) != lastSettings.mode) {
    changed = true;
    lastSettings.mode = mb.Hreg(REG_MODE);
  }
  if (mb.Hreg(REG_TEMP_SET) != lastSettings.temp) {
    changed = true;
    lastSettings.temp = mb.Hreg(REG_TEMP_SET);
  }
  if (mb.Hreg(REG_FAN) != lastSettings.fan) {
    changed = true;
    lastSettings.fan = mb.Hreg(REG_FAN);
  }
  if (mb.Hreg(REG_H_SWING) != lastSettings.swingH) {
    changed = true;
    lastSettings.swingH = mb.Hreg(REG_H_SWING);
  }
  if (mb.Hreg(REG_V_SWING) != lastSettings.swingV) {
    changed = true;
    lastSettings.swingV = mb.Hreg(REG_V_SWING);
  }

  if (changed) {
    Serial.println("Settings changed → saving to flash + sending to AC");
    prefs.putBytes("state", &lastSettings, sizeof(lastSettings));  // Save instantly
    applySettingsToAC();
  }
}

// Helper: send current settings to the real AC
void applySettingsToAC() {
  lastSettings.power ? ac.on() : ac.off();

  switch (lastSettings.mode) {
    case 0: ac.setMode(kTcl112AcAuto); break;
    case 1: ac.setMode(kTcl112AcCool); break;
    case 2: ac.setMode(kTcl112AcHeat); break;
    case 3: ac.setMode(kTcl112AcDry); break;
    case 4: ac.setMode(kTcl112AcFan); break;
    default: ac.setMode(kTcl112AcCool); break;
  }

  ac.setTemp(constrain(lastSettings.temp / 10, 16, 30));

  switch (lastSettings.fan) {
    case 0: ac.setFan(kTcl112AcFanAuto); break;
    case 1: ac.setFan(kTcl112AcFanLow); break;
    case 2: ac.setFan(kTcl112AcFanMed); break;
    case 3: ac.setFan(kTcl112AcFanHigh); break;
    default: ac.setFan(kTcl112AcFanAuto); break;
  }

  ac.setSwingHorizontal(lastSettings.swingH);
  ac.setSwingVertical(lastSettings.swingV);

  // 3 repeats for reliability
  ac.send();
  delay(80);
  ac.send();
  delay(80);
  ac.send();

  Serial.printf("→(REG) Power:%d Mode:%d Temp:%d Fan:%d H-Swing:%d V-Swing:%d\n",
                mb.Hreg(0), mb.Hreg(1), mb.Hreg(2), mb.Hreg(3), mb.Hreg(4), mb.Hreg(5));
  Serial.printf("→(INN) Power:%d Mode:%d Temp:%d Fan:%d H-Swing:%d V-Swing:%d\n",
                lastSettings.power, lastSettings.mode, lastSettings.temp, lastSettings.fan, lastSettings.swingH, lastSettings.swingV);
}