// Game.cpp
// Implements validation, Run/Wait/Cancel rules, queue movement and money.
// All money is stored as integer cents to avoid floating-point rounding.
#include "Game.h"

namespace {

bool hasText(const char* text) {
  return text != 0 && text[0] != '\0';
}

}  // namespace

Game::Game()
    : facility_{false, false, 0, 0},
      jobs_{},
      jobCount_(0),
      totalCents_(0),
      ready_(false),
      hasResult_(false),
      lastResult_{} {
}

bool Game::begin(
    const Facility& facility,
    const Job jobs[],
    uint8_t jobCount) {
  // Reject invalid starting data before changing any game state.
  if (!validFacility(facility) || jobs == 0 ||
      jobCount == 0 || jobCount > MAX_JOBS) {
    return false;
  }

  for (uint8_t i = 0; i < jobCount; i++) {
    if (!validJob(jobs[i])) {
      return false;
    }
  }

  // Copy the supplied scenarios into the game's fixed local storage.
  facility_ = facility;
  for (uint8_t i = 0; i < jobCount; i++) {
    jobs_[i] = jobs[i];
  }
  jobCount_ = jobCount;
  totalCents_ = 0;
  ready_ = true;
  hasResult_ = false;
  lastResult_ = Result{};
  return true;
}

bool Game::setFacility(const Facility& facility) {
  if (!ready_ || !validFacility(facility)) {
    return false;
  }
  facility_ = facility;
  return true;
}

Result Game::apply(Action action) {
  // Actions cannot be processed before setup or after the queue is empty.
  if (!ready_) {
    return finish(action, 0, false, Reason::INVALID_STATE, 0);
  }
  if (jobCount_ == 0) {
    return finish(action, 0, false, Reason::QUEUE_EMPTY, 0);
  }

  Job& job = jobs_[0];
  const char* jobName = job.name;
  const int16_t projectedTemp = facility_.tempC + job.tempRiseC;

  if (action == Action::RUN) {
    // Run must pass capacity, electricity and temperature checks in order.
    if (!facility_.capacityAvailable) {
      return finish(action, jobName, false, Reason::NO_CAPACITY, 0);
    }
    if (!facility_.powerAvailable) {
      return finish(action, jobName, false, Reason::NO_POWER, 0);
    }
    if (projectedTemp > facility_.tempLimitC) {
      return finish(action, jobName, false, Reason::TOO_HOT, 0);
    }

    // A successful Run earns the value, raises temperature and removes the job.
    const int32_t value = job.valueCents;
    facility_.tempC = projectedTemp;
    removeFront();
    return finish(action, jobName, true, Reason::COMPLETED, value);
  }

  if (action == Action::WAIT) {
    // Each job may move to the queue rear only once.
    if (!job.canWait) {
      return finish(action, jobName, false, Reason::ALREADY_WAITED, 0);
    }

    // Shift the remaining jobs left, then place this job at the back.
    Job deferred = job;
    deferred.canWait = false;
    for (uint8_t i = 1; i < jobCount_; i++) {
      jobs_[i - 1] = jobs_[i];
    }
    jobs_[jobCount_ - 1] = deferred;
    return finish(action, jobName, true, Reason::QUEUED, 0);
  }

  if (action == Action::CANCEL) {
    // Cancel removes the job and records its penalty as negative money.
    const int32_t penalty = -job.penaltyCents;
    removeFront();
    return finish(action, jobName, true, Reason::CANCELLED, penalty);
  }

  return finish(action, jobName, false, Reason::INVALID_STATE, 0);
}

bool Game::isReady() const {
  return ready_;
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

bool Game::validJob(const Job& job) {
  return hasText(job.name) &&
         job.tempRiseC >= 0 && job.tempRiseC <= 15 &&
         job.valueCents > 0 && job.valueCents <= 100000000 &&
         job.penaltyCents > 0 && job.penaltyCents <= 100000000;
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

  lastResult_ = {
      action,
      jobName,
      accepted,
      reason,
      deltaCents,
      totalCents_};
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
    case Action::RUN:
      return "run";
    case Action::WAIT:
      return "wait";
    case Action::CANCEL:
      return "cancel";
  }
  return "unknown";
}

const char* reasonName(Reason reason) {
  switch (reason) {
    case Reason::COMPLETED:
      return "completed";
    case Reason::QUEUED:
      return "queued";
    case Reason::CANCELLED:
      return "cancelled";
    case Reason::NO_CAPACITY:
      return "no_capacity";
    case Reason::NO_POWER:
      return "no_power";
    case Reason::TOO_HOT:
      return "too_hot";
    case Reason::ALREADY_WAITED:
      return "already_waited";
    case Reason::QUEUE_EMPTY:
      return "queue_empty";
    case Reason::INVALID_STATE:
      return "invalid_state";
  }
  return "unknown";
}
