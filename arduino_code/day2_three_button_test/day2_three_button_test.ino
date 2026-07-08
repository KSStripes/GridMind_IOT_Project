/* 
 *  GridMind - Node B - Three Button Test
 *  
 *  Purpose:
 *  Verify the Run, Defer and Reduce decision buttons independently using
 *  non-blocking debounce logic suitable for later networking integration.
 *  
 *  Pins: 
 *  D5 = Run (yellow)
 *  D6 = Defer (orange)
 *  D7 = Reduce (green)
 *  G = shared ground rail (black)
 *  
 *  All inputs use INPUT_PULLUP:
 *  Released = HIGH
 *  Pressed = LOW
 */

const unsigned long DEBOUNCE_TIME_MS = 30;

struct ButtonState {
  uint8_t pin;
  const char* label;
  int lastRawState;
  int stableState;
  unsigned long lastRawChangeTime;
};

ButtonState buttons[] = {
  {D5, "RUN", HIGH, HIGH, 0},
  {D6, "DEFER", HIGH, HIGH, 0},
  {D7, "REDUCE", HIGH, HIGH, 0}
};

const size_t BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// Report one accepted transition for the specified decision button.
void printButtonState(const ButtonState& button) {
  Serial.print(button.label);
  Serial.print(": ");
  Serial.println(button.stableState == LOW ? "PRESSED" : "RELEASED");
}

void setup() {
  Serial.begin(115200); // baud set
  Serial.println();
  Serial.println("GridMind button test started.");

  // Initialize each debounce state from its physical input.
  for (size_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    buttons[i].stableState = digitalRead(buttons[i].pin);
    buttons[i].lastRawState = buttons[i].stableState;
    printButtonState(buttons[i]);
  }
}

void loop() {
  const unsigned long now = millis();

  for (size_t i = 0; i < BUTTON_COUNT; i++) {
    ButtonState& button = buttons[i];
    const int rawState = digitalRead(button.pin);

    // Restart this button's stability interval after a raw transition.
    if (rawState != button.lastRawState) {
      button.lastRawState = rawState;
      button.lastRawChangeTime = now;
    }

    // Accept the transition after the input remains stable for 30 ms.
    if ((now - button.lastRawChangeTime >= DEBOUNCE_TIME_MS) &&
          (rawState != button.stableState)) {
        button.stableState = rawState;
        printButtonState(button);
    }
  }
}
