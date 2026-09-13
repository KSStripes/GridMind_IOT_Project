# GridMind Node B — Contract Station

Last updated: 13 September 2026

## Purpose

Node B presents a queue of ten fictional data-centre contracts. The learner
uses physical Run, Wait and Cancel buttons while Node B applies facility data
received from Node A, updates the queue and maintains a financial total.

## Decision rules

Run succeeds only when:

```text
facility data is available
AND capacity is available
AND electricity is available
AND current temperature + contract heat <= 28°C
```

- A successful Run removes the contract, adds its value and raises Node B's
  local simulated temperature.
- A rejected Run leaves the queue and total unchanged.
- Wait moves the current contract to the back of the queue. Each contract can
  be moved this way once unless waiting is unavailable for that contract.
- Cancel removes the contract and subtracts its penalty.

Result codes are `completed`, `queued`, `cancelled`, `no_capacity`, `no_power`,
`too_hot`, `already_waited`, `queue_empty` and `invalid_state`.

## Contract queue

Values and penalties are stored as integer cents.

| # | Contract | Heat | Value | Cancel penalty | Can wait |
|---:|---|---:|---:|---:|---|
| 1 | Northstar Research — AI training | +5°C | 3,000,000 | 800,000 | Yes |
| 2 | BluePeak Media — Video rendering | +2°C | 1,800,000 | 400,000 | Yes |
| 3 | Helios Weather — Regional forecast | +3°C | 2,500,000 | 1,000,000 | Yes |
| 4 | Atlas Storage — Data backup | +1°C | 900,000 | 150,000 | Yes |
| 5 | MedCore Health — Medical analytics | +2°C | 3,200,000 | 1,500,000 | No |
| 6 | Civic Systems — Batch reporting | +1°C | 600,000 | 100,000 | Yes |
| 7 | Sentinel Security — Threat analysis | +4°C | 2,800,000 | 1,200,000 | No |
| 8 | ArchiveWorks — Data compression | +2°C | 1,000,000 | 200,000 | Yes |
| 9 | Aurora Labs — AI training | +6°C | 3,600,000 | 1,100,000 | Yes |
| 10 | FrameForge Studio — Video rendering | +3°C | 2,000,000 | 500,000 | Yes |

## Node A integration

Node B polls `http://172.20.10.4/api/status` once per second. It accepts a
scenario ID, capacity state, electricity state, temperature and temperature
limit after checking their types and ranges.

Facility values are reapplied when the Node A scenario ID changes. Repeated
responses for the same scenario refresh link health without erasing temperature
added by completed contracts. Data becomes stale after six seconds without a
valid response. Node B then retains the last values for display but disables
Run with `invalid_state`; Wait and Cancel remain available.

## Physical interface

| Function | Pin | Behaviour |
|---|---|---|
| Run button | `D5` | Active-low `INPUT_PULLUP` |
| Wait button | `D6` | Active-low `INPUT_PULLUP` |
| Cancel button | `D7` | Active-low `INPUT_PULLUP` |
| Feedback LED | `D0` | Blue LED through 220 ohms |

Buttons use 30 ms debounce. The feedback LED is steady after a positive result,
flashes slowly twice after an accepted zero-value Wait, and flashes quickly
three times after a rejection or negative result.

## Firmware architecture

```text
node_b.ino           setup, loop and action coordination
Game_b.h/.cpp        queue, rules and financial total
Panel_b.h/.cpp       buttons and feedback LED
Scenarios_b.h        ten contracts and initial facility placeholder
Web_b.h/.cpp         Wi-Fi, Node A polling, dashboard and HTTP routes
Tests_b.h/.cpp       deterministic rule and payload checks
Secrets_example_b.h Wi-Fi credential template
```

`Game` contains the decision rules, `Panel` owns physical I/O, and `Web` owns
network communication. All three use the same game state.

## HTTP interface

| Route | Content |
|---|---|
| `GET /` | Read-only contract dashboard |
| `GET /api/health` | Node identity, Wi-Fi state and uptime |
| `GET /api/status` | Link, facility, contract, queue, result and total state |

The dashboard displays facility conditions, projected temperature, current
contract, last decision and running total. It refreshes every two seconds using
HTML meta refresh.

## Startup checks

The six deterministic checks cover successful Run, all Run rejection
conditions, one-time Wait, Cancel, cumulative money, an empty queue and Node A
payload validation.
