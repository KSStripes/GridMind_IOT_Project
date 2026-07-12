// Deterministic teaching scenarios shared by Node A's model and tests.
#ifndef GRIDMIND_NODE_A_SCENARIOS_H
#define GRIDMIND_NODE_A_SCENARIOS_H

#include <stdint.h>

struct FacilityScenario {
  const char* name;
  bool capacityAvailable;
  bool powerAvailable;
  int16_t tempC;
  int16_t tempLimitC;
};

static const FacilityScenario SCENARIOS[] = {
    {"Hot but otherwise available", true, true, 26, 28},
    {"Capacity unavailable", false, true, 24, 28},
    {"Electricity unavailable", true, false, 23, 28},
    {"Cool and available", true, true, 22, 28}};

static const uint8_t SCENARIO_COUNT =
    sizeof(SCENARIOS) / sizeof(SCENARIOS[0]);

#endif
