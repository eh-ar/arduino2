



int vin = A0;
int sensorV = A1;
float vin_m;
float vin_measure;
int delaySensor = 5;  //sec


void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(vin, INPUT);
  pinMode(sensorV, OUTPUT);
  delay(100);

  Serial.flush();
  delay(100);

  //node.begin(50, mySerial);
  // Set up the watchdog timer to wake up every 8 seconds
}

void loop() {

  Serial.begin(115200);
  Serial.println("Reading Cycle");
  delay(1000);



  //Serial.println(" Vin");
  //turnOnADC();
  delay(100);
  float vin_m = analogRead(vin);
  vin_measure = vin_m * 0.00978;  //* (8/4);
  delay(100);

  delay(100);
  Serial.println("Turn on Sensor");
  digitalWrite(sensorV, HIGH);
  delay(delaySensor * 1000);


  Serial.println(", Turn off Sensor");
  digitalWrite(sensorV, LOW);

  delay(delaySensor * 1000);

  //delay(loopDelay * 1000);
}

