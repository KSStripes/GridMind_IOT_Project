# GridMind Node A — Facility Station

Last updated: 13 September 2026

## Purpose

Node A represents simulated data-centre operating conditions. A physical button
cycles through five deterministic facility scenarios. A three-LED traffic light,
the browser dashboard and the `/api/status` route expose the same state.

## Physical interface

| Function | Pin | Wiring |
|---|---|---|
| Safe LED | `D1` | Green LED through 220 ohms |
| Caution LED | `D2` | Yellow LED through 220 ohms |
| Scenario button | `D5` | Active-low `INPUT_PULLUP` to ground |
| Blocked LED | `D6` | Red LED through 220 ohms |
| Shared ground | `G` | Ground rail |

The button uses 30 ms debounce. The panel shows one traffic-light state: green
when operation is safe, yellow when temperature is within 2°C of the limit, and
red when capacity or electricity is unavailable or temperature reaches the
limit. Temperature is scenario-generated rather than physically measured.

The final breadboard is labelled `SAFE`, `CAUTION`, `BLOCKED` and `NEXT`. The
wide Lolin V3 remains beside the breadboard and connects through jumper wires;
seating it directly covered the usable GPIO rows and left only the separate
power rails accessible.

## Facility scenarios

| ID | Scenario | Capacity | Electricity | Temperature | Limit | Panel output |
|---:|---|---|---|---:|---:|---|
| 1 | Cool and available | Available | Available | 22°C | 28°C | Green — safe |
| 2 | Warm with limited headroom | Available | Available | 26°C | 28°C | Yellow — caution |
| 3 | Too hot — cooling required | Available | Available | 29°C | 28°C | Red — blocked |
| 4 | Capacity unavailable | Unavailable | Available | 24°C | 28°C | Red — blocked |
| 5 | Electricity unavailable | Available | Unavailable | 23°C | 28°C | Red — blocked |

Each debounced button press advances one scenario. Scenario 5 wraps to
Scenario 1.

## Firmware architecture

```text
node_a.ino           setup, loop and coordination
Facility_a.h/.cpp    current scenario and wraparound
Panel_a.h/.cpp       button debounce and LED output
Scenarios_a.h        five deterministic facility scenarios
Web_a.h/.cpp         Wi-Fi, dashboard and HTTP routes
Tests_a.h/.cpp       initial-state and wraparound checks
Secrets_example_a.h Wi-Fi credential template
```

## HTTP interface

| Route | Content |
|---|---|
| `GET /` | Read-only facility dashboard |
| `GET /api/health` | Node identity, Wi-Fi state and uptime |
| `GET /api/status` | Current scenario and facility conditions |

Node B reads these fields from `/api/status`:

```json
{
  "scenarioId": 1,
  "capacityAvailable": true,
  "powerAvailable": true,
  "tempC": 22,
  "tempLimitC": 28
}
```

The response also contains the node identity, scenario name and warning state.
The dashboard refreshes every two seconds using HTML meta refresh.
