/*
  Defines Node B's Run, Wait and Cancel buttons and its result LED.
  Button and LED updates do not block the web server.
  Dependencies: Arduino core and Game_b.h.
*/
#ifndef GRIDMIND_NODE_B_PANEL_H
#define GRIDMIND_NODE_B_PANEL_H

#include <Arduino.h>

#include "Game_b.h"

class Panel {
 public:
  Panel();

  void begin();
  bool poll(Action& action);
  void show(const Result& result);
  void update();

 private:
  // rawState changes immediately; stableState changes after debounce time.
  struct Button {
    uint8_t pin;
    Action action;
    int rawState;
    int stableState;
    unsigned long changedAt;
  };

  enum LedMode {
    LED_OFF,
    LED_STEADY,
    LED_BLINKING
  };

  static const uint8_t BUTTON_COUNT = 3;
  static const unsigned long DEBOUNCE_MS = 30;
  static const uint8_t LED_PIN = D0;

  Button buttons_[BUTTON_COUNT];
  LedMode ledMode_;
  bool ledOn_;
  uint8_t togglesLeft_;
  unsigned long blinkMs_;
  unsigned long changedAt_;

  void setLed(bool on);
  void blink(uint8_t flashes, unsigned long intervalMs);
};

#endif
