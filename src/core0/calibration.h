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