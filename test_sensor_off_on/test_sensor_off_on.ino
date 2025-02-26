
int vin = A0;
int sensorV = A1;
float vin_m;
float vin_measure;

int delaySensor = 5;

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  Serial.println("Setup");

  
  pinMode(vin, INPUT);
  pinMode(sensorV, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

      Serial.println("Turn on Sensor");
      digitalWrite(sensorV, HIGH);
      delay(1 * 1000);

      Serial.println(", Turn off Sensor");
      digitalWrite(sensorV, LOW);
      delay(6 * 1000);
}
