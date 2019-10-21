int solenoidPin = 7;
int counter = 1;

void setup() {
	pinMode(solenoidPin, OUTPUT);
}

void loop() {
	fireSolenoid(150); //delay in ms
	Serial.println("The solenoid has fired: ");
	Serial.print(counter);
	Serial.print(" time(s).");
	counter++;
}

void fireSolenoid(int ms) {
	digitalWrite(solenoidPin, HIGH);
	delay(ms);
	//digitalWrite(solenoidPin, LOW);
	delay(ms);
}