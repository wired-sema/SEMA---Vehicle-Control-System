# Project Files Export

Export time: 5/19/2026, 1:28:24 AM

Source directory: `src`

Output file: `summary.md`

## Directory Structure

```
src
├── config
│   ├── constants.h
│   └── pins.h
├── core0
│   ├── calibration.cpp
│   ├── calibration.h
│   ├── mode.cpp
│   ├── mode.h
│   ├── uart.cpp
│   ├── uart.h
│   ├── web_ui.h
│   ├── web.cpp
│   └── web.h
├── core1
│   ├── actr.cpp
│   ├── actr.h
│   ├── hall.cpp
│   ├── hall.h
│   ├── rev.cpp
│   ├── rev.h
│   ├── steer.cpp
│   ├── steer.h
│   ├── ui.cpp
│   └── ui.h
├── main.cpp
├── state.cpp
└── state.h
```

## File Statistics

- Total files: 24
- Total size: 54.8 KB

### File Type Distribution

| Extension | Files | Total Size |
| --- | --- | --- |
| .h | 13 | 21.4 KB |
| .cpp | 11 | 33.4 KB |

## File Contents

### config\constants.h

```plaintext
// config\constants.h
#pragma once

// ════════════════════════════════════════════════════════════════
//  LEDC CHANNELS
// ════════════════════════════════════════════════════════════════
#define STEPPER_LEDC_CH    0        // Stepper motor control (PWM frequency dynamically adjusted by PID)
#define STEPPER_LEDC_RES   8        // 8-bit resolution for fine-grained control at low speeds
#define TB6612_LEDC_CH     1        // TB6612FNG motor driver (Fixed 10 kHz to avoid audible noise)
#define TB6612_LEDC_FREQ   10000    // 10 kHz fixed frequency for TB6612 to prevent whining noise
#define TB6612_LEDC_RES    8        // 8-bit resolution for smooth throttle control

// ════════════════════════════════════════════════════════════════
//  THROTTLE (DAC & LM358)
// ════════════════════════════════════════════════════════════════
#define THROTTLE_R1_OHMS     10000.0f  // R1 is the fixed resistor to 5V
#define THROTTLE_R2_OHMS     18000.0f  // R2 is the variable resistor to GND (potentiometer)
#define THROTTLE_DEADBAND_MV   600     // Minimum voltage change required to register as a valid throttle input (to prevent noise at low speeds)
#define THROTTLE_MIN_MV        650     // Minimum voltage corresponding to 0% throttle (calibrated from testing)
#define THROTTLE_MAX_MV       3000     // Maximum voltage corresponding to 100% throttle (calibrated from testing)

// ════════════════════════════════════════════════════════════════
//  STEERING POTENTIOMETER PHYSICAL LIMITS
// ════════════════════════════════════════════════════════════════
#define STEPS_FULL_L      -2000     // Full left corresponds to -2000 steps from center
#define STEPS_FULL_R       2000     // Full right corresponds to +2000 steps from center

// ════════════════════════════════════════════════════════════════
//  FILTERS & PID
// ════════════════════════════════════════════════════════════════
#define MEDIAN_SAMPLES   7        // Number of samples for the median filter (must be odd)
#define EMA_ALPHA        0.10f    // Smoothing factor for the exponential moving average (0.10 = 10% new, 90% old)
#define PID_KP           3.0f     // Proportional gain for the PID controller (tuned for responsive yet stable steering)
#define PID_KD           0.4f     // Derivative gain for the PID controller (tuned to dampen overshoot without causing sluggishness)
#define PID_KI           0.0f     // Integral gain for the PID controller (set to 0 to prevent windup, as the system is responsive enough without it)
#define PID_HZ_MIN       300      // Minimum update frequency for the PID loop (below this, we risk oscillations due to slow response)
#define PID_HZ_MAX       4000     // Maximum update frequency for the PID loop (above this, we risk excessive CPU usage without meaningful performance gains)
#define PID_DEADBAND     15       // Deadband in steps where no correction is applied (to prevent jitter around the target position)
#define PID_I_MAX        100.0f   // Maximum integral term to prevent windup (not used since KI is 0, but defined for future tuning)

// ════════════════════════════════════════════════════════════════
//  STEPPER FOLLOW (MODE A)
// ════════════════════════════════════════════════════════════════
#define FOLLOW_UPDATE_MS        30 // Update interval for the follow mode (in milliseconds)
#define FOLLOW_DELTA_THRESHOLD   7 // Minimum step difference required to trigger a position update in follow mode (to prevent jitter from small target changes)
#define FOLLOW_HZ_MIN          400 // Minimum update frequency for follow mode (below this, the steering will feel unresponsive)
#define FOLLOW_HZ_MAX         4000 // Maximum update frequency for follow mode (above this, we risk excessive CPU usage without meaningful performance gains)
#define FOLLOW_HZ_SCALE         60 // Scaling factor for follow mode frequency based on target speed (higher speeds require more frequent updates to maintain control)

// ════════════════════════════════════════════════════════════════
//  ACTUATOR HARD TIMEOUTS
// ════════════════════════════════════════════════════════════════
#define BRAKE_EXTEND_TIMEOUT_MS 3000  // Maximum time allowed for the brake to be extended before triggering a safety fault (to prevent runaway braking in case of a stuck command)

// ════════════════════════════════════════════════════════════════
//  ODOMETRY (HALL SENSOR)
// ════════════════════════════════════════════════════════════════
#define WHEEL_CIRCUMFERENCE_M         1.2764f       // Circumference of the wheel in meters (calibrated from actual measurements)
#define POLE_PAIRS                    23            // Number of hall sensor transitions per mechanical revolution (calibrated from testing)
#define RPM_CALIBRATION_FACTOR        0.9587f       // Calibration factor to correct raw RPM readings from the hall sensor (accounts for timing inaccuracies and wheel slip, calibrated from testing against a GPS-based speedometer)
#define HALL_TRANSITIONS_PER_MECH_REV 46            // Total hall sensor transitions per mechanical revolution (2 transitions per pole pair)
#define RPM_SAMPLE_WINDOW_MS          500           // Time window for calculating RPM from hall sensor transitions (in milliseconds, balances responsiveness with stability of the RPM reading)
#define RPM_TIMEOUT_MS               1000           // Time after which RPM is considered zero if no hall transitions are detected (in milliseconds, prevents stale RPM readings when stopped)
#define HALL_DEBOUNCE_US              800           // Minimum time between valid hall sensor transitions to prevent false readings from noise (in microseconds, calibrated from oscilloscope measurements of the hall sensor signal)
#define MAX_SPEED_KMPH               45.0f          // Maximum expected speed of the vehicle in km/h (used for scaling and sanity checks on telemetry data, calibrated from testing)

// ════════════════════════════════════════════════════════════════
//  UART COMMUNICATIONS & OLED
// ════════════════════════════════════════════════════════════════
#define UART_BAUD   115200  // Baud rate for Jetson communication (must match the sender's baud rate)
#define PACKET_LEN  14      // Total packet length in bytes (2 start bytes + 1 type byte + 10 data bytes + 1 CRC byte)
#define PKT_START1  0xAA    // First start byte for packet synchronization (0xAA is a common choice for framing bytes due to its alternating bit pattern)
#define PKT_START2  0x55    // Second start byte for packet synchronization (0x55 complements 0xAA to help detect framing errors)
#define PKT_END     0xFF    // End byte for packet validation (not strictly necessary if we have a fixed packet length, but can help detect certain types of corruption)
#define PKT_TYPE_TX 0x02    // Packet type for telemetry data sent from Core 1 to the Jetson (arbitrary value, just needs to be distinct from PKT_TYPE_RX)
#define PKT_TYPE_RX 0x01    // Packet type for control commands received from the Jetson (arbitrary value, just needs to be distinct from PKT_TYPE_TX)

#define OLED_WIDTH   128   // OLED display width in pixels (common size for small automotive displays)
#define OLED_HEIGHT   64   // OLED display height in pixels (common size for small automotive displays)
#define OLED_ADDR   0x3C   // I2C address for the OLED display (0x3C is a common default for SSD1306-based displays)
```

