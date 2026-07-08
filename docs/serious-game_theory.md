# GridMind Serious-Game Theory and Parameter Rationale

Status: design rationale, 8 July 2026.

## Purpose

This document explains how GridMind translates its learning objective into concrete game state, player actions, consequences and feedback. It also distinguishes decisions supported by literature from GridMind's deliberately simplified educational assumptions.

GridMind is an educational simulation. It is not a real grid controller, energy meter, data-centre scheduler or validated behavioural intervention. Its parameters are synthetic game units, and its score must never be presented as production policy or physical performance data.

The exact executable Node B rules and manually calculated acceptance cases are maintained in [`node_b_logic_spec.md`](./node_b_logic_spec.md).

## Core concept in energy-system terms

Node B represents a simplified demand-side scheduler. The learner decides when and how a computational workload should consume electricity while balancing emissions, operational limits and service delivery.

The central problem is multi-objective rather than simple energy minimisation:

```text
complete useful work
while managing carbon exposure,
capacity and thermal constraints,
and deadline or quality-of-service obligations
```

Always running protects service delivery but may create avoidable emissions, overload and cooling stress. Always deferring reduces immediate demand but eventually breaches service obligations. Always reducing demand avoids some operational stress but unnecessarily destroys service value when conditions are favourable. The game is designed so that no action is universally correct.

## Intended learning objective

After progressing through the scenarios, a learner should be able to:

1. Explain why renewable availability and carbon intensity vary over time.
2. Distinguish urgent workloads from temporally flexible workloads.
3. Recognise temporal load shifting as a demand-response action.
4. Recognise workload reduction as a trade between resource consumption and delivered service.
5. Identify capacity and thermal headroom as operational constraints rather than environmental scores.
6. Explain why an urgent workload may rationally run during a carbon-intensive period.
7. Explain why curtailment can also be wasteful when clean energy and operational headroom are available.
8. Compare decisions through visible, reproducible consequences rather than memorising a fixed best action.

## Translation from concepts to game parameters

### Grid state

| Game parameter | Energy-system interpretation | Teaching purpose | Deliberate simplification |
|---|---|---|---|
| `capacity` | Operational ability to serve additional compute demand in the current interval | Shows that demand must fit within an available envelope | A scalar game limit, not network power flow, kW, ramping or reserve modelling |
| `renewablePercent` | Availability of low-marginal-carbon renewable generation | Makes variable supply visible and helps explain changing conditions | Synthetic percentage, not metered or forecast renewable production |
| `carbonIndex` | Consequential signal associated with consuming energy in the interval | Gives consumption a time-dependent environmental consequence | A 0–100 index, not gCO2e/kWh and not a claim about marginal versus average emissions |
| `thermalHeadroom` | Remaining ability to accept compute heat without additional cooling stress | Couples computational activity to physical cooling constraints | A scalar heat allowance, not a temperature, CFD model or HVAC controller |

Renewable availability remains visible but is not independently penalised. Its environmental consequence is represented through the carbon index, which avoids double-counting two related signals. The values should usually be directionally coherent, but the game must not imply that renewable percentage uniquely determines carbon intensity.

### Workload state

| Game parameter | Scheduling interpretation | Teaching purpose |
|---|---|---|
| `energyCost` | Electrical demand of executing the full job | Makes the consumption consequence comparable with capacity and carbon |
| `heatCost` | Thermal burden created by the job | Shows that compute demand also affects cooling requirements |
| `value` | Service, business or societal benefit of completion | Prevents the false lesson that minimum consumption is always optimal |
| `flexible` | Ability to move execution in time | Distinguishes shiftable demand from urgent service |
| `deadline` | Remaining service window | Makes delay and quality-of-service consequences explicit |
| `progress` | Pending or completed job state | Provides an observable state transition and prevents duplicate execution |

### Player actions

| Action | Energy-professional interpretation | Intended lesson |
|---|---|---|
| Run | Dispatch the full workload now | Full service can be rational when deadlines or favourable conditions justify it |
| Defer | Temporal load shifting | Flexible consumption can wait for a cleaner or less constrained interval, but delay consumes deadline margin |
| Reduce | Demand shaping, workload right-sizing or degraded service | Lower resource use can avoid operational penalties, but it sacrifices some delivered value |

