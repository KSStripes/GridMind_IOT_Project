/*
 * GridMind Node B - Immutable Scenario Data Interface
 *
 * Keeps fictional customers, workloads, grid conditions, and reviewed rates
 * outside the game rules so they can be inspected and recalculated directly.
 */

#ifndef GRIDMIND_NODE_B_SCENARIO_DATA_H
#define GRIDMIND_NODE_B_SCENARIO_DATA_H

#include "NodeBGame.h"

namespace gridmind {

struct NodeBScenario {
  const char* name;
  GridState grid;
  GridState gridAfterDefer;
  WorkloadState workload;
};

class NodeBScenarioData {
 public:
  static const ScenarioRates& rates();
  static const NodeBScenario& scenarioA();
  static const NodeBScenario& scenarioB();
  static const NodeBScenario& scenarioC();
  static const NodeBScenario& scenarioD();
};

}  // namespace gridmind

#endif