### config\pins.h

```plaintext
// config\pins.h
#pragma once

// ════════════════════════════════════════════════════════════════
//  PINS (ESP32-WROOM-32 DevKit)
// ════════════════════════════════════════════════════════════════

// ─── COMMUNICATION ───
#define JETSON_RX_PIN       16  // UART RX from Jetson
#define JETSON_TX_PIN       17  // UART TX to Jetson
#define I2C_SDA_PIN         21  // OLED / Sensors
#define I2C_SCL_PIN         22  

// ─── ANALOG SENSORS & ODOMETRY ───
#define THROTTLE_ADC_PIN    34  // Physical pedal input
#define STEERING_ADC_PIN    35  // Steering angle potentiometer
#define HALL_C_PIN          32  // Wheel speed interrupt

// ─── SAFETY & SWITCHES (DIGITAL IN) ───
#define DEADMAN_1_PIN       33  // AND-Gate grip 1
#define DEADMAN_2_PIN       27  // AND-Gate grip 2
#define BRAKE_SWITCH_PIN    14  // Hard physical brake pedal
#define REVERSE_SW_PIN      26  // Forward/Reverse toggle
#define LIMIT_SWITCH_PIN    13  // Brake actuator boundary

// ─── MOTOR CONTROL & ACTUATORS (DIGITAL/PWM OUT) ───
#define THROTTLE_DAC_PIN    25  // 0-3.3V Output to Motor Controller
#define STEPPER_PUL_PIN     18  // Steering Pulses
#define STEPPER_DIR_PIN     19  // Steering Direction
#define STEPPER_ENA_PIN     23  // Steering Enable
#define TB6612_IN1_PIN       4  // Brake linear actuator Dir 1
#define TB6612_IN2_PIN       2  // Brake linear actuator Dir 2
#define TB6612_PWM_PIN       5  // Brake linear actuator Power

// ─── RELAYS & STATUS (DIGITAL OUT) ───
#define RELAY_PIN           15  // Organizer Strobe (NO = Auto, NC = Manual)
#define BRAKE_MC_PIN        12  // Brake indicator/relay
```

### core0\calibration.cpp

```cpp
// core0\calibration.cpp
#include "core0/calibration.h"
#include <Preferences.h>
#include "state.h" // Needed to check FSM state before writing to flash

VCS_Calibration_t sysCal;
Preferences prefs;

void Calibration_LoadDefaults() {
    sysCal.pot_mv_center    = 1501;
    sysCal.pot_mv_full_l    = 142;
    sysCal.pot_mv_full_r    = 2776;
    sysCal.brake_retract_ms = 900;
    sysCal.brake_pwm_power  = 255;
    Serial.println("[CALIBRATION] Defaults loaded.");
}

void Calibration_Init() {
    prefs.begin("sidlak_vcs", false); // Open NVS namespace in read/write mode
    
    // Read from flash. If key doesn't exist, provide the default fallback.
    sysCal.pot_mv_center    = prefs.getUInt("pot_c", 1501);
    sysCal.pot_mv_full_l    = prefs.getUInt("pot_l", 142);
    sysCal.pot_mv_full_r    = prefs.getUInt("pot_r", 2776);
    sysCal.brake_retract_ms = prefs.getUInt("brk_ms", 900);
    sysCal.brake_pwm_power  = prefs.getUInt("brk_pwm", 255);
    
    Serial.println("[CALIBRATION] NVS Vault initialized and loaded.");
}

void Calibration_Save() {
    // ─── SAFETY INTERLOCK ───
    // Prevent flash writes (which cause massive latency) if the car is autonomous.
    VCS_State_t current_state = StateHub_GetSnapshot();
    if (current_state.target_mode == 50) {
        Serial.println("[CALIBRATION] ERROR: Cannot save to flash while in AUTONOMOUS mode.");
        return;
    }

    prefs.putUInt("pot_c",   sysCal.pot_mv_center);
    prefs.putUInt("pot_l",   sysCal.pot_mv_full_l);
    prefs.putUInt("pot_r",   sysCal.pot_mv_full_r);
    prefs.putUInt("brk_ms",  sysCal.brake_retract_ms);
    prefs.putUInt("brk_pwm", sysCal.brake_pwm_power);
    
    Serial.println("[CALIBRATION] Saved to NVS Vault.");
}
```

