// Web.h
// Provides Node B's Wi-Fi connection, dashboard and read-only JSON routes.
// It reads the same Game object used by the physical controls.
#ifndef GRIDMIND_WEB_H
#define GRIDMIND_WEB_H

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "Game.h"

class Web {
 public:
  Web(const char* ssid, const char* password, const Game& game);

  bool begin();
  void update();
  bool connected() const;
  IPAddress address() const;

 private:
  ESP8266WebServer server_;
  const char* ssid_;
  const char* password_;
  const Game& game_;

  void handlePage();
  void handleHealth();
  void handleStatus();
};

#endif
