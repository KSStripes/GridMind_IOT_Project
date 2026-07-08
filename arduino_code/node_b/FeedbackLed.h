/*
 * GridMind Node B - Ordinary Feedback LED Adapter
 *
 * Converts a DecisionResult into one of three visible outcomes on D0 without
 * using delay(), so buttons and future network services can keep running.
 */

#ifndef GRIDMIND_FEEDBACK_LED_H
#define GRIDMIND_FEEDBACK_LED_H

#include <Arduino.h>

#include "NodeBGame.h"

namespace gridmind {

class FeedbackLed {
 public:
  FeedbackLed();

  // Configure D0 as an output and guarantee that the LED starts off.
  void begin();

  // Select a visible pattern from one accepted decision result.
  void show(const DecisionResult& result);

  // Advance an active pattern; call repeatedly from loop().
  void update();

 private:
  enum class Mode : uint8_t {
    OFF,
    STEADY,
    BLINKING
  };

  static const uint8_t LED_PIN = D0;

  // Minimal state required to advance a pattern without delay().
  Mode mode_;
  bool ledOn_;
  uint8_t togglesRemaining_;
  unsigned long intervalMs_;
  unsigned long lastChangeTime_;

  // Keep the remembered state and physical output synchronized.
  void setLed(bool on);

  // Start immediately on, then toggle until the flash count is complete.
  void startBlink(uint8_t flashes, unsigned long intervalMs);
};

}  // namespace gridmind

#endif