### core0\calibration.h

```plaintext
// core0\calibration.h
#pragma once
#include <Arduino.h>

// ════════════════════════════════════════════════════════════════
//  DYNAMIC CALIBRATION STRUCTURE
//  Holds values that are actively tuned and saved to NVS Flash.
// ════════════════════════════════════════════════════════════════
struct VCS_Calibration_t {
    uint32_t pot_mv_center;
    uint32_t pot_mv_full_l;
    uint32_t pot_mv_full_r;
    uint32_t brake_retract_ms;
    uint32_t brake_pwm_power;
    float pid_kp;
    float pid_ki;
    float pid_kd;
    float ema_alpha;
};

// Global calibration instance
extern VCS_Calibration_t sysCal;

// Core functions
void Calibration_Init();
void Calibration_Save();
void Calibration_LoadDefaults();
```

### core0\mode.cpp

```cpp
// core0\mode.cpp
#include "core0/mode.h"
#include "state.h"
#include "config/pins.h"

void Mode_Init() {
    // Configure safety inputs with pullups
    pinMode(DEADMAN_1_PIN, INPUT_PULLUP);
    pinMode(DEADMAN_2_PIN, INPUT_PULLUP);
    pinMode(BRAKE_SWITCH_PIN, INPUT_PULLUP);
    
    Serial.println("[MODE] Master FSM & Safety Interlocks Initialized.");
}

void Mode_Update() {
    // 1. Read Physical Safety Hardware
    bool dm1 = (digitalRead(DEADMAN_1_PIN) == LOW);
    bool dm2 = (digitalRead(DEADMAN_2_PIN) == LOW);
    
    // STRICT AND GATE: Both buttons must be pressed to request Autonomous mode
    bool is_deadman_active = (dm1 && dm2); 
    bool is_brake_pressed  = (digitalRead(BRAKE_SWITCH_PIN) == LOW);

    // 2. Lock the StateHub to perform safety checks and data updates
    portENTER_CRITICAL(&stateMux);
    
    // Update the hardware cache so the Jetson telemetry packet can read it later
    globalState.deadman_active = is_deadman_active;

    // ════════════════════════════════════════════════════════════════
    //  MASTER SAFETY OVERRIDES (The FSM Rules)
    // ════════════════════════════════════════════════════════════════

    // RULE A: Deadman Authorization (The AND Gate)
    // If BOTH buttons are pressed, authorize Autonomous Mode (50).
    // Otherwise, default to the safe hardware state: Manual Mode (100).
    if (is_deadman_active) {
        globalState.target_mode = 50;
    } else {
        globalState.target_mode = 100;
    }

    // RULE B: Physical Brake Override (Hard Failsafe)
    // If the driver stomps the physical brake pedal, kill all drive power instantly, 
    // and forcefully revoke Autonomous mode (kicking it back to Manual/NC Relay).
    if (is_brake_pressed) {
        globalState.target_mode = 100; 
        globalState.target_speed_pwm = 0;
        
        // Optionally command the linear actuator to assist braking
        globalState.target_brake = 100; 
    }

    portEXIT_CRITICAL(&stateMux);
}
```

### core0\mode.h

```plaintext
// core0\mode.h
#pragma once
#include <Arduino.h>

void Mode_Init();
void Mode_Update();
```

### core0\uart.cpp

```cpp
// core0\uart.cpp
#include "core0/uart.h"
#include "state.h"
#include "config/pins.h"
#include "config/constants.h"

static uint8_t rx_buf[PACKET_LEN];
static uint8_t tx_buf[PACKET_LEN];
static uint32_t last_tx_time = 0;

// CRC16 Security Validation
static uint16_t calculate_crc16(uint8_t *d, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= d[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

void Uart_Init() {
    Serial2.begin(UART_BAUD, SERIAL_8N1, JETSON_RX_PIN, JETSON_TX_PIN);
    Serial.println("[UART] Gateway initialized on UART2.");
}

static void send_telemetry() {
    VCS_State_t snap = StateHub_GetSnapshot();
    
    int16_t  rv = (int16_t)snap.mechanical_rpm;
    // Scale physical steering mV back to a 0-10000 Jetson percentage
    uint16_t sv = (uint16_t)map(snap.pot_filtered_mv, 0, 3300, 0, 10000); 

    tx_buf[0] = PKT_START1; 
    tx_buf[1] = PKT_START2;
    tx_buf[2] = PKT_TYPE_TX; 
    tx_buf[3] = 0x07;
    tx_buf[4] = (rv >> 8) & 0xFF; 
    tx_buf[5] = rv & 0xFF;
    tx_buf[6] = (sv >> 8) & 0xFF; 
    tx_buf[7] = sv & 0xFF;
    tx_buf[8] = snap.brake_at_limit ? 100 : 0; 
    tx_buf[9] = 0; 
    tx_buf[10] = snap.reverse_switch ? 100 : 0;
    
    uint16_t crc = calculate_crc16(&tx_buf[2], 9);
    tx_buf[11] = (crc >> 8) & 0xFF; 
    tx_buf[12] = crc & 0xFF; 
    tx_buf[13] = PKT_END;
    
    Serial2.write(tx_buf, PACKET_LEN);
}

void Uart_Update() {
    // 1. Intercept structured command arrays
    if (Serial2.available() >= PACKET_LEN) {
        if (Serial2.read() == PKT_START1 && Serial2.peek() == PKT_START2) {
            rx_buf[0] = PKT_START1;
            Serial2.readBytes(&rx_buf[1], PACKET_LEN - 1);
            
            if (rx_buf[PACKET_LEN - 1] == PKT_END && rx_buf[2] == PKT_TYPE_RX) {
                uint16_t calculated_crc = calculate_crc16(&rx_buf[2], 9);
                uint16_t received_crc   = (rx_buf[11] << 8) | rx_buf[12];
                
                // 2. Perform CRC16 validation
                if (calculated_crc == received_crc) {
                    uint8_t  mode  = rx_buf[4];
                    uint16_t steer = (rx_buf[5] << 8) | rx_buf[6];
                    uint16_t speed = (rx_buf[7] << 8) | rx_buf[8];
                    uint8_t  brake = rx_buf[9];
                    uint8_t  dir   = rx_buf[10];

                    // 3. Pass frames into shared state registers safely
                    StateHub_UpdateTargets(mode, steer, speed, brake, dir);
                }
            }
        }
    }

    // Push physical odometry metrics back to Jetson at 10Hz
    if (millis() - last_tx_time >= 100) {
        send_telemetry();
        last_tx_time = millis();
    }
}
```

