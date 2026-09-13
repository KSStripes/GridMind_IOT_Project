// Implements scenario validation, selection and wraparound.
#include "Facility_a.h"

Facility::Facility()
    : scenarios_(0), scenarioCount_(0), scenarioIndex_(0) {
}

bool Facility::begin(
    const FacilityScenario scenarios[], uint8_t scenarioCount) {
  if (scenarios == 0 || scenarioCount == 0) {
    return false;
  }

  scenarios_ = scenarios;
  scenarioCount_ = scenarioCount;
  scenarioIndex_ = 0;
  return true;
}

bool Facility::advance() {
  if (!isReady()) {
    return false;
  }
  // Modulo returns the final scenario to scenario one.
  scenarioIndex_ = (scenarioIndex_ + 1) % scenarioCount_;
  return true;
}

bool Facility::isReady() const {
  return scenarios_ != 0;
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
