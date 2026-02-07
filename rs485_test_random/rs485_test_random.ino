#include <ModbusMaster.h>
#include <SoftwareSerial.h>

// RS485 on SoftwareSerial (auto-direction module — no DE/RE needed)
SoftwareSerial rs485(3, 4);  // RX=10, TX=11

#define SLAVE_ID 32

ModbusMaster node;

// Simulation mode control
bool simulationMode = false;
unsigned long lastSimulationTime = 0;
unsigned long simulationInterval = 10000;  // 30 seconds between simulations

// Random operation parameters
enum OperationType {
  OP_POWER = 0,
  OP_MODE,
  OP_TEMP,
  OP_FAN,
  OP_HSWING,
  OP_VSWING,
  OP_READ,
  OP_CLIMATE,
  OP_COUNT
};

void setup() {
  Serial.begin(115200);  // Debug console
  rs485.begin(9600);     // Modbus baudrate (match ESP32 slave)

  node.begin(SLAVE_ID, rs485);  // Start master (auto-direction)

  // Seed random number generator
  randomSeed(analogRead(A0));

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
  Serial.println(F("  climate → Read temperature"));
  Serial.println(F("  simon   → Start random simulation"));
  Serial.println(F("  simoff  → Stop random simulation"));
  Serial.println(F("  interval N → Set simulation interval in ms"));
}

void loop() {
  // Handle manual commands from Serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();
    
    handleManualCommand(cmd);
  }

  // Handle automatic simulation if enabled
  if (simulationMode) {
    unsigned long currentTime = millis();
    if (currentTime - lastSimulationTime >= simulationInterval) {
      performRandomOperation();
      lastSimulationTime = currentTime;
      
      // Randomly change interval between 2-10 seconds
      if (random(0, 10) == 0) {  // 10% chance
        simulationInterval = random(2000, 10001);
        Serial.print(" → Interval changed to: ");
        Serial.print(simulationInterval);
        Serial.println(" ms");
      }
    }
  }

  delay(100);  // Small delay for stability
}

void handleManualCommand(String cmd) {
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
    int t = (cmd.substring(5)).toInt()*10;
    if (t >= 160 && t <= 300) {
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
    toggleSwing(4, "H-Swing");
  } else if (cmd == "vswing") {
    toggleSwing(5, "V-Swing");
  } else if (cmd == "read") {
    readMultipleRegisters(0, 10);
  } else if (cmd == "climate") {
    readTemperature();
  } else if (cmd == "simon") {
    simulationMode = true;
    lastSimulationTime = millis();
    Serial.println(" → Simulation mode STARTED");
    Serial.print(" → Interval: ");
    Serial.print(simulationInterval);
    Serial.println(" ms");
  } else if (cmd == "simoff") {
    simulationMode = false;
    Serial.println(" → Simulation mode STOPPED");
  } else if (cmd.startsWith("interval ")) {
    long newInterval = cmd.substring(9).toInt();
    if (newInterval >= 500 && newInterval <= 60000) {
      simulationInterval = newInterval;
      Serial.print(" → Simulation interval set to: ");
      Serial.print(simulationInterval);
      Serial.println(" ms");
    } else {
      Serial.println(" → Invalid interval (500-60000 ms)");
    }
  } else {
    Serial.println("Unknown command");
  }
}

void performRandomOperation() {
  OperationType op = static_cast<OperationType>(random(0, OP_COUNT));
  uint8_t result;
  
  Serial.print("SIM ");
  
  switch(op) {
    case OP_POWER: {
      uint16_t powerState = random(0, 2);  // 0 or 1
      result = node.writeSingleRegister(0, powerState);
      Serial.print("Power ");
      Serial.print(powerState == 1 ? "ON" : "OFF");
      printResult(result);
      break;
    }
    
    case OP_MODE: {
      uint16_t mode = random(0, 5);  // 0-4
      result = node.writeSingleRegister(1, mode);
      Serial.print("Mode ");
      switch(mode) {
        case 0: Serial.print("Auto"); break;
        case 1: Serial.print("Cool"); break;
        case 2: Serial.print("Heat"); break;
        case 3: Serial.print("Dry"); break;
        case 4: Serial.print("Fan"); break;
      }
      printResult(result);
      break;
    }
    
    case OP_TEMP: {
      uint16_t temp = random(160, 300);  // 16-30
      result = node.writeSingleRegister(2, temp * 1);
      Serial.print("Temp ");
      Serial.print(temp);
      Serial.print("°C");
      printResult(result);
      break;
    }
    
    case OP_FAN: {
      uint16_t fanSpeed = random(0, 4);  // 0-3
      result = node.writeSingleRegister(3, fanSpeed);
      Serial.print("Fan speed ");
      Serial.print(fanSpeed);
      printResult(result);
      break;
    }
    
    case OP_HSWING: {
      toggleSwing(4, "H-Swing");
      break;
    }
    
    case OP_VSWING: {
      toggleSwing(5, "V-Swing");
      break;
    }
    
    case OP_READ: {
      // Random number of registers (3-10)
      uint16_t startReg = random(0, 5);
      uint16_t numRegs = random(3, 7);
      readMultipleRegisters(startReg, numRegs);
      break;
    }
    
    case OP_CLIMATE: {
      readTemperature();
      break;
    }
    
    default:
      break;
  }
  
  // Random delay between operations (0-500ms)
  delay(random(1000, 20000));
}

void toggleSwing(uint16_t registerAddr, const char* swingName) {
  uint8_t result = node.readHoldingRegisters(registerAddr, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t val = node.getResponseBuffer(0);
    uint16_t newVal = (val == 0) ? 1 : 0;
    result = node.writeSingleRegister(registerAddr, newVal);
    Serial.print(swingName);
    Serial.print(": ");
    Serial.print(newVal);
    printResult(result);
  } else {
    Serial.print(" → ");
    Serial.print(swingName);
    Serial.println(" read error");
  }
}

void readMultipleRegisters(uint16_t start, uint16_t count) {
  uint8_t result = node.readHoldingRegisters(start, count);
  if (result == node.ku8MBSuccess) {
    Serial.print("Read ");
    Serial.print(count);
    Serial.print(" registers from 4000");
    Serial.print(start + 1);
    Serial.print(": ");
    for (int i = 0; i < count; i++) {
      uint16_t val = node.getResponseBuffer(i);
      Serial.print(val);
      if (i < count - 1) Serial.print(", ");
    }
    Serial.println();
  } else {
    printResult(result);
  }
}

void readTemperature() {
  uint8_t result = node.readHoldingRegisters(6, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t val = node.getResponseBuffer(0);
    Serial.print("Light: ");
    Serial.print(val);
    
    Serial.println();
  } else {
    printResult(result);
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