#pragma once
#include <Arduino.h>

// ════════════════════════════════════════════════════════════════
//  EMBEDDED HTML/JS DASHBOARD (Dark Mode)
// ════════════════════════════════════════════════════════════════
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>SIDLAK 2 VCS</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #121212; color: #ffffff; margin: 0; padding: 20px; }
    h2 { color: #00ffcc; border-bottom: 1px solid #333; padding-bottom: 10px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 15px; margin-bottom: 30px; }
    .card { background-color: #1e1e1e; padding: 15px; border-radius: 8px; text-align: center; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    .card-title { font-size: 0.9rem; color: #aaaaaa; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 5px; }
    .card-value { font-size: 1.8rem; font-weight: bold; color: #ffffff; }
    
    .tuner-section { background-color: #1e1e1e; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    .slider-group { margin-bottom: 20px; }
    .slider-header { display: flex; justify-content: space-between; margin-bottom: 8px; }
    input[type=range] { width: 100%; cursor: pointer; accent-color: #00ffcc; }
    .status { text-align: center; margin-top: 20px; font-size: 0.9rem; color: #ff5555; }
  </style>
</head>
<body>

  <h2>Live Telemetry</h2>
  <div class="grid">
    <div class="card"><div class="card-title">System Mode</div><div class="card-value" id="mode" style="color:#00ffcc;">--</div></div>
    <div class="card"><div class="card-title">Speed (km/h)</div><div class="card-value" id="kmh">0.00</div></div>
    <div class="card"><div class="card-title">Wheel RPM</div><div class="card-value" id="rpm">0.0</div></div>
    <div class="card"><div class="card-title">Tgt Steer (%)</div><div class="card-value" id="tgt_str">0</div></div>
    <div class="card"><div class="card-title">Act Steer (mV)</div><div class="card-value" id="act_str">0</div></div>
  </div>

  <h2>Dynamics Tuning</h2>
  <div class="tuner-section">
    <div class="slider-group">
      <div class="slider-header"><span>Steering Kp</span><span id="kp_val">3.0</span></div>
      <input type="range" id="kp" min="0" max="10" step="0.1" value="3.0" onchange="sendTune()">
    </div>
    <div class="slider-group">
      <div class="slider-header"><span>Steering Kd</span><span id="kd_val">0.4</span></div>
      <input type="range" id="kd" min="0" max="2" step="0.05" value="0.4" onchange="sendTune()">
    </div>
    <div class="slider-group">
      <div class="slider-header"><span>ADC Filter Alpha</span><span id="alpha_val">0.10</span></div>
      <input type="range" id="alpha" min="0.01" max="1.0" step="0.01" value="0.10" onchange="sendTune()">
    </div>
  </div>
  <div class="status" id="ws_status">Connecting...</div>

  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;

    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = function() { document.getElementById('ws_status').style.color = '#00ffcc'; document.getElementById('ws_status').innerText = 'Connected'; };
      websocket.onclose = function() { document.getElementById('ws_status').style.color = '#ff5555'; document.getElementById('ws_status').innerText = 'Disconnected'; setTimeout(initWebSocket, 2000); };
      websocket.onmessage = function(event) {
        var data = JSON.parse(event.data);
        
        // Update Mode String
        let modeStr = "IDLE";
        if (data.mode === 50) modeStr = "AUTO";
        else if (data.mode === 100) modeStr = "MANUAL";
        document.getElementById('mode').innerText = modeStr;
        
        // Update Telemetry
        document.getElementById('tgt_str').innerText = (data.tgt_str / 100).toFixed(1) + "%";
        document.getElementById('act_str').innerText = data.act_str;
        document.getElementById('rpm').innerText = data.act_rpm.toFixed(1);
        document.getElementById('kmh').innerText = data.act_kmh.toFixed(2);
      };
    }

    function sendTune() {
      var kp = document.getElementById('kp').value;
      var kd = document.getElementById('kd').value;
      var alpha = document.getElementById('alpha').value;
      
      // Update UI labels instantly
      document.getElementById('kp_val').innerText = kp;
      document.getElementById('kd_val').innerText = kd;
      document.getElementById('alpha_val').innerText = alpha;
      
      // Send JSON to ESP32
      var msg = { tune: { kp: parseFloat(kp), kd: parseFloat(kd), alpha: parseFloat(alpha), ki: 0.0 } };
      websocket.send(JSON.stringify(msg));
    }

    window.addEventListener('load', initWebSocket);
  </script>
</body>
</html>
)rawliteral";