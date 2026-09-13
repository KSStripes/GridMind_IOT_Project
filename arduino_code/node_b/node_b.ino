// GridMind Node B - contract queue station
// Three buttons apply Run, Wait or Cancel to a fictional job queue.
// One Game object supplies shared state to Serial, Panel and Web outputs.
#include "Secrets_b.h"
#include "Game_b.h"
#include "Panel_b.h"
#include "Scenarios_b.h"
#include "Tests_b.h"
#include "Web_b.h"

const bool RUN_STARTUP_TESTS = true;
// Node B polls this read-only Node A route for live facility conditions.
// Recheck this address if the hotspot gives Node A a different IP.
const char NODE_A_STATUS_URL[] = "http://172.20.10.4/api/status";

// These are the only long-lived objects used by setup() and loop().
Game game;
Panel panel;
// Web serves Node B's routes and also performs the Node A polling.
Web web(WIFI_SSID, WIFI_PASSWORD, NODE_A_STATUS_URL, game);

void printCurrentJob() {
  const Job* job = game.currentJob();
  if (job == 0) {
    Serial.println("Queue empty.");
    return;
  }
  Serial.print("Current job: ");
  Serial.println(job->name);
}

void handleAction(Action action) {
  // Game now rejects Run if Web has not supplied fresh Node A data.
  // Wait and Cancel remain local queue decisions.
  const Result result = game.apply(action);

  // Print a short human-readable record of the result and new queue state.
  Serial.println();
  Serial.print("Action: ");
  Serial.println(actionName(result.action));
  Serial.print("Result: ");
  Serial.println(reasonName(result.reason));
  Serial.print("Change (cents): ");
  Serial.println(result.deltaCents);
  Serial.print("Total (cents): ");
  Serial.println(result.totalCents);
  Serial.print("Queue size: ");
  Serial.println(game.queueSize());
  printCurrentJob();

  // Translate the same result into the documented LED pattern.
  panel.show(result);
}

void setup() {
  // Serial is used for startup tests and physical-action evidence.
  Serial.begin(115200);
  delay(100);

  // Rule tests run before the real game state is created.
  if (RUN_STARTUP_TESTS) {
    const uint8_t passed = Tests::run();
    Serial.println();
    Serial.print("RESULT: ");
    Serial.print(passed);
    Serial.print('/');
    Serial.print(Tests::COUNT);
    Serial.println(passed == Tests::COUNT ? " PASS" : " FAIL");
  }

  // Start the real queue from the first facility and fictional job list.
  if (!game.begin(INITIAL_FACILITY, JOBS, JOB_COUNT)) {
    Serial.println("ERROR: game did not start.");
  }

  // Start physical I/O first, then the dashboard and JSON service.
  panel.begin();
  const bool wifiConnected = web.begin();
  Serial.println(wifiConnected ? "Wi-Fi connected." : "Wi-Fi timed out.");
  if (wifiConnected) {
    Serial.println(web.address());
  }
  Serial.print("Node A status: ");
  Serial.println(NODE_A_STATUS_URL);

  Serial.println("Buttons: Run=D5, Wait=D6, Cancel=D7");
  printCurrentJob();
}

void loop() {
  // Both updates are non-blocking and must run as often as possible.
  panel.update();
  web.update();

  // poll() returns true only for a newly debounced button press.
  Action action = ACT_RUN;
  if (panel.poll(action)) {
    handleAction(action);
  }
}
