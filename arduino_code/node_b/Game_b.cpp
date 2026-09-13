// Game_b.cpp
// Implements validation, Run/Wait/Cancel rules, queue movement and money.
// All money is stored as integer cents to avoid floating-point rounding.
#include "Game_b.h"

Game::Game() {
  facility_.capacityAvailable = false;
  facility_.powerAvailable = false;
  facility_.tempC = 0;
  facility_.tempLimitC = 0;
  jobCount_ = 0;
  totalCents_ = 0;
  ready_ = false;
  facilityAvailable_ = false;
  hasResult_ = false;
}

bool Game::begin(
    const Facility& facility,
    const Job jobs[],
    uint8_t jobCount) {
  if (!validFacility(facility) || jobs == 0 ||
      jobCount == 0 || jobCount > MAX_JOBS) {
    return false;
  }

  // Copy the supplied scenarios into the game's fixed local storage.
  facility_ = facility;
  for (uint8_t i = 0; i < jobCount; i++) {
    jobs_[i] = jobs[i];
  }
  jobCount_ = jobCount;
  totalCents_ = 0;
  ready_ = true;
  facilityAvailable_ = true;
  hasResult_ = false;
  return true;
}

bool Game::setFacility(const Facility& facility) {
  if (!ready_ || !validFacility(facility)) {
    return false;
  }
  facility_ = facility;
  return true;
}

void Game::setFacilityAvailable(bool available) {
  facilityAvailable_ = available;
}

Result Game::apply(Action action) {
  // Actions cannot be processed before setup or after the queue is empty.
  if (!ready_) {
    return finish(action, 0, false, RSN_INVALID_STATE, 0);
  }
  if (jobCount_ == 0) {
    return finish(action, 0, false, RSN_QUEUE_EMPTY, 0);
  }
  // Never admit work using missing, stale or rejected Node A data.
  // Wait and Cancel do not depend on facility conditions.
  if (action == ACT_RUN && !facilityAvailable_) {
    return finish(action, jobs_[0].name, false, RSN_INVALID_STATE, 0);
  }

  Job& job = jobs_[0];
  const char* jobName = job.name;
  const int16_t projectedTemp = facility_.tempC + job.tempRiseC;

  if (action == ACT_RUN) {
    // Run must pass capacity, electricity and temperature checks in order.
    if (!facility_.capacityAvailable) {
      return finish(action, jobName, false, RSN_NO_CAPACITY, 0);
    }
    if (!facility_.powerAvailable) {
      return finish(action, jobName, false, RSN_NO_POWER, 0);
    }
    if (projectedTemp > facility_.tempLimitC) {
      return finish(action, jobName, false, RSN_TOO_HOT, 0);
    }

    // A successful Run earns the value, raises temperature and removes the job.
    const int32_t value = job.valueCents;
    facility_.tempC = projectedTemp;
    removeFront();
    return finish(action, jobName, true, RSN_COMPLETED, value);
  }

  if (action == ACT_WAIT) {
    // Each job may move to the queue rear only once.
    if (!job.canWait) {
      return finish(action, jobName, false, RSN_ALREADY_WAITED, 0);
    }

    // Shift the remaining jobs left, then place this job at the back.
    Job deferred = job;
    deferred.canWait = false;
    for (uint8_t i = 1; i < jobCount_; i++) {
      jobs_[i - 1] = jobs_[i];
    }
    jobs_[jobCount_ - 1] = deferred;
    return finish(action, jobName, true, RSN_QUEUED, 0);
  }

  if (action == ACT_CANCEL) {
    // Cancel removes the job and records its penalty as negative money.
    const int32_t penalty = -job.penaltyCents;
    removeFront();
    return finish(action, jobName, true, RSN_CANCELLED, penalty);
  }

  return finish(action, jobName, false, RSN_INVALID_STATE, 0);
}

bool Game::isReady() const {
  return ready_;
}

bool Game::facilityAvailable() const {
  return facilityAvailable_;
}

const Facility& Game::facility() const {
  return facility_;
}

const Job* Game::currentJob() const {
  return jobCount_ == 0 ? 0 : &jobs_[0];
}

uint8_t Game::queueSize() const {
  return jobCount_;
}

int32_t Game::totalCents() const {
  return totalCents_;
}

bool Game::hasResult() const {
  return hasResult_;
}

const Result& Game::lastResult() const {
  return lastResult_;
}

bool Game::validFacility(const Facility& facility) {
  return facility.tempC >= 0 && facility.tempC <= 60 &&
         facility.tempLimitC >= 0 && facility.tempLimitC <= 60;
}

Result Game::finish(
    Action action,
    const char* jobName,
    bool accepted,
    Reason reason,
    int32_t deltaCents) {
  // Rejected actions never change the cumulative money total.
  if (accepted) {
    totalCents_ += deltaCents;
  }

  lastResult_.action = action;
  lastResult_.jobName = jobName;
  lastResult_.accepted = accepted;
  lastResult_.reason = reason;
  lastResult_.deltaCents = deltaCents;
  lastResult_.totalCents = totalCents_;
  hasResult_ = true;
  return lastResult_;
}

void Game::removeFront() {
  // Close the gap left by a completed or cancelled first job.
  for (uint8_t i = 1; i < jobCount_; i++) {
    jobs_[i - 1] = jobs_[i];
  }
  jobCount_ -= 1;
}

const char* actionName(Action action) {
  switch (action) {
    case ACT_RUN:    return "run";
    case ACT_WAIT:   return "wait";
    case ACT_CANCEL: return "cancel";
  }
  return "unknown";
}

const char* reasonName(Reason reason) {
  switch (reason) {
    case RSN_COMPLETED:      return "completed";
    case RSN_QUEUED:         return "queued";
    case RSN_CANCELLED:      return "cancelled";
    case RSN_NO_CAPACITY:    return "no_capacity";
    case RSN_NO_POWER:       return "no_power";
    case RSN_TOO_HOT:        return "too_hot";
    case RSN_ALREADY_WAITED: return "already_waited";
    case RSN_QUEUE_EMPTY:    return "queue_empty";
    case RSN_INVALID_STATE:  return "invalid_state";
  }
  return "unknown";
}
