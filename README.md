# GyroGloves

# Gyro Gloves Control using MPU6500 and Arduino

This project turns an **MPU6500 accelerometer** into a motion-based **mouse controller**. Move your hand **left/right and forward/backward** to control the mouse cursor. The system does not require rolling—just slight tilts to control movement.

## Features

- Uses an **MPU6500 accelerometer** to control mouse movement
- Supports **forward/backward (Y-axis) and left/right (X-axis)** movements
- Works by tilting, not rolling
- Adjustable **sensitivity, dead zones, and filtering** for smoother movement
- Uses **USB HID** to act as a mouse (compatible with boards that support `Mouse.h`)

## Hardware Requirements

- Arduino **Leonardo / Pro Micro** (or any board supporting USB HID)
- **MPU6500** accelerometer
- Jumper wires

## Setup & Installation

### 1. Wiring the MPU6500

| MPU6500 Pin | Arduino Pin |
|-------------|------------|
| VCC         | 3.3V       |
| GND         | GND        |
| SDA         | A4 (SDA)   |
| SCL         | A5 (SCL)   |

### 2. Uploading the Code

1. Install the **Arduino IDE** and connect your Arduino board.
2. Install any required **MPU6500 libraries** (if needed).
3. Copy the **[GyroGloves.ino](GyroGloves.ino)** file into the IDE.
4. Upload the code to your board.

## How It Works

- **Tilt forward** → Moves the mouse up
- **Tilt backward** → Moves the mouse down
- **Tilt left/right** → Moves the mouse left/right
- **Deadzone prevents unintentional movement**

Mouse sensitivity and dead zones can be adjusted in the **configuration section** of the code.

## Configuration

To fine-tune movement, modify these values in the code:

```cpp
const float DEADZONE = 10.0;  // Minimum tilt angle required for movement
const int MOUSE_SCALE = 3;    // Mouse movement per degree of tilt
const float FILTER_COEFF = 0.2; // Smoothing factor for stability
```

## License

This project is licensed under the **MIT License**.

## Contributions & Issues

Feel free to contribute by opening a **Pull Request** or reporting any issues in the **Issues** tab.