### core0\uart.h

```plaintext
// core0\uart.h
#pragma once
#include <Arduino.h>

void Uart_Init();
void Uart_Update();
```

### core0\web_ui.h

```plaintext
// core0\web_ui.h
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
```

### core0\web.cpp

```cpp
// core0\web.cpp
#include "core0/web.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "state.h"
#include "core0/calibration.h"

// ─── IMPORT THE SEPARATED HTML UI ───
#include "web_ui.h" 

// ─── LOCAL STATE ────────────────────────────────────────────────
static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

// ─── WEBSOCKET EVENT HANDLER ────────────────────────────────────
static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                    void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            data[len] = 0; 
            
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, (char*)data);
            
            if (!err && doc.containsKey("tune")) {
                if (doc["tune"].containsKey("kp")) sysCal.pid_kp = doc["tune"]["kp"];
                if (doc["tune"].containsKey("ki")) sysCal.pid_ki = doc["tune"]["ki"];
                if (doc["tune"].containsKey("kd")) sysCal.pid_kd = doc["tune"]["kd"];
                if (doc["tune"].containsKey("alpha")) sysCal.ema_alpha = doc["tune"]["alpha"];
                
                Serial.printf("[WEB] Tuner Update: Kp=%.2f, Kd=%.2f, Alpha=%.2f\n", 
                              sysCal.pid_kp, sysCal.pid_kd, sysCal.ema_alpha);
                
                Calibration_Save(); 
            }
        }
    }
}

// ─── PUBLIC API ─────────────────────────────────────────────────
void Web_Init() {
    // ─── Simulation Mode Note ───
    // If testing in Wokwi, change softAP to: WiFi.begin("Wokwi-GUEST", "", 6);
    WiFi.softAP("SIDLAK_VCS_LIVE", "sidlak2026");
    
    // Serve the separated HTML file when a device connects
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();
    
    Serial.println("[WEB] Async AP & Web Dashboard Initialized.");
    Serial.print("[WEB] Connect to IP: ");
    Serial.println(WiFi.softAPIP());
}

void Web_Update() {
    ws.cleanupClients();
    
    if (ws.count() > 0) {
        VCS_State_t snap = StateHub_GetSnapshot();
        
        char json_payload[256];
        snprintf(json_payload, sizeof(json_payload),
            "{\"mode\":%d,\"tgt_str\":%u,\"act_str\":%lu,\"tgt_spd\":%u,\"act_rpm\":%.1f,\"act_kmh\":%.2f}",
            snap.target_mode, 
            snap.target_steering, 
            snap.pot_filtered_mv, 
            snap.target_speed_pwm, 
            snap.mechanical_rpm,
            snap.velocity_kmh);
            
        ws.textAll(json_payload);
    }
}
```

### core0\web.h

```plaintext
// core0\web.h
#pragma once
#include <Arduino.h>

void Web_Init();
void Web_Update();
```

### core1\actr.cpp

```cpp
// core1\actr.cpp
#include "actr.h"
#include "state.h"
#include "config/pins.h"
#include "config/constants.h"
#include "core0/calibration.h"

// ─── RELAY CONSTANTS ────────────────────────────────────────────
// Normally Closed (NC) = Manual | Normally Open (NO) = Auto
#define RELAY_ENERGIZED   LOW   // Coil gets power, NO closes
#define RELAY_DEENERGIZED HIGH  // Coil loses power, NC closes

enum ActuatorState { ACT_IDLE, ACT_EXTENDING, ACT_RETRACTING };
static ActuatorState act_state = ACT_IDLE;
static uint32_t act_start_ms = 0;

// ─── PRIVATE HARDWARE FUNCTIONS ─────────────────────────────────
static void tb6612_pwm(uint8_t v) { ledcWrite(TB6612_LEDC_CH, v); }

static void actuator_stop() {
    digitalWrite(TB6612_IN1_PIN, LOW); digitalWrite(TB6612_IN2_PIN, LOW);
    tb6612_pwm(0); act_state = ACT_IDLE;
}

static void set_throttle_pwm(uint16_t jetson_speed_target) {
    // Jetson sends 0 to 10000 (0.00% to 100.00%)
    // Map to 8-bit DAC output (0-255)
    uint8_t dac_val = (uint8_t)map(jetson_speed_target, 0, 10000, 0, 255);
    dacWrite(THROTTLE_DAC_PIN, dac_val);
}

// ─── PUBLIC API ─────────────────────────────────────────────────
void Actr_Init() {
    // Throttle DAC
    dacWrite(THROTTLE_DAC_PIN, 0); // Absolute safety zero at boot

    // TB6612 Brake H-Bridge
    pinMode(TB6612_IN1_PIN, OUTPUT);
    pinMode(TB6612_IN2_PIN, OUTPUT);
    ledcSetup(TB6612_LEDC_CH, TB6612_LEDC_FREQ, TB6612_LEDC_RES);
    ledcAttachPin(TB6612_PWM_PIN, TB6612_LEDC_CH);
    actuator_stop();

    // Relays
    pinMode(BRAKE_MC_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    
    // Default to Safe State on boot
    digitalWrite(BRAKE_MC_PIN, HIGH); // Engaged/Safe
    digitalWrite(RELAY_PIN, RELAY_DEENERGIZED); // OFF (NC - Manual Mode)
    
    // Limit Switch
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

    Serial.println("[ACTR] Throttle, Brake, and Relays Initialized.");
}

void Actr_Update() {
    VCS_State_t snap = StateHub_GetSnapshot();
    bool at_limit = (digitalRead(LIMIT_SWITCH_PIN) == LOW);

    // 1. Update Hardware State Cache
    portENTER_CRITICAL(&stateMux);
    globalState.brake_at_limit = at_limit;
    portEXIT_CRITICAL(&stateMux);

    // 2. Throttle Delivery & Relay Status Firewalls
    if (snap.target_mode == 50) { 
        // --- AUTONOMOUS ---
        digitalWrite(RELAY_PIN, RELAY_ENERGIZED); // Clicks to NO
        set_throttle_pwm(snap.target_speed_pwm);
    } else { 
        // --- IDLE / MANUAL ---
        digitalWrite(RELAY_PIN, RELAY_DEENERGIZED); // Falls back to NC
        set_throttle_pwm(0); // Hardware safety cutoff
    }

    // 3. Brake Actuator State Machine (Non-Blocking)
    // If Jetson commands > 50% brake, extend the actuator. Else, retract to safe position.
    if (snap.target_mode == 50) {
        if (snap.target_brake > 50 && act_state == ACT_IDLE && !at_limit) {
            digitalWrite(TB6612_IN1_PIN, HIGH); digitalWrite(TB6612_IN2_PIN, LOW);
            tb6612_pwm((uint8_t)sysCal.brake_pwm_power);
            act_state = ACT_EXTENDING;
            act_start_ms = millis();
        } 
        else if (snap.target_brake <= 50 && act_state == ACT_IDLE) {
            // Optional: Retract logic when brake is released
        }
    }

    // Process Active Brake Strokes
    if (act_state == ACT_EXTENDING) {
        if (at_limit || (millis() - act_start_ms >= BRAKE_EXTEND_TIMEOUT_MS)) {
            actuator_stop();
        }
    } else if (act_state == ACT_RETRACTING) {
        if (millis() - act_start_ms >= sysCal.brake_retract_ms) {
            actuator_stop();
        }
    }
}
```

