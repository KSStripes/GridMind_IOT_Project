/*
  Connects Node A to Wi-Fi and serves its dashboard and JSON responses.
  The web output uses the same facility state as the physical LEDs.
  Dependencies: Arduino core and Web_a.h.
*/
#include <Arduino.h>

#include "Web_a.h"

// ESP8266WebServer callbacks are plain functions, so they forward to this object.
static Web* g_web = 0;

static void onPage()   { g_web->handlePage(); }
static void onHealth() { g_web->handleHealth(); }
static void onStatus() { g_web->handleStatus(); }

Web::Web(
    const char* ssid,
    const char* password,
    const Facility& facility)
    : server_(80),
      ssid_(ssid),
      password_(password),
      facility_(facility) {
}

bool Web::begin() {
  g_web = this;

  WiFi.mode(WIFI_STA);
  WiFi.hostname("gridmind-node-a");
  WiFi.begin(ssid_, password_);
  WiFi.setAutoReconnect(true);

  // Limit setup waiting; automatic reconnection continues after a timeout.
  const unsigned long startedAt = millis();
  while (!connected() && millis() - startedAt < 15000) {
    delay(500);
  }

  // Keep the public surface to the dashboard and two documented JSON routes.
  server_.on("/",           HTTP_GET, onPage);
  server_.on("/api/health", HTTP_GET, onHealth);
  server_.on("/api/status", HTTP_GET, onStatus);
  server_.begin();
  return connected();
}

void Web::update() {
  server_.handleClient();
}

bool Web::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress Web::address() const {
  return WiFi.localIP();
}

void Web::handlePage() {
  if (!facility_.isReady()) {
    server_.send(503, "text/html",
      "<html><body><h1>GridMind Node A</h1><p>Starting up...</p></body></html>");
    return;
  }

  const FacilityScenario& scenario = facility_.current();
  String html;
  html.reserve(3000);
  html += F("<!doctype html><html><head><meta charset='utf-8'>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  // HTML meta refresh updates the page without JavaScript or a web library.
  html += F("<meta http-equiv='refresh' content='2'>");
  html += F("<title>GridMind Node A</title><style>");
  html += F("body{font-family:Arial,sans-serif;background:#f2f2f2;color:#222;margin:0;padding:20px}");
  html += F("main{max-width:800px;margin:auto}header{display:flex;justify-content:space-between;align-items:center;gap:12px}");
  html += F("h1{font-size:1.7rem}.status{background:#dff3e4;padding:7px 10px;border-radius:6px;font-weight:bold}");
  html += F("section{background:#fff;border:1px solid #ccc;padding:16px;margin:14px 0}");
  html += F(".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}");
  html += F(".item{border-left:4px solid #3478c0;padding:6px 10px}.label{color:#555;font-size:.85rem}.value{font-size:1.2rem;font-weight:bold;margin-top:5px}");
  html += F("footer{color:#555;margin-top:14px}a{color:#1557a0}</style></head><body><main>");
  html += F("<header><h1>GridMind Node A</h1><span class='status'>");
  html += connected() ? "Wi-Fi connected" : "Wi-Fi degraded";
  html += F("</span></header><section><div class='label'>Current simulated scenario</div><div class='value'>");
  html += String(facility_.number());
  html += F(" of ");
  html += String(facility_.count());
  html += F(": ");
  html += scenario.name;
  html += F("</div></section><section><h2>Facility conditions</h2><div class='grid'>");
  html += F("<div class='item'><div class='label'>Compute capacity</div><div class='value'>");
  html += scenario.capacityAvailable ? "Available" : "Unavailable";
  html += F("</div></div><div class='item'><div class='label'>Electricity</div><div class='value'>");
  html += scenario.powerAvailable ? "Available" : "Unavailable";
  html += F("</div></div><div class='item'><div class='label'>Simulated temperature</div><div class='value'>");
  html += String(scenario.tempC);
  html += F(" &deg;C</div><div>Safe limit: ");
  html += String(scenario.tempLimitC);
  html += F(" &deg;C</div></div></div></section>");
  html += F("<footer>Press the physical Scenario button to advance.<br>API: <a href='/api/status'>status</a> | <a href='/api/health'>health</a></footer>");
  html += F("</main></body></html>");

  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "text/html", html);
}

void Web::handleHealth() {
  String json = F("{\"node\":\"node-a\",\"role\":\"facility-station\",\"status\":\"");
  json += connected() ? "ok" : "degraded";
  json += F("\",\"wifiConnected\":");
  json += connected() ? "true" : "false";
  json += F(",\"uptimeMs\":");
  json += String(millis());
  json += '}';
  server_.send(200, "application/json", json);
}

void Web::handleStatus() {
  if (!facility_.isReady()) {
    server_.send(503, "application/json",
      "{\"error\":\"facility not ready\"}");
    return;
  }

  const FacilityScenario& scenario = facility_.current();
  String json = F("{\"node\":\"node-a\",\"scenarioId\":");
  json += String(facility_.number());
  json += F(",\"scenarioName\":\"");
  json += scenario.name;
  json += F("\",\"capacityAvailable\":");
  json += scenario.capacityAvailable ? "true" : "false";
  json += F(",\"powerAvailable\":");
  json += scenario.powerAvailable ? "true" : "false";
  json += F(",\"tempC\":");
  json += String(scenario.tempC);
  json += F(",\"tempLimitC\":");
  json += String(scenario.tempLimitC);
  json += F(",\"temperatureWarning\":");
  json += facility_.temperatureWarning() ? "true" : "false";
  json += '}';
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "application/json", json);
}