The three actions are intentionally few. This makes every consequence inspectable and allows a learner to reason about trade-offs rather than delegate the decision to an optimiser.

## Scoring as an explanatory objective

The conceptual objective is:

```text
score = completed workload value
        - carbon penalty
        - overload penalty
        - cooling/thermal penalty
        - missed-deadline penalty
```

This is a weighted scalarisation of competing objectives. Real scheduling research may preserve a Pareto set or apply operational constraints rather than combine everything into points. GridMind uses a single score because a learner needs immediate, comparable feedback. The explanation and individual penalty components are therefore as important as the total score.

The current exact weights are:

- Carbon penalty: rounded `energy used × carbon index / 20`.
- Overload penalty: 10 points per unit above capacity.
- Thermal penalty: 8 points per unit above thermal headroom.
- Missed deadline: 30 points for a flexible workload and 50 for an urgent workload.
- Reduce: approximately half the energy and heat while retaining 60% of service value.

These numbers are GridMind design parameters, not values obtained from the cited research. They were selected to create legible contrasts between actions using integer arithmetic on an ESP8266. They must be described as transparent educational weights and subjected to sensitivity and scenario testing before being frozen.

## Scenario progression and intended learning

### Scenario A — shift flexible compute to a cleaner interval

A flexible workload first encounters high carbon intensity and later encounters a cleaner, unconstrained interval. Deferring and subsequently running produces a better total result than immediate execution.

The learner should infer that time flexibility has operational value. This is the compute analogue of demand response: consumption is moved rather than simply eliminated. The scenario assumes that the next interval is known or forecast within the game; it does not model forecast uncertainty.

### Scenario B — protect an imminent service deadline

An urgent workload has one interval remaining during high-carbon conditions. Running incurs a carbon penalty, but deferring causes a larger missed-deadline penalty.

The learner should infer that carbon awareness is one decision dimension rather than an absolute instruction to curtail. Reliability, urgency and quality of service can legitimately dominate an environmental preference in a constrained situation.

### Scenario C — reduce under capacity and thermal stress

The full workload exceeds both available capacity and thermal headroom. A reduced form delivers less service value but avoids disproportionate overload and cooling penalties.

The learner should infer that a third option can exist between full operation and non-operation. Workload right-sizing or quality degradation may preserve partial value while respecting operational constraints.

### Scenario D — use clean, unconstrained capacity

Renewable availability is high, carbon intensity is low, and capacity and thermal headroom are sufficient. Full execution produces more value than unnecessary reduction or delay.

The learner should infer that curtailment is not inherently virtuous. Useful consumption during favourable conditions can be the best system outcome.

Together, the first scenarios establish this reasoning pattern:

| Conditions | Usually rational action | Reason |
|---|---|---|
| Carbon-intensive now, cleaner later, flexible deadline | Defer | Preserve the job while shifting its environmental impact |
| Carbon-intensive now, deadline imminent | Run | Avoid a larger service failure |
| Capacity or thermal headroom inadequate | Reduce | Retain partial service value while avoiding operational violations |
| Clean and unconstrained | Run | Use favourable capacity to deliver full value |

These are not hard-coded answers. They must emerge from the visible state and published scoring rules. Later scenarios should combine the dimensions so that the learner must explain a decision rather than associate one colour or signal with one button.

## Feedback and interface implications

The physical controls, Serial interface, feedback LED, dashboard, JSON API and future OLED are different adapters around one state model. They must not implement separate scoring or transition logic.

After each action, feedback should expose:

- the selected action;
- energy and heat used;
- value earned;
- each penalty applied;
- deadline and completion state;
- score delta and cumulative score;
- a short causal explanation.

The LED may provide an immediate positive, warning or penalty cue, but it cannot carry the full explanation. Serial and the browser dashboard must make the causal chain visible. This supports learning through consequences rather than points alone.

## Literature basis and limits

### Multi-objective data-centre scheduling

