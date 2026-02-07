#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Tcl.h>
#include <Preferences.h>
#include <ModbusRTU.h>

// ---------- IR ----------
const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);
IRTcl112Ac ac(kIrLed);

// ---------- Modbus ----------
ModbusRTU mb;
Preferences prefs;

// MAX485 pins
#define RS485_TX_PIN 7
#define RS485_RX_PIN 5
#define RS485_DE_RE 6
#define SLAVE_ID 32
#define MB_BAUD 9600

// ---------- Analog Inputs ----------
#define LDR 3
#define A2 2
#define A1 1
#define A0 0

// ---------- Registers ----------
#define REG_POWER 0     // 40001
#define REG_MODE 1      // 40002
#define REG_TEMP_SET 2  // 40003  (×10, e.g. 240 = 24.0°C)
#define REG_FAN 3       // 40004
#define REG_H_SWING 4   // 40005
#define REG_V_SWING 5   // 40006
#define REG_COUNT 6     // 40007  ← PLC heartbeat counter
#define REG_LIGHT 10    // 40011
#define REG_A2 11
#define REG_A1 12
#define REG_A0 13
#define REG_COUNT2 14  // 40015  ← echo


struct Settings {
  uint16_t power;
  uint16_t mode;
  uint16_t temp;
  uint16_t fan;
  uint16_t swingH;
  uint16_t swingV;
} lastSettings;

void setup() {
  Serial.begin(115200);
  pinMode(RS485_DE_RE, OUTPUT);
  digitalWrite(RS485_DE_RE, LOW);

  Serial1.begin(MB_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  mb.begin(&Serial1, RS485_DE_RE);
  mb.slave(SLAVE_ID);

  // Load last state
  prefs.begin("tcl-ac", false);
  if (prefs.getBytesLength("state") == sizeof(lastSettings)) {
    prefs.getBytes("state", &lastSettings, sizeof(lastSettings));
    Serial.println("Loaded last state from flash");
  } else {
    lastSettings = { 2, 3, 200, 1, 1, 1 };  // ON, Heat, 20.0°C, Auto
    Serial.println("No saved state → defaults");
  }

  // Init registers
  mb.addHreg(REG_POWER, lastSettings.power);
  mb.addHreg(REG_MODE, lastSettings.mode);
  mb.addHreg(REG_TEMP_SET, lastSettings.temp);
  mb.addHreg(REG_FAN, lastSettings.fan);
  mb.addHreg(REG_H_SWING, lastSettings.swingH);
  mb.addHreg(REG_V_SWING, lastSettings.swingV);
  mb.addHreg(REG_COUNT, 0);
  mb.addHreg(REG_LIGHT, 0);
  mb.addHreg(REG_A2, 0);
  mb.addHreg(REG_A1, 0);
  mb.addHreg(REG_A0, 0);
  mb.addHreg(REG_COUNT2, 0);

  ac.begin();

  Serial.println("TCL AC Modbus Slave");
}

void loop() {
  mb.task();

  // Update analog inputs every 30s
  static uint32_t lastAnalog = 0;
  if (millis() - lastAnalog >= 30000) {
    lastAnalog = millis();
    mb.Hreg(REG_LIGHT, analogRead(LDR));
    mb.Hreg(REG_A2, analogRead(A2));
    mb.Hreg(REG_A1, analogRead(A1));
    mb.Hreg(REG_A0, analogRead(A0));

    Serial.printf("LDR:%d A2:%d A1:%d A0:%d\n",
                  analogRead(LDR), analogRead(A2), analogRead(A1), analogRead(A0));
  }

  // === Normal parameter update (skip if all zero) ===
  bool changed = false;

  uint16_t p = mb.Hreg(REG_POWER);
  uint16_t m = mb.Hreg(REG_MODE);
  uint16_t t = mb.Hreg(REG_TEMP_SET);
  uint16_t f = mb.Hreg(REG_FAN);
  uint16_t sh = mb.Hreg(REG_H_SWING);
  uint16_t sv = mb.Hreg(REG_V_SWING);
  uint16_t count = mb.Hreg(REG_COUNT);


  if (p != lastSettings.power) changed = true;
  if (m != lastSettings.mode) changed = true;
  if (t != lastSettings.temp) changed = true;
  if (f != lastSettings.fan) changed = true;
  if (sh != lastSettings.swingH) changed = true;
  if (sv != lastSettings.swingV) changed = true;

  // Block all-zero write
  if (p == 0 && m == 0 && t == 0 && f == 0 && sh == 0 && sv == 0) {
    if (changed) {
      Serial.println("All control registers zero → ignored");
      changed = false;
    }
  }

  if (t < 160 || t > 300) {
    Serial.println("Invalid temperature → ignored");
    changed = false;  // ← Block the entire update
  }

  if (changed) {
    lastSettings.power = p;
    lastSettings.mode = m;
    lastSettings.temp = t;
    lastSettings.fan = f;
    lastSettings.swingH = sh;
    lastSettings.swingV = sv;

    mb.Hreg(REG_COUNT2, count);

    prefs.putBytes("state", &lastSettings, sizeof(lastSettings));
    applySettingsToAC();

    Serial.println("Valid change → saved + sent to AC");
  }
}

void applySettingsToAC() {
  Serial.println("Applying settings to AC");

  switch (lastSettings.power) {
    case 1: ac.off(); break;
    case 2: ac.on(); break;
    default: ac.on(); break;
  }

  switch (lastSettings.mode) {
    case 1: ac.setMode(kTcl112AcAuto); break;
    case 2: ac.setMode(kTcl112AcCool); break;
    case 3: ac.setMode(kTcl112AcHeat); break;
    case 4: ac.setMode(kTcl112AcDry); break;
    case 5: ac.setMode(kTcl112AcFan); break;
    default: ac.setMode(kTcl112AcCool); break;
  }

  float temp = lastSettings.temp / 10.0;
  ac.setTemp(constrain(temp, 16.0, 30.0));

  switch (lastSettings.fan) {
    case 1: ac.setFan(kTcl112AcFanAuto); break;
    case 2: ac.setFan(kTcl112AcFanLow); break;
    case 3: ac.setFan(kTcl112AcFanMed); break;
    case 4: ac.setFan(kTcl112AcFanHigh); break;
    default: ac.setFan(kTcl112AcFanAuto); break;
  }

  switch (lastSettings.swingH) {
    case 1: ac.setSwingHorizontal(0); break;
    case 2: ac.setSwingHorizontal(1); break;
    default: ac.setSwingHorizontal(0); break;
  }
  switch (lastSettings.swingV) {
    case 1: ac.setSwingVertical(0); break;
    case 2: ac.setSwingVertical(1); break;
    default: ac.setSwingVertical(0); break;
  }

  ac.send();
  delay(50);
  ac.send();

  Serial.printf("→(REG) Power:%d Mode:%d Temp:%d Fan:%d H-Swing:%d V-Swing:%d counter:%d\n",
                mb.Hreg(REG_POWER),
                mb.Hreg(REG_MODE),
                mb.Hreg(REG_TEMP_SET),
                mb.Hreg(REG_FAN),
                mb.Hreg(REG_H_SWING),
                mb.Hreg(REG_V_SWING),
                mb.Hreg(REG_COUNT));

  Serial.printf("→(INN) Power:%d Mode:%d Temp:%d Fan:%d H-Swing:%d V-Swing:%d  counter:%d\n",
                lastSettings.power,
                lastSettings.mode,
                lastSettings.temp,
                lastSettings.fan,
                lastSettings.swingH,
                lastSettings.swingV,
                mb.Hreg(REG_COUNT2));
}