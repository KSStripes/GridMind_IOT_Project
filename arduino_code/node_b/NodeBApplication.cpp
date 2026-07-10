/*
 * GridMind Node B - Application Coordinator Implementation
 *
 * Starts deterministic tests and adapters in a safe order, polls each
 * non-blocking component, and presents the shared game result through Serial
 * and the feedback LED.
 */

#include <Arduino.h>

#include "NodeBApplication.h"
#include "NodeBInitTests.h"
#include "NodeBScenarioData.h"

using namespace gridmind;

namespace {

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

const char* errorName(DecisionError error) {
  switch (error) {
    case DecisionError::NONE:
      return "none";
    case DecisionError::NOT_INITIALIZED:
      return "game not initialized";
    case DecisionError::INVALID_ACTION:
      return "invalid action";
    case DecisionError::INVALID_GRID:
      return "invalid grid state";
    case DecisionError::INVALID_WORKLOAD:
      return "invalid workload";
    case DecisionError::INVALID_RATES:
      return "invalid scenario rates";
    case DecisionError::INVALID_TIME:
      return "virtual time outside the day";
    case DecisionError::WORKLOAD_NOT_PENDING:
      return "workload is no longer pending";
  }
  return "unknown error";
}

void printTime(int32_t minuteOfDay) {
  const int32_t hours = minuteOfDay / 60;
  const int32_t minutes = minuteOfDay % 60;
  if (hours < 10) {
    Serial.print('0');
  }
  Serial.print(hours);
  Serial.print(':');
  if (minutes < 10) {
    Serial.print('0');
  }
  Serial.print(minutes);
}

void printEuroCents(int32_t cents) {
  const bool negative = cents < 0;
  const uint32_t amount = negative
      ? static_cast<uint32_t>(-static_cast<int64_t>(cents))
      : static_cast<uint32_t>(cents);
  if (negative) {
    Serial.print('-');
  }
  Serial.print("EUR ");
  Serial.print(amount / 100);
  Serial.print('.');
  const uint8_t remainder = amount % 100;
  if (remainder < 10) {
    Serial.print('0');
  }
  Serial.print(remainder);
}

void printMoneyLine(const char* label, int32_t cents) {
  Serial.print(label);
  printEuroCents(cents);
  Serial.println();
}

}  // namespace

NodeBApplication::NodeBApplication(
    const char* wifiSsid,
    const char* wifiPassword,
    bool runStartupTests)
    : wifiConnection_(wifiSsid, wifiPassword),
      webApi_(wifiConnection_, game_),
      runStartupTests_(runStartupTests) {
}

void NodeBApplication::begin() {
  Serial.begin(115200);
  delay(100);

  if (runStartupTests_) {
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
  feedbackLed_.update();
  webApi_.update();

  if (buttonPanel_.poll(action)) {
    DecisionResult result = game_.apply(action);

    // Scenario C keeps the same constrained conditions after its first Defer.
    // The time equality prevents this one transition record being reused later.
    const NodeBScenario& scenario = NodeBScenarioData::scenarioC();
    if (result.accepted &&
        action == Action::DEFER &&
        result.statusAfter == WorkloadStatus::PENDING &&
        game_.grid().nowMin == scenario.gridAfterDefer.nowMin) {
      game_.setGrid(scenario.gridAfterDefer);
    }

    printDecision(result);
    feedbackLed_.show(result);
  }
}

void NodeBApplication::beginInteractiveScenario() {
  const NodeBScenario& scenario = NodeBScenarioData::scenarioC();
  const bool started = game_.begin(
      scenario.grid,
      scenario.workload,
      NodeBScenarioData::rates());

  Serial.println();
  Serial.println("Interactive Scenario C - constrained demand");
  Serial.print("Customer: ");
  Serial.println(scenario.workload.customer);
  Serial.print("Job: ");
  Serial.println(scenario.workload.job);
  Serial.print("Capacity: ");
  Serial.print(scenario.grid.capacityKw / 1000.0, 1);
  Serial.println(" MW");
  Serial.print("Renewable availability: ");
  Serial.print(scenario.grid.renewablePct);
  Serial.println('%');
  Serial.print("Carbon intensity: ");
  Serial.print(scenario.grid.co2eGPerKwh);
  Serial.println(" gCO2e/kWh");
  Serial.print("Temperature: ");
  Serial.print(scenario.grid.tempC);
  Serial.print(" C; game limit: ");
  Serial.print(scenario.grid.tempLimitC);
  Serial.println(" C");
  Serial.print("Workload: ");
  Serial.print(scenario.workload.energyKwh);
  Serial.print(" kWh over ");
  Serial.print(scenario.workload.durationMin);
  Serial.print(" min; demand ");
  Serial.print(game_.currentDemandKw() / 1000.0, 1);
  Serial.println(" MW");
  Serial.print("Full contract value: simulated EUR ");
  Serial.println(scenario.workload.contractEur);
  Serial.print("Virtual time: ");
  printTime(scenario.grid.nowMin);
  Serial.print("; deadline: ");
  printTime(scenario.workload.deadlineMin);
  Serial.println();
  Serial.println("Press exactly one button: Run, Defer, or Reduce.");
  Serial.println("Reset the board before testing a different first action.");

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
    Serial.print("Decision rejected: ");
    Serial.println(errorName(result.error));
    return;
  }

  if (result.action == Action::DEFER) {
    Serial.print("Virtual time after Defer: ");
    printTime(result.nowMinAfter);
    Serial.println();
    Serial.print("Earliest completion: ");
    printTime(result.completionMin);
    Serial.println();
    Serial.print("Deadline slack: ");
    Serial.print(result.slackMinAfter);
    Serial.println(" min");
  } else {
    Serial.print("Energy used: ");
    Serial.print(result.energyUsedKwh);
    Serial.println(" kWh");
    Serial.print("Average demand: ");
    Serial.print(result.demandKw / 1000.0, 1);
    Serial.println(" MW");
    Serial.print("Capacity breach: ");
    Serial.print(result.overloadMw);
    Serial.println(" MW");
    Serial.print("Emissions: ");
    Serial.print(result.emissionsKg);
    Serial.println(" kgCO2e");
    Serial.print("Projected temperature: ");
    Serial.print(result.projectedTempC);
    Serial.print(" C; game limit: ");
    Serial.print(result.tempLimitC);
    Serial.println(" C");
    Serial.print("Temperature above limit: ");
    Serial.print(result.excessTempC);
    Serial.println(" C");
  }

  printMoneyLine("Delivered contract value: ", result.deliveredCents);
  printMoneyLine("Simulated carbon cost: ", result.co2CostCents);
  printMoneyLine("Simulated capacity-breach cost: ", result.overloadCostCents);
  printMoneyLine("Simulated cooling-intervention cost: ", result.coolingCostCents);
  printMoneyLine("Simulated missed-deadline cost: ", result.lateCostCents);
  printMoneyLine("Simulated net outcome: ", result.netCents);
  printMoneyLine("Cumulative simulated outcome: ", result.totalCents);
  Serial.print("Status: ");
  Serial.println(statusName(result.statusAfter));
}
