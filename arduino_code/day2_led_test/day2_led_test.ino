const uint8_t LED_PIN = D5;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
