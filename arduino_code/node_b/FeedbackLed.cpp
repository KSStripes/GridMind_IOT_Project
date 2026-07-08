/*
 * GridMind Node B - Ordinary Feedback LED Implementation
 *
 * Positive score: steady on.
 * Zero score: two slow flashes.
 * Negative score: three fast flashes.
 */

#include "FeedbackLed.h"

namespace gridmind {

FeedbackLed::FeedbackLed()
    : mode_(Mode::OFF),
      ledOn_(false),
      togglesRemaining_(0),
      intervalMs_(0),
      lastChangeTime_(0) {
}

void FeedbackLed::begin() {
  // Starting off prevents a stale or misleading outcome indication.
  pinMode(LED_PIN, OUTPUT);
  setLed(false);
  mode_ = Mode::OFF;
}

void FeedbackLed::setLed(bool on) {
  ledOn_ = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

void FeedbackLed::startBlink(
    uint8_t flashes,
    unsigned long intervalMs) {
  mode_ = Mode::BLINKING;
  intervalMs_ = intervalMs;
  // The first ON state is immediate, so one fewer later toggle is required.
  togglesRemaining_ = flashes * 2 - 1;
  lastChangeTime_ = millis();
  setLed(true);
}

void FeedbackLed::show(const DecisionResult& result) {
  // Rejected decisions do not replace the feedback for an accepted result.
  if (!result.accepted) {
    return;
  }

  if (result.scoreDelta > 0) {
    // Positive outcome remains visible until reset or another accepted result.
    mode_ = Mode::STEADY;
    togglesRemaining_ = 0;
    setLed(true);
  } else if (result.scoreDelta == 0) {
    // Neutral or warning outcome: two slower flashes.
    startBlink(2, 300);
  } else {
    // Penalty outcome: three faster flashes.
    startBlink(3, 100);
  }
}

void FeedbackLed::update() {
  if (mode_ != Mode::BLINKING) {
    return;
  }

  // millis() timing keeps the main loop responsive while the LED blinks.
  const unsigned long now = millis();
  if (now - lastChangeTime_ < intervalMs_) {
    return;
  }

  lastChangeTime_ = now;
  setLed(!ledOn_);
  togglesRemaining_ -= 1;

  if (togglesRemaining_ == 0) {
    // Every finite blink pattern ends in a known OFF state.
    setLed(false);
    mode_ = Mode::OFF;
  }
}

}  // namespace gridmind
