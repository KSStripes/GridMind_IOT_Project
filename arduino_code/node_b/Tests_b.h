/*
  Declares Node B's six startup checks for its rules and Node A data.
  Dependencies: standard integer types only.
*/
#ifndef GRIDMIND_NODE_B_TESTS_H
#define GRIDMIND_NODE_B_TESTS_H

#include <stdint.h>

class Tests {
 public:
  static const uint8_t COUNT = 6;
  static uint8_t run();
};

#endif
