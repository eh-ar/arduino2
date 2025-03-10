
void pinInit() {
  pinMode(batteryPin, INPUT);
  pinMode(solarPin, INPUT);
  pinMode(sensorPin1, OUTPUT);
  pinMode(sensorPin2, OUTPUT);
  pinMode(sensorPin3, OUTPUT);

  digitalWrite(sensorPin1, LOW);
  digitalWrite(sensorPin2, LOW);
  digitalWrite(sensorPin3, LOW);
}

void turnSensor(int sensorPin, String stat) {
  if (stat == "on") {
    digitalWrite(sensorPin, HIGH);
    delay(5000);  // Wait for sensor to stabilize (example 2 seconds)
  } else if (stat == "off") {
    digitalWrite(sensorPin, LOW);
    delay(50);
  }
}

void readRS485(uint8_t deviceAddress, uint8_t st, uint8_t n) {
  mySerial.begin(4800);
  Serial.print("rs485 " + String(deviceAddress) + ", ");
  delay(10);
  Serial.print(", reading ");

  node.begin(deviceAddress, mySerial);  // Set the Modbus address and use the SoftwareSerial connection
  //Serial.println("Check RS485");
  int result = node.readHoldingRegisters(st, n);
  Serial.print(" parsing ");
  if (result == node.ku8MBSuccess) {
    int val = node.getResponseBuffer(0);
    sensorData[0] = deviceAddress;
    for (int j = 1; j < n; j++) {
      int val = node.getResponseBuffer(j);
      //Serial.println(val);
      sensorData[j] = val;
    }
  }
  mySerial.flush();
}