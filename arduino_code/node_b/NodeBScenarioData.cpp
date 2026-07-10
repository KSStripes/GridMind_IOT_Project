/*
 * GridMind Node B - Immutable Scenario Data
 *
 * All values are local, fictional, and reviewed for educational balance.
 * Nothing in this file is fetched from a market, customer, or live grid.
 */

#include "NodeBScenarioData.h"

namespace gridmind {
namespace {

const ScenarioRates REVIEWED_RATES = {
    100,    // co2eEurPerT
    9000,   // overloadEurPerMw
    6000,   // coolingEurPerC
    30000,  // flexLateEur
    50000,  // urgentLateEur
    60,     // reducePct
    15      // deferMin
};

const NodeBScenario SCENARIO_A = {
    "A - cleaner supply",
    {10000, 20, 800, 22, 27, 540},
    {10000, 80, 200, 22, 27, 555},
    {"Northstar Research", "AI model training", 6000, 60, 4, 40000,
     true, 630, 0, WorkloadStatus::PENDING}};

const NodeBScenario SCENARIO_B = {
    "B - urgent forecast",
    {10000, 20, 800, 22, 27, 540},
    {10000, 80, 200, 22, 27, 555},
    {"Helios Weather", "Regional forecast", 6000, 60, 4, 40000,
     false, 600, 0, WorkloadStatus::PENDING}};

const NodeBScenario SCENARIO_C = {
    "C - constrained demand",
    {4000, 60, 400, 24, 27, 540},
    {4000, 60, 400, 24, 27, 555},
    {"BluePeak Media", "Video rendering", 6000, 60, 4, 40000,
     true, 630, 0, WorkloadStatus::PENDING}};

const NodeBScenario SCENARIO_D = {
    "D - clean capacity",
    {10000, 90, 100, 22, 27, 540},
    {10000, 90, 100, 22, 27, 555},
    {"Northstar Research", "AI model training", 6000, 60, 4, 40000,
     true, 630, 0, WorkloadStatus::PENDING}};

}  // namespace

const ScenarioRates& NodeBScenarioData::rates() {
  return REVIEWED_RATES;
}

const NodeBScenario& NodeBScenarioData::scenarioA() {
  return SCENARIO_A;
}

const NodeBScenario& NodeBScenarioData::scenarioB() {
  return SCENARIO_B;
}

const NodeBScenario& NodeBScenarioData::scenarioC() {
  return SCENARIO_C;
}

const NodeBScenario& NodeBScenarioData::scenarioD() {
  return SCENARIO_D;
}

}  // namespace gridmind
