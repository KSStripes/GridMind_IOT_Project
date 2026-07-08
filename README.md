# GridMind

GridMind is a physical IoT serious-game project developed for the **CM3040 Physical IoT** course. It explores how tangible controls and connected devices can help learners understand carbon-aware compute scheduling.

## Project overview

The learner is presented with changing electricity-system conditions and a computational workload. They choose one of three actions:

- **Run** the full workload now.
- **Defer** a flexible workload to a later period.
- **Reduce** the workload's resource use and delivered value.

Each choice illustrates trade-offs between renewable availability, carbon intensity, available capacity, thermal constraints, workload flexibility, deadlines, and service value. There is no action that is always correct: the appropriate decision depends on the complete scenario.

GridMind is an educational simulation. It is not a real electricity-grid controller, energy meter, data-centre scheduler, or production optimisation system.

## Two-node concept

GridMind uses two ESP8266 NodeMCU devices connected through REST/HTTP and JSON.

### Node A — energy-grid station

Node A represents changing grid and environmental conditions. It combines synthetic scenarios with a light-dependent resistor as a tangible proxy for renewable availability. Physical indicators and a browser dashboard communicate the current conditions.

### Node B — compute-scheduling station

Node B represents a compute consumer with urgent and flexible workloads. Three physical buttons accept Run, Defer, and Reduce decisions. The node applies transparent game rules and communicates the result through Serial output, physical feedback, and a browser dashboard.

## Learning goals

GridMind is designed to help learners:

- Understand that renewable availability and carbon intensity vary over time.
- Distinguish urgent workloads from flexible workloads.
- Recognise temporal load shifting as a demand-response action.
- Understand the relationship between compute demand and thermal constraints.
- Balance environmental impact with capacity, deadlines, reliability, and service value.
- Explain why the same action is not appropriate in every situation.

## Design approach

The project uses deterministic and reproducible scenarios. After each decision, the learner can inspect the value earned and any carbon, capacity, thermal, or deadline penalties.

The core game model is separated from buttons, LEDs, dashboards, and networking. This allows every interface to use the same state transitions and scoring rules.

The educational rationale and literature basis are described in [Serious-Game Theory and Parameter Rationale](docs/serious-game_theory.md). The state model, actions, scoring rules, and acceptance scenarios are documented in the [Node B Logic Specification](docs/node_b_logic_spec.md).

## Technology

- ESP8266 NodeMCU boards
- Arduino IDE and C++
- Physical buttons, LEDs, and an LDR
- Wi-Fi communication
- REST/HTTP and JSON
- Embedded browser dashboards
- Optional I2C OLED displays

## Repository

```text
.
├── README.md       Project overview
├── context.md      Detailed project decisions and verified progress
├── inventory.md    Hardware inventory and handling notes
├── docs/           Design rationale and specifications
├── firmware/       Arduino sketches and firmware modules
├── evidence/       Development and test evidence
├── reading/        Research material
├── schemas/        Interface definitions
└── screenshots/    Supporting screenshots
```

## Academic context

This repository supports an individual CM3040 Physical IoT course project. Synthetic values and scoring weights are used for teaching and prototyping; they must not be interpreted as operational grid or data-centre policy.
