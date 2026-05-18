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