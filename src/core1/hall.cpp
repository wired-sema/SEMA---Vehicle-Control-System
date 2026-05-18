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