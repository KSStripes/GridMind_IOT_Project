/*
 * GridMind Node B - Game Model Implementation
 *
 * Implements validated Run/Defer/Reduce transitions using kWh, kW, gCO2e,
 * Celsius, virtual minutes, and explicitly simulated euro cents. It has no
 * Arduino hardware or networking dependencies.
 */

#include "NodeBGame.h"

namespace gridmind {
namespace {

int32_t positivePart(int32_t value) {
  return value > 0 ? value : 0;
}

bool isKnownAction(Action action) {
  return action == Action::RUN ||
         action == Action::DEFER ||
         action == Action::REDUCE;
}

bool hasText(const char* text) {
  return text != 0 && text[0] != '\0';
}

int32_t demandFor(int32_t energyKwh, uint16_t durationMin) {
  const int64_t numerator =
      static_cast<int64_t>(energyKwh) * 60 + durationMin / 2;
  return static_cast<int32_t>(numerator / durationMin);
}

int32_t carbonCostCents(
    int32_t energyKwh,
    uint16_t co2eGPerKwh,
    int32_t co2eEurPerT) {
  const int64_t numerator =
      static_cast<int64_t>(energyKwh) *
      co2eGPerKwh *
      co2eEurPerT;
  return static_cast<int32_t>((numerator + 5000) / 10000);
}

}  // namespace

NodeBGame::NodeBGame()
    : grid_{0, 0, 0, 0, 0, 0},
      workload_{"", "", 1000, 60, 0, 1000, true, 60, 0,
                WorkloadStatus::PENDING},
      rates_{0, 0, 0, 0, 0, 60, 15},
      lastResult_{},
      totalCents_(0),
      ready_(false),
      hasResult_(false) {
}

bool NodeBGame::begin(
    const GridState& grid,
    const WorkloadState& workload,
    const ScenarioRates& rates) {
  if (!isValidGrid(grid) ||
      !isValidWorkload(workload) ||
      !isValidRates(rates) ||
      workload.status != WorkloadStatus::PENDING) {
    return false;
  }

  const int32_t startingSlack =
      static_cast<int32_t>(workload.deadlineMin) -
      grid.nowMin -
      workload.durationMin;
  if (startingSlack < 0) {
    return false;
  }

  grid_ = grid;
  workload_ = workload;
  rates_ = rates;
  lastResult_ = DecisionResult{};
  totalCents_ = 0;
  ready_ = true;
  hasResult_ = false;
  return true;
}

bool NodeBGame::setGrid(const GridState& grid) {
  if (!ready_ ||
      workload_.status != WorkloadStatus::PENDING ||
      !isValidGrid(grid) ||
      grid.nowMin != grid_.nowMin) {
    return false;
  }

  grid_ = grid;
  return true;
}

bool NodeBGame::isReady() const {
  return ready_;
}

int32_t NodeBGame::totalCents() const {
  return totalCents_;
}

int32_t NodeBGame::currentDemandKw() const {
  if (!ready_ || workload_.durationMin == 0) {
    return 0;
  }
  return demandFor(workload_.energyKwh, workload_.durationMin);
}

int32_t NodeBGame::slackMin() const {
  if (!ready_) {
    return 0;
  }
  if (workload_.status != WorkloadStatus::PENDING && hasResult_) {
    return lastResult_.slackMinAfter;
  }
  return static_cast<int32_t>(workload_.deadlineMin) -
         grid_.nowMin -
         workload_.durationMin;
}

bool NodeBGame::hasResult() const {
  return hasResult_;
}

const GridState& NodeBGame::grid() const {
  return grid_;
}

const WorkloadState& NodeBGame::workload() const {
  return workload_;
}

const ScenarioRates& NodeBGame::rates() const {
  return rates_;
}

const DecisionResult& NodeBGame::lastResult() const {
  return lastResult_;
}

bool NodeBGame::isValidGrid(const GridState& grid) {
  return grid.capacityKw >= 0 && grid.capacityKw <= 20000 &&
         grid.renewablePct <= 100 &&
         grid.co2eGPerKwh <= 1000 &&
         grid.tempC >= 0 && grid.tempC <= 50 &&
         grid.tempLimitC >= 0 && grid.tempLimitC <= 50 &&
         grid.nowMin <= 1439;
}

bool NodeBGame::isValidWorkload(const WorkloadState& workload) {
  const bool knownStatus =
      workload.status == WorkloadStatus::PENDING ||
      workload.status == WorkloadStatus::COMPLETE ||
      workload.status == WorkloadStatus::FAILED;

  if (!knownStatus ||
      !hasText(workload.customer) ||
      !hasText(workload.job) ||
      workload.energyKwh < 1000 || workload.energyKwh > 20000 ||
      workload.durationMin < 15 || workload.durationMin > 240 ||
      workload.tempRiseC < 0 || workload.tempRiseC > 10 ||
      workload.contractEur < 1000 || workload.contractEur > 100000 ||
      workload.deadlineMin > 1439) {
    return false;
  }

  if (workload.status == WorkloadStatus::PENDING) {
    return workload.progressPct == 0;
  }
  if (workload.status == WorkloadStatus::COMPLETE) {
    return workload.progressPct == 100;
  }
  return workload.progressPct == 0;
}

bool NodeBGame::isValidRates(const ScenarioRates& rates) {
  return rates.co2eEurPerT >= 0 && rates.co2eEurPerT <= 10000 &&
         rates.overloadEurPerMw >= 0 &&
         rates.overloadEurPerMw <= 100000 &&
         rates.coolingEurPerC >= 0 &&
         rates.coolingEurPerC <= 100000 &&
         rates.flexLateEur >= 0 && rates.flexLateEur <= 100000 &&
         rates.urgentLateEur >= rates.flexLateEur &&
         rates.urgentLateEur <= 100000 &&
         rates.reducePct >= 1 && rates.reducePct <= 100 &&
         rates.deferMin >= 1 && rates.deferMin <= 240;
}

DecisionResult NodeBGame::rejected(
    Action action,
    DecisionError error) const {
  DecisionResult result = {};
  result.accepted = false;
  result.error = error;
  result.action = action;
  result.projectedTempC = grid_.tempC;
  result.tempLimitC = grid_.tempLimitC;
  result.totalCents = totalCents_;
  result.nowMinAfter = grid_.nowMin;
  result.completionMin = grid_.nowMin + workload_.durationMin;
  result.slackMinAfter = slackMin();
  result.statusAfter = workload_.status;
  return result;
}

DecisionResult NodeBGame::apply(Action action) {
  if (!ready_) {
    return rejected(action, DecisionError::NOT_INITIALIZED);
  }
  if (!isKnownAction(action)) {
    return rejected(action, DecisionError::INVALID_ACTION);
  }
  if (!isValidGrid(grid_)) {
    return rejected(action, DecisionError::INVALID_GRID);
  }
  if (!isValidWorkload(workload_)) {
    return rejected(action, DecisionError::INVALID_WORKLOAD);
  }
  if (!isValidRates(rates_)) {
    return rejected(action, DecisionError::INVALID_RATES);
  }
  if (workload_.status != WorkloadStatus::PENDING) {
    return rejected(action, DecisionError::WORKLOAD_NOT_PENDING);
  }

  DecisionResult result = {};
  result.accepted = true;
  result.error = DecisionError::NONE;
  result.action = action;
  result.projectedTempC = grid_.tempC;
  result.tempLimitC = grid_.tempLimitC;

  if (action == Action::DEFER) {
    const int32_t nextNow = grid_.nowMin + rates_.deferMin;
    if (nextNow > 1439) {
      return rejected(action, DecisionError::INVALID_TIME);
    }

    grid_.nowMin = static_cast<uint16_t>(nextNow);
    result.nowMinAfter = grid_.nowMin;
    result.completionMin = grid_.nowMin + workload_.durationMin;
    result.slackMinAfter =
        static_cast<int32_t>(workload_.deadlineMin) -
        result.completionMin;

    if (result.slackMinAfter < 0) {
      const int32_t lateEur =
          workload_.flexible ? rates_.flexLateEur : rates_.urgentLateEur;
      result.lateCostCents = lateEur * 100;
      result.netCents = -result.lateCostCents;
      workload_.status = WorkloadStatus::FAILED;
    }
  } else {
    if (action == Action::RUN) {
      result.energyUsedKwh = workload_.energyKwh;
      result.tempRiseUsedC = workload_.tempRiseC;
      result.deliveredCents = workload_.contractEur * 100;
    } else {
      result.energyUsedKwh = (workload_.energyKwh + 1) / 2;
      result.tempRiseUsedC = (workload_.tempRiseC + 1) / 2;
      const int64_t fullContractCents =
          static_cast<int64_t>(workload_.contractEur) * 100;
      result.deliveredCents = static_cast<int32_t>(
          (fullContractCents * rates_.reducePct + 50) / 100);
    }

    result.demandKw = demandFor(
        result.energyUsedKwh,
        workload_.durationMin);

    const int32_t overloadKw =
        positivePart(result.demandKw - grid_.capacityKw);
    result.overloadMw = (overloadKw + 999) / 1000;

    result.projectedTempC = grid_.tempC + result.tempRiseUsedC;
    result.excessTempC = positivePart(
        result.projectedTempC - grid_.tempLimitC);

    const int64_t emissionsG =
        static_cast<int64_t>(result.energyUsedKwh) *
        grid_.co2eGPerKwh;
    result.emissionsKg = static_cast<int32_t>(
        (emissionsG + 500) / 1000);

    result.co2CostCents = carbonCostCents(
        result.energyUsedKwh,
        grid_.co2eGPerKwh,
        rates_.co2eEurPerT);
    result.overloadCostCents =
        result.overloadMw * rates_.overloadEurPerMw * 100;
    result.coolingCostCents =
        result.excessTempC * rates_.coolingEurPerC * 100;
    result.netCents =
        result.deliveredCents -
        result.co2CostCents -
        result.overloadCostCents -
        result.coolingCostCents;

    result.completionMin = grid_.nowMin + workload_.durationMin;
    result.slackMinAfter =
        static_cast<int32_t>(workload_.deadlineMin) -
        result.completionMin;
    grid_.nowMin = static_cast<uint16_t>(result.completionMin);
    result.nowMinAfter = grid_.nowMin;
    workload_.progressPct = 100;
    workload_.status = WorkloadStatus::COMPLETE;
  }

  const int64_t updatedTotal =
      static_cast<int64_t>(totalCents_) + result.netCents;
  totalCents_ = static_cast<int32_t>(updatedTotal);
  result.totalCents = totalCents_;
  result.statusAfter = workload_.status;
  lastResult_ = result;
  hasResult_ = true;
  return result;
}

}  // namespace gridmind
