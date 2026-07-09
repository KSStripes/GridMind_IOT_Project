/*
 * GridMind Node B - Web/API Adapter Interface
 *
 * Owns the ESP8266 HTTP server and exposes the read-only dashboard, health,
 * and game-status routes. It reads existing Wi-Fi and game objects without
 * duplicating networking policy, state transitions, or scoring rules.
 */

#ifndef GRIDMIND_NODE_B_WEB_API_H
#define GRIDMIND_NODE_B_WEB_API_H

#include <ESP8266WebServer.h>

#include "NodeBGame.h"
#include "WiFiConnection.h"

class NodeBWebApi {
 public:
  NodeBWebApi(
      const WiFiConnection& wifiConnection,
      const gridmind::NodeBGame& game);

  void begin();
  void update();

 private:
  ESP8266WebServer server_;
  // Non-owning, read-only references to the application's authoritative state.
  const WiFiConnection& wifiConnection_;
  const gridmind::NodeBGame& game_;

  void handleDashboard();
  void handleHealth();
  void handleStatus();
};

#endif
