/*
 * GridMind Node B - Web/API Adapter Implementation
 *
 * Serves a self-contained dashboard from flash and converts current Wi-Fi and
 * NodeBGame state into JSON. All handlers are read-only; physical controls and
 * NodeBGame remain authoritative for decisions.
 */

#include <Arduino.h>

#include "NodeBWebApi.h"

namespace {

// Store the complete offline dashboard in flash rather than scarce ESP8266 RAM.
const char DASHBOARD_HTML[] PROGMEM = R"GRIDMIND(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>GridMind Compute Station</title>
  <style>
    :root { color-scheme: dark; font-family: Arial, sans-serif; }
    body { margin: 0; background: #0b1220; color: #e5eefc; }
    main { width: min(92%, 760px); margin: 0 auto; padding: 24px 0 40px; }
    header { display: flex; justify-content: space-between; align-items: center; gap: 16px; }
    h1 { margin: 0; font-size: 1.6rem; }
    h2 { margin: 0 0 14px; font-size: 1.05rem; color: #a9c7f7; }
    .badge { padding: 6px 10px; border-radius: 999px; background: #6b7280; font-weight: bold; }
    .connected { background: #166534; }
    .unavailable { background: #991b1b; }
    .score { margin: 22px 0; padding: 20px; border-radius: 14px; background: #17243a; }
    .score strong { display: block; margin-top: 4px; font-size: 2.2rem; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(230px, 1fr)); gap: 16px; }
    section { padding: 18px; border: 1px solid #334766; border-radius: 14px; background: #111c2e; }
    dl { display: grid; grid-template-columns: 1fr auto; gap: 10px 18px; margin: 0; }
    dt { color: #aabbd3; }
    dd { margin: 0; font-weight: bold; text-align: right; }
    #message { min-height: 1.3em; margin-top: 18px; color: #fca5a5; }
    footer { margin-top: 20px; color: #7f94b2; font-size: .85rem; }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>GridMind Compute Station</h1>
      <span id="connection" class="badge" aria-live="polite">Connecting...</span>
    </header>

    <div class="score">
      Cumulative score
      <strong id="score">--</strong>
    </div>

    <div class="grid">
      <section>
        <h2>Grid conditions</h2>
        <dl>
          <dt>Capacity</dt><dd id="capacity">--</dd>
          <dt>Renewable availability</dt><dd id="renewable">--</dd>
          <dt>Carbon index</dt><dd id="carbon">--</dd>
          <dt>Thermal headroom</dt><dd id="thermal">--</dd>
        </dl>
      </section>

      <section>
        <h2>Workload</h2>
        <dl>
          <dt>Energy cost</dt><dd id="energy">--</dd>
          <dt>Heat cost</dt><dd id="heat">--</dd>
          <dt>Value</dt><dd id="value">--</dd>
          <dt>Type</dt><dd id="type">--</dd>
          <dt>Deadline</dt><dd id="deadline">--</dd>
          <dt>Progress</dt><dd id="progress">--</dd>
          <dt>Status</dt><dd id="workload-status">--</dd>
        </dl>
      </section>
    </div>

    <p id="message" role="alert" aria-live="polite"></p>
    <footer>Read-only view. Physical controls remain authoritative.</footer>
  </main>

  <script>
    // Update text safely without interpreting JSON values as HTML.
    function show(id, value) {
      document.getElementById(id).textContent = value;
    }

    async function refreshStatus() {
      const connection = document.getElementById('connection');
      const message = document.getElementById('message');

      try {
        // Always request the authoritative model snapshot, not a cached response.
        const response = await fetch('/api/status', { cache: 'no-store' });
        if (!response.ok) throw new Error('HTTP ' + response.status);

        const state = await response.json();
        show('score', state.cumulativeScore);
        show('capacity', state.grid.capacity + ' units');
        show('renewable', state.grid.renewablePercent + '%');
        show('carbon', state.grid.carbonIndex);
        show('thermal', state.grid.thermalHeadroom + ' units');
        show('energy', state.workload.energyCost + ' units');
        show('heat', state.workload.heatCost + ' units');
        show('value', state.workload.value + ' points');
        show('type', state.workload.flexible ? 'Flexible' : 'Urgent');
        show('deadline', state.workload.deadline + ' rounds');
        show('progress', state.workload.progressPercent + '%');
        show('workload-status', state.workload.status);

        connection.textContent = 'Connected';
        connection.className = 'badge connected';
        message.textContent = '';
      } catch (error) {
        connection.textContent = 'Unavailable';
        connection.className = 'badge unavailable';
        message.textContent = 'Status unavailable; retrying automatically.';
      }
    }

    // Load immediately, then keep physical decisions visible without reloading.
    refreshStatus();
    setInterval(refreshStatus, 1000);
  </script>
</body>
</html>
)GRIDMIND";

const char* workloadStatusName(gridmind::WorkloadStatus status) {
  switch (status) {
    case gridmind::WorkloadStatus::PENDING:
      return "pending";
    case gridmind::WorkloadStatus::COMPLETE:
      return "complete";
    case gridmind::WorkloadStatus::FAILED:
      return "failed";
  }
  return "unknown";
}

}  // namespace

NodeBWebApi::NodeBWebApi(
    const WiFiConnection& wifiConnection,
    const gridmind::NodeBGame& game)
    : server_(80),
      wifiConnection_(wifiConnection),
      game_(game) {
}

void NodeBWebApi::begin() {
  // Register only read-only GET routes during this development increment.
  server_.on("/", HTTP_GET, [this]() {
    handleDashboard();
  });
  server_.on("/api/health", HTTP_GET, [this]() {
    handleHealth();
  });
  server_.on("/api/status", HTTP_GET, [this]() {
    handleStatus();
  });
  server_.begin();
}

void NodeBWebApi::update() {
  // Service one pending client opportunity without blocking buttons or LED timing.
  server_.handleClient();
}

void NodeBWebApi::handleDashboard() {
  server_.send_P(200, PSTR("text/html; charset=utf-8"), DASHBOARD_HTML);
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

void NodeBWebApi::handleStatus() {
  if (!game_.isReady()) {
    // Avoid presenting zero-filled structures as genuine game state.
    server_.send(
        503,
        "application/json",
        "{\"error\":{\"code\":\"game_not_ready\","
        "\"message\":\"Node B game has not been initialised.\"}}");
    return;
  }

  const gridmind::GridState& grid = game_.grid();
  const gridmind::WorkloadState& workload = game_.workload();

  // Serialize one consistent read-only snapshot of the authoritative model.
  String json = "{";
  json += "\"node\":\"node-b\",";
  json += "\"gameReady\":true,";
  json += "\"cumulativeScore\":";
  json += String(game_.score());
  json += ",\"grid\":{";
  json += "\"capacity\":";
  json += String(grid.capacity);
  json += ",\"renewablePercent\":";
  json += String(grid.renewablePercent);
  json += ",\"carbonIndex\":";
  json += String(grid.carbonIndex);
  json += ",\"thermalHeadroom\":";
  json += String(grid.thermalHeadroom);
  json += "},\"workload\":{";
  json += "\"energyCost\":";
  json += String(workload.energyCost);
  json += ",\"heatCost\":";
  json += String(workload.heatCost);
  json += ",\"value\":";
  json += String(workload.value);
  json += ",\"flexible\":";
  json += workload.flexible ? "true" : "false";
  json += ",\"deadline\":";
  json += String(workload.deadline);
  json += ",\"progressPercent\":";
  json += String(workload.progress);
  json += ",\"status\":\"";
  json += workloadStatusName(workload.status);
  json += "\"}}";

  server_.send(200, "application/json", json);
}
