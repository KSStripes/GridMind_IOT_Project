// Tests.h
// Exposes the five deterministic game-rule tests run during startup.
#ifndef GRIDMIND_TESTS_H
#define GRIDMIND_TESTS_H

#include <stdint.h>

class Tests {
 public:
  static const uint8_t COUNT = 5;
  static uint8_t run();
};

#endif
