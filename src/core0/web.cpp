#include "core0/web.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "state.h"
#include "core0/calibration.h"

// ─── LOCAL STATE ────────────────────────────────────────────────
static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

// ─── WEBSOCKET EVENT HANDLER ────────────────────────────────────
static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                    void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            data[len] = 0; // Null-terminate the string
            
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, (char*)data);
            
            if (!err && doc["tune"].is<JsonObject>()) { {
                // 1. Catch runtime slider adjustments
                if (doc["tune"]["kp"].is<float>()) sysCal.pid_kp = doc["tune"]["kp"];
                if (doc["tune"]["ki"].is<float>()) sysCal.pid_ki = doc["tune"]["ki"];
                if (doc["tune"]["kd"].is<float>()) sysCal.pid_kd = doc["tune"]["kd"];
                if (doc["tune"]["alpha"].is<float>()) sysCal.ema_alpha = doc["tune"]["alpha"];
                
                Serial.printf("[WEB] Tuner Update: Kp=%.2f, Kd=%.2f, Alpha=%.2f\n", 
                              sysCal.pid_kp, sysCal.pid_kd, sysCal.ema_alpha);
                
                // 2. Save permanently to flash memory (NVS)
                // Note: Calibration_Save() automatically locks out writes if in Autonomous mode
                Calibration_Save(); 
                }
            }
        }
    }
}
// ─── PUBLIC API ─────────────────────────────────────────────────
void Web_Init() {
    // Host local Wi-Fi Access Point
    WiFi.softAP("SIDLAK_VCS_LIVE", "sidlak2026");
    
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();
    
    Serial.println("[WEB] Async AP & WebSockets Initialized.");
    Serial.print("[WEB] Connect to IP: ");
    Serial.println(WiFi.softAPIP());
}

void Web_Update() {
    // Clean up disconnected clients
    ws.cleanupClients();
    
    // Only compile and send JSON if someone is actively listening
    if (ws.count() > 0) {
        VCS_State_t snap = StateHub_GetSnapshot();
        
        // Manual JSON compilation is much faster than ArduinoJson for high-frequency 20Hz streams
        char json_payload[256];
        snprintf(json_payload, sizeof(json_payload),
            "{\"mode\":%d,\"tgt_str\":%u,\"act_str\":%lu,\"tgt_spd\":%u,\"act_rpm\":%.1f,\"act_kmh\":%.2f}",
            snap.target_mode, 
            snap.target_steering, 
            snap.pot_filtered_mv, 
            snap.target_speed_pwm, 
            snap.mechanical_rpm,
            snap.velocity_kmh);
            
        // Broadcast to all connected Web Tuner dashboards
        ws.textAll(json_payload);
    }
}