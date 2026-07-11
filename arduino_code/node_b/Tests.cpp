// Tests.cpp
// Exercises Game without buttons, Wi-Fi or delays.
// Each function covers one important learner-visible rule.
#include <string.h>

#include "Game.h"
#include "Tests.h"

static bool testSuccessfulRun() {
  const Facility facility = {true, true, 22, 28};
  const Job jobs[] = {{"Job A", 2, 200000, 50000, true}};
  Game game;
  const bool started = game.begin(facility, jobs, 1);
  const Result result = game.apply(ACT_RUN);

  return started && result.accepted &&
         result.reason == RSN_COMPLETED &&
         result.deltaCents == 200000 && result.totalCents == 200000 &&
         strcmp(result.jobName, "Job A") == 0 &&
         game.facility().tempC == 24 && game.queueSize() == 0;
}

static bool testRunChecksEveryCondition() {
  const Job jobs[] = {{"Job A", 2, 200000, 50000, true}};
  Game game;
  if (!game.begin({false, true, 22, 28}, jobs, 1) ||
      game.apply(ACT_RUN).reason != RSN_NO_CAPACITY) {
    return false;
  }
  if (!game.setFacility({true, false, 22, 28}) ||
      game.apply(ACT_RUN).reason != RSN_NO_POWER) {
    return false;
  }
  if (!game.setFacility({true, true, 27, 28}) ||
      game.apply(ACT_RUN).reason != RSN_TOO_HOT) {
    return false;
  }
  return game.queueSize() == 1 && game.totalCents() == 0;
}

static bool testWaitMovesJobOnce() {
  const Facility facility = {true, true, 22, 28};
  const Job jobs[] = {
      {"Job A", 2, 200000, 50000, true},
      {"Job B", 1, 100000, 20000, true}};
  Game game;
  if (!game.begin(facility, jobs, 2)) {
    return false;
  }

  const Result first = game.apply(ACT_WAIT);
  const bool movedToB = game.currentJob() != 0 &&
                        strcmp(game.currentJob()->name, "Job B") == 0;
  const Result second = game.apply(ACT_WAIT);
  const bool returnedToA = game.currentJob() != 0 &&
                           strcmp(game.currentJob()->name, "Job A") == 0;
  const Result third = game.apply(ACT_WAIT);

  return first.accepted && second.accepted && movedToB && returnedToA &&
         !third.accepted && third.reason == RSN_ALREADY_WAITED &&
         game.queueSize() == 2;
}

static bool testCancelAppliesPenalty() {
  const Facility facility = {true, true, 22, 28};
  const Job jobs[] = {{"Job A", 2, 200000, 50000, true}};
  Game game;
  const bool started = game.begin(facility, jobs, 1);
  const Result result = game.apply(ACT_CANCEL);

  return started && result.accepted &&
         result.reason == RSN_CANCELLED &&
         result.deltaCents == -50000 && result.totalCents == -50000 &&
         strcmp(result.jobName, "Job A") == 0 &&
         game.queueSize() == 0;
}

static bool testTotalAcrossTwoJobs() {
  const Facility facility = {true, true, 22, 28};
  const Job jobs[] = {
      {"Job A", 1, 100000, 10000, true},
      {"Job B", 1, 80000, 20000, true}};
  Game game;
  if (!game.begin(facility, jobs, 2)) {
    return false;
  }

  const Result completed = game.apply(ACT_RUN);
  const Result cancelled = game.apply(ACT_CANCEL);
  return completed.accepted && cancelled.accepted &&
         cancelled.totalCents == 80000 && game.queueSize() == 0 &&
         game.apply(ACT_RUN).reason == RSN_QUEUE_EMPTY;
}

uint8_t Tests::run() {
  // Count passes so Serial can show one simple RESULT: 5/5 PASS line.
  uint8_t passed = 0;
  passed += testSuccessfulRun()           ? 1 : 0;
  passed += testRunChecksEveryCondition() ? 1 : 0;
  passed += testWaitMovesJobOnce()        ? 1 : 0;
  passed += testCancelAppliesPenalty()    ? 1 : 0;
  passed += testTotalAcrossTwoJobs()      ? 1 : 0;
  return passed;
}
