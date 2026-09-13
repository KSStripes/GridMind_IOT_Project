// Panel_b.cpp
// Reads active-low INPUT_PULLUP buttons and drives timed LED feedback.
// No delay() calls are used so the web server loop stays responsive.
#include "Panel_b.h"

Panel::Panel() {
  const uint8_t pins[BUTTON_COUNT] = {D5, D6, D7};
  const Action actions[BUTTON_COUNT] = {ACT_RUN, ACT_WAIT, ACT_CANCEL};
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    buttons_[i].pin = pins[i];
    buttons_[i].action = actions[i];
    buttons_[i].rawState = HIGH;
    buttons_[i].stableState = HIGH;
    buttons_[i].changedAt = 0;
  }

  ledMode_ = LED_OFF;
  ledOn_ = false;
  togglesLeft_ = 0;
  blinkMs_ = 0;
  changedAt_ = 0;
}

void Panel::begin() {
  // Record each button's real starting state to prevent a false first press.
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons_[i].pin, INPUT_PULLUP);
    buttons_[i].stableState = digitalRead(buttons_[i].pin);
    buttons_[i].rawState = buttons_[i].stableState;
    buttons_[i].changedAt = millis();
  }

  pinMode(LED_PIN, OUTPUT);
  setLed(false);
}

bool Panel::poll(Action& action) {
  const unsigned long now = millis();

  // Check every button and report at most one newly stable press per loop.
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    Button& button = buttons_[i];
    const int raw = digitalRead(button.pin);

    // Restart the debounce timer whenever the electrical reading changes.
    if (raw != button.rawState) {
      button.rawState = raw;
      button.changedAt = now;
    }

    if (now - button.changedAt >= DEBOUNCE_MS &&
        raw != button.stableState) {
      button.stableState = raw;
      if (raw == LOW) {
        action = button.action;
        return true;
      }
    }
  }
  return false;
}

void Panel::show(const Result& result) {
  // Rejected/negative: fast flashes; zero-money accept: slow flashes; positive: steady.
  if (!result.accepted || result.deltaCents < 0) {
    blink(3, 100);
  } else if (result.deltaCents > 0) {
    ledMode_ = LED_STEADY;
    togglesLeft_ = 0;
    setLed(true);
  } else {
    blink(2, 300);
  }
}

void Panel::update() {
  // Advance an active blink only when its interval has elapsed.
  if (ledMode_ != LED_BLINKING) {
    return;
  }

  const unsigned long now = millis();
  if (now - changedAt_ < blinkMs_) {
    return;
  }

  changedAt_ = now;
  setLed(!ledOn_);
  togglesLeft_ -= 1;
  if (togglesLeft_ == 0) {
    setLed(false);
    ledMode_ = LED_OFF;
  }
}

void Panel::setLed(bool on) {
  ledOn_ = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

void Panel::blink(uint8_t flashes, unsigned long intervalMs) {
  // Starting ON means an N-flash pattern needs (N * 2 - 1) later toggles.
  ledMode_ = LED_BLINKING;
  togglesLeft_ = flashes * 2 - 1;
  blinkMs_ = intervalMs;
  changedAt_ = millis();
  setLed(true);
}
