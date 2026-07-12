// Owns Node A's validated deterministic scenario state.
#ifndef GRIDMIND_NODE_A_FACILITY_H
#define GRIDMIND_NODE_A_FACILITY_H

#include <stdint.h>

#include "Scenarios_a.h"

class Facility {
 public:
  Facility();

  bool begin(const FacilityScenario scenarios[], uint8_t scenarioCount);
  bool advance();

  bool isReady() const;
  const FacilityScenario& current() const;
  uint8_t number() const;
  uint8_t count() const;
  bool temperatureWarning() const;

 private:
  static const uint8_t MAX_SCENARIOS = 8;

  FacilityScenario scenarios_[MAX_SCENARIOS];
  uint8_t scenarioCount_;
  uint8_t scenarioIndex_;
  bool ready_;

  static bool validScenario(const FacilityScenario& scenario);
};

#endif
