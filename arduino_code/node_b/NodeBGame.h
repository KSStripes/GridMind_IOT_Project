/*
 * GridMind Node B - Game Model Interface
 *
 * Defines the physical-unit scenario data and the deterministic result shared
 * by Serial, LED, web, and future display adapters. Hardware and presentation
 * code must not duplicate these rules.
 */

#ifndef GRIDMIND_NODE_B_GAME_H
#define GRIDMIND_NODE_B_GAME_H

#include <stdint.h>

namespace gridmind {

enum class Action : uint8_t {
  RUN,
  DEFER,
  REDUCE
};

enum class WorkloadStatus : uint8_t {
  PENDING,
  COMPLETE,
  FAILED
};

enum class DecisionError : uint8_t {
  NONE,
  NOT_INITIALIZED,
  INVALID_ACTION,
  INVALID_GRID,
  INVALID_WORKLOAD,
  INVALID_RATES,
  INVALID_TIME,
  WORKLOAD_NOT_PENDING
};

// Synthetic data-centre conditions at one virtual time.
struct GridState {
  int32_t capacityKw;
  uint8_t renewablePct;
  uint16_t co2eGPerKwh;
  int16_t tempC;
  int16_t tempLimitC;
  uint16_t nowMin;
};

// One fictional customer workload and its service constraints.
struct WorkloadState {
  const char* customer;
  const char* job;
  int32_t energyKwh;
  uint16_t durationMin;
  int16_t tempRiseC;
  int32_t contractEur;
  bool flexible;
  uint16_t deadlineMin;
  uint8_t progressPct;
  WorkloadStatus status;
};

// Fictional educational rates. None are market or operational values.
struct ScenarioRates {
  int32_t co2eEurPerT;
  int32_t overloadEurPerMw;
  int32_t coolingEurPerC;
  int32_t flexLateEur;
  int32_t urgentLateEur;
  uint8_t reducePct;
  uint16_t deferMin;
};

// Complete, inspectable consequence of one attempted decision.
struct DecisionResult {
  bool accepted;
  DecisionError error;
  Action action;
  int32_t energyUsedKwh;
  int32_t demandKw;
  int32_t overloadMw;
  int32_t emissionsKg;
  int16_t tempRiseUsedC;
  int16_t projectedTempC;
  int16_t tempLimitC;
  int16_t excessTempC;
  int32_t deliveredCents;
  int32_t co2CostCents;
  int32_t overloadCostCents;
  int32_t coolingCostCents;
  int32_t lateCostCents;
  int32_t netCents;
  int32_t totalCents;
  int32_t nowMinAfter;
  int32_t completionMin;
  int32_t slackMinAfter;
  WorkloadStatus statusAfter;
};

// Owns the current grid, workload, fictional rates, and accumulated outcome.
class NodeBGame {
 public:
  NodeBGame();

  bool begin(
      const GridState& grid,
      const WorkloadState& workload,
      const ScenarioRates& rates);

  // Load new conditions after Defer without changing the advanced clock.
  bool setGrid(const GridState& grid);

  DecisionResult apply(Action action);

  bool isReady() const;
  int32_t totalCents() const;
  int32_t currentDemandKw() const;
  int32_t slackMin() const;
  bool hasResult() const;
  const GridState& grid() const;
  const WorkloadState& workload() const;
  const ScenarioRates& rates() const;
  const DecisionResult& lastResult() const;

  static bool isValidGrid(const GridState& grid);
  static bool isValidWorkload(const WorkloadState& workload);
  static bool isValidRates(const ScenarioRates& rates);

 private:
  GridState grid_;
  WorkloadState workload_;
  ScenarioRates rates_;
  DecisionResult lastResult_;
  int32_t totalCents_;
  bool ready_;
  bool hasResult_;

  DecisionResult rejected(Action action, DecisionError error) const;
};

}  // namespace gridmind

#endif
