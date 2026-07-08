/*
 * GridMind Node B - Game Model Interface
 *
 * Defines the data exchanged with the game engine and the public NodeBGame
 * class. Hardware, Serial, web, and display code should use this interface
 * rather than implement their own scoring rules.
 */

#ifndef GRIDMIND_NODE_B_GAME_H
#define GRIDMIND_NODE_B_GAME_H

#include <stdint.h>

namespace gridmind {

// The three decisions available to the learner.
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

// Controlled failure reasons returned without changing the game state.
enum class DecisionError : uint8_t {
  NONE,
  NOT_INITIALIZED,
  INVALID_ACTION,
  INVALID_GRID,
  INVALID_WORKLOAD,
  WORKLOAD_NOT_PENDING
};

// Synthetic electricity-system conditions for one decision round.
struct GridState {
  int capacity;
  int renewablePercent;
  int carbonIndex;
  int thermalHeadroom;
};

// Current compute workload and its service constraints.
struct WorkloadState {
  int energyCost;
  int heatCost;
  int value;
  bool flexible;
  int deadline;
  int progress;
  WorkloadStatus status;
};

// Complete, inspectable consequence of one attempted decision.
struct DecisionResult {
  bool accepted;
  DecisionError error;
  Action action;
  int energyUsed;
  int heatProduced;
  int valueEarned;
  int carbonPenalty;
  int overloadPenalty;
  int thermalPenalty;
  int deadlinePenalty;
  int scoreDelta;
  int cumulativeScore;
  int deadlineAfter;
  WorkloadStatus statusAfter;
};

// Owns the current grid, workload, and cumulative score.
class NodeBGame {
 public:
  NodeBGame();

  // Start a new scenario and reset its cumulative score to zero.
  bool begin(const GridState& grid, const WorkloadState& workload);

  // Replace the grid conditions before the next decision round.
  bool setGrid(const GridState& grid);

  // Apply one learner decision to the current workload.
  DecisionResult apply(Action action);

  bool isReady() const;
  int score() const;
  const GridState& grid() const;
  const WorkloadState& workload() const;

  // Public validators can later be reused by JSON and Serial interfaces.
  static bool isValidGrid(const GridState& grid);
  static bool isValidWorkload(const WorkloadState& workload);

 private:
  // State is private so every change must pass through the class rules.
  GridState grid_;
  WorkloadState workload_;
  int cumulativeScore_;
  bool ready_;

  DecisionResult rejected(Action action, DecisionError error) const;
};

}  // namespace gridmind

#endif