### core1\actr.h

```plaintext
// core1\actr.h
#pragma once
#include <Arduino.h>

void Actr_Init();
void Actr_Update();
```

### core1\hall.cpp

```cpp
// core1\hall.cpp
#include "hall.h"
#include "state.h"
#include "config/pins.h"
#include "config/constants.h"

// ─── LOCAL ISR STATE (Isolated from StateHub) ───────────────────
static volatile uint32_t hall_pulse_count = 0;
static volatile uint32_t hall_pulses_this_window = 0;
static volatile uint64_t last_hall_us = 0;
static volatile uint32_t last_pulse_ms = 0;

static portMUX_TYPE hallMux = portMUX_INITIALIZER_UNLOCKED;

// Local calculation state
static uint32_t last_window_ms = 0;
static uint32_t last_total_pulses = 0;
static float    local_rpm = 0.0f;
static float    local_kmh = 0.0f;
static float    local_odo = 0.0f;

// ─── HARDWARE INTERRUPT (Runs instantly on magnet pass) ─────────
void IRAM_ATTR hall_isr() {
    uint64_t now_us = esp_timer_get_time();
    
    // Hardware Debounce (Blocks false triggers)
    if (now_us - last_hall_us < HALL_DEBOUNCE_US) return;
    last_hall_us = now_us;
    
    // Lock the local variables, increment, and unlock fast
    portENTER_CRITICAL_ISR(&hallMux);
    hall_pulse_count++;
    hall_pulses_this_window++;
    last_pulse_ms = (uint32_t)(now_us / 1000);
    portEXIT_CRITICAL_ISR(&hallMux);
}

// ─── PUBLIC API ─────────────────────────────────────────────────
void Hall_Init() {
    pinMode(HALL_C_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(HALL_C_PIN), hall_isr, CHANGE);
    last_window_ms = millis();
    Serial.println("[HALL] Wheel Odometer ISR Attached.");
}

void Hall_Update() {
    uint32_t now = millis();
    uint32_t elapsed = now - last_window_ms;

    // Only run the heavy math every 500ms (RPM_SAMPLE_WINDOW_MS)
    if (elapsed >= RPM_SAMPLE_WINDOW_MS) {
        last_window_ms = now;
        
        // 1. Safely extract counts from the ISR
        portENTER_CRITICAL(&hallMux);
        uint32_t window_pulses = hall_pulses_this_window;
        uint32_t total_pulses  = hall_pulse_count;
        uint32_t time_since_last_pulse = now - last_pulse_ms;
        hall_pulses_this_window = 0; // Reset for next window
        portEXIT_CRITICAL(&hallMux);

        // 2. Check for vehicle stop (Timeout)
        if (time_since_last_pulse > RPM_TIMEOUT_MS) {
            local_rpm = 0.0f;
            local_kmh = 0.0f;
            last_total_pulses = total_pulses;
        } 
        // 3. Calculate RPM & Speed
        else if (window_pulses > 0) {
            float rpm = (window_pulses / (float)HALL_TRANSITIONS_PER_MECH_REV) / (elapsed / 60000.0f) * RPM_CALIBRATION_FACTOR;
            float kmh = rpm * WHEEL_CIRCUMFERENCE_M / 60.0f * 3.6f;
            
            // Reject impossible speed spikes
            if (kmh <= MAX_SPEED_KMPH) {
                uint32_t new_pulses = total_pulses - last_total_pulses;
                float dist = new_pulses / (float)HALL_TRANSITIONS_PER_MECH_REV * WHEEL_CIRCUMFERENCE_M * RPM_CALIBRATION_FACTOR;
                
                local_rpm = rpm;
                local_kmh = kmh;
                local_odo += dist;
            }
            last_total_pulses = total_pulses;
        }

        // 4. Push the final, clean math directly to the StateHub
        StateHub_UpdateTelemetry(local_rpm, local_kmh, local_odo);
    }
}
```

### core1\hall.h

```plaintext
// core1\hall.h
#pragma once
#include <Arduino.h>

void Hall_Init();
void Hall_Update();
```

### core1\rev.cpp

