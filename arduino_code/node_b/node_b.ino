/*
 * GridMind Compute Station (Node B)
 *
 * The Arduino entry point only creates and runs the application. Game rules,
 * startup tests, hardware adapters, and networking live in separate modules.
 */

#include "Secrets.h"
#include "NodeBApplication.h"

const bool RUN_STARTUP_TESTS = true;

NodeBApplication application(
    WIFI_SSID,
    WIFI_PASSWORD,
    RUN_STARTUP_TESTS);

void setup() {
  application.begin();
}

void loop() {
  application.update();
}
