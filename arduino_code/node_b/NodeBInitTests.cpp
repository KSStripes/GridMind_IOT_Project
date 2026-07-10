#include <Arduino.h>
#include <string.h>

#include "NodeBGame.h"
#include "NodeBInitTests.h"
#include "NodeBScenarioData.h"

using namespace gridmind;

namespace {

int testsPassed = 0;
int testsRun = 0;

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

bool beginScenario(NodeBGame& game, const NodeBScenario& scenario) {
  return game.begin(
      scenario.grid,
      scenario.workload,
      NodeBScenarioData::rates());
}

void reportResult(
    const char* name,
    int32_t expectedNetCents,
    const DecisionResult& actual,
    bool passed) {
  testsRun += 1;
  if (passed) {
    testsPassed += 1;
  }

  Serial.print("Test: ");
  Serial.println(name);
  Serial.print("Expected net cents: ");
  Serial.println(expectedNetCents);
  Serial.print("Actual net cents: ");
  Serial.println(actual.netCents);
  Serial.print("Time after: ");
  Serial.println(actual.nowMinAfter);
  Serial.print("Slack after: ");
  Serial.println(actual.slackMinAfter);
  Serial.print("Status after: ");
  Serial.println(statusName(actual.statusAfter));
  Serial.println(passed ? "PASS" : "FAIL");
  Serial.println();
}

void reportCheck(const char* name, bool passed) {
  testsRun += 1;
  if (passed) {
    testsPassed += 1;
  }

  Serial.print("Test: ");
  Serial.println(name);
  Serial.println(passed ? "PASS" : "FAIL");
  Serial.println();
}

void testDemandDerivation() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioC());
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.energyUsedKwh == 6000 &&
      result.demandKw == 6000;
  reportResult("1 derive 6000 kW demand", 1576000, result, passed);
}

void testCarbonCalculation() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioA());
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.emissionsKg == 4800 &&
      result.co2CostCents == 48000 &&
      result.netCents == 3952000;
  reportResult("2 carbon and cents calculation", 3952000, result, passed);
}

void testInvalidInputs() {
  const NodeBScenario& scenario = NodeBScenarioData::scenarioA();
  WorkloadState zeroDuration = scenario.workload;
  zeroDuration.durationMin = 0;
  WorkloadState impossibleDeadline = scenario.workload;
  impossibleDeadline.deadlineMin = 599;

  NodeBGame first;
  NodeBGame second;
  const bool passed =
      !first.begin(scenario.grid, zeroDuration, NodeBScenarioData::rates()) &&
      !second.begin(
          scenario.grid,
          impossibleDeadline,
          NodeBScenarioData::rates());
  reportCheck("3 reject duration and time errors", passed);
}

void testExactOperationalBoundaries() {
  GridState grid = NodeBScenarioData::scenarioC().grid;
  grid.capacityKw = 6000;
  grid.co2eGPerKwh = 0;
  grid.tempC = 23;
  grid.tempLimitC = 27;

  NodeBGame game;
  const bool started = game.begin(
      grid,
      NodeBScenarioData::scenarioC().workload,
      NodeBScenarioData::rates());
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.demandKw == grid.capacityKw &&
      result.projectedTempC == grid.tempLimitC &&
      result.overloadCostCents == 0 &&
      result.coolingCostCents == 0;
  reportResult("4 exact capacity and temperature", 4000000, result, passed);
}

void testScenarioADefer() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioA());
  const DecisionResult result = game.apply(Action::DEFER);
  const bool passed =
      started &&
      result.accepted &&
      result.netCents == 0 &&
      result.nowMinAfter == 555 &&
      result.completionMin == 615 &&
      result.slackMinAfter == 15 &&
      result.statusAfter == WorkloadStatus::PENDING;
  reportResult("5 A Defer remains feasible", 0, result, passed);
}

void testScenarioADeferredRun() {
  NodeBGame game;
  const NodeBScenario& scenario = NodeBScenarioData::scenarioA();
  const bool started = beginScenario(game, scenario);
  const DecisionResult deferred = game.apply(Action::DEFER);
  const bool gridUpdated = game.setGrid(scenario.gridAfterDefer);
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      deferred.accepted &&
      gridUpdated &&
      result.accepted &&
      result.emissionsKg == 1200 &&
      result.co2CostCents == 12000 &&
      result.netCents == 3988000 &&
      result.totalCents == 3988000 &&
      result.statusAfter == WorkloadStatus::COMPLETE;
  reportResult("6 A Defer then cleaner Run", 3988000, result, passed);
}

void testScenarioBRun() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioB());
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.netCents == 3952000 &&
      result.completionMin == 600 &&
      result.slackMinAfter == 0 &&
      result.statusAfter == WorkloadStatus::COMPLETE;
  reportResult("7 B Run completes at deadline", 3952000, result, passed);
}

void testScenarioBUrgentFailure() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioB());
  const DecisionResult result = game.apply(Action::DEFER);
  const bool passed =
      started &&
      result.accepted &&
      result.lateCostCents == 5000000 &&
      result.netCents == -5000000 &&
      result.slackMinAfter == -15 &&
      result.statusAfter == WorkloadStatus::FAILED;
  reportResult("8 B urgent Defer fails", -5000000, result, passed);
}

