/*
  GridMind - Debounced Pushbutton Test
  Date: 7 July 2026
  Board: NodeMCU V3 ESP8266
  Arduino board setting: NodeMCU 1.0 (ESP-12E Module)

  Purpose:
  Verify that the NodeMCU detects one physical pushbutton reliably
  while filtering the rapid electrical changes caused by contact bounce.

  Circuit:
  D6 -> pushbutton -> GND

  Verified breadboard wiring:
  D6 -> C25/D25
  GND -> C27/D27

  INPUT_PULLUP behaviour:
  Released = HIGH
  Pressed  = LOW

  Expected result:
  Serial Monitor reports one PRESSED event when the button is pushed
  and one RELEASED event when it is released. Holding the button must
  not generate repeated events.

  Debounce method:
  A raw input change is accepted only after it remains stable for
  30 milliseconds. millis() is used instead of delay() so future
  networking and dashboard work will not be paused.
*/

const uint8_t BUTTON_PIN = D6;                  // NodeMCU D6 (ESP8266 GPIO12)
const unsigned long DEBOUNCE_TIME_MS = 30;     // Required stable interval

int lastRawState = HIGH;                       // Most recent GPIO sample
int stableState = HIGH;                        // Last accepted debounced state
unsigned long lastRawChangeTime = 0;           // millis() at last raw transition

// Report an accepted state transition to Serial Monitor.
void printButtonState(int state) {
  if (state == LOW) {
    Serial.println("Button: PRESSED");
  } else {
    Serial.println("Button: RELEASED");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Released = HIGH; pressed = LOW

  // Initialise from the physical input to avoid a false startup transition.
  stableState = digitalRead(BUTTON_PIN);
  lastRawState = stableState;

  Serial.println();
  Serial.println("GridMind debounced button test started");
  printButtonState(stableState);
}

void loop() {
  int rawState = digitalRead(BUTTON_PIN);

  // Restart the stability interval after every raw transition.
  if (rawState != lastRawState) {
    lastRawChangeTime = millis();
    lastRawState = rawState;
  }

  // Accept a transition only after the raw input has remained stable.
  // Unsigned subtraction keeps the elapsed-time test safe across rollover.
  if ((millis() - lastRawChangeTime >= DEBOUNCE_TIME_MS) &&
      (rawState != stableState)) {
    stableState = rawState;
    printButtonState(stableState);
  }
}
