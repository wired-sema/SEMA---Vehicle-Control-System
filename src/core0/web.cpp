#include "core0/web.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "state.h"
#include "core0/calibration.h"

// ─── IMPORT THE SEPARATED HTML UI ───
#include "web_ui.h" 

// ─── LOCAL STATE ────────────────────────────────────────────────
static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

// ─── WEBSOCKET EVENT HANDLER ────────────────────────────────────
static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                    void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            data[len] = 0; 
            
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, (char*)data);
            
            if (!err && doc.containsKey("tune")) {
                if (doc["tune"].containsKey("kp")) sysCal.pid_kp = doc["tune"]["kp"];
                if (doc["tune"].containsKey("ki")) sysCal.pid_ki = doc["tune"]["ki"];
                if (doc["tune"].containsKey("kd")) sysCal.pid_kd = doc["tune"]["kd"];
                if (doc["tune"].containsKey("alpha")) sysCal.ema_alpha = doc["tune"]["alpha"];
                
                Serial.printf("[WEB] Tuner Update: Kp=%.2f, Kd=%.2f, Alpha=%.2f\n", 
                              sysCal.pid_kp, sysCal.pid_kd, sysCal.ema_alpha);
                
                Calibration_Save(); 
            }
        }
    }
}

// ─── PUBLIC API ─────────────────────────────────────────────────
void Web_Init() {
    // ─── Simulation Mode Note ───
    // If testing in Wokwi, change softAP to: WiFi.begin("Wokwi-GUEST", "", 6);
    WiFi.softAP("SIDLAK_VCS_LIVE", "sidlak2026");
    
    // Serve the separated HTML file when a device connects
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();
    
    Serial.println("[WEB] Async AP & Web Dashboard Initialized.");
    Serial.print("[WEB] Connect to IP: ");
    Serial.println(WiFi.softAPIP());
}

void Web_Update() {
    ws.cleanupClients();
    
    if (ws.count() > 0) {
        VCS_State_t snap = StateHub_GetSnapshot();
        
        char json_payload[256];
        snprintf(json_payload, sizeof(json_payload),
            "{\"mode\":%d,\"tgt_str\":%u,\"act_str\":%lu,\"tgt_spd\":%u,\"act_rpm\":%.1f,\"act_kmh\":%.2f}",
            snap.target_mode, 
            snap.target_steering, 
            snap.pot_filtered_mv, 
            snap.target_speed_pwm, 
            snap.mechanical_rpm,
            snap.velocity_kmh);
            
        ws.textAll(json_payload);
    }
}