// Tests the scenario model without buttons, LEDs or Wi-Fi.
#include "Facility_a.h"
#include "Scenarios_a.h"
#include "Tests_a.h"

static bool testInitialScenario() {
  Facility facility;
  return facility.begin(SCENARIOS, SCENARIO_COUNT) &&
         facility.isReady() && facility.number() == 1 &&
         facility.count() == 5 &&
         facility.current().capacityAvailable &&
         facility.current().powerAvailable &&
         facility.current().tempC == 22 &&
         !facility.temperatureWarning();
}

static bool testAdvanceAndWrap() {
  Facility facility;
  if (!facility.begin(SCENARIOS, SCENARIO_COUNT)) {
    return false;
  }
  for (uint8_t expected = 2; expected <= SCENARIO_COUNT; expected++) {
    if (!facility.advance() || facility.number() != expected) {
      return false;
    }
  }
  return facility.advance() && facility.number() == 1;
}

uint8_t Tests::run() {
  uint8_t passed = 0;
  passed += testInitialScenario() ? 1 : 0;
  passed += testAdvanceAndWrap()  ? 1 : 0;
  return passed;
}
