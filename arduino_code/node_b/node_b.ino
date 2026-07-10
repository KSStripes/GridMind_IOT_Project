// GridMind Node B - contract queue station
// Three buttons apply Run, Wait or Cancel to a fictional job queue.
// One Game object supplies Serial output, LED feedback, JSON and the dashboard.
#include "Secrets.h"
#include "Game.h"
#include "Panel.h"
#include "Scenarios.h"
#include "Tests.h"
#include "Web.h"

const bool RUN_STARTUP_TESTS = true;

// These are the only long-lived objects used by setup() and loop().
Game game;
Panel panel;
Web web(WIFI_SSID, WIFI_PASSWORD, game);
uint8_t facilityIndex = 0;

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
  // Every physical decision passes through the same game-rule function.
  const Result result = game.apply(action);

  // Until Node A exists, an accepted Wait changes local facility conditions.
  if (result.accepted && action == Action::WAIT) {
    facilityIndex = (facilityIndex + 1) % FACILITY_COUNT;
    game.setFacility(FACILITIES[facilityIndex]);
  }

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
  if (!game.begin(FACILITIES[0], JOBS, JOB_COUNT)) {
    Serial.println("ERROR: game did not start.");
  }

  // Start physical I/O first, then the dashboard and JSON service.
  panel.begin();
  const bool wifiConnected = web.begin();
  Serial.println(wifiConnected ? "Wi-Fi connected." : "Wi-Fi timed out.");
  if (wifiConnected) {
    Serial.println(web.address());
  }

  Serial.println("Buttons: Run=D5, Wait=D6, Cancel=D7");
  printCurrentJob();
}

void loop() {
  // Both updates are non-blocking and must run as often as possible.
  panel.update();
  web.update();

  // poll() returns true only for a newly debounced button press.
  Action action = Action::RUN;
  if (panel.poll(action)) {
    handleAction(action);
  }
}
