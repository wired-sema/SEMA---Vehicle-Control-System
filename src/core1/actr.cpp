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