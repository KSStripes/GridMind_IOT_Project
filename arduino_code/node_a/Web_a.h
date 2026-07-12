// Provides Node A's Wi-Fi connection, dashboard and JSON routes.
#ifndef GRIDMIND_NODE_A_WEB_H
#define GRIDMIND_NODE_A_WEB_H

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "Facility_a.h"

class Web {
 public:
  Web(const char* ssid, const char* password, const Facility& facility);

  bool begin();
  void update();
  bool connected() const;
  IPAddress address() const;

  void handlePage();
  void handleHealth();
  void handleStatus();

 private:
  ESP8266WebServer server_;
  const char* ssid_;
  const char* password_;
  const Facility& facility_;
};

#endif