void testFlexibleDeadlineFailure() {
  const NodeBScenario& scenario = NodeBScenarioData::scenarioB();
  WorkloadState flexibleWorkload = scenario.workload;
  flexibleWorkload.flexible = true;

  NodeBGame game;
  const bool started = game.begin(
      scenario.grid,
      flexibleWorkload,
      NodeBScenarioData::rates());
  const DecisionResult result = game.apply(Action::DEFER);
  const bool passed =
      started &&
      result.accepted &&
      result.lateCostCents == 3000000 &&
      result.netCents == -3000000 &&
      result.statusAfter == WorkloadStatus::FAILED;
  reportResult("9 flexible late-cost boundary", -3000000, result, passed);
}

void testScenarioCRun() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioC());
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.demandKw == 6000 &&
      result.emissionsKg == 2400 &&
      result.overloadMw == 2 &&
      result.projectedTempC == 28 &&
      result.excessTempC == 1 &&
      result.co2CostCents == 24000 &&
      result.overloadCostCents == 1800000 &&
      result.coolingCostCents == 600000 &&
      result.netCents == 1576000;
  reportResult("10 C constrained Run", 1576000, result, passed);
}

void testScenarioCReduce() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioC());
  const DecisionResult result = game.apply(Action::REDUCE);
  const bool passed =
      started &&
      result.accepted &&
      result.energyUsedKwh == 3000 &&
      result.demandKw == 3000 &&
      result.emissionsKg == 1200 &&
      result.overloadMw == 0 &&
      result.projectedTempC == 26 &&
      result.excessTempC == 0 &&
      result.deliveredCents == 2400000 &&
      result.co2CostCents == 12000 &&
      result.overloadCostCents == 0 &&
      result.coolingCostCents == 0 &&
      result.netCents == 2388000;
  reportResult("11 C Reduce avoids constraints", 2388000, result, passed);
}

void testScenarioDRun() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioD());
  const DecisionResult result = game.apply(Action::RUN);
  const bool passed =
      started &&
      result.accepted &&
      result.emissionsKg == 600 &&
      result.co2CostCents == 6000 &&
      result.netCents == 3994000;
  reportResult("12 D clean Run", 3994000, result, passed);
}

void testScenarioDReduce() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioD());
  const DecisionResult result = game.apply(Action::REDUCE);
  const bool passed =
      started &&
      result.accepted &&
      result.emissionsKg == 300 &&
      result.co2CostCents == 3000 &&
      result.deliveredCents == 2400000 &&
      result.netCents == 2397000;
  reportResult("13 D unnecessary Reduce", 2397000, result, passed);
}

void testRejectFinishedWorkload() {
  NodeBGame game;
  const bool started = beginScenario(game, NodeBScenarioData::scenarioD());
  const DecisionResult completed = game.apply(Action::RUN);
  const DecisionResult result = game.apply(Action::REDUCE);
  const bool passed =
      started &&
      completed.accepted &&
      !result.accepted &&
      result.error == DecisionError::WORKLOAD_NOT_PENDING &&
      result.netCents == 0 &&
      result.totalCents == 3994000 &&
      result.slackMinAfter == 30 &&
      result.statusAfter == WorkloadStatus::COMPLETE;
  reportResult("14 reject finished workload", 0, result, passed);
}

void testImmutableScenarioRecords() {
  const ScenarioRates& rates = NodeBScenarioData::rates();
  const NodeBScenario& a = NodeBScenarioData::scenarioA();
  const NodeBScenario& b = NodeBScenarioData::scenarioB();
  const NodeBScenario& c = NodeBScenarioData::scenarioC();
  const NodeBScenario& d = NodeBScenarioData::scenarioD();

  NodeBGame gameA;
  NodeBGame gameB;
  NodeBGame gameC;
  NodeBGame gameD;
  const bool passed =
      rates.co2eEurPerT == 100 &&
      rates.overloadEurPerMw == 9000 &&
      rates.coolingEurPerC == 6000 &&
      rates.flexLateEur == 30000 &&
      rates.urgentLateEur == 50000 &&
      rates.reducePct == 60 &&
      rates.deferMin == 15 &&
      strcmp(a.workload.customer, "Northstar Research") == 0 &&
      strcmp(b.workload.customer, "Helios Weather") == 0 &&
      strcmp(c.workload.customer, "BluePeak Media") == 0 &&
      strcmp(d.workload.job, "AI model training") == 0 &&
      gameA.begin(a.grid, a.workload, rates) &&
      gameB.begin(b.grid, b.workload, rates) &&
      gameC.begin(c.grid, c.workload, rates) &&
      gameD.begin(d.grid, d.workload, rates);
  reportCheck("15 immutable scenario records", passed);
}

}  // namespace

bool NodeBInitTests::run() {
  testsPassed = 0;
  testsRun = 0;

  Serial.println();
  Serial.println("GridMind Node B physical/euro model self-test");
  Serial.println("Hardware inputs and outputs are disabled during tests.");
  Serial.println();

  testDemandDerivation();
  testCarbonCalculation();
  testInvalidInputs();
  testExactOperationalBoundaries();
  testScenarioADefer();
  testScenarioADeferredRun();
  testScenarioBRun();
  testScenarioBUrgentFailure();
  testFlexibleDeadlineFailure();
  testScenarioCRun();
  testScenarioCReduce();
  testScenarioDRun();
  testScenarioDReduce();
  testRejectFinishedWorkload();
  testImmutableScenarioRecords();

  const bool passed = testsPassed == testsRun;
  Serial.print("RESULT: ");
  Serial.print(testsPassed);
  Serial.print("/");
  Serial.print(testsRun);
  Serial.println(passed ? " PASS" : " FAIL");
  return passed;
}
