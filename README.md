# GridMind

GridMind is a two-node physical IoT serious game developed for the **CM3040 Physical IoT** course project. It is designed to help learners explore the trade-offs involved in carbon-aware compute scheduling.

The learner responds to changing simulated grid conditions by deciding whether a computational workload should **Run**, **Defer**, or **Reduce**. Each decision balances renewable availability, carbon intensity, capacity, thermal constraints, workload flexibility, deadlines, and service value.

> GridMind is an educational simulation. It is not a real electricity-grid controller, energy meter, data-centre scheduler, or production optimisation system.

## Project objective

GridMind investigates the question:

> How can two REST-connected physical IoT nodes help learners understand trade-offs between renewable availability, thermal and cooling constraints, workload urgency, and carbon intensity?

The project makes these otherwise abstract relationships tangible through physical controls, LED feedback, browser dashboards, transparent scoring, and JSON communication between two ESP8266 nodes.

## System concept

### Node A — energy-grid station

Node A represents changing electricity-system and environmental conditions. Its planned responsibilities are:

- Read an LDR as a tangible proxy for simulated solar availability.
- Generate predetermined capacity, carbon-intensity, and thermal scenarios.
- Advance scenarios using a physical button.
- Provide local LED feedback.
- Host a browser dashboard and JSON API.
- Exchange grid state and demand information with Node B.

### Node B — compute-scheduling station

Node B represents a flexible compute consumer. Its responsibilities are:

- Present a workload with energy, heat, value, flexibility, and deadline properties.
- Accept **Run**, **Defer**, and **Reduce** decisions through physical buttons.
- Apply deterministic state transitions and transparent scoring.
- Explain outcomes through Serial, LED, and browser feedback.
- Host a browser dashboard and JSON API.
- Exchange decisions and results with Node A.

## Decision model

The conceptual score is:

```text
score = completed workload value
        - carbon penalty
        - cooling or thermal penalty
        - overload penalty
        - missed-deadline or quality-of-service penalty
```

No action is universally correct:

- A flexible workload may be deferred until a cleaner interval.
- An urgent workload may need to run despite high carbon intensity.
- A workload may be reduced when capacity or thermal headroom is limited.
- Full execution is preferable when clean energy and operational headroom are available.

The scoring values are transparent educational parameters rather than real operational policy. The complete rationale is documented in [Serious-Game Theory and Parameter Rationale](docs/serious-game_theory.md), while the exact proposed state transitions and acceptance scenarios are defined in the [Node B Logic Specification](docs/node_b_logic_spec.md).

## Planned interfaces

Each node will provide its own browser dashboard and communicate using REST/HTTP with JSON.

Candidate common routes:

```text
GET /api/status
GET /api/health
```

Candidate Node A routes:

```text
GET  /api/grid
POST /api/demand
```

Candidate Node B routes:

```text
GET  /api/workloads
POST /api/decision
```

The final schemas and error responses will be frozen after local state logic has been verified.

## Hardware

The prototype uses no more than two ESP8266 NodeMCU boards.

Planned Node A components:

- One LDR
- One Scenario/Next button
- Three ordinary LEDs with individual resistors
- Optional I2C OLED

Planned Node B components:

- Three decision buttons
- One ordinary feedback LED with a 220 Ω resistor
- Optional I2C OLED

The current Node B pin allocation is:

| Function | NodeMCU pin |
|---|---|
| Run button | `D5` |
| Defer button | `D6` |
| Reduce button | `D7` |
| Feedback LED | `D0` |
| Future I2C SCL | `D1` |
| Future I2C SDA | `D2` |

## Current verified progress

The following milestones have been physically verified:

- Both intended NodeMCU boards compile, upload, and run a Blink test.
- An external LED with a 220 Ω resistor works on the Node A candidate.
- Node B reads one button reliably using `INPUT_PULLUP` and non-blocking debounce.
- Node B reads independent Run, Defer, and Reduce buttons on `D5`, `D6`, and `D7`.
- Each button produces one clean press and release event in Serial Monitor.

The following are designed or planned but must not yet be treated as verified implementation:

- Deterministic workload scoring and state transitions
- Logic self-tests
- Node B feedback LED integration
- Wi-Fi connectivity
- Browser dashboards
- REST/JSON APIs
- LDR input
- OLED output
- Two-node communication

## Development approach

The implementation is being developed in independently testable layers:

1. Verify individual boards and electrical components.
2. Freeze safe pin maps.
3. Define deterministic state transitions and manually calculated outcomes.
4. Test pure decision logic independently of hardware and networking.
5. Integrate buttons, Serial feedback, and the ordinary LED.
6. Add browser dashboards and JSON APIs using the same internal state.
7. Connect the two nodes and test request handling, latency, failure, and recovery.
8. Add optional OLED presentation only after the core interfaces are stable.

## Repository structure

```text
.
├── README.md
├── context.md
├── inventory.md
├── docs/
│   ├── node_b_logic_spec.md
│   └── serious-game_theory.md
├── firmware/
│   ├── day2_led_test/
│   ├── day2_button_test/
│   └── day2_three_button_test/
├── evidence/
├── reading/
├── schemas/
└── screenshots/
```

- `context.md` is the compact project handoff and verified-status record.
- `inventory.md` records the available components and handling constraints.
- `docs/` contains the system rationale and implementation specifications.
- `firmware/` contains Arduino sketches and incremental hardware tests.
- `evidence/` and `screenshots/` contain development evidence.
- `reading/` contains local research material used by the course report.
- `schemas/` is reserved for the final JSON interface definitions.

## Electrical safety

- ESP8266 GPIO operates at 3.3 V; never apply 5 V to a GPIO pin.
- Every active ordinary or RGB LED channel requires its own current-limiting resistor.
- Buttons connect GPIO to ground using `INPUT_PULLUP`.
- ESP8266 boot-strapping pins must be reviewed before attaching peripherals.
- Circuits must be powered off before wiring is changed.
- The LDR must remain disconnected until the safe `A0` input arrangement and divider circuit are confirmed.

## Educational and research basis

GridMind draws its decision dimensions from research on renewable-aware, deadline-constrained data-centre scheduling. Its interaction and feedback approach is informed by research on demand-response games, serious energy games, and energy gamification.

The literature supports the use of competing workload, deadline, renewable, energy, and cooling considerations. It does not establish GridMind's exact scoring weights. Those values remain explicit, reproducible design choices that require scenario and sensitivity testing.

## Course scope

This repository supports an individual CM3040 Physical IoT course submission. The project is intentionally scoped to two ESP-based nodes, physical inputs and outputs, browser dashboards, and REST/JSON communication. Optional hardware must not displace the core learning interaction or networking requirements.
