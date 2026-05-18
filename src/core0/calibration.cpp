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