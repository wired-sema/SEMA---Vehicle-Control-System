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