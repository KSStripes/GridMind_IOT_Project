#ifndef GRIDMIND_NODE_B_WEB_API_H
#define GRIDMIND_NODE_B_WEB_API_H

#include <ESP8266WebServer.h>

#include "WiFiConnection.h"

class NodeBWebApi {
 public:
  explicit NodeBWebApi(const WiFiConnection& wifiConnection);

  void begin();
  void update();

 private:
  ESP8266WebServer server_;
  const WiFiConnection& wifiConnection_;

  void handleHealth();
};

#endif
