#pragma once

// ════════════════════════════════════════════════════════════════
//  PINS (ESP32-WROOM-32 DevKit)
// ════════════════════════════════════════════════════════════════

// ─── COMMUNICATION ───
#define JETSON_RX_PIN       16  // UART RX from Jetson
#define JETSON_TX_PIN       17  // UART TX to Jetson
#define I2C_SDA_PIN         21  // OLED / Sensors
#define I2C_SCL_PIN         22  

// ─── ANALOG SENSORS & ODOMETRY ───
#define THROTTLE_ADC_PIN    34  // Physical pedal input
#define STEERING_ADC_PIN    35  // Steering angle potentiometer
#define HALL_C_PIN          32  // Wheel speed interrupt

// ─── SAFETY & SWITCHES (DIGITAL IN) ───
#define DEADMAN_1_PIN       33  // AND-Gate grip 1
#define DEADMAN_2_PIN       27  // AND-Gate grip 2
#define BRAKE_SWITCH_PIN    14  // Hard physical brake pedal
#define REVERSE_SW_PIN      26  // Forward/Reverse toggle
#define LIMIT_SWITCH_PIN    13  // Brake actuator boundary

// ─── MOTOR CONTROL & ACTUATORS (DIGITAL/PWM OUT) ───
#define THROTTLE_DAC_PIN    25  // 0-3.3V Output to Motor Controller
#define STEPPER_PUL_PIN     18  // Steering Pulses
#define STEPPER_DIR_PIN     19  // Steering Direction
#define STEPPER_ENA_PIN     23  // Steering Enable
#define TB6612_IN1_PIN       4  // Brake linear actuator Dir 1
#define TB6612_IN2_PIN       2  // Brake linear actuator Dir 2
#define TB6612_PWM_PIN       5  // Brake linear actuator Power

// ─── RELAYS & STATUS (DIGITAL OUT) ───
#define RELAY_PIN           15  // Organizer Strobe (NO = Auto, NC = Manual)
#define BRAKE_MC_PIN        12  // Brake indicator/relay