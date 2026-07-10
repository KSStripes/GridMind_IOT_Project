// Web.cpp
// Connects to Wi-Fi, registers the three GET routes and builds compact JSON.
// The web layer displays state but does not own queue or money rules.
#include <Arduino.h>

#include "Page.h"
#include "Web.h"

namespace {

void addJsonString(String& json, const char* value) {
  // Escape the two characters that could break a JSON string value.
  json += '"';
  while (value != 0 && *value != '\0') {
    if (*value == '"' || *value == '\\') {
      json += '\\';
    }
    json += *value;
    value += 1;
  }
  json += '"';
}

}  // namespace

Web::Web(const char* ssid, const char* password, const Game& game)
    : server_(80), ssid_(ssid), password_(password), game_(game) {
}

bool Web::begin() {
  // Join as a station and allow the ESP8266 to reconnect after signal loss.
  WiFi.mode(WIFI_STA);
  WiFi.hostname("gridmind-node-b");
  WiFi.begin(ssid_, password_);
  WiFi.setAutoReconnect(true);

  // Do not hold setup forever when the configured network is unavailable.
  const unsigned long startedAt = millis();
  while (!connected() && millis() - startedAt < 15000) {
    delay(500);
  }

  // Register the minimum coursework routes, then start the HTTP server.
  server_.on("/", HTTP_GET, [this]() { handlePage(); });
  server_.on("/api/health", HTTP_GET, [this]() { handleHealth(); });
  server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server_.begin();
  return connected();
}

void Web::update() {
  // Called on every loop so browser requests remain responsive.
  server_.handleClient();
}

bool Web::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress Web::address() const {
  return WiFi.localIP();
}

void Web::handlePage() {
  server_.send_P(200, PSTR("text/html; charset=utf-8"), PAGE_HTML);
}

void Web::handleHealth() {
  // Health is deliberately independent of the game queue.
  String json = "{\"node\":\"node-b\",\"role\":\"compute-station\",";
  json += "\"status\":\"";
  json += connected() ? "ok" : "degraded";
  json += "\",\"wifiConnected\":";
  json += connected() ? "true" : "false";
  json += ",\"uptimeMs\":";
  json += String(millis());
  json += "}";
  server_.send(200, "application/json", json);
}

void Web::handleStatus() {
  // Return a controlled error instead of incomplete state before game setup.
  if (!game_.isReady()) {
    server_.send(
        503,
        "application/json",
        "{\"error\":{\"code\":\"game_not_ready\","
        "\"message\":\"Node B game has not been initialised.\"}}");
    return;
  }

  // Read one authoritative snapshot from Game and serialize each section.
  const Facility& facility = game_.facility();
  const Job* job = game_.currentJob();
  String json;
  json.reserve(650);
  json = "{\"facility\":{";
  json += "\"capacityAvailable\":";
  json += facility.capacityAvailable ? "true" : "false";
  json += ",\"powerAvailable\":";
  json += facility.powerAvailable ? "true" : "false";
  json += ",\"tempC\":";
  json += String(facility.tempC);
  json += ",\"tempLimitC\":";
  json += String(facility.tempLimitC);
  json += "},\"job\":";

  // The current job becomes null after the final job leaves the queue.
  if (job == 0) {
    json += "null";
  } else {
    json += "{\"name\":";
    addJsonString(json, job->name);
    json += ",\"tempRiseC\":";
    json += String(job->tempRiseC);
    json += ",\"valueCents\":";
    json += String(job->valueCents);
    json += ",\"penaltyCents\":";
    json += String(job->penaltyCents);
    json += ",\"canWait\":";
    json += job->canWait ? "true" : "false";
    json += "}";
  }

  json += ",\"queueSize\":";
  json += String(game_.queueSize());
  json += ",\"result\":";
  // No result exists until the learner attempts the first action.
  if (!game_.hasResult()) {
    json += "null";
  } else {
    // result.job names the processed job; top-level job is the new queue front.
    const Result& result = game_.lastResult();
    json += "{\"job\":";
    if (result.jobName == 0) {
      json += "null";
    } else {
      addJsonString(json, result.jobName);
    }
    json += ",\"action\":\"";
    json += actionName(result.action);
    json += "\",\"reason\":\"";
    json += reasonName(result.reason);
    json += "\",\"deltaCents\":";
    json += String(result.deltaCents);
    json += ",\"totalCents\":";
    json += String(result.totalCents);
    json += "}";
  }
  json += "}";

  server_.send(200, "application/json", json);
}
