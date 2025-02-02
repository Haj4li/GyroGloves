import serial
import time
import pyautogui
import numpy as np

# === CONFIGURATION ===
mouse_movement = True  # Enable or disable mouse control
reverse_x = False      # Reverse horizontal movement
reverse_y = False      # Reverse vertical movement
calibration_samples = 100  # Number of samples for calibration
smoothing_factor = 0.1  # Adjust for smoother movement (0.1 - 0.5 recommended)

# Open serial port
ser = serial.Serial("COM5", 115200, timeout=1)  # Change to your COM port
time.sleep(2)  # Allow time for connection

# === Calibration (Average Baseline) ===
print("Calibrating... Move the sensor steadily for a few seconds.")
calibration_data = []

for _ in range(calibration_samples):
    line = ser.readline().decode("utf-8").strip()
    if line:
        try:
            values = list(map(int, line.split(",")))
            calibration_data.append(values[3:5])  # Use GyroX and GyroY
        except:
            pass  # Ignore parsing errors

gyro_baseline = np.mean(calibration_data, axis=0)
print(f"Calibration done: GyroX: {gyro_baseline[0]:.2f}, GyroY: {gyro_baseline[1]:.2f}")

# Smoothing variables
prev_x, prev_y = 0, 0

def get_gyro():
    """ Reads gyro data from serial and applies calibration. """
    line = ser.readline().decode("utf-8").strip()
    if not line:
        return None, None
    
    try:
        values = list(map(int, line.split(",")))
        gx, gy = values[3], values[4]  # GyroX, GyroY
        gx -= gyro_baseline[0]  # Apply calibration
        gy -= gyro_baseline[1]
        return gx, gy
    except:
        return None, None  # Ignore errors

def smooth(value, prev_value):
    """ Apply smoothing using exponential moving average. """
    return (smoothing_factor * value) + ((1 - smoothing_factor) * prev_value)

while True:
    gx, gy = get_gyro()
    if gx is None or gy is None:
        continue  # Skip invalid readings

    # Smooth the movements
    smoothed_x = smooth(gx, prev_x)
    smoothed_y = smooth(gy, prev_y)

    # Adjust movement
    move_x = int(smoothed_x * 0.5)  # Scaling factor
    move_y = int(smoothed_y * 0.5)

    # Reverse directions if enabled
    if reverse_x:
        move_x = -move_x
    if reverse_y:
        move_y = -move_y

    # Apply movement
    if mouse_movement and (abs(move_x) > 1 or abs(move_y) > 1):
        pyautogui.moveRel(move_x, -move_y)  # Invert Y for natural feel

    # Store last values for smoothing
    prev_x, prev_y = smoothed_x, smoothed_y

    time.sleep(0.01)  # Adjust delay for responsiveness
