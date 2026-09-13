/*
  Reads Node A's button without blocking and updates its condition LEDs.
  Dependencies: Arduino core through Panel_a.h.
*/
#include "Panel_a.h"

Panel::Panel()
    : rawButtonState_(HIGH),
      stableButtonState_(HIGH),
      buttonChangedAt_(0) {
}

void Panel::begin() {
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Read the real startup level so booting cannot create a false press.
  stableButtonState_ = digitalRead(BUTTON_PIN);
  rawButtonState_ = stableButtonState_;
  buttonChangedAt_ = millis();
}

bool Panel::pollScenarioButton() {
  const int reading = digitalRead(BUTTON_PIN);
  // A raw change restarts the interval required for a stable reading.
  if (reading != rawButtonState_) {
    rawButtonState_ = reading;
    buttonChangedAt_ = millis();
  }

  // Accept one event only after the new level remains stable for 30 ms.
  if (millis() - buttonChangedAt_ >= DEBOUNCE_MS &&
      rawButtonState_ != stableButtonState_) {
    stableButtonState_ = rawButtonState_;
    return stableButtonState_ == LOW;
  }
  return false;
}

void Panel::show(
    const FacilityScenario& scenario, bool temperatureWarning) {
  const bool blocked = !scenario.capacityAvailable ||
                       !scenario.powerAvailable ||
                       scenario.tempC >= scenario.tempLimitC;
  const bool caution = !blocked && temperatureWarning;

  digitalWrite(GREEN_LED_PIN, !blocked && !caution ? HIGH : LOW);
  digitalWrite(YELLOW_LED_PIN, caution ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, blocked ? HIGH : LOW);
}
