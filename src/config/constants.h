#pragma once

// ════════════════════════════════════════════════════════════════
//  LEDC CHANNELS
// ════════════════════════════════════════════════════════════════
#define STEPPER_LEDC_CH    0        // Stepper motor control (PWM frequency dynamically adjusted by PID)
#define STEPPER_LEDC_RES   8        // 8-bit resolution for fine-grained control at low speeds
#define TB6612_LEDC_CH     1        // TB6612FNG motor driver (Fixed 10 kHz to avoid audible noise)
#define TB6612_LEDC_FREQ   10000    // 10 kHz fixed frequency for TB6612 to prevent whining noise
#define TB6612_LEDC_RES    8        // 8-bit resolution for smooth throttle control

// ════════════════════════════════════════════════════════════════
//  THROTTLE (DAC & LM358)
// ════════════════════════════════════════════════════════════════
#define THROTTLE_R1_OHMS     10000.0f  // R1 is the fixed resistor to 5V
#define THROTTLE_R2_OHMS     18000.0f  // R2 is the variable resistor to GND (potentiometer)
#define THROTTLE_DEADBAND_MV   600     // Minimum voltage change required to register as a valid throttle input (to prevent noise at low speeds)
#define THROTTLE_MIN_MV        650     // Minimum voltage corresponding to 0% throttle (calibrated from testing)
#define THROTTLE_MAX_MV       3000     // Maximum voltage corresponding to 100% throttle (calibrated from testing)

// ════════════════════════════════════════════════════════════════
//  STEERING POTENTIOMETER PHYSICAL LIMITS
// ════════════════════════════════════════════════════════════════
#define STEPS_FULL_L      -2000     // Full left corresponds to -2000 steps from center
#define STEPS_FULL_R       2000     // Full right corresponds to +2000 steps from center

// ════════════════════════════════════════════════════════════════
//  FILTERS & PID
// ════════════════════════════════════════════════════════════════
#define MEDIAN_SAMPLES   7        // Number of samples for the median filter (must be odd)
#define EMA_ALPHA        0.10f    // Smoothing factor for the exponential moving average (0.10 = 10% new, 90% old)
#define PID_KP           3.0f     // Proportional gain for the PID controller (tuned for responsive yet stable steering)
#define PID_KD           0.4f     // Derivative gain for the PID controller (tuned to dampen overshoot without causing sluggishness)
#define PID_KI           0.0f     // Integral gain for the PID controller (set to 0 to prevent windup, as the system is responsive enough without it)
#define PID_HZ_MIN       300      // Minimum update frequency for the PID loop (below this, we risk oscillations due to slow response)
#define PID_HZ_MAX       4000     // Maximum update frequency for the PID loop (above this, we risk excessive CPU usage without meaningful performance gains)
#define PID_DEADBAND     15       // Deadband in steps where no correction is applied (to prevent jitter around the target position)
#define PID_I_MAX        100.0f   // Maximum integral term to prevent windup (not used since KI is 0, but defined for future tuning)

// ════════════════════════════════════════════════════════════════
//  STEPPER FOLLOW (MODE A)
// ════════════════════════════════════════════════════════════════
#define FOLLOW_UPDATE_MS        30 // Update interval for the follow mode (in milliseconds)
#define FOLLOW_DELTA_THRESHOLD   7 // Minimum step difference required to trigger a position update in follow mode (to prevent jitter from small target changes)
#define FOLLOW_HZ_MIN          400 // Minimum update frequency for follow mode (below this, the steering will feel unresponsive)
#define FOLLOW_HZ_MAX         4000 // Maximum update frequency for follow mode (above this, we risk excessive CPU usage without meaningful performance gains)
#define FOLLOW_HZ_SCALE         60 // Scaling factor for follow mode frequency based on target speed (higher speeds require more frequent updates to maintain control)

// ════════════════════════════════════════════════════════════════
//  ACTUATOR HARD TIMEOUTS
// ════════════════════════════════════════════════════════════════
#define BRAKE_EXTEND_TIMEOUT_MS 3000  // Maximum time allowed for the brake to be extended before triggering a safety fault (to prevent runaway braking in case of a stuck command)

// ════════════════════════════════════════════════════════════════
//  ODOMETRY (HALL SENSOR)
// ════════════════════════════════════════════════════════════════
#define WHEEL_CIRCUMFERENCE_M         1.2764f       // Circumference of the wheel in meters (calibrated from actual measurements)
#define POLE_PAIRS                    23            // Number of hall sensor transitions per mechanical revolution (calibrated from testing)
#define RPM_CALIBRATION_FACTOR        0.9587f       // Calibration factor to correct raw RPM readings from the hall sensor (accounts for timing inaccuracies and wheel slip, calibrated from testing against a GPS-based speedometer)
#define HALL_TRANSITIONS_PER_MECH_REV 46            // Total hall sensor transitions per mechanical revolution (2 transitions per pole pair)
#define RPM_SAMPLE_WINDOW_MS          500           // Time window for calculating RPM from hall sensor transitions (in milliseconds, balances responsiveness with stability of the RPM reading)
#define RPM_TIMEOUT_MS               1000           // Time after which RPM is considered zero if no hall transitions are detected (in milliseconds, prevents stale RPM readings when stopped)
#define HALL_DEBOUNCE_US              800           // Minimum time between valid hall sensor transitions to prevent false readings from noise (in microseconds, calibrated from oscilloscope measurements of the hall sensor signal)
#define MAX_SPEED_KMPH               45.0f          // Maximum expected speed of the vehicle in km/h (used for scaling and sanity checks on telemetry data, calibrated from testing)

// ════════════════════════════════════════════════════════════════
//  UART COMMUNICATIONS & OLED
// ════════════════════════════════════════════════════════════════
#define UART_BAUD   115200  // Baud rate for Jetson communication (must match the sender's baud rate)
#define PACKET_LEN  14      // Total packet length in bytes (2 start bytes + 1 type byte + 10 data bytes + 1 CRC byte)
#define PKT_START1  0xAA    // First start byte for packet synchronization (0xAA is a common choice for framing bytes due to its alternating bit pattern)
#define PKT_START2  0x55    // Second start byte for packet synchronization (0x55 complements 0xAA to help detect framing errors)
#define PKT_END     0xFF    // End byte for packet validation (not strictly necessary if we have a fixed packet length, but can help detect certain types of corruption)
#define PKT_TYPE_TX 0x02    // Packet type for telemetry data sent from Core 1 to the Jetson (arbitrary value, just needs to be distinct from PKT_TYPE_RX)
#define PKT_TYPE_RX 0x01    // Packet type for control commands received from the Jetson (arbitrary value, just needs to be distinct from PKT_TYPE_TX)

#define OLED_WIDTH   128   // OLED display width in pixels (common size for small automotive displays)
#define OLED_HEIGHT   64   // OLED display height in pixels (common size for small automotive displays)
#define OLED_ADDR   0x3C   // I2C address for the OLED display (0x3C is a common default for SSD1306-based displays)