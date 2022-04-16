int forcePin = D2;
int forceReading;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Serial Monitor for FSR readings
  pinMode(5, OUTPUT); // LED
  pinMode(14, OUTPUT); // Fan
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Start with LED and fan turned off
  digitalWrite(5, LOW);
  digitalWrite(14, LOW);

  // Reading value from FSR
  forceReading = analogRead(forcePin);
  
  // Logging value from FSR in Serial Monitor
  Serial.print("Analog reading = ");
  Serial.println(forceReading);

  if (forceReading)
  {
    digitalWrite(5, HIGH); // Turn LED on
    digitalWrite(14, HIGH); // Turn Fan on
    delay(100);
  }
  else
  {
    digitalWrite(5, LOW); // Turn LED off
    digitalWrite(14, LOW); // Turn Fan off
  }

  delay(100);
}
