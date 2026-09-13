/*
  Declares the two startup tests used to check Node A's scenario model.
  Dependencies: standard integer types only.
*/
#ifndef GRIDMIND_NODE_A_TESTS_H
#define GRIDMIND_NODE_A_TESTS_H

#include <stdint.h>

class Tests {
 public:
  static const uint8_t COUNT = 2;
  static uint8_t run();
};

#endif
