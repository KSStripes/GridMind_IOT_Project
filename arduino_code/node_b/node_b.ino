/*
 * GridMind - Node B - Logic Self-Test
 *
 * This stage runs deterministic decision tests, then connects the verified
 * three-button panel to one interactive NodeBGame scenario. It does not drive
 * an LED, connect to Wi-Fi, or use a display.
 *
 * The sketch contains ten separately written tests. Much of it repeats the
 * same pattern:
 *   1. Create a grid.
 *   2. Create a workload.
 *   3. Apply an action.
 *   4. Compare expected and actual results.
 *   5. Print PASS or FAIL.
 */

#include "NodeBGame.h"
#include "ButtonPanel.h"
#include "FeedbackLed.h"

using namespace gridmind;

// Overall self-test counters printed once after all cases have run.
int testsPassed = 0;
int testsRun = 0;

// Interactive objects are separate: buttons produce Actions; the game scores them.
ButtonPanel buttonPanel;
NodeBGame liveGame;
FeedbackLed feedbackLed;

// Predetermined grid factories keep scenario inputs readable in each test.
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
  return {
    6,
    4,
    40,
    flexible,
    deadline,
    0,
    WorkloadStatus::PENDING
  };
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

const char* actionName(Action action) {
  switch (action) {
    case Action::RUN:
      return "RUN";
    case Action::DEFER:
      return "DEFER";
    case Action::REDUCE:
      return "REDUCE";
  }
  return "UNKNOWN";
}

void reportTest(
    const char* name,
    int expectedScore,
    const DecisionResult& actual,
    bool passed) {
  // Count the result, then print the same evidence format for every test.
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
  // Scenario A1: a flexible job safely waits through a dirty interval.
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
  // Scenario A2: the deferred job runs when cleaner supply is available.
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
  // Scenario A3: verify the complete Defer-then-Run path and total score.
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
  // Scenario B1: an urgent job runs despite the high carbon penalty.
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
  // Scenario B2: deferring at the final deadline fails the urgent job.
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
  // Scenario C1: full execution incurs capacity and thermal penalties.
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
  // Scenario C2: Reduce avoids the operational constraint penalties.
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
  // Scenario D1: full execution is best under favourable conditions.
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
  // Scenario D2: unnecessary reduction earns less value than full Run.
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
  // Safety case E: a completed workload cannot be scored a second time.
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

void runSelfTests() {
  Serial.println();
  Serial.println("GridMind Node B logic self-test");
  Serial.println("Hardware inputs and outputs are disabled.");
  Serial.println();

  // Execute the acceptance cases in the same order as the specification.
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

  Serial.print("RESULT: ");
  Serial.print(testsPassed);
  Serial.print("/");
  Serial.print(testsRun);
  Serial.println(testsPassed == testsRun ? " PASS" : " FAIL");
}

void printDecision(const DecisionResult& result) {
  Serial.println();
  Serial.print("Action: ");
  Serial.println(actionName(result.action));

  if (!result.accepted) {
    Serial.println("Decision rejected: workload is no longer pending.");
    return;
  }

  Serial.print("Energy used: ");
  Serial.println(result.energyUsed);
  Serial.print("Heat produced: ");
  Serial.println(result.heatProduced);
  Serial.print("Value earned: ");
  Serial.println(result.valueEarned);
  Serial.print("Carbon penalty: ");
  Serial.println(result.carbonPenalty);
  Serial.print("Overload penalty: ");
  Serial.println(result.overloadPenalty);
  Serial.print("Thermal penalty: ");
  Serial.println(result.thermalPenalty);
  Serial.print("Deadline penalty: ");
  Serial.println(result.deadlinePenalty);
  Serial.print("Score delta: ");
  Serial.println(result.scoreDelta);
  Serial.print("Cumulative score: ");
  Serial.println(result.cumulativeScore);
  Serial.print("Status: ");
  Serial.println(statusName(result.statusAfter));
}

void beginInteractiveScenario() {
  // Scenario C exposes distinct consequences for all three actions.
  const bool started =
      liveGame.begin(constrainedGrid(), workload(true, 1));

  Serial.println();
  Serial.println("Interactive Scenario C");
  Serial.println("Capacity 4, renewable 60%, carbon 40, thermal headroom 3");
  Serial.println("Workload: energy 6, heat 4, value 40, flexible, deadline 1");
  Serial.println("Press exactly one button: Run, Defer, or Reduce.");
  Serial.println("Reset the board before testing a different action.");

  if (!started) {
    Serial.println("ERROR: interactive scenario did not initialise.");
  }
}

void setup() {
  // Serial is the only Arduino interface used by this self-test stage.
  Serial.begin(115200);
  delay(100);
  runSelfTests();
  buttonPanel.begin();
  feedbackLed.begin();
  beginInteractiveScenario();
}

void loop() {
  Action action = Action::RUN;

  // Advance any active blink pattern without blocking button polling.
  feedbackLed.update();

  // Route one debounced physical press through the verified game object.
  if (buttonPanel.poll(action)) {
    DecisionResult result = liveGame.apply(action);
    printDecision(result);
    feedbackLed.show(result);
  }
}
