#include <Arduino.h>

#include "NodeBInitTests.h"
#include "NodeBGame.h"

using namespace gridmind;

namespace {

int testsPassed = 0;
int testsRun = 0;

GridState highCarbonGrid() {
  return {10, 20, 80, 10};
}

GridState cleanGrid() {
  return {10, 80, 20, 10};
}

GridState constrainedGrid() {
  return {4, 60, 40, 3};
}

GridState veryCleanGrid() {
  return {10, 90, 10, 10};
}

WorkloadState workload(bool flexible, int deadline) {
  return {6, 4, 40, flexible, deadline, 0, WorkloadStatus::PENDING};
}

const char* statusName(WorkloadStatus status) {
  switch (status) {
    case WorkloadStatus::PENDING:
      return "PENDING";
    case WorkloadStatus::COMPLETE:
      return "COMPLETE";
    case WorkloadStatus::FAILED:
      return "FAILED";
  }
  return "UNKNOWN";
}

void reportTest(
    const char* name,
    int expectedScore,
    const DecisionResult& actual,
    bool passed) {
  testsRun += 1;
  if (passed) {
    testsPassed += 1;
  }

  Serial.print("Test: ");
  Serial.println(name);
  Serial.print("Expected score: ");
  Serial.println(expectedScore);
  Serial.print("Actual score: ");
  Serial.println(actual.scoreDelta);
  Serial.print("Deadline after: ");
  Serial.println(actual.deadlineAfter);
  Serial.print("Status after: ");
  Serial.println(statusName(actual.statusAfter));
  Serial.println(passed ? "PASS" : "FAIL");
  Serial.println();
}

void testFlexibleDefer() {
  NodeBGame game;
  const bool started = game.begin(highCarbonGrid(), workload(true, 2));
  DecisionResult result = game.apply(Action::DEFER);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 0 &&
      result.deadlineAfter == 1 &&
      result.statusAfter == WorkloadStatus::PENDING;
  reportTest("A1 high-carbon flexible + Defer", 0, result, passed);
}

void testCleanRunAfterDefer() {
  NodeBGame game;
  const bool started = game.begin(cleanGrid(), workload(true, 1));
  DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 34 &&
      result.carbonPenalty == 6 &&
      result.statusAfter == WorkloadStatus::COMPLETE;
  reportTest("A2 cleaner grid + Run", 34, result, passed);
}

void testTwoRoundFlexiblePath() {
  NodeBGame game;
  const bool started = game.begin(highCarbonGrid(), workload(true, 2));
  DecisionResult deferred = game.apply(Action::DEFER);
  const bool gridUpdated = game.setGrid(cleanGrid());
  DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      deferred.accepted &&
      gridUpdated &&
      result.accepted &&
      result.scoreDelta == 34 &&
      game.score() == 34 &&
      result.cumulativeScore == 34;
  reportTest("A3 Defer then Run total", 34, result, passed);
}

void testUrgentRun() {
  NodeBGame game;
  const bool started = game.begin(highCarbonGrid(), workload(false, 1));
  DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 16 &&
      result.statusAfter == WorkloadStatus::COMPLETE;
  reportTest("B1 high-carbon urgent + Run", 16, result, passed);
}

void testUrgentDeferFailure() {
  NodeBGame game;
  const bool started = game.begin(highCarbonGrid(), workload(false, 1));
  DecisionResult result = game.apply(Action::DEFER);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == -50 &&
      result.deadlinePenalty == 50 &&
      result.deadlineAfter == 0 &&
      result.statusAfter == WorkloadStatus::FAILED;
  reportTest("B2 urgent deadline + Defer", -50, result, passed);
}

void testConstrainedRun() {
  NodeBGame game;
  const bool started = game.begin(constrainedGrid(), workload(true, 1));
  DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 0 &&
      result.carbonPenalty == 12 &&
      result.overloadPenalty == 20 &&
      result.thermalPenalty == 8;
  reportTest("C1 constrained grid + Run", 0, result, passed);
}

void testConstrainedReduce() {
  NodeBGame game;
  const bool started = game.begin(constrainedGrid(), workload(true, 1));
  DecisionResult result = game.apply(Action::REDUCE);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 18 &&
      result.energyUsed == 3 &&
      result.heatProduced == 2 &&
      result.valueEarned == 24 &&
      result.carbonPenalty == 6;
  reportTest("C2 constrained grid + Reduce", 18, result, passed);
}

void testVeryCleanRun() {
  NodeBGame game;
  const bool started = game.begin(veryCleanGrid(), workload(true, 2));
  DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 37 &&
      result.carbonPenalty == 3;
  reportTest("D1 clean unconstrained + Run", 37, result, passed);
}

void testVeryCleanReduce() {
  NodeBGame game;
  const bool started = game.begin(veryCleanGrid(), workload(true, 2));
  DecisionResult result = game.apply(Action::REDUCE);
  const bool passed =
      started &&
      result.accepted &&
      result.scoreDelta == 22 &&
      result.carbonPenalty == 2;
  reportTest("D2 clean unconstrained + Reduce", 22, result, passed);
}

void testRejectCompletedWorkload() {
  NodeBGame game;
  const bool started = game.begin(veryCleanGrid(), workload(true, 2));
  DecisionResult completed = game.apply(Action::RUN);
  DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      completed.accepted &&
      !result.accepted &&
      result.error == DecisionError::WORKLOAD_NOT_PENDING &&
      result.scoreDelta == 0 &&
      result.cumulativeScore == 37 &&
      result.statusAfter == WorkloadStatus::COMPLETE;
  reportTest("E completed workload rejects another action", 0, result, passed);
}

}  // namespace

bool NodeBInitTests::run() {
  testsPassed = 0;
  testsRun = 0;

  Serial.println();
  Serial.println("GridMind Node B logic self-test");
  Serial.println("Hardware inputs and outputs are disabled during tests.");
  Serial.println();

  testFlexibleDefer();
  testCleanRunAfterDefer();
  testTwoRoundFlexiblePath();
  testUrgentRun();
  testUrgentDeferFailure();
  testConstrainedRun();
  testConstrainedReduce();
  testVeryCleanRun();
  testVeryCleanReduce();
  testRejectCompletedWorkload();

  const bool passed = testsPassed == testsRun;
  Serial.print("RESULT: ");
  Serial.print(testsPassed);
  Serial.print("/");
  Serial.print(testsRun);
  Serial.println(passed ? " PASS" : " FAIL");
  return passed;
}