```cpp
// core1\rev.cpp
#include "core1/rev.h"
#include "state.h"
#include "config/pins.h"

// The safe RPM threshold to allow a physical gear/direction change
#define STOPPED_ENTER_RPM 3.0f

// Local tracking to prevent spamming StateHub
static bool current_validated_direction = false; // false = Fwd, true = Rev

void Rev_Init() {
    // Assuming switch pulls to GND when active
    pinMode(REVERSE_SW_PIN, INPUT_PULLUP);
    
    // Read initial state at boot (assuming car is stopped at boot)
    current_validated_direction = (digitalRead(REVERSE_SW_PIN) == LOW);
    
    portENTER_CRITICAL(&stateMux);
    globalState.reverse_switch = current_validated_direction;
    portEXIT_CRITICAL(&stateMux);
    
    Serial.println("[REV] Directional Interlock Initialized.");
}

void Rev_Update() {
    bool switch_is_reversed = (digitalRead(REVERSE_SW_PIN) == LOW);

    // If the physical switch doesn't match our validated state, a change was requested
    if (switch_is_reversed != current_validated_direction) {
        
        // Grab a snapshot to check the current vehicle speed
        VCS_State_t snap = StateHub_GetSnapshot();

        // Safety Interlock: Only allow shift if wheels are stopped
        if (snap.mechanical_rpm <= STOPPED_ENTER_RPM) {
            current_validated_direction = switch_is_reversed;
            
            // Push validated state to StateHub
            portENTER_CRITICAL(&stateMux);
            globalState.reverse_switch = current_validated_direction;
            portEXIT_CRITICAL(&stateMux);
            
            Serial.printf("[REV] Direction Shift Validated. Reverse = %s\n", current_validated_direction ? "TRUE" : "FALSE");
        } else {
            // Speed is too high. Ignore the physical switch request.
            // (Optional: You could trigger an OLED warning here via StateHub)
        }
    }
}
```

### core1\rev.h

```plaintext
// core1\rev.h
#pragma once
#include <Arduino.h>

void Rev_Init();
void Rev_Update();
```

### core1\steer.cpp

```cpp
// core1\steer.cpp
#include "core1/steer.h"
#include "state.h"
#include "config/pins.h"
#include "config/constants.h"
#include "core0/calibration.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"

// ─── LOCAL STATE ────────────────────────────────────────────────
static esp_adc_cal_characteristics_t adc_chars;
static float    ema_val = -1.0f;
static uint32_t med_buf[MEDIAN_SAMPLES];

// PID State
static float    pid_integral = 0.0f;
static float    pid_prev_err = 0.0f;
static uint32_t pid_last_ms  = 0;
static uint32_t pid_hz       = 0;
static bool     pid_running  = false;
static bool     pid_dir      = true;

// ─── PRIVATE HARDWARE FUNCTIONS ─────────────────────────────────
static void stepper_ledc_run(uint32_t hz) {
    ledcSetup(STEPPER_LEDC_CH, hz, STEPPER_LEDC_RES);
    ledcAttachPin(STEPPER_PUL_PIN, STEPPER_LEDC_CH);
    ledcWrite(STEPPER_LEDC_CH, 128);
}

static void stepper_ledc_stop() { 
    ledcWrite(STEPPER_LEDC_CH, 0); 
}

// ─── SENSOR ACQUISITION ─────────────────────────────────────────
static uint32_t adc_read_steering_raw() {
    uint32_t s = 0;
    for (int i = 0; i < 16; i++) s += adc1_get_raw(ADC1_CHANNEL_7);
    return esp_adc_cal_raw_to_voltage(s / 16, &adc_chars);
}

static uint32_t pot_read_filtered() {
    // Median Filter
    for (int i = 0; i < MEDIAN_SAMPLES; i++) med_buf[i] = adc_read_steering_raw();
    for (int i = 1; i < MEDIAN_SAMPLES; i++) {
        uint32_t k = med_buf[i]; int j = i - 1;
        while (j >= 0 && med_buf[j] > k) { med_buf[j + 1] = med_buf[j]; j--; }
        med_buf[j + 1] = k;
    }
    uint32_t m = med_buf[MEDIAN_SAMPLES / 2];

    // EMA Filter
    if (ema_val < 0.0f) ema_val = (float)m;
    else ema_val = EMA_ALPHA * (float)m + (1.0f - EMA_ALPHA) * ema_val;
    
    return constrain((uint32_t)ema_val, sysCal.pot_mv_full_l, sysCal.pot_mv_full_r);
}

static int32_t pot_to_steps(uint32_t mv) {
    return constrain(
        (int32_t)map((long)mv, (long)sysCal.pot_mv_full_l, (long)sysCal.pot_mv_full_r,
                     (long)STEPS_FULL_L, (long)STEPS_FULL_R),
        STEPS_FULL_L, STEPS_FULL_R);
}

// ─── PUBLIC API ─────────────────────────────────────────────────
void Steer_Init() {
    // ADC Setup
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);

    // Stepper Setup
    pinMode(STEPPER_DIR_PIN, OUTPUT);
    pinMode(STEPPER_ENA_PIN, OUTPUT);
    digitalWrite(STEPPER_DIR_PIN, HIGH);
    digitalWrite(STEPPER_ENA_PIN, HIGH); // ENA HIGH = Enabled (Holds position)
    
    ledcSetup(STEPPER_LEDC_CH, 1000, STEPPER_LEDC_RES);
    ledcAttachPin(STEPPER_PUL_PIN, STEPPER_LEDC_CH);
    stepper_ledc_stop();
    
    Serial.println("[STEER] PID Step Engine Initialized.");
}

void Steer_Update() {
    // 1. Read Physical Sensor & Push to StateHub
    uint32_t pot_mv = pot_read_filtered();
    int32_t  current_steps = pot_to_steps(pot_mv);

    portENTER_CRITICAL(&stateMux);
    globalState.pot_filtered_mv = pot_mv;
    portEXIT_CRITICAL(&stateMux);

    // 2. Pull Target Command from StateHub
    VCS_State_t snap = StateHub_GetSnapshot();

    // 3. Execute Control Logic based on FSM Mode
    if (snap.target_mode == 50) { // AUTONOMOUS MODE
        
        // Map Jetson's 0-10000% target to physical step limits
        int32_t target_steps = map(snap.target_steering, 0, 10000, STEPS_FULL_L, STEPS_FULL_R);
        int32_t err = target_steps - current_steps;

        uint32_t now = millis();
        float dt = (now - pid_last_ms) / 1000.0f;
        pid_last_ms = now;
        if (dt <= 0.0f || dt > 0.5f) dt = 0.02f;

        if (abs(err) <= PID_DEADBAND) {
            if (pid_running) { stepper_ledc_stop(); pid_running = false; }
            pid_integral *= 0.9f;
            pid_prev_err  = 0.0f;
        } else {
            float P = PID_KP * (float)err;
            pid_integral += PID_KI * (float)err * dt;
            pid_integral  = constrain(pid_integral, -PID_I_MAX, PID_I_MAX);
            float D = PID_KD * ((float)err - pid_prev_err) / dt;
            pid_prev_err = (float)err;
            
            float output = P + pid_integral + D;
            uint32_t hz  = (uint32_t)constrain(fabsf(output) * ((float)PID_HZ_MAX / 100.0f), (float)PID_HZ_MIN, (float)PID_HZ_MAX);
            bool new_dir = (output >= 0.0f);
            
            if (!pid_running || new_dir != pid_dir) {
                stepper_ledc_stop(); delayMicroseconds(10);
                if (new_dir != pid_dir) { pid_integral = 0.0f; pid_prev_err = 0.0f; }
                digitalWrite(STEPPER_DIR_PIN, new_dir ? HIGH : LOW);
                delayMicroseconds(5);
                pid_dir = new_dir; pid_hz = hz; pid_running = true;
                stepper_ledc_run(hz);
            } else if (abs((int)hz - (int)pid_hz) > 50) {
                pid_hz = hz; stepper_ledc_run(hz);
            }
        }
    } else {
        // IDLE or MANUAL (Safely halt step generation)
        if (pid_running) {
            stepper_ledc_stop();
            pid_running = false;
            pid_integral = 0.0f;
        }
    }
}
```

