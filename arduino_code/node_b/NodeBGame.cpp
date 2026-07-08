/*
 * GridMind Node B - Game Model Implementation
 *
 * Implements validation, Run/Defer/Reduce transitions, and transparent
 * integer scoring. It has no Arduino hardware or networking dependencies.
 */

#include "NodeBGame.h"

namespace gridmind {
namespace {

// File-local helpers are implementation details, not part of the public API.
int positivePart(int value) {
  return value > 0 ? value : 0;
}

bool isKnownAction(Action action) {
  return action == Action::RUN ||
         action == Action::DEFER ||
         action == Action::REDUCE;
}

int roundedCarbonPenalty(int energyUsed, int carbonIndex) {
  return (energyUsed * carbonIndex + 10) / 20;
}

int roundedReducedValue(int fullValue) {
  return (fullValue * 60 + 50) / 100;
}

}  // namespace

NodeBGame::NodeBGame()
    : grid_{0, 0, 0, 0},
      workload_{1, 1, 1, true, 1, 0, WorkloadStatus::PENDING},
      cumulativeScore_(0),
      ready_(false) {
}

bool NodeBGame::begin(
    const GridState& grid,
    const WorkloadState& workload) {
  // A scenario can begin only with valid conditions and a pending workload.
  if (!isValidGrid(grid) ||
      !isValidWorkload(workload) ||
      workload.status != WorkloadStatus::PENDING) {
    return false;
  }

  // Copy the starting state into the object and reset the scenario score.
  grid_ = grid;
  workload_ = workload;
  cumulativeScore_ = 0;
  ready_ = true;
  return true;
}

bool NodeBGame::setGrid(const GridState& grid) {
  // A deferred workload can receive new conditions for its next round.
  if (!ready_ || !isValidGrid(grid)) {
    return false;
  }

  grid_ = grid;
  return true;
}

bool NodeBGame::isReady() const {
  return ready_;
}

int NodeBGame::score() const {
  return cumulativeScore_;
}

const GridState& NodeBGame::grid() const {
  return grid_;
}

const WorkloadState& NodeBGame::workload() const {
  return workload_;
}

bool NodeBGame::isValidGrid(const GridState& grid) {
  // Keep all synthetic inputs within the ranges defined by the specification.
  return grid.capacity >= 0 && grid.capacity <= 10 &&
         grid.renewablePercent >= 0 && grid.renewablePercent <= 100 &&
         grid.carbonIndex >= 0 && grid.carbonIndex <= 100 &&
         grid.thermalHeadroom >= 0 && grid.thermalHeadroom <= 10;
}

bool NodeBGame::isValidWorkload(const WorkloadState& workload) {
  const bool knownStatus =
      workload.status == WorkloadStatus::PENDING ||
      workload.status == WorkloadStatus::COMPLETE ||
      workload.status == WorkloadStatus::FAILED;

  if (!knownStatus ||
      workload.energyCost < 1 || workload.energyCost > 10 ||
      workload.heatCost < 1 || workload.heatCost > 10 ||
      workload.value < 1 || workload.value > 100 ||
      workload.deadline < 0 || workload.deadline > 5) {
    return false;
  }

  // Status, progress, and deadline must describe one consistent state.
  if (workload.status == WorkloadStatus::PENDING) {
    return workload.deadline >= 1 && workload.progress == 0;
  }

  if (workload.status == WorkloadStatus::COMPLETE) {
    return workload.progress == 100;
  }

  return workload.progress == 0 && workload.deadline == 0;
}

DecisionResult NodeBGame::rejected(
    Action action,
    DecisionError error) const {
  // Rejections report the current state but never alter it or its score.
  DecisionResult result = {};
  result.accepted = false;
  result.error = error;
  result.action = action;
  result.cumulativeScore = cumulativeScore_;
  result.deadlineAfter = workload_.deadline;
  result.statusAfter = workload_.status;
  return result;
}

DecisionResult NodeBGame::apply(Action action) {
  // Reject invalid calls before any game state can be changed.
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

  if (workload_.status != WorkloadStatus::PENDING) {
    return rejected(action, DecisionError::WORKLOAD_NOT_PENDING);
  }

  // From this point the action is valid and may update the owned state.
  DecisionResult result = {};
  result.accepted = true;
  result.error = DecisionError::NONE;
  result.action = action;

  if (action == Action::DEFER) {
    // Defer uses no resources but consumes one round of deadline margin.
    workload_.deadline -= 1;

    if (workload_.deadline == 0) {
      // Missing the deadline fails the job and applies its QoS penalty.
      result.deadlinePenalty = workload_.flexible ? 30 : 50;
      result.scoreDelta = -result.deadlinePenalty;
      workload_.status = WorkloadStatus::FAILED;
    }
  } else {
    if (action == Action::RUN) {
      // Run delivers full value using the workload's full energy and heat.
      result.energyUsed = workload_.energyCost;
      result.heatProduced = workload_.heatCost;
      result.valueEarned = workload_.value;
    } else {
      // Reduce completes a lower-quality job with reduced resource use.
      result.energyUsed = (workload_.energyCost + 1) / 2;
      result.heatProduced = (workload_.heatCost + 1) / 2;
      result.valueEarned = roundedReducedValue(workload_.value);
    }

    // Calculate each penalty separately so interfaces can explain the result.
    result.carbonPenalty =
        roundedCarbonPenalty(result.energyUsed, grid_.carbonIndex);
    result.overloadPenalty =
        positivePart(result.energyUsed - grid_.capacity) * 10;
    result.thermalPenalty =
        positivePart(result.heatProduced - grid_.thermalHeadroom) * 8;
    result.scoreDelta =
        result.valueEarned -
        result.carbonPenalty -
        result.overloadPenalty -
        result.thermalPenalty;

    // Both Run and Reduce finish the current workload in this model.
    workload_.progress = 100;
    workload_.status = WorkloadStatus::COMPLETE;
  }

  // Commit the accepted decision and return an immutable result snapshot.
  cumulativeScore_ += result.scoreDelta;
  result.cumulativeScore = cumulativeScore_;
  result.deadlineAfter = workload_.deadline;
  result.statusAfter = workload_.status;
  return result;
}

}  // namespace gridmind