Iturriaga and Nesmachnow model data-centre scheduling through computing workload, task due dates, renewable-energy availability, server operation, cooling operation, temperature constraints, energy use and quality of service. This supports GridMind's selection of decision dimensions. Their work applies multi-objective evolutionary optimisation; GridMind does not adopt or claim to reproduce their algorithms or results.

Source: Santiago Iturriaga and Sergio Nesmachnow (2016), *Scheduling Energy Efficient Data Centers Using Renewable Energy*, especially pp. 1–4 and 14–15. Local copy: [`2016_Iturriaga_Nesmachnow_Data_Center_Scheduling.pdf`](../reading/2016_Iturriaga_Nesmachnow_Data_Center_Scheduling.pdf).

### Intrinsic serious-game decisions and balanced scoring

Nykyri et al.'s EcoDream prototype makes energy consumption, energy cost, photovoltaic availability and avatar well-being jointly affect its score. The authors explain that scoring only minimum energy would be an unrealistic representation because it would neglect the service or well-being delivered. They also emphasise showing the consequences of actions so that players can learn through successes and mistakes. This supports GridMind's service-value term, immediate feedback and scenario-based decisions.

Source: Mikko Nykyri et al. (2023), *Tutorial Serious Game for Demonstrating Demand Response in an Energy Community*, especially pp. 3–6, DOI 10.1109/ISGTEUROPE56780.2023.10407502. Local copy: [`2023_nykyri_et_al_tutorial_serious_game.pdf`](../reading/2023_nykyri_et_al_tutorial_serious_game.pdf).

### Understandable feedback, informed choice and appropriate complexity

Nasrollahi et al. review serious energy games and emphasise understandable feedback, informed decision-making, appropriately balanced difficulty and the risk of oversimplifying emerging energy-system needs. This supports GridMind's progressive scenario design and its requirement for consistent explanatory outputs.

Source: Hossein Nasrollahi et al. (2023), *Review of Serious Energy Games: Objectives, Approaches, Applications, Data Integration, and Performance Assessment*, especially pp. 2, 7–8, 22–23 and 33–35, DOI 10.3390/en16196948. Local copy: [`2023_energies-16-06948-Nasrollahi.pdf`](../reading/2023_energies-16-06948-Nasrollahi.pdf).

### Evidence and evaluation caution

Johnson et al.'s systematic review finds feedback and challenges among the most frequently used elements in domestic-energy games and reports predominantly positive outcomes across the reviewed studies. It also identifies methodological weaknesses and mixed evidence for some knowledge outcomes. This supports using serious-game feedback as a plausible teaching mechanism, while preventing any claim that GridMind is educationally effective before user evaluation.

Source: Daniel Johnson et al. (2017), *Gamification and Serious Games within the Domain of Domestic Energy Consumption: A Systematic Review*, especially pp. 12–13, DOI 10.1016/j.rser.2017.01.134. Local copy: [`2017_Johnson_etal- Gamification.pdf`](../reading/2017_Johnson_etal-%20Gamification.pdf).

## Claims GridMind may and may not make

GridMind may claim that:

- its decision dimensions are informed by data-centre scheduling literature;
- its feedback and scenario approach is informed by serious-energy-game literature;
- its rules are deterministic, transparent and reproducible;
- the prototype is designed to help learners explore trade-offs.

GridMind may not claim that:

- its point weights are industry-standard or empirically optimal;
- its synthetic values measure a real grid or data centre;
- renewable percentage and carbon intensity have a universal fixed relationship;
- its decisions are suitable for operational control;
- it improves learning or behaviour before an appropriate evaluation demonstrates that outcome.

## Validation implications

Before the model is frozen, validation should include:

1. Hand calculations and logic self-tests for every scenario.
2. Boundary tests for zero headroom, exact capacity, deadline exhaustion and completed workloads.
3. Sensitivity checks showing which weight changes reverse the preferred action.
4. Consistency checks across physical, Serial, dashboard and JSON outputs.
5. Later learner-facing checks of whether users can explain why an action produced its result.

This separates three kinds of evidence: software correctness, internal model consistency and educational effectiveness. Passing the first two does not prove the third.
