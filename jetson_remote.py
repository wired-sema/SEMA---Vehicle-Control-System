import serial
import struct
import time
import threading
import sys

# Configure your Jetson's serial port (Update '/dev/ttyUSB0' if using direct GPIO UART like '/dev/ttyTHS1')
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200

try:
    esp32 = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
except Exception as e:
    print(f"Failed to open port: {e}")
    sys.exit(1)

# ─── VEHICLE STATE ───
target_mode = 100    # Default to Manual (100)
target_steer = 5000  # Center (0 to 10000)
target_speed = 0     # 0% (0 to 10000)
target_brake = 0     # 0%
target_dir = 0       # Forward (0)

def calculate_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1: crc = (crc >> 1) ^ 0xA001
            else: crc >>= 1
    return crc

def send_command():
    """Compiles and sends the 14-byte security payload."""
    payload = bytearray([
        0x01, 0x07, target_mode,
        (target_steer >> 8) & 0xFF, target_steer & 0xFF,
        (target_speed >> 8) & 0xFF, target_speed & 0xFF,
        target_brake, target_dir
    ])
    crc = calculate_crc16(payload)
    packet = bytearray([0xAA, 0x55]) + payload + bytearray([(crc >> 8) & 0xFF, crc & 0xFF, 0xFF])
    esp32.write(packet)

def telemetry_listener():
    """Runs in the background, listening for the ESP32's 10Hz feedback."""
    while True:
        if esp32.in_waiting >= 14:
            if esp32.read(1) == b'\xAA' and esp32.peek(1) == b'\x55':
                packet = b'\xAA' + esp32.read(13)
                if packet[13] == 0xFF:
                    calc_crc = calculate_crc16(packet[2:11])
                    recv_crc = (packet[11] << 8) | packet[12]
                    
                    if calc_crc == recv_crc:
                        rpm = struct.unpack('>h', packet[4:6])[0]
                        steer_mv = struct.unpack('>h', packet[6:8])[0]
                        print(f"\r[TELEMETRY] RPM: {rpm} | Steer ADC: {steer_mv}mV | Actuator Limit: {packet[8]==100}", end="")
        time.sleep(0.01)

# Start background telemetry listener
threading.Thread(target=telemetry_listener, daemon=True).start()

# ─── THE CLI REMOTE ───
print("\n=== SIDLAK VCS TELEOP REMOTE ===")
print("Commands:")
print("  'E' : Enable AUTO Mode (50)")
print("  'D' : Disable AUTO Mode (100 - Manual)")
print("  'W' : Throttle +10%")
print("  'S' : Throttle -10%")
print("  'A' : Steer Left")
print("  'F' : Steer Right")
print("  'Q' : Quit & Kill Power")
print("================================\n")

try:
    while True:
        cmd = input("\n> ").upper()
        
        if cmd == 'E':
            target_mode = 50
            print(">> REQUESTING AUTO MODE")
        elif cmd == 'D':
            target_mode = 100
            target_speed = 0
            print(">> REVERTING TO MANUAL MODE")
        elif cmd == 'W':
            target_speed = min(10000, target_speed + 1000)
            print(f">> SPEED: {target_speed/100}%")
        elif cmd == 'S':
            target_speed = max(0, target_speed - 1000)
            print(f">> SPEED: {target_speed/100}%")
        elif cmd == 'A':
            target_steer = max(0, target_steer - 1000)
            print(f">> STEER TARGET: {target_steer}")
        elif cmd == 'F':
            target_steer = min(10000, target_steer + 1000)
            print(f">> STEER TARGET: {target_steer}")
        elif cmd == 'Q':
            target_mode = 100
            target_speed = 0
            send_command()
            print(">> EMERGENCY STOP. Exiting.")
            sys.exit(0)
            
        send_command()
        time.sleep(0.1) # Give telemetry time to print
        
except KeyboardInterrupt:
    target_mode = 100
    target_speed = 0
    send_command()
    print("\nExiting.")