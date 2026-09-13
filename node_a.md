# GridMind Node A — Facility Station

Last updated: 13 September 2026

## Purpose

Node A represents simulated data-centre operating conditions. A physical button
cycles through five deterministic facility scenarios. Three LEDs, a browser
dashboard and the `/api/status` route all expose the same state.

## Physical interface

| Function | Pin | Wiring |
|---|---|---|
| Capacity LED | `D1` | Green LED through 220 ohms |
| Electricity LED | `D2` | Yellow LED through 220 ohms |
| Scenario button | `D5` | Active-low `INPUT_PULLUP` to ground |
| Temperature-warning LED | `D6` | Red LED through 220 ohms |
| Shared ground | `G` | Ground rail |

The button uses 30 ms debounce. The temperature LED switches on when the
simulated temperature is within 2°C of the 28°C limit or exceeds it. Temperature
is scenario-generated rather than physically measured.

## Facility scenarios

| ID | Scenario | Capacity | Electricity | Temperature | Limit | Warning LED |
|---:|---|---|---|---:|---:|---|
| 1 | Cool and available | Available | Available | 22°C | 28°C | Off |
| 2 | Warm with limited headroom | Available | Available | 26°C | 28°C | On |
| 3 | Too hot — cooling required | Available | Available | 29°C | 28°C | On |
| 4 | Capacity unavailable | Unavailable | Available | 24°C | 28°C | Off |
| 5 | Electricity unavailable | Available | Unavailable | 23°C | 28°C | Off |

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
