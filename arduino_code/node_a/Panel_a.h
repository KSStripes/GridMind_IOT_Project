/*
  Defines Node A's Scenario button and traffic-light status LEDs.
  D1 is safe, D2 caution, D5 the button and D6 blocked.
  Dependencies: Arduino core and Scenarios_a.h.
*/
#ifndef GRIDMIND_NODE_A_PANEL_H
#define GRIDMIND_NODE_A_PANEL_H

#include <Arduino.h>

#include "Scenarios_a.h"

class Panel {
 public:
  Panel();

  void begin();
  bool pollScenarioButton();
  void show(const FacilityScenario& scenario, bool temperatureWarning);

 private:
  static const uint8_t GREEN_LED_PIN = D1;
  static const uint8_t YELLOW_LED_PIN = D2;
  static const uint8_t BUTTON_PIN = D5;
  static const uint8_t RED_LED_PIN = D6;
  static const unsigned long DEBOUNCE_MS = 30;

  int rawButtonState_;
  int stableButtonState_;
  unsigned long buttonChangedAt_;
};

#endif
