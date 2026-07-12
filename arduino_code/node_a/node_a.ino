// GridMind Node A - facility station
// Coordinates scenario state, the physical panel and the web service.
#include "Secrets_a.h"
#include "Facility_a.h"
#include "Panel_a.h"
#include "Scenarios_a.h"
#include "Tests_a.h"
#include "Web_a.h"

const bool RUN_STARTUP_TESTS = true;

Facility facility;
Panel panel;
Web web(WIFI_SSID, WIFI_PASSWORD, facility);

void printScenario() {
  const FacilityScenario& scenario = facility.current();
  Serial.println();
  Serial.print("Scenario ");
  Serial.print(facility.number());
  Serial.print('/');
  Serial.print(facility.count());
  Serial.print(": ");
  Serial.println(scenario.name);
  Serial.print("Capacity: ");
  Serial.println(scenario.capacityAvailable ? "available" : "unavailable");
  Serial.print("Electricity: ");
  Serial.println(scenario.powerAvailable ? "available" : "unavailable");
  Serial.print("Simulated temperature: ");
  Serial.print(scenario.tempC);
  Serial.print(" C (limit ");
  Serial.print(scenario.tempLimitC);
  Serial.println(" C)");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Test the model before creating the real scenario state.
  if (RUN_STARTUP_TESTS) {
    const uint8_t passed = Tests::run();
    Serial.print("RESULT: ");
    Serial.print(passed);
    Serial.print('/');
    Serial.print(Tests::COUNT);
    Serial.println(passed == Tests::COUNT ? " PASS" : " FAIL");
  }

  // Start shared state first, then the physical and network layers that read it.
  if (!facility.begin(SCENARIOS, SCENARIO_COUNT)) {
    Serial.println("ERROR: facility scenarios did not start.");
  }

  panel.begin();
  panel.show(facility.current(), facility.temperatureWarning());
  printScenario();

  const bool wifiConnected = web.begin();
  Serial.println(wifiConnected ? "Wi-Fi connected." : "Wi-Fi timed out.");
  if (wifiConnected) {
    Serial.print("Dashboard: http://");
    Serial.println(web.address());
  }
}

void loop() {
  // Both operations are non-blocking so HTTP and button input stay responsive.
  web.update();

  if (panel.pollScenarioButton()) {
    facility.advance();
    panel.show(facility.current(), facility.temperatureWarning());
    printScenario();
  }
}
