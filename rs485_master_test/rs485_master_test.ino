#include <ModbusMaster.h>
#include <SoftwareSerial.h>  // For UNO/Nano (uses pins 10 RX, 11 TX)

// RS485 on SoftwareSerial (auto-direction module — no DE/RE needed)
SoftwareSerial rs485(11, 10);  // RX=10, TX=11

#define SLAVE_ID 32

ModbusMaster node;

void setup() {
  Serial.begin(115200);  // Debug console
  rs485.begin(9600);     // Modbus baudrate (match ESP32 slave)

  node.begin(SLAVE_ID, rs485);  // Start master (auto-direction)

  Serial.println(F("TCL AC Modbus Master Ready (auto RS485)"));
  Serial.println(F("Commands:"));
  Serial.println(F("  on      → Power ON"));
  Serial.println(F("  off     → Power OFF"));
  Serial.println(F("  cool    → Cool mode"));
  Serial.println(F("  heat    → Heat mode"));
  Serial.println(F("  dry     → Dry mode"));
  Serial.println(F("  fanmode → Fan only"));
  Serial.println(F("  auto    → Auto mode"));
  Serial.println(F("  temp 24 → Set 24°C"));
  Serial.println(F("  fan 2   → Medium fan"));
  Serial.println(F("  hswing  → Toggle horizontal swing"));
  Serial.println(F("  vswing  → Toggle vertical swing"));
  Serial.println(F("  read    → Read all registers"));
  Serial.println(F("  climate    → Read temprature"));
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    uint8_t result;

    if (cmd == "on") {
      result = node.writeSingleRegister(0, 1);  // 40001 = Power ON
      printResult(result);
    } else if (cmd == "off") {
      result = node.writeSingleRegister(0, 0);  // 40001 = Power OFF
      printResult(result);
    } else if (cmd == "cool") {
      result = node.writeSingleRegister(1, 1);  // 40002 = Cool
      printResult(result);
    } else if (cmd == "heat") {
      result = node.writeSingleRegister(1, 2);  // Heat
      printResult(result);
    } else if (cmd == "dry") {
      result = node.writeSingleRegister(1, 3);  // Dry
      printResult(result);
    } else if (cmd == "fanmode") {
      result = node.writeSingleRegister(1, 4);  // Fan only
      printResult(result);
    } else if (cmd == "auto") {
      result = node.writeSingleRegister(1, 0);  // Auto
      printResult(result);
    } else if (cmd.startsWith("temp ")) {
      int t = cmd.substring(5).toInt();
      if (t >= 16 && t <= 30) {
        result = node.writeSingleRegister(2, t);  // 40003 = Temp
        printResult(result);
        Serial.print(" → Temp set to ");
        Serial.println(t);
      }
    } else if (cmd.startsWith("fan ")) {
      int f = cmd.substring(4).toInt();
      if (f >= 0 && f <= 3) {
        result = node.writeSingleRegister(3, f);  // 40004 = Fan
        printResult(result);
        Serial.print(" → Fan speed ");
        Serial.println(f);
      }
    } else if (cmd == "hswing") {
      // Read current, then toggle
      result = node.readHoldingRegisters(4, 1);  // 40005
      if (result == node.ku8MBSuccess) {
        uint16_t val = node.getResponseBuffer(0);
        uint16_t newVal = (val == 0) ? 1 : 0;
        result = node.writeSingleRegister(4, newVal);
        printResult(result);
        Serial.print(" → H-Swing: ");
        Serial.println(newVal);
      } else {
        Serial.println(" → Read error");
      }
    } else if (cmd == "vswing") {
      result = node.readHoldingRegisters(5, 1);  // 40006
      if (result == node.ku8MBSuccess) {
        uint16_t val = node.getResponseBuffer(0);
        uint16_t newVal = (val == 0) ? 1 : 0;
        result = node.writeSingleRegister(5, newVal);
        printResult(result);
        Serial.print(" → V-Swing: ");
        Serial.println(newVal);
      } else {
        Serial.println(" → Read error");
      }
    } else if (cmd == "read") {
      result = node.readHoldingRegisters(0, 10);  // Read all 40001–40006
      if (result == node.ku8MBSuccess) {
        Serial.print(" → Registers: ");
        for (int i = 0; i < 10; i++) {
          uint16_t val = node.getResponseBuffer(i);
          Serial.print("4000");
          Serial.print(1 + i);
          Serial.print("=");
          Serial.print(val);
          Serial.print(" ");
        }
        Serial.println();
      } else {
        printResult(result);
      }
    } else if (cmd == "climate") {
      result = node.readHoldingRegisters(9, 1);  // Read temperature from register 40010
      if (result == node.ku8MBSuccess) {
        uint16_t val = node.getResponseBuffer(0);  // Use index 0, not i
        Serial.print(" → Temperature: ");
        Serial.print(val);
        Serial.println();
      } else {
        printResult(result);  // Show error if any
      }
    } else {
      Serial.println("Unknown command");
    }

    delay(200);  // Modbus stability delay
  }
}

// Helper function to print Modbus result
void printResult(uint8_t result) {
  if (result == node.ku8MBSuccess) {
    Serial.println(" → OK");
  } else {
    Serial.print(" → Modbus error: ");
    Serial.println(result);
  }
}