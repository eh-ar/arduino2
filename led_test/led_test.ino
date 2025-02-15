// define led according to pin diagram
int led = 8;
void setup() {
  // initialize digital pin led as an output
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}
void loop() {
  Serial.println("LED On");
  digitalWrite(led, HIGH);  // turn the LED on
  delay(1000);              // wait for a second
  Serial.println("LED Off");
  digitalWrite(led, LOW);   // turn the LED off
  delay(1000);              // wait for a second
}