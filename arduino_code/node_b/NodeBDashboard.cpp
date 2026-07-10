/*
 * GridMind Node B - Dashboard Asset Implementation
 *
 * Stores the self-contained dashboard in ESP8266 flash. The page presents the
 * API snapshot but owns no scenario, decision, or monetary rules.
 */

#include "NodeBDashboard.h"

namespace gridmind {

const char NODE_B_DASHBOARD_HTML[] PROGMEM = R"GRIDMIND(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>GridMind Compute Station</title>
  <style>
    :root { color-scheme: dark; font-family: Arial, sans-serif; }
    body { margin: 0; background: #0b1220; color: #e5eefc; }
    main { width: min(92%, 980px); margin: 0 auto; padding: 24px 0 40px; }
    header { display: flex; justify-content: space-between; align-items: center; gap: 16px; }
    h1 { margin: 0; font-size: 1.6rem; }
    h2 { margin: 0 0 14px; font-size: 1.05rem; color: #a9c7f7; }
    .badge { padding: 6px 10px; border-radius: 999px; background: #6b7280; font-weight: bold; }
    .connected { background: #166534; }
    .unavailable { background: #991b1b; }
    .total { margin: 22px 0; padding: 20px; border-radius: 14px; background: #17243a; }
    .total strong { display: block; margin-top: 4px; font-size: 2.2rem; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(270px, 1fr)); gap: 16px; }
    section { padding: 18px; border: 1px solid #334766; border-radius: 14px; background: #111c2e; }
    dl { display: grid; grid-template-columns: 1fr auto; gap: 10px 18px; margin: 0; }
    dt { color: #aabbd3; }
    dd { margin: 0; font-weight: bold; text-align: right; }
    #message { min-height: 1.3em; margin-top: 18px; color: #fca5a5; }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>GridMind Compute Station</h1>
      <span id="connection" class="badge" aria-live="polite">Connecting...</span>
    </header>

    <div class="total">
      Cumulative simulated outcome
      <strong id="total">--</strong>
    </div>

    <div class="grid">
      <section>
        <h2>Grid conditions</h2>
        <dl>
          <dt>Compute capacity</dt><dd id="capacity">--</dd>
          <dt>Renewable availability</dt><dd id="renewable">--</dd>
          <dt>Carbon intensity</dt><dd id="carbon">--</dd>
          <dt>Current temperature</dt><dd id="temperature">--</dd>
          <dt>Game temperature limit</dt><dd id="temp-limit">--</dd>
          <dt>Virtual time</dt><dd id="now">--</dd>
        </dl>
      </section>

      <section>
        <h2>Customer workload</h2>
        <dl>
          <dt>Customer</dt><dd id="customer">--</dd>
          <dt>Job</dt><dd id="job">--</dd>
          <dt>Full energy</dt><dd id="energy">--</dd>
          <dt>Duration</dt><dd id="duration">--</dd>
          <dt>Full demand</dt><dd id="demand">--</dd>
          <dt>Temperature rise</dt><dd id="temp-rise">--</dd>
          <dt>Fictional contract value</dt><dd id="contract">--</dd>
          <dt>Service type</dt><dd id="type">--</dd>
          <dt>Deadline</dt><dd id="deadline">--</dd>
          <dt>Scheduling slack</dt><dd id="slack">--</dd>
          <dt>Progress</dt><dd id="progress">--</dd>
          <dt>Status</dt><dd id="workload-status">--</dd>
        </dl>
      </section>

      <section>
        <h2>Latest simulated outcome</h2>
        <dl>
          <dt>Action</dt><dd id="action">Awaiting button press</dd>
          <dt>Energy used</dt><dd id="used-energy">--</dd>
          <dt>Average demand</dt><dd id="used-demand">--</dd>
          <dt>Capacity breach</dt><dd id="overload">--</dd>
          <dt>Emissions</dt><dd id="emissions">--</dd>
          <dt>Projected temperature</dt><dd id="projected-temp">--</dd>
          <dt>Temperature above limit</dt><dd id="temp-excess">--</dd>
          <dt>Earliest completion</dt><dd id="completion">--</dd>
          <dt>Deadline slack after action</dt><dd id="result-slack">--</dd>
          <dt>Delivered contract value</dt><dd id="delivered">--</dd>
          <dt>Carbon cost</dt><dd id="co2-cost">--</dd>
          <dt>Capacity-breach cost</dt><dd id="overload-cost">--</dd>
          <dt>Cooling-intervention cost</dt><dd id="cooling-cost">--</dd>
          <dt>Missed-deadline cost</dt><dd id="late-cost">--</dd>
          <dt>Net outcome</dt><dd id="net">--</dd>
        </dl>
      </section>
    </div>

    <p id="message" role="alert" aria-live="polite"></p>
  </main>

  <script>
    const euro = new Intl.NumberFormat('en-IE', {
      style: 'currency',
      currency: 'EUR',
      minimumFractionDigits: 2,
      maximumFractionDigits: 2
    });

    function show(id, value) {
      document.getElementById(id).textContent = value;
    }

    function money(cents) {
      return euro.format(cents / 100);
    }

    function megawatts(kilowatts) {
      return (kilowatts / 1000).toFixed(1) + ' MW';
    }

    function clock(minuteOfDay) {
      const hours = Math.floor(minuteOfDay / 60);
      const minutes = minuteOfDay % 60;
      return String(hours).padStart(2, '0') + ':' +
        String(minutes).padStart(2, '0');
    }

    function titleCase(value) {
      return value.charAt(0).toUpperCase() + value.slice(1);
    }

    function clearDecision() {
      show('action', 'Awaiting button press');
      ['used-energy', 'used-demand', 'overload', 'emissions', 'projected-temp',
       'temp-excess',
       'completion', 'result-slack', 'delivered', 'co2-cost',
       'overload-cost', 'cooling-cost', 'late-cost', 'net']
        .forEach(id => show(id, '--'));
    }

    function showDecision(result) {
      if (!result) {
        clearDecision();
        return;
      }

      show('action', titleCase(result.action));
      show('used-energy', result.energyUsedKwh + ' kWh');
      show('used-demand', megawatts(result.demandKw));
      show('overload', result.overloadMw + ' MW');
      show('emissions', result.emissionsKg + ' kgCO2e');
      show('projected-temp', result.projectedTempC + '\u00b0C');
      show('temp-excess', result.excessTempC + '\u00b0C');
      show('completion', clock(result.completionMin));
      show('result-slack', result.slackMinAfter + ' min');
      show('delivered', money(result.deliveredCents));
      show('co2-cost', money(result.co2CostCents));
      show('overload-cost', money(result.overloadCostCents));
      show('cooling-cost', money(result.coolingCostCents));
      show('late-cost', money(result.lateCostCents));
      show('net', money(result.netCents));
    }

    async function refreshStatus() {
      const connection = document.getElementById('connection');
      const message = document.getElementById('message');

      try {
        const response = await fetch('/api/status', { cache: 'no-store' });
        if (!response.ok) throw new Error('HTTP ' + response.status);

        const state = await response.json();
        show('total', money(state.totalCents));
        show('capacity', megawatts(state.grid.capacityKw));
        show('renewable', state.grid.renewablePct + '%');
        show('carbon', state.grid.co2eGPerKwh + ' gCO2e/kWh');
        show('temperature', state.grid.tempC + '\u00b0C');
        show('temp-limit', state.grid.tempLimitC + '\u00b0C');
        show('now', clock(state.grid.nowMin));
        show('customer', state.workload.customer);
        show('job', state.workload.job);
        show('energy', state.workload.energyKwh + ' kWh');
        show('duration', state.workload.durationMin + ' min');
        show('demand', megawatts(state.workload.demandKw));
        show('temp-rise', '+' + state.workload.tempRiseC + '\u00b0C');
        show('contract', euro.format(state.workload.contractEur));
        show('type', state.workload.flexible ? 'Flexible' : 'Urgent');
        show('deadline', clock(state.workload.deadlineMin));
        show('slack', state.workload.slackMin + ' min');
        show('progress', state.workload.progressPct + '%');
        show('workload-status', titleCase(state.workload.status));
        showDecision(state.lastDecision);

        connection.textContent = 'Connected';
        connection.className = 'badge connected';
        message.textContent = '';
      } catch (error) {
        connection.textContent = 'Unavailable';
        connection.className = 'badge unavailable';
        message.textContent = 'Status unavailable; retrying automatically.';
      }
    }

    refreshStatus();
    setInterval(refreshStatus, 1000);
  </script>
</body>
</html>
)GRIDMIND";

}  // namespace gridmind
