/*
  Defines the small model used to hold and change Node A's facility scenario.
  Dependencies: standard integer types and Scenarios_a.h.
*/
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
  const FacilityScenario* scenarios_;
  uint8_t scenarioCount_;
  uint8_t scenarioIndex_;
};

#endif
