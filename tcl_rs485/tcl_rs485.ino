#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Tcl.h>        // TCL112AC support

// ---------- IR Setup ----------
const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);
IRTcl112Ac ac(kIrLed);

// ---------- ModbusRTU Setup ----------
#include <ModbusRTU.h>
ModbusRTU mb;

#define SLAVE_ID  32
#define MB_BAUD   9600

// UART1 pins for RS485 module (auto-direction, no DE/RE needed)
#define RX_PIN    5
#define TX_PIN    6

uint16_t mb_regs[20] = {0};  // Internal register array (we'll use 0–5)

void setup() {
  Serial.begin(115200);  // Debug serial

  // Start UART1 for Modbus (RX=16, TX=17)
  Serial1.begin(MB_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  // Modbus setup (auto-direction RS485 — no third param)
  mb.begin(&Serial1);    // ← Auto-direction module
  mb.slave(SLAVE_ID);    // Set slave ID to 1

  // Add holding registers (address 0 = 40001 in PLC)
  // mb.addHreg(offset, initial_value, num_regs=1)
  mb.addHreg(0, 0);      // 40001: Power (0=OFF, 1=ON)
  mb.addHreg(1, 1);      // 40002: Mode (0=Auto,1=Cool,2=Heat,3=Dry,4=Fan)
  mb.addHreg(2, 24);     // 40003: Temp (16–30 °C)
  mb.addHreg(3, 0);      // 40004: Fan (0=Auto,1=Low,2=Med,3=High)
  mb.addHreg(4, 0);      // 40005: Horizontal swing (0=OFF,1=ON)
  mb.addHreg(5, 0);      // 40006: Vertical swing (0=OFF,1=ON)

  // IR setup
  irsend.begin();
  ac.begin();

  Serial.println("TCL AC Modbus RTU Slave – Ready!");
  Serial.println("Slave ID: 1 | Baud: 19200 8N1 | RS485 Auto-Direction");
  Serial.println("PLC Registers: 40001=Power, 40002=Mode, 40003=Temp, etc.");
}

void loop() {
  mb.task();             // Handle Modbus requests
  delay(5);              // Small delay for stability

  // Check for changes in registers 0–5
  static uint16_t last[6] = {99,99,99,99,99,99};
  bool changed = false;
  for (int i = 0; i < 6; i++) {
    if (mb.Hreg(i) != last[i]) {
      changed = true;
      last[i] = mb.Hreg(i);
    }
  }

  if (changed) {
    Serial.println("Modbus change detected → Sending to AC");

    // Power
    mb.Hreg(0) == 1 ? ac.on() : ac.off();

    // Mode
    switch (mb.Hreg(1)) {
      case 0: ac.setMode(kTcl112AcAuto); break;
      case 1: ac.setMode(kTcl112AcCool); break;
      case 2: ac.setMode(kTcl112AcHeat); break;
      case 3: ac.setMode(kTcl112AcDry);  break;
      case 4: ac.setMode(kTcl112AcFan);  break;
      default: ac.setMode(kTcl112AcCool); break;
    }

    // Temperature (clamp 16–30)
    uint8_t temp = constrain(mb.Hreg(2), 16, 30);
    ac.setTemp(temp);

    // Fan
    switch (mb.Hreg(3)) {
      case 0: ac.setFan(kTcl112AcFanAuto); break;
      case 1: ac.setFan(kTcl112AcFanLow);  break;
      case 2: ac.setFan(kTcl112AcFanMed);  break;
      case 3: ac.setFan(kTcl112AcFanHigh); break;
      default: ac.setFan(kTcl112AcFanAuto); break;
    }

    // Swing
    ac.setSwingHorizontal(mb.Hreg(4) == 1);
    ac.setSwingVertical(mb.Hreg(5) == 1);

    // Send 3 repeats (reliable for TCL)
    ac.send(); delay(80);
    ac.send(); delay(80);
    ac.send();

    Serial.printf("→ Power:%d Mode:%d Temp:%d Fan:%d H-Swing:%d V-Swing:%d\n",
                  mb.Hreg(0), mb.Hreg(1), temp, mb.Hreg(3), mb.Hreg(4), mb.Hreg(5));
  }
}