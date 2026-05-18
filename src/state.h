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