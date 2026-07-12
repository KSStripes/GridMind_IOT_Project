// Implements scenario validation, selection and wraparound.
#include "Facility_a.h"

Facility::Facility()
    : scenarioCount_(0), scenarioIndex_(0), ready_(false) {
}

bool Facility::begin(
    const FacilityScenario scenarios[], uint8_t scenarioCount) {
  if (scenarios == 0 || scenarioCount == 0 ||
      scenarioCount > MAX_SCENARIOS) {
    return false;
  }

  // Validate the complete set before changing the live facility state.
  for (uint8_t i = 0; i < scenarioCount; i++) {
    if (!validScenario(scenarios[i])) {
      return false;
    }
  }

  // Copy into fixed storage so the model owns its deterministic data.
  for (uint8_t i = 0; i < scenarioCount; i++) {
    scenarios_[i] = scenarios[i];
  }
  scenarioCount_ = scenarioCount;
  scenarioIndex_ = 0;
  ready_ = true;
  return true;
}

bool Facility::advance() {
  if (!ready_) {
    return false;
  }
  // Modulo returns the final scenario to scenario one.
  scenarioIndex_ = (scenarioIndex_ + 1) % scenarioCount_;
  return true;
}

bool Facility::isReady() const {
  return ready_;
}

const FacilityScenario& Facility::current() const {
  return scenarios_[scenarioIndex_];
}

uint8_t Facility::number() const {
  return scenarioIndex_ + 1;
}

uint8_t Facility::count() const {
  return scenarioCount_;
}

bool Facility::temperatureWarning() const {
  const FacilityScenario& scenario = current();
  return scenario.tempC >= scenario.tempLimitC - 2;
}

bool Facility::validScenario(const FacilityScenario& scenario) {
  return scenario.name != 0 && scenario.name[0] != '\0' &&
         scenario.tempC >= 0 && scenario.tempC <= 60 &&
         scenario.tempLimitC >= 0 && scenario.tempLimitC <= 60;
}
