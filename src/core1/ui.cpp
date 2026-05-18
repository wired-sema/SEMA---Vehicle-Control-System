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