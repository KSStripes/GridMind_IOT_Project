// GridMind Node B network layer.
// Serves Node B routes and polls validated facility data from Node A.
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

#include "Web_b.h"

// Poll often enough for the dashboards while allowing brief packet loss.
static const unsigned long POLL_INTERVAL_MS = 2000;
static const unsigned long FACILITY_STALE_MS = 6000;
static const uint16_t HTTP_TIMEOUT_MS = 750;

static Web* g_web = 0;

static void onPage()   { g_web->handlePage(); }
static void onHealth() { g_web->handleHealth(); }
static void onStatus() { g_web->handleStatus(); }

static bool isSpace(char value) {
  return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}

static int valueStart(const String& json, const char* key) {
  String token = "\"";
  token += key;
  token += "\"";

  int position = json.indexOf(token);
  if (position < 0) {
    return -1;
  }
  position += token.length();
  while (position < static_cast<int>(json.length()) &&
         isSpace(json[position])) {
    position += 1;
  }
  if (position >= static_cast<int>(json.length()) ||
      json[position] != ':') {
    return -1;
  }
  position += 1;
  while (position < static_cast<int>(json.length()) &&
         isSpace(json[position])) {
    position += 1;
  }
  return position;
}

static bool validValueEnd(const String& json, int position) {
  while (position < static_cast<int>(json.length()) &&
         isSpace(json[position])) {
    position += 1;
  }
  return position == static_cast<int>(json.length()) ||
         json[position] == ',' || json[position] == '}';
}

static bool readBool(
    const String& json, const char* key, bool& value) {
  const int start = valueStart(json, key);
  if (start < 0) {
    return false;
  }
  if (json.substring(start, start + 4) == "true" &&
      validValueEnd(json, start + 4)) {
    value = true;
    return true;
  }
  if (json.substring(start, start + 5) == "false" &&
      validValueEnd(json, start + 5)) {
    value = false;
    return true;
  }
  return false;
}

static bool readLong(
    const String& json, const char* key, long& value) {
  const int start = valueStart(json, key);
  if (start < 0) {
    return false;
  }

  int end = start;
  if (end < static_cast<int>(json.length()) && json[end] == '-') {
    end += 1;
  }
  const int firstDigit = end;
  while (end < static_cast<int>(json.length()) &&
         json[end] >= '0' && json[end] <= '9') {
    end += 1;
  }
  if (end == firstDigit || !validValueEnd(json, end)) {
    return false;
  }

  value = json.substring(start, end).toInt();
  return true;
}

Web::Web(
    const char* ssid,
    const char* password,
    const char* nodeAStatusUrl,
    Game& game)
    : server_(80),
      ssid_(ssid),
      password_(password),
      nodeAStatusUrl_(nodeAStatusUrl),
      game_(game),
      lastPollAt_(0),
      lastValidAt_(0),
      scenarioId_(0),
      hasScenario_(false) {
}

bool Web::begin() {
  g_web = this;
  // The placeholder facility must not permit Run before the first poll.
  game_.setFacilityAvailable(false);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("gridmind-node-b");
  WiFi.begin(ssid_, password_);
  WiFi.setAutoReconnect(true);

  const unsigned long startedAt = millis();
  while (!connected() && millis() - startedAt < 15000) {
    delay(500);
  }

  server_.on("/",           HTTP_GET, onPage);
  server_.on("/api/health", HTTP_GET, onHealth);
  server_.on("/api/status", HTTP_GET, onStatus);
  server_.begin();
  return connected();
}

void Web::update() {
  server_.handleClient();

  // Polling and stale checks share loop() with buttons and LED timing.
  if (millis() - lastPollAt_ >= POLL_INTERVAL_MS) {
    pollFacility();
  }
  if (hasScenario_ &&
      millis() - lastValidAt_ > FACILITY_STALE_MS) {
    game_.setFacilityAvailable(false);
  }
}

bool Web::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress Web::address() const {
  return WiFi.localIP();
}

bool Web::parseFacilityStatus(
    const String& json,
    uint8_t& scenarioId,
    Facility& facility) {
  long parsedScenarioId = 0;
  long tempC = 0;
  long tempLimitC = 0;
  bool capacityAvailable = false;
  bool powerAvailable = false;

  // Reject missing fields and wrong primitive types before touching Game.
  if (!readLong(json, "scenarioId", parsedScenarioId) ||
      !readBool(json, "capacityAvailable", capacityAvailable) ||
      !readBool(json, "powerAvailable", powerAvailable) ||
      !readLong(json, "tempC", tempC) ||
      !readLong(json, "tempLimitC", tempLimitC)) {
    return false;
  }
  // Match Game's accepted temperature range and a nonzero scenario ID.
  if (parsedScenarioId < 1 || parsedScenarioId > 255 ||
      tempC < 0 || tempC > 60 ||
      tempLimitC < 0 || tempLimitC > 60) {
    return false;
  }

  scenarioId = static_cast<uint8_t>(parsedScenarioId);
  facility.capacityAvailable = capacityAvailable;
  facility.powerAvailable = powerAvailable;
  facility.tempC = static_cast<int16_t>(tempC);
  facility.tempLimitC = static_cast<int16_t>(tempLimitC);
  return true;
}

