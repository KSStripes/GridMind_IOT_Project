// Web.cpp
// Connects to Wi-Fi, registers three GET routes and builds JSON and HTML.
// The web layer displays state but does not own queue or money rules.
#include <Arduino.h>

#include "Web.h"

// Global pointer so plain functions can forward calls to the Web instance.
// The ESP8266WebServer requires plain (non-member) callback functions.
static Web* g_web = 0;

static void onPage()   { g_web->handlePage(); }
static void onHealth() { g_web->handleHealth(); }
static void onStatus() { g_web->handleStatus(); }

Web::Web(const char* ssid, const char* password, const Game& game)
    : server_(80), ssid_(ssid), password_(password), game_(game) {
}

bool Web::begin() {
  g_web = this;

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
  server_.on("/",           HTTP_GET, onPage);
  server_.on("/api/health", HTTP_GET, onHealth);
  server_.on("/api/status", HTTP_GET, onStatus);
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
  // Build a simple HTML page with current game values baked in.
  // The meta-refresh tag reloads the page every 2 seconds automatically
  // so no JavaScript is needed.
  if (!game_.isReady()) {
    server_.send(200, "text/html",
      "<html><body><h1>GridMind Node B</h1><p>Starting up...</p></body></html>");
    return;
  }

  const Facility& f = game_.facility();
  const Job* job = game_.currentJob();

  String html = "<!doctype html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>GridMind Node B</title>";
  html += "</head><body>";
  html += "<h1>GridMind Compute Station</h1>";

  html += "<h2>Facility</h2>";
  html += "<p>Capacity: ";
  html += f.capacityAvailable ? "Available" : "Unavailable";
  html += "</p>";
  html += "<p>Power: ";
  html += f.powerAvailable ? "Available" : "Unavailable";
  html += "</p>";
  html += "<p>Temperature: ";
  html += String(f.tempC);
  html += " C (limit ";
  html += String(f.tempLimitC);
  html += " C)</p>";

  html += "<h2>Current Job</h2>";
  if (job == 0) {
    html += "<p>No jobs remaining</p>";
  } else {
    html += "<p>Job: ";
    html += job->name;
    html += "</p>";
    html += "<p>Value: EUR ";
    html += String(job->valueCents / 100);
    html += "</p>";
    html += "<p>Penalty: EUR ";
    html += String(job->penaltyCents / 100);
    html += "</p>";
    html += "<p>Temp rise: +";
    html += String(job->tempRiseC);
    html += " C</p>";
    html += "<p>Wait available: ";
    html += job->canWait ? "Yes" : "No";
    html += "</p>";
  }
  html += "<p>Queue size: ";
  html += String(game_.queueSize());
  html += "</p>";

  if (game_.hasResult()) {
    const Result& r = game_.lastResult();
    html += "<h2>Last Action</h2>";
    html += "<p>Action: ";
    html += actionName(r.action);
    if (r.jobName != 0) {
      html += " - ";
      html += r.jobName;
    }
    html += "</p>";
    html += "<p>Result: ";
    html += reasonName(r.reason);
    html += "</p>";
    html += "<p>Money change: EUR ";
    html += String(r.deltaCents / 100);
    html += "</p>";
  }

  html += "<h2>Total: EUR ";
  html += String(game_.totalCents() / 100);
  html += "</h2>";
  html += "</body></html>";

  server_.send(200, "text/html", html);
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
  if (!game_.isReady()) {
    server_.send(503, "application/json",
      "{\"error\":\"game not ready\"}");
    return;
  }

  // Read one snapshot from Game and build JSON for each section.
  const Facility& facility = game_.facility();
  const Job* job = game_.currentJob();

  String json = "{\"facility\":{";
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
    json += "{\"name\":\"";
    json += job->name;
    json += "\",\"tempRiseC\":";
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
    const Result& result = game_.lastResult();
    json += "{\"job\":";
    if (result.jobName == 0) {
      json += "null";
    } else {
      json += "\"";
      json += result.jobName;
      json += "\"";
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
