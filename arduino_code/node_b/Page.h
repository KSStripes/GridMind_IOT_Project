// Page.h
// Stores the complete read-only dashboard in flash memory (PROGMEM).
// JavaScript polls /api/status; all decisions still come from physical buttons.
#ifndef GRIDMIND_PAGE_H
#define GRIDMIND_PAGE_H

#include <Arduino.h>

static const char PAGE_HTML[] PROGMEM = R"GRIDMIND(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>GridMind Compute Station</title>
  <style>
    body{margin:0;background:#0b1220;color:#e5eefc;font:16px Arial,sans-serif}
    main{width:min(92%,850px);margin:auto;padding:24px 0}
    header{display:flex;justify-content:space-between;align-items:center;gap:12px}
    h1{font-size:1.5rem} h2{font-size:1rem;color:#a9c7f7}
    .badge{padding:6px 10px;border-radius:20px;background:#6b7280}
    .ok{background:#166534}.bad{background:#991b1b}
    .score{margin:18px 0;padding:18px;border-radius:12px;background:#17243a}
    .score strong{display:block;font-size:2rem}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:14px}
    section{padding:16px;border:1px solid #334766;border-radius:12px;background:#111c2e}
    dl{display:grid;grid-template-columns:1fr auto;gap:9px 14px;margin:0}
    dt{color:#aabbd3}dd{margin:0;text-align:right;font-weight:bold}
    #message{color:#fca5a5;min-height:1.2em}
  </style>
</head>
<body>
<main>
  <header>
    <h1>GridMind Compute Station</h1>
    <span id="connection" class="badge">Connecting...</span>
  </header>

  <div class="score">Operator total<strong id="total">€0.00</strong></div>

  <div class="grid">
    <section>
      <h2>Facility</h2>
      <dl>
        <dt>Compute capacity</dt><dd id="capacity">--</dd>
        <dt>Electricity</dt><dd id="power">--</dd>
        <dt>Temperature</dt><dd id="temperature">--</dd>
      </dl>
    </section>

    <section>
      <h2>Current contract</h2>
      <dl>
        <dt>Job</dt><dd id="job">--</dd>
        <dt>Temperature rise</dt><dd id="rise">--</dd>
        <dt>Value</dt><dd id="value">--</dd>
        <dt>Failure penalty</dt><dd id="penalty">--</dd>
        <dt>Wait available</dt><dd id="wait">--</dd>
        <dt>Jobs in queue</dt><dd id="queue">--</dd>
      </dl>
    </section>

    <section>
      <h2>Latest action</h2>
      <dl>
        <dt>Action</dt><dd id="action">Awaiting input</dd>
        <dt>Result</dt><dd id="result">--</dd>
        <dt>Money change</dt><dd id="delta">--</dd>
      </dl>
    </section>
  </div>
  <p id="message" role="alert"></p>
</main>

<script>
  const euro=new Intl.NumberFormat('en-IE',{style:'currency',currency:'EUR'});
  const text={
    completed:'Contract completed',queued:'Moved to back of queue',
    cancelled:'Contract cancelled',no_capacity:'Not enough compute capacity',
    no_power:'Not enough electricity',too_hot:'Temperature would exceed limit',
    already_waited:'This job has already waited',queue_empty:'Queue is empty',
    invalid_state:'Game is not ready'
  };
  const show=(id,value)=>document.getElementById(id).textContent=value;
  const money=cents=>euro.format(cents/100);

  async function refresh(){
    const badge=document.getElementById('connection');
    const message=document.getElementById('message');
    try{
      const response=await fetch('/api/status',{cache:'no-store'});
      if(!response.ok)throw new Error('HTTP '+response.status);
      const state=await response.json();
      show('capacity',state.facility.capacityAvailable?'Available':'Unavailable');
      show('power',state.facility.powerAvailable?'Available':'Unavailable');
      show('temperature',state.facility.tempC+'°C / '+state.facility.tempLimitC+'°C limit');
      show('queue',state.queueSize);

      if(state.job){
        show('job',state.job.name);
        show('rise','+'+state.job.tempRiseC+'°C');
        show('value',money(state.job.valueCents));
        show('penalty',money(state.job.penaltyCents));
        show('wait',state.job.canWait?'Yes':'No');
      }else{
        show('job','No jobs remaining');
        ['rise','value','penalty','wait'].forEach(id=>show(id,'--'));
      }

      if(state.result){
        const actedOn=state.result.job?' - '+state.result.job:'';
        show('action',state.result.action+actedOn);
        show('result',text[state.result.reason]||state.result.reason);
        show('delta',money(state.result.deltaCents));
        show('total',money(state.result.totalCents));
      }

      badge.textContent='Connected';badge.className='badge ok';message.textContent='';
    }catch(error){
      badge.textContent='Unavailable';badge.className='badge bad';
      message.textContent='Status unavailable; retrying.';
    }
  }
  refresh();setInterval(refresh,1000);
</script>
</body>
</html>
)GRIDMIND";

#endif
