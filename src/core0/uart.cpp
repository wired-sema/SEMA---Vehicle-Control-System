#include "core0/uart.h"
#include "state.h"
#include "config/pins.h"
#include "config/constants.h"

static uint8_t rx_buf[PACKET_LEN];
static uint8_t tx_buf[PACKET_LEN];
static uint32_t last_tx_time = 0;

// CRC16 Security Validation
static uint16_t calculate_crc16(uint8_t *d, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= d[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

void Uart_Init() {
    Serial2.begin(UART_BAUD, SERIAL_8N1, JETSON_RX_PIN, JETSON_TX_PIN);
    Serial.println("[UART] Gateway initialized on UART2.");
}

static void send_telemetry() {
    VCS_State_t snap = StateHub_GetSnapshot();
    
    int16_t  rv = (int16_t)snap.mechanical_rpm;
    // Scale physical steering mV back to a 0-10000 Jetson percentage
    uint16_t sv = (uint16_t)map(snap.pot_filtered_mv, 0, 3300, 0, 10000); 

    tx_buf[0] = PKT_START1; 
    tx_buf[1] = PKT_START2;
    tx_buf[2] = PKT_TYPE_TX; 
    tx_buf[3] = 0x07;
    tx_buf[4] = (rv >> 8) & 0xFF; 
    tx_buf[5] = rv & 0xFF;
    tx_buf[6] = (sv >> 8) & 0xFF; 
    tx_buf[7] = sv & 0xFF;
    tx_buf[8] = snap.brake_at_limit ? 100 : 0; 
    tx_buf[9] = 0; 
    tx_buf[10] = snap.reverse_switch ? 100 : 0;
    
    uint16_t crc = calculate_crc16(&tx_buf[2], 9);
    tx_buf[11] = (crc >> 8) & 0xFF; 
    tx_buf[12] = crc & 0xFF; 
    tx_buf[13] = PKT_END;
    
    Serial2.write(tx_buf, PACKET_LEN);
}

void Uart_Update() {
    // 1. Intercept structured command arrays
    if (Serial2.available() >= PACKET_LEN) {
        if (Serial2.read() == PKT_START1 && Serial2.peek() == PKT_START2) {
            rx_buf[0] = PKT_START1;
            Serial2.readBytes(&rx_buf[1], PACKET_LEN - 1);
            
            if (rx_buf[PACKET_LEN - 1] == PKT_END && rx_buf[2] == PKT_TYPE_RX) {
                uint16_t calculated_crc = calculate_crc16(&rx_buf[2], 9);
                uint16_t received_crc   = (rx_buf[11] << 8) | rx_buf[12];
                
                // 2. Perform CRC16 validation
                if (calculated_crc == received_crc) {
                    uint8_t  mode  = rx_buf[4];
                    uint16_t steer = (rx_buf[5] << 8) | rx_buf[6];
                    uint16_t speed = (rx_buf[7] << 8) | rx_buf[8];
                    uint8_t  brake = rx_buf[9];
                    uint8_t  dir   = rx_buf[10];

                    // 3. Pass frames into shared state registers safely
                    StateHub_UpdateTargets(mode, steer, speed, brake, dir);
                }
            }
        }
    }

    // Push physical odometry metrics back to Jetson at 10Hz
    if (millis() - last_tx_time >= 100) {
        send_telemetry();
        last_tx_time = millis();
    }
}