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

const char* actionName(gridmind::Action action) {
  switch (action) {
    case gridmind::Action::RUN:
      return "run";
    case gridmind::Action::DEFER:
      return "defer";
    case gridmind::Action::REDUCE:
      return "reduce";
  }
  return "unknown";
}

void appendJsonString(String& json, const char* value) {
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
  const gridmind::ScenarioRates& rates = game_.rates();

  // Serialize one consistent read-only snapshot of the authoritative model.
  String json;
  json.reserve(1400);
  json = "{";
  json += "\"node\":\"node-b\",";
  json += "\"gameReady\":true,";
  json += "\"totalCents\":";
  json += String(game_.totalCents());
  json += ",\"grid\":{";
  json += "\"capacityKw\":";
  json += String(grid.capacityKw);
  json += ",\"renewablePct\":";
  json += String(grid.renewablePct);
  json += ",\"co2eGPerKwh\":";
  json += String(grid.co2eGPerKwh);
  json += ",\"tempC\":";
  json += String(grid.tempC);
  json += ",\"tempLimitC\":";
  json += String(grid.tempLimitC);
  json += ",\"nowMin\":";
  json += String(grid.nowMin);
  json += "},\"workload\":{";
  json += "\"customer\":";
  appendJsonString(json, workload.customer);
  json += ",\"job\":";
  appendJsonString(json, workload.job);
  json += ",\"energyKwh\":";
  json += String(workload.energyKwh);
  json += ",\"durationMin\":";
  json += String(workload.durationMin);
  json += ",\"demandKw\":";
  json += String(game_.currentDemandKw());
  json += ",\"tempRiseC\":";
  json += String(workload.tempRiseC);
  json += ",\"contractEur\":";
  json += String(workload.contractEur);
  json += ",\"flexible\":";
  json += workload.flexible ? "true" : "false";
  json += ",\"deadlineMin\":";
  json += String(workload.deadlineMin);
  json += ",\"slackMin\":";
  json += String(game_.slackMin());
  json += ",\"progressPct\":";
  json += String(workload.progressPct);
  json += ",\"status\":\"";
  json += workloadStatusName(workload.status);
  json += "\"},\"rates\":{";
  json += "\"co2eEurPerT\":";
  json += String(rates.co2eEurPerT);
  json += ",\"overloadEurPerMw\":";
  json += String(rates.overloadEurPerMw);
  json += ",\"coolingEurPerC\":";
  json += String(rates.coolingEurPerC);
  json += ",\"flexLateEur\":";
  json += String(rates.flexLateEur);
  json += ",\"urgentLateEur\":";
  json += String(rates.urgentLateEur);
  json += ",\"reducePct\":";
  json += String(rates.reducePct);
  json += ",\"deferMin\":";
  json += String(rates.deferMin);
  json += "},\"lastDecision\":";

  if (!game_.hasResult()) {
    json += "null";
  } else {
    const gridmind::DecisionResult& result = game_.lastResult();
    json += "{";
    json += "\"action\":\"";
    json += actionName(result.action);
    json += "\",\"energyUsedKwh\":";
    json += String(result.energyUsedKwh);
    json += ",\"demandKw\":";
    json += String(result.demandKw);
    json += ",\"overloadMw\":";
    json += String(result.overloadMw);
    json += ",\"emissionsKg\":";
    json += String(result.emissionsKg);
    json += ",\"tempRiseUsedC\":";
    json += String(result.tempRiseUsedC);
    json += ",\"projectedTempC\":";
    json += String(result.projectedTempC);
    json += ",\"tempLimitC\":";
    json += String(result.tempLimitC);
    json += ",\"excessTempC\":";
    json += String(result.excessTempC);
    json += ",\"deliveredCents\":";
    json += String(result.deliveredCents);
    json += ",\"co2CostCents\":";
    json += String(result.co2CostCents);
    json += ",\"overloadCostCents\":";
    json += String(result.overloadCostCents);
    json += ",\"coolingCostCents\":";
    json += String(result.coolingCostCents);
    json += ",\"lateCostCents\":";
    json += String(result.lateCostCents);
    json += ",\"netCents\":";
    json += String(result.netCents);
    json += ",\"totalCents\":";
    json += String(result.totalCents);
    json += ",\"nowMinAfter\":";
    json += String(result.nowMinAfter);
    json += ",\"completionMin\":";
    json += String(result.completionMin);
    json += ",\"slackMinAfter\":";
    json += String(result.slackMinAfter);
    json += ",\"statusAfter\":\"";
    json += workloadStatusName(result.statusAfter);
    json += "\"}";
  }

  json += "}";

  server_.send(200, "application/json", json);
}
