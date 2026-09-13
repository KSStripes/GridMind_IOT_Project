# GridMind

GridMind is a student IoT project that uses a physical serious game to explain
simple data-centre workload decisions.

# GridMind

GridMind is a two-node physical IoT learning game. It introduces simple
data-centre workload decisions by asking the learner whether to run, wait or
cancel fictional contracts under changing facility conditions.

This is a university prototype. It does not control real data-centre equipment
and its facility, contract and financial values are fictional.

## System overview

- **Node A - facility station:** cycles through five fixed facility scenarios
  using one button. Three LEDs show capacity, electricity and temperature
  warning states.
- **Node B - contract station:** presents ten fictional contracts. Run, Wait and
  Cancel buttons change the queue, financial total and result LED.
- Both ESP8266 boards provide a read-only browser dashboard and REST/HTTP JSON
  routes.
- Node B requests Node A's facility status once per second. Run is disabled if
  that data is missing, invalid or more than six seconds old.

## Hardware

- Two ESP8266 NodeMCU boards
- Four push buttons
- Four LEDs
- Four 220-ohm resistors
- Breadboards and jumper wires
- A shared 2.4 GHz Wi-Fi network

The full pin tables and behaviour are documented in node_a.md and node_b.md.

## Software and dependencies

The project was developed for:

- Arduino IDE 1.8.19
- NodeMCU 1.0 (ESP-12E Module)
- ESP8266 Arduino core 3.1.2

The ESP8266 core supplies the required `ESP8266WiFi`, `ESP8266WebServer` and
`ESP8266HTTPClient` libraries. No additional third-party Arduino library is
needed.

## Source structure

```text
arduino_code/
  node_a/
    node_a.ino          main facility-station sketch
    Facility_a.*        facility scenario state
    Panel_a.*           button and LED handling
    Scenarios_a.h       five fixed scenarios
    Tests_a.*           two startup checks
    Web_a.*             Wi-Fi, dashboard and JSON routes
    Secrets_example_a.h Wi-Fi settings template
  node_b/
    node_b.ino          main contract-station sketch
    Game_b.*            queue, decisions and money
    Panel_b.*           buttons and result LED
    Scenarios_b.h       ten fictional contracts
    Tests_b.*           six startup checks
    Web_b.*             Wi-Fi, dashboard, JSON and Node A polling
    Secrets_example_b.h Wi-Fi settings template
```

## Configure the sketches

1. Copy `Secrets_example_a.h` to `Secrets_a.h` in the `node_a` folder.
2. Copy `Secrets_example_b.h` to `Secrets_b.h` in the `node_b` folder.
3. Put the same 2.4 GHz Wi-Fi name and password in both new files.
4. Check `NODE_A_STATUS_URL` near the top of `node_b.ino`. Change its IP address
   if Node A receives a different address from the network.

The completed secret files contain local credentials and should not be shared
or added to a public repository.

## Compile and upload

1. Open `arduino_code/node_a/node_a.ino` in Arduino IDE.
2. Select **NodeMCU 1.0 (ESP-12E Module)** and the correct serial port.
3. Verify the sketch, upload it to the Node A board and open Serial Monitor at
   115200 baud.
4. Note the dashboard IP address printed by Node A.
5. Open `arduino_code/node_b/node_b.ino` and confirm that
   `NODE_A_STATUS_URL` uses that address.
6. Verify and upload Node B, then open its Serial Monitor at 115200 baud.

The startup checks are enabled by `RUN_STARTUP_TESTS`. Node A prints a `2/2`
result and Node B prints a `6/6` result when all built-in checks pass.

## Use the learning game

1. On Node A, press **Scenario** to select a facility condition.
2. On Node B, review the current contract and projected temperature.
3. Press **Run**, **Wait** or **Cancel**.
4. Read the result from the LED, Serial Monitor or browser dashboard.
5. Continue until the queue is empty.

The five Node A scenarios cover cool operation, limited thermal headroom,
excessive temperature, unavailable capacity and unavailable electricity. The
contracts vary in heat, value, cancellation penalty and whether they can wait.

## HTTP routes

Each node provides:

- `GET /` - browser dashboard
- `GET /api/health` - node, Wi-Fi and uptime information
- `GET /api/status` - current state as JSON

The dashboards are read-only and refresh automatically. Physical buttons remain
the controls for the learning game.

## Safety

- ESP8266 GPIO uses 3.3 V. Cannot use 5 V with a GPIO pin.
- Use a 220-ohm resistor with each external LED.
