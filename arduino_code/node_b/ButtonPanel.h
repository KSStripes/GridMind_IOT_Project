/*
 * GridMind Node B - Physical Button Adapter
 *
 * Converts debounced D5, D6, and D7 button presses into game Actions.
 * It knows nothing about workloads, scoring, Serial, LEDs, or networking.
 */

#ifndef GRIDMIND_BUTTON_PANEL_H
#define GRIDMIND_BUTTON_PANEL_H

#include <Arduino.h>

#include "NodeBGame.h"

namespace gridmind {

class ButtonPanel {
 public:
  ButtonPanel();

  // Configure all three buttons as active-low INPUT_PULLUP inputs.
  void begin();

  // Return true once for each clean press and place its action in output.
  bool poll(Action& output);

 private:
  // Debounce history and game action associated with one physical input.
  struct ButtonState {
    uint8_t pin;
    Action action;
    int lastRawState;
    int stableState;
    unsigned long lastRawChangeTime;
  };

  static const size_t BUTTON_COUNT = 3;
  static const unsigned long DEBOUNCE_MS = 30;

  // Fixed panel order: Run, Defer, Reduce.
  ButtonState buttons_[BUTTON_COUNT];
};

}  // namespace gridmind

#endif
