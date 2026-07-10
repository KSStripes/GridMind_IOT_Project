/*
 * GridMind Node B - Web/API Adapter Implementation
 *
 * Serves the dashboard asset and converts current Wi-Fi and NodeBGame state
 * into JSON. All handlers are read-only; physical controls and NodeBGame remain
 * authoritative for decisions.
 */

#include <Arduino.h>

#include "NodeBDashboard.h"
#include "NodeBWebApi.h"

namespace {

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
  server_.send_P(
      200,
      PSTR("text/html; charset=utf-8"),
      gridmind::NODE_B_DASHBOARD_HTML);
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
