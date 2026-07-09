#include <Arduino.h>

#include "NodeBWebApi.h"

NodeBWebApi::NodeBWebApi(const WiFiConnection& wifiConnection)
    : server_(80),
      wifiConnection_(wifiConnection) {
}

void NodeBWebApi::begin() {
  server_.on("/api/health", HTTP_GET, [this]() {
    handleHealth();
  });
  server_.begin();
}

void NodeBWebApi::update() {
  server_.handleClient();
}

void NodeBWebApi::handleHealth() {
  const bool connected = wifiConnection_.isConnected();

  String json = "{";
  json += "\"node\":\"node-b\",";
  json += "\"role\":\"compute-station\",";
  json += "\"status\":\"";
  json += connected ? "ok" : "degraded";
  json += "\",";
  json += "\"wifiConnected\":";
  json += connected ? "true" : "false";
  json += ",\"uptimeMs\":";
  json += String(millis());
  json += "}";

  server_.send(200, "application/json", json);
}