### core1\steer.h

```plaintext
// core1\steer.h
#pragma once
#include <Arduino.h>

void Steer_Init();
void Steer_Update();
```

### core1\ui.cpp

```cpp
// core1\ui.cpp
#include "ui.h"
#include "state.h"
#include "config/pins.h"
#include "config/constants.h"
#include <Wire.h>

// ─── OLED DISPLAY (SH1106) ──────────────────────────────────────
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> 

Adafruit_SH1106G oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool oled_ok = false;

void UI_Init() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    // Using your original SH110X initialization parameters
    if (!oled.begin(OLED_ADDR, true)) { 
        Serial.println("[UI] SH1106 not found. Check I2C wiring."); 
        oled_ok = false; 
        return; 
    }
    
    oled_ok = true; 
    oled.clearDisplay(); 
    oled.setTextSize(1); 
    oled.setTextColor(SH110X_WHITE);
    oled.setCursor(0, 0);  
    oled.println("SIDLAK 2 VCS");
    oled.println("System Ready.");
    oled.display(); 
    
    Serial.println("[UI] SH1106 OK.");
}

void UI_Update() {
    if (!oled_ok) return;

    // Grab a safe snapshot of the hardware state from Core 1
    // Reads pre-calculated, cached data arrays instead of calculating inside its own loop
    VCS_State_t snap = StateHub_GetSnapshot();

    oled.clearDisplay(); 
    oled.setCursor(0, 0);
    
    // Map original diagnostic layout directly to the StateHub variables
    oled.printf("RPM: %.1f\n", snap.mechanical_rpm);
    oled.printf("SPD: %.2f km/h\n", snap.velocity_kmh);
    oled.printf("ODO: %.2f m\n", snap.odometer_m);
    oled.printf("STR: %lu mV\n", snap.pot_filtered_mv);
    
    // Output warning registers and system statuses directly to the operator
    if (snap.target_mode == 50) oled.println("MODE: AUTONOMOUS");
    else if (snap.target_mode == 100) oled.println("MODE: MANUAL");
    else oled.println("MODE: IDLE");

    if (snap.brake_at_limit) {
        oled.println("WARN: BRAKE LIMIT HIT");
    }

    oled.display();
}
```

### core1\ui.h

```plaintext
// core1\ui.h
#pragma once
#include <Arduino.h>

void UI_Init();
void UI_Update();
```

### main.cpp

