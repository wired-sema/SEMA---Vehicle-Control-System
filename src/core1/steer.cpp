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