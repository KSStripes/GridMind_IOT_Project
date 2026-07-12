// Web_b.h
// Provides Node B's server routes and polls Node A's read-only status route.
// Valid remote conditions update the same Game used by physical controls.
#ifndef GRIDMIND_NODE_B_WEB_H
#define GRIDMIND_NODE_B_WEB_H

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "Game_b.h"

class Web {
 public:
  Web(const char* ssid, const char* password,
      const char* nodeAStatusUrl, Game& game);

  bool begin();
  void update();
  bool connected() const;
  IPAddress address() const;

  // Kept public so startup tests can exercise valid and invalid payloads.
  static bool parseFacilityStatus(
      const String& json, uint8_t& scenarioId, Facility& facility);

  void handlePage();
  void handleHealth();
  void handleStatus();

 private:
  ESP8266WebServer server_;
  const char* ssid_;
  const char* password_;
  const char* nodeAStatusUrl_;
  Game& game_;
  unsigned long lastPollAt_;
  unsigned long lastValidAt_;
  uint8_t scenarioId_;
  bool hasScenario_;

  // Fetches one Node A response without adding a new Node B route.
  void pollFacility();
};

#endif