```cpp
// main.cpp
#include <Arduino.h>
#include <esp_task_wdt.h>

// ─── CORE 0 MODULES ───
#include "core0/uart.h"
#include "core0/mode.h"
#include "core0/web.h"
#include "core0/calibration.h"

// ─── CORE 1 MODULES ───
#include "core1/steer.h"
#include "core1/actr.h"
#include "core1/hall.h"
#include "core1/rev.h"
#include "core1/ui.h"

// ─── TASK HANDLES ───────────────────────────────────────────────
TaskHandle_t TaskHandle_Control = NULL;
TaskHandle_t TaskHandle_Display = NULL;
TaskHandle_t TaskHandle_Comm    = NULL;
TaskHandle_t TaskHandle_UI      = NULL;
TaskHandle_t TaskHandle_Web     = NULL;

// ─── WATCHDOG CONFIGURATION ─────────────────────────────────────
#define WDT_TIMEOUT_SECONDS 10

// ════════════════════════════════════════════════════════════════
//  CORE 1: HARD REAL-TIME EXECUTION (THE REFLEX)
//  Cannot be interrupted by background serial/Wi-Fi operations.
// ════════════════════════════════════════════════════════════════

void Task_Control(void *pvParameters) {

    esp_task_wdt_add(NULL);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10 ms execution loop

    for (;;) {
        esp_task_wdt_reset();

        // 1. Evaluate Master FSM & Safety Overrides
        Mode_Update();
        
        // 2. Drive the Stepper PID Engine
        Steer_Update();
        
        // 3. Write target speeds to DAC and evaluate Relay states
        Actr_Update();
        
        // 4. Validate directional shifts
        Rev_Update();

        // Halt task until the 10ms window completes
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void Task_Display(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 50 ms execution loop
    
    for (;;) {
        // Reads cached memory arrays to update the OLED without calculating data
        UI_Update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ════════════════════════════════════════════════════════════════
//  CORE 0: COMMUNICATION & BACKGROUND (THE BRIDGE)
//  Isolates network interactions from physical vehicle control.
// ════════════════════════════════════════════════════════════════

void Task_Comm(void *pvParameters) {

    esp_task_wdt_add(NULL);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10 ms execution loop
    
    for (;;) {

        esp_task_wdt_reset();

        // Intercept and CRC-validate Jetson UART packets
        Uart_Update(); 
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void Task_UI(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 50 ms execution loop
    
    for (;;) {
        // Manage high-level diagnostic data structures
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void Task_Web(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 50 ms execution loop
    
    for (;;) {
        // Push 20Hz telemetry via WebSockets to local dashboard
        Web_Update(); 
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ════════════════════════════════════════════════════════════════
//  SYSTEM BOOT
// ════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n[SYSTEM] SIDLAK 2 VCS - Booting Dual-Core Architecture");

    esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);

    // ─── Initialize Modules (Hardware Setup) ───
    Calibration_Init();
    Uart_Init();
    Mode_Init();
    Web_Init();
    
    Steer_Init();
    Actr_Init();
    Hall_Init();      // Attaches the wheel encoder ISR
    Rev_Init();
    UI_Init();

    Serial.println("[SYSTEM] Hardware initialized. Dispatching FreeRTOS tasks...");

    // ─── CORE 1: THE REFLEX (Safety & Hardware) ───
    xTaskCreatePinnedToCore(
        Task_Control,          // Function to implement the task
        "ControlTask",         // Name of the task
        4096,                  // Stack size in words
        NULL,                  // Task input parameter
        5,                     // Priority: 5 (Highest)
        &TaskHandle_Control,   // Task handle
        1);                    // Core: 1

    xTaskCreatePinnedToCore(
        Task_Display,          
        "DisplayLoop",         //
        4096,                  
        NULL,                  
        1,                     // Priority: 1
        &TaskHandle_Display,   
        1);                    // Core: 1

    // ─── CORE 0: THE BRIDGE (Comms & Networking) ───
    xTaskCreatePinnedToCore(
        Task_Comm,             
        "ESP32_CommLoop",      //
        4096,                  
        NULL,                  
        4,                     // Priority: 4
        &TaskHandle_Comm,      
        0);                    // Core: 0

    xTaskCreatePinnedToCore(
        Task_UI,               
        "ESP32_UILoop",        //
        2048,                  
        NULL,                  
        2,                     // Priority: 2
        &TaskHandle_UI,        
        0);                    // Core: 0

    xTaskCreatePinnedToCore(
        Task_Web,              
        "WebServerTask",       //
        4096,                  
        NULL,                  
        1,                     // Priority: 1 (Lowest)
        &TaskHandle_Web,       
        0);                    // Core: 0

    Serial.println("[SYSTEM] Scheduler active.");

    // Delete the default Arduino loop() task to reclaim memory, 
    // as FreeRTOS is now entirely in control.
    vTaskDelete(NULL);
}

void loop() {
    // Execution will never reach this block.
}
```

### state.cpp

```cpp
// state.cpp
#include "state.h"

// Instantiate the actual struct in memory (zero-initialized)
VCS_State_t globalState = {0};

// Instantiate and unlock the FreeRTOS spinlock
portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
```

### state.h

```plaintext
// state.h
#pragma once
#include <Arduino.h>

// ════════════════════════════════════════════════════════════════
//  VCS STATE STRUCTURE
//  Holds all targets, telemetry, and physical sensor states.
// ════════════════════════════════════════════════════════════════
struct VCS_State_t {
    // ─── JETSON TARGETS (Written by Core 0, Read by Core 1) ───
    uint8_t  target_mode;         // 0 = Idle, 50 = Auto, 100 = Manual
    uint16_t target_steering;     // 0 to 10000 (Maps to absolute steps)
    uint16_t target_speed_pwm;    // 0 to 10000 (High-res DAC output)
    uint8_t  target_brake;        // 0 to 100 (%)
    uint8_t  target_direction;    // 0 = Fwd, 100 = Rev

    // ─── TELEMETRY (Written by Core 1, Read by Core 0) ────────
    float    mechanical_rpm;
    float    velocity_kmh;
    float    odometer_m;

    // ─── HARDWARE SENSOR CACHE (Written by Core 1) ────────────
    uint32_t pot_filtered_mv;     // Smoothed steering position
    bool     deadman_active;      // Master safety switch state
    bool     brake_at_limit;      // Linear actuator limit switch
    bool     reverse_switch;      // Physical dashboard toggle
};

// ════════════════════════════════════════════════════════════════
//  GLOBAL EXPORTS
// ════════════════════════════════════════════════════════════════
// The actual memory struct holding the data
extern VCS_State_t globalState;

// The FreeRTOS hardware spinlock
extern portMUX_TYPE stateMux;

// ════════════════════════════════════════════════════════════════
//  THREAD-SAFE ACCESS FUNCTIONS
//  Use these instead of accessing globalState directly.
// ════════════════════════════════════════════════════════════════

// Call this from Core 0 to safely inject new Jetson targets
inline void StateHub_UpdateTargets(uint8_t mode, uint16_t steer, uint16_t speed, uint8_t brake, uint8_t dir) {
    portENTER_CRITICAL(&stateMux);
    globalState.target_mode      = mode;
    globalState.target_steering  = steer;
    globalState.target_speed_pwm = speed;
    globalState.target_brake     = brake;
    globalState.target_direction = dir;
    portEXIT_CRITICAL(&stateMux);
}

// Call this from Core 1 to safely inject new odometry
inline void StateHub_UpdateTelemetry(float rpm, float kmh, float odo) {
    portENTER_CRITICAL(&stateMux);
    globalState.mechanical_rpm = rpm;
    globalState.velocity_kmh   = kmh;
    globalState.odometer_m     = odo;
    portEXIT_CRITICAL(&stateMux);
}

// Call this from ANY core to get a safe, instantaneous snapshot of the entire car
inline VCS_State_t StateHub_GetSnapshot() {
    VCS_State_t snapshot;
    portENTER_CRITICAL(&stateMux);
    snapshot = globalState; // Fast block-copy of the struct
    portEXIT_CRITICAL(&stateMux);
    return snapshot;
}
```

