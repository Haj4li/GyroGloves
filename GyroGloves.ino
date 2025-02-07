#include <Wire.h>
#include <Mouse.h>

#define MPU6500_ADDRESS  0x68
#define ACCEL_XOUT_H     0x3B

// Configuration
const float DEADZONE = 10.0;        // Degrees where no movement occurs
const float MAX_TILT = 30.0;        // Maximum effective tilt angle
const int MOUSE_SCALE = 3;          // Pixels per degree of tilt
const float FILTER_COEFF = 0.2;     // Smoothing factor (0.1-0.3)

// Calibration offsets
float ax_offset = 0, ay_offset = 0, az_offset = 0;

// Filtered values
float smooth_pitch = 0, smooth_roll = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Mouse.begin();
  
  // Initialize MPU6500
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  calibrateMPU();
  Serial.println("MPU6500 Ready");
}

void loop() {
  // Read raw accelerometer data
  int16_t ax, ay, az;
  readAccel(ax, ay, az);

  // Apply calibration
  float ax_cal = ax - ax_offset;
  float ay_cal = ay - ay_offset;
  float az_cal = az - az_offset;

  // Convert to gravity units (1g = 16384 LSB)
  float ax_g = ax_cal / 16384.0;
  float ay_g = ay_cal / 16384.0;
  float az_g = az_cal / 16384.0;

  // Calculate tilt angles
  float pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180 / PI;
  float roll = atan2(ay_g, az_g) * 180 / PI;

  // Apply smoothing
  smooth_pitch = smooth_pitch * (1 - FILTER_COEFF) + pitch * FILTER_COEFF;
  smooth_roll = smooth_roll * (1 - FILTER_COEFF) + roll * FILTER_COEFF;

  // Calculate mouse movement
  int moveX = 0, moveY = 0;

  // Move Forward/Backward using pitch
  if (abs(smooth_pitch) > DEADZONE) {
    moveY = constrain((smooth_pitch - copysign(DEADZONE, smooth_pitch)) * MOUSE_SCALE, -127, 127);
  }

  // Move Left/Right using Y-axis tilt (ay_g), NOT roll
  if (abs(ay_g) > 0.1) {  // Small threshold to prevent drift
    moveX = constrain(ay_g * MOUSE_SCALE * 20, -127, 127);  // Scale factor for sensitivity
  }

  // Send mouse movement
  if (moveX != 0 || moveY != 0) {
    Mouse.move(moveX, moveY);
  }

  delay(10);
}

void readAccel(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6500_ADDRESS, 6);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}

void calibrateMPU() {
  Serial.println("Calibrating... Keep sensor flat!");
  delay(2000);

  const int samples = 500;
  long axSum = 0, aySum = 0, azSum = 0;

  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az;
    readAccel(ax, ay, az);
    axSum += ax;
    aySum += ay;
    azSum += az;
    delay(5);
  }

  ax_offset = axSum / samples;
  ay_offset = aySum / samples;
  az_offset = (azSum / samples) - 16384; // Account for 1G Z-axis

  Serial.println("Calibration complete");
}
