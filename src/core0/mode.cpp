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
    
    // Assuming either grip being held satisfies the deadman condition
    bool is_deadman_active = (dm1 || dm2); 
    bool is_brake_pressed  = (digitalRead(BRAKE_SWITCH_PIN) == LOW);

    // 2. Lock the StateHub to perform safety checks and data updates
    portENTER_CRITICAL(&stateMux);
    
    // Update the hardware cache so the Jetson telemetry packet can read it later
    globalState.deadman_active = is_deadman_active;

    // ════════════════════════════════════════════════════════════════
    //  MASTER SAFETY OVERRIDES (The FSM Rules)
    // ════════════════════════════════════════════════════════════════

    // RULE A: Physical Brake Override
    // If the driver stomps the physical brake pedal, kill all drive power instantly, 
    // regardless of whether the car is in Auto (50) or Manual (100) mode.
    if (is_brake_pressed) {
        globalState.target_speed_pwm = 0;
        
        // Optionally command the linear actuator to assist braking
        globalState.target_brake = 100; 
    }

    // RULE B: Manual Mode Deadman Interlock
    // If the car is in Manual Mode (100) but the driver releases the deadman grips,
    // force the system into Idle (0) and cut the throttle.
    if (globalState.target_mode == 100 && !is_deadman_active) {
        globalState.target_mode = 0;
        globalState.target_speed_pwm = 0;
    }

    portEXIT_CRITICAL(&stateMux);
}