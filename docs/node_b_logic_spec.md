# GridMind Node B — Deterministic Local Logic Specification

Status: proposed implementation specification, first drafted 8 July 2026. This is the permanent logic foundation for Node B, not a disposable Day 3 test. Hardware remains unpowered while this model is reviewed and self-tested.

The educational rationale, intended learning progression and literature basis for this model are documented separately in [`serious-game_theory.md`](./serious-game_theory.md). This file defines the executable rules and acceptance results.

## Design rules

- Use integer arithmetic so every result is reproducible by hand and on the ESP8266.
- Keep the decision function independent of buttons, Serial, LEDs, Wi-Fi and displays.
- Treat all values as transparent educational game units, not measurements or production policy.
- Renewable availability helps explain the grid state but is not separately scored because its effect is already represented by carbon intensity. This avoids double-counting.
- Every interface must call the same decision function.

## State

### Grid state for the current round

| Field | Range | Meaning |
|---|---:|---|
| `capacity` | 0–10 units | Compute energy capacity available this round |
| `renewablePercent` | 0–100% | Synthetic renewable availability shown to the learner |
| `carbonIndex` | 0–100 | Synthetic carbon intensity; higher is worse |
| `thermalHeadroom` | 0–10 units | Heat that can be accepted without a cooling penalty |

### Workload state

| Field | Range | Meaning |
|---|---:|---|
| `energyCost` | 1–10 units | Energy used by a full Run action |
| `heatCost` | 1–10 units | Heat produced by a full Run action |
| `value` | 1–100 points | Service value earned by completing the full workload |
| `flexible` | true/false | Whether missing the deadline receives the flexible or urgent penalty |
| `deadline` | 1–5 rounds | Rounds remaining, including the current round |
| `progress` | 0 or 100% | Whether the workload is pending or complete |

The Day 3 model handles one pending workload at a time. A completed or failed workload cannot accept another decision.

## Actions and transitions

Every accepted action consumes one round.

### Run

- Use the workload's full `energyCost` and `heatCost`.
- Earn 100% of its value.
- Mark progress as 100% and the workload complete.
- Apply carbon, overload and thermal penalties.

### Defer

- Use no energy and produce no heat.
- Earn no value and apply no carbon, overload or thermal penalty.
- Decrease the deadline by one.
- If the new deadline is zero, mark the workload failed and apply the missed-deadline penalty.
- Otherwise keep the workload pending for the next grid round.

### Reduce

- Run a reduced-quality version immediately.
- Use `(energyCost + 1) / 2` energy units and `(heatCost + 1) / 2` heat units, giving an exact round-up for odd values.
- Earn 60% of the workload value using `(value * 60 + 50) / 100` to round to the nearest whole point.
- Mark progress as 100% and the workload complete.
- Apply carbon, overload and thermal penalties to the reduced energy and heat values.

Run and Reduce complete the workload during the current round, so they do not receive a missed-deadline penalty when the starting deadline is at least one.

## Scoring

For Run or Reduce:

```text
carbon penalty  = round(energy used × carbon index / 20)
overload penalty = max(0, energy used - capacity) × 10
thermal penalty  = max(0, heat produced - thermal headroom) × 8

score delta = value earned
              - carbon penalty
              - overload penalty
              - thermal penalty
```

Integer implementation of the carbon rounding rule:

```text
(energy used × carbon index + 10) / 20
```

For a Defer action that exhausts the deadline:

```text
missed-deadline penalty = 30 points for a flexible workload
missed-deadline penalty = 50 points for an urgent workload
score delta = -missed-deadline penalty
```

A safe Defer has a score delta of zero. Total score is the sum of score deltas across rounds.

## Manually calculated acceptance scenarios

All scenarios use a workload with energy cost 6, heat cost 4 and full value 40.

### Scenario A — flexible workload should wait for cleaner supply

Starting workload: flexible, deadline 2, progress 0%.

Round 1 grid: capacity 10, renewable 20%, carbon index 80, thermal headroom 10.

- Defer: energy 0, value 0, deadline becomes 1, score delta 0.

Round 2 grid: capacity 10, renewable 80%, carbon index 20, thermal headroom 10.

- Run carbon penalty: `(6 × 20 + 10) / 20 = 6`.
- No overload or thermal penalty.
- Run score delta: `40 - 6 = 34`.
- Two-round total: `0 + 34 = 34`.

Comparison: running in Round 1 would score `40 - ((6 × 80 + 10) / 20) = 16`. Deferring then running scores 34, an 18-point improvement.

### Scenario B — an urgent workload should not miss its deadline

Starting workload: urgent, deadline 1, progress 0%. Use Scenario A's Round 1 high-carbon grid.

- Run score delta: `40 - 24 = 16`; workload completes.
- Defer makes the deadline zero: score delta `-50`; workload fails.

The deadline penalty makes Run preferable despite dirty supply.

### Scenario C — Reduce avoids capacity and thermal penalties

Starting workload: flexible, deadline 1, progress 0%.

Grid: capacity 4, renewable 60%, carbon index 40, thermal headroom 3.

Run:

- Value earned: 40.
- Carbon penalty: `(6 × 40 + 10) / 20 = 12`.
- Overload penalty: `(6 - 4) × 10 = 20`.
- Thermal penalty: `(4 - 3) × 8 = 8`.
- Score delta: `40 - 12 - 20 - 8 = 0`.

Reduce:

- Energy used: `(6 + 1) / 2 = 3`.
- Heat produced: `(4 + 1) / 2 = 2`.
- Value earned: `(40 × 60 + 50) / 100 = 24`.
- Carbon penalty: `(3 × 40 + 10) / 20 = 6`.
- No overload or thermal penalty.
- Score delta: `24 - 6 = 18`.

Reduce is preferable by 18 points.

### Scenario D — Run is best under clean, unconstrained conditions

Starting workload: flexible, deadline 2, progress 0%.

Grid: capacity 10, renewable 90%, carbon index 10, thermal headroom 10.

Run:

- Carbon penalty: `(6 × 10 + 10) / 20 = 3`.
- Score delta: `40 - 3 = 37`.

Reduce:

- Energy used 3, heat produced 2 and value earned 24.
- Carbon penalty: `(3 × 10 + 10) / 20 = 2`.
- Score delta: `24 - 2 = 22`.

Run is preferable by 15 points.

## Required self-tests before hardware integration

1. Scenario A Defer transition: delta 0, deadline 1, still pending.
2. Scenario A Round 2 Run: delta 34, progress 100%, complete.
3. Scenario A two-round total: 34.
4. Scenario B Run: delta 16 and complete.
5. Scenario B Defer: delta -50 and failed.
6. Scenario C Run: delta 0 and complete.
7. Scenario C Reduce: delta 18 and complete.
8. Scenario D Run: delta 37 and complete.
9. Scenario D Reduce: delta 22 and complete.
10. Reject an action on a workload that is already complete or failed without changing state or score.

No buttons, LED, Wi-Fi or display code may be added to the self-test sketch.
