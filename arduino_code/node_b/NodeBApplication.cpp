/*
 * GridMind Node B - Application Coordinator Implementation
 *
 * Starts regression tests and adapters in a safe order, polls each non-blocking
 * component, and sends accepted game results to Serial and the feedback LED.
 */

#include <Arduino.h>

#include "NodeBApplication.h"
#include "NodeBInitTests.h"

using namespace gridmind;

namespace {

// Fixed Scenario C values keep physical and web acceptance checks reproducible.
GridState constrainedGrid() {
  return {4, 60, 40, 3};
}

WorkloadState interactiveWorkload() {
  return {6, 4, 40, true, 1, 0, WorkloadStatus::PENDING};
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

}  // namespace

NodeBApplication::NodeBApplication(
    const char* wifiSsid,
    const char* wifiPassword,
    bool runStartupTests)
    : wifiConnection_(wifiSsid, wifiPassword),
      // The web adapter observes the same model used by physical buttons.
      webApi_(wifiConnection_, game_),
      runStartupTests_(runStartupTests) {
}

void NodeBApplication::begin() {
  Serial.begin(115200);
  delay(100);

  if (runStartupTests_) {
    // Protect stable domain behavior before interactive adapters start.
    NodeBInitTests::run();
  }

  buttonPanel_.begin();
  feedbackLed_.begin();
  beginInteractiveScenario();
  beginWiFi();
  webApi_.begin();
}

void NodeBApplication::update() {
  Action action = Action::RUN;
  // These updates are non-blocking so input, feedback, and HTTP can coexist.
  feedbackLed_.update();
  webApi_.update();

  if (buttonPanel_.poll(action)) {
    DecisionResult result = game_.apply(action);
    printDecision(result);
    feedbackLed_.show(result);
  }
}

void NodeBApplication::beginInteractiveScenario() {
  const bool started = game_.begin(constrainedGrid(), interactiveWorkload());

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

void NodeBApplication::beginWiFi() {
  Serial.println();
  Serial.println("Starting GridMind Compute Station Wi-Fi...");

  if (wifiConnection_.begin()) {
    Serial.print("Wi-Fi connected. Address: ");
    Serial.println(wifiConnection_.address());
  } else {
    Serial.println("Wi-Fi connection timed out.");
  }
}

void NodeBApplication::printDecision(const DecisionResult& result) const {
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
