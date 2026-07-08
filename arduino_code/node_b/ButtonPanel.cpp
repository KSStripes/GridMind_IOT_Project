/*
 * GridMind Node B - Physical Button Adapter Implementation
 *
 * Uses non-blocking millis() debounce. A held button produces one action,
 * and its release updates state without producing another action.
 */

#include "ButtonPanel.h"

namespace gridmind {

ButtonPanel::ButtonPanel()
    // Map each verified physical pin to exactly one domain action.
    : buttons_{
        {D5, Action::RUN, HIGH, HIGH, 0},
        {D6, Action::DEFER, HIGH, HIGH, 0},
        {D7, Action::REDUCE, HIGH, HIGH, 0}} {
}

void ButtonPanel::begin() {
  // Initialise debounce state from the actual electrical level at startup.
  for (size_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons_[i].pin, INPUT_PULLUP);
    buttons_[i].stableState = digitalRead(buttons_[i].pin);
    buttons_[i].lastRawState = buttons_[i].stableState;
    buttons_[i].lastRawChangeTime = millis();
  }
}

bool ButtonPanel::poll(Action& output) {
  // One timestamp serves all buttons during this non-blocking loop pass.
  const unsigned long now = millis();

  for (size_t i = 0; i < BUTTON_COUNT; i++) {
    ButtonState& button = buttons_[i];
    const int rawState = digitalRead(button.pin);

    // Restart this button's debounce interval after an electrical change.
    if (rawState != button.lastRawState) {
      button.lastRawState = rawState;
      button.lastRawChangeTime = now;
    }

    // Accept a state change only after it remains stable for 30 ms.
    if ((now - button.lastRawChangeTime >= DEBOUNCE_MS) &&
        (rawState != button.stableState)) {
      button.stableState = rawState;

      // INPUT_PULLUP is active-low: only LOW is a press event.
      if (button.stableState == LOW) {
        output = button.action;
        return true;
      }
    }
  }

  // No new debounced press occurred during this poll.
  return false;
}

}  // namespace gridmind