void Web::pollFacility() {
  lastPollAt_ = millis();
  if (!connected()) {
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  if (!http.begin(client, nodeAStatusUrl_)) {
    game_.setFacilityAvailable(false);
    return;
  }

  const int statusCode = http.GET();
  if (statusCode == HTTP_CODE_OK) {
    const String payload = http.getString();
    uint8_t receivedScenarioId = 0;
    Facility receivedFacility;

    if (parseFacilityStatus(
            payload, receivedScenarioId, receivedFacility)) {
      // Reapply values only when the physical Scenario button changes Node A.
      // This preserves Node B's temperature rise after a successful Run.
      const bool scenarioChanged =
          !hasScenario_ || receivedScenarioId != scenarioId_;
      if (scenarioChanged && !game_.setFacility(receivedFacility)) {
        game_.setFacilityAvailable(false);
        http.end();
        return;
      }

      const bool wasAvailable = game_.facilityAvailable();
      scenarioId_ = receivedScenarioId;
      hasScenario_ = true;
      // Every valid response refreshes link health, even in the same scenario.
      lastValidAt_ = millis();
      game_.setFacilityAvailable(true);

      if (!wasAvailable) {
        Serial.println("Node A link: live");
      }
      if (scenarioChanged) {
        Serial.print("Node A scenario received: ");
        Serial.println(scenarioId_);
      }
    } else {
      // Keep the last valid values visible but disable Run immediately.
      game_.setFacilityAvailable(false);
      Serial.println("Node A data rejected: invalid JSON fields");
    }
  }
  http.end();
}

void Web::handlePage() {
  if (!game_.isReady()) {
    server_.send(200, "text/html",
      "<html><body><h1>GridMind Node B</h1><p>Starting up...</p></body></html>");
    return;
  }

  const Facility& f = game_.facility();
  const Job* job = game_.currentJob();

  String html;
  html.reserve(4200);
  html += "<!doctype html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>GridMind Node B</title><style>";
  html += "body{font-family:Arial,sans-serif;background:#f2f2f2;color:#222;margin:0;padding:20px}";
  html += "main{max-width:800px;margin:auto}header{display:flex;justify-content:space-between;align-items:center;gap:12px}";
  html += "h1{font-size:1.7rem}.status{padding:7px 10px;border-radius:6px;font-weight:bold}.live{background:#dff3e4}.offline{background:#ffe2df}";
  html += "section{background:#fff;border:1px solid #ccc;padding:16px;margin:14px 0}";
  html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}";
  html += ".item{border-left:4px solid #3478c0;padding:6px 10px}.label{color:#555;font-size:.85rem}.value{font-size:1.2rem;font-weight:bold;margin-top:5px}";
  html += ".warning{background:#fff2cc;padding:10px;border-left:4px solid #d6a400}footer{color:#555;margin-top:14px}a{color:#1557a0}</style>";
  html += "</head><body><main><header><h1>GridMind Node B</h1>";
  // Make the inter-node dependency visible to the learner and tester.
  html += game_.facilityAvailable()
      ? "<span class='status live'>Node A: Live</span>"
      : "<span class='status offline'>Node A: Unavailable</span>";
  html += "</header>";
  if (!game_.facilityAvailable()) {
    html += "<p class='warning'><strong>Run is disabled</strong> until valid facility data returns.</p>";
  }

  html += "<section><h2>Facility conditions</h2><div class='grid'>";
  html += "<div class='item'><div class='label'>Compute capacity</div><div class='value'>";
  html += f.capacityAvailable ? "Available" : "Unavailable";
  html += "</div></div><div class='item'><div class='label'>Electricity</div><div class='value'>";
  html += f.powerAvailable ? "Available" : "Unavailable";
  html += "</div></div><div class='item'><div class='label'>Temperature check</div><div class='value'>";
  if (job == 0) {
    html += String(f.tempC);
    html += " &deg;C";
  } else {
    html += String(f.tempC);
    html += " + ";
    html += String(job->tempRiseC);
    html += " = ";
    html += String(f.tempC + job->tempRiseC);
    html += " &deg;C";
  }
  html += "</div><div>Safe limit: ";
  html += String(f.tempLimitC);
  html += " &deg;C</div></div></div></section>";

  html += "<section><h2>Current contract</h2>";
  if (job == 0) {
    html += "<p>No contracts remaining.</p>";
  } else {
    html += "<div class='value'>";
    html += job->name;
    html += "</div><div class='grid' style='margin-top:14px'>";
    html += "<div class='item'><div class='label'>Completion value</div><div class='value'>EUR ";
    html += String(job->valueCents / 100);
    html += "</div></div><div class='item'><div class='label'>Cancellation penalty</div><div class='value'>EUR ";
    html += String(job->penaltyCents / 100);
    html += "</div></div><div class='item'><div class='label'>Wait available</div><div class='value'>";
    html += job->canWait ? "Yes" : "No";
    html += "</div></div><div class='item'><div class='label'>Queue size</div><div class='value'>";
    html += String(game_.queueSize());
    html += "</div></div></div>";
  }
  html += "</section>";

  if (game_.hasResult()) {
    const Result& r = game_.lastResult();
    String readableReason = reasonName(r.reason);
    readableReason.replace('_', ' ');
    html += "<section><h2>Last decision</h2><div class='value'>";
    html += actionName(r.action);
    if (r.jobName != 0) {
      html += " - ";
      html += r.jobName;
    }
    html += "</div><div class='grid' style='margin-top:14px'>";
    html += "<div class='item'><div class='label'>Result</div><div class='value'>";
    html += readableReason;
    html += "</div></div><div class='item'><div class='label'>Money change</div><div class='value'>EUR ";
    html += String(r.deltaCents / 100);
    html += "</div></div></div></section>";
  }

  html += "<section><div class='label'>Running total</div><div class='value'>EUR ";
  html += String(game_.totalCents() / 100);
  html += "</div></section>";
  html += "<footer>Use the physical Run, Wait and Cancel buttons.<br>API: <a href='/api/status'>status</a> | <a href='/api/health'>health</a></footer>";
  html += "</main></body></html>";
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "text/html", html);
}

void Web::handleHealth() {
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

  const Facility& facility = game_.facility();
  const Job* job = game_.currentJob();

  // Consumers can distinguish live data from the retained last-valid values.
  String json = "{\"facilityDataAvailable\":";
  json += game_.facilityAvailable() ? "true" : "false";
  json += ",\"facility\":{";
  json += "\"capacityAvailable\":";
  json += facility.capacityAvailable ? "true" : "false";
  json += ",\"powerAvailable\":";
  json += facility.powerAvailable ? "true" : "false";
  json += ",\"tempC\":";
  json += String(facility.tempC);
  json += ",\"tempLimitC\":";
  json += String(facility.tempLimitC);
  json += "},\"job\":";

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
