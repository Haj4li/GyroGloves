#include <Wire.h>
#include <Mouse.h>

#define MPU6500_ADDRESS  0x68
#define ACCEL_XOUT_H     0x3B
#define GYRO_XOUT_H      0x43
#define ACTION_PIN 2

// Calibration parameters
float ax_offset = 0, ay_offset = 0, az_offset = 0;
float gx_offset = 0, gy_offset = 0, gz_offset = 0;

// Movement parameters
const float DEADZONE = 0.15;          // 15% tilt deadzone
const float MAX_ANGLE = 45.0;         // Maximum tilt angle (degrees)
const float RESPONSIVENESS = 0.2;     // Movement responsiveness (0.1-1.0)
const float DRIFT_COMPENSATION = 0.8; // Velocity retention factor
const bool REVERSE_MOUSE = true;

// Smoothing parameters
const float FILTER_ALPHA = 0.90;      // Low-pass filter coefficient
const float GRAVITY = 9.81;           // m/s²

// State tracking
float velocityX = 0, velocityY = 0;
unsigned long lastUpdate = 0;
float lp_ax = 0, lp_ay = 0, lp_az = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Mouse.begin();
  
  // Initialize MPU6500
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  pinMode(ACTION_PIN, INPUT_PULLUP);
  calibrateMPU();
  Serial.println("MPU6500 Ready");
}

void loop() {
  if (digitalRead(ACTION_PIN) == LOW) return; // Pause when button pressed
  
  int16_t ax, ay, az;
  float gx, gy, gz;
  
  // Read sensor data
  readSensorData(ax, ay, az, gx, gy, gz);
  
  // Calculate time delta
  unsigned long now = millis();
  float dt = (now - lastUpdate) / 1000.0;
  lastUpdate = now;

  // Convert to real-world units
  ax = (ax - ax_offset) * (GRAVITY / 16384.0);
  ay = (ay - ay_offset) * (GRAVITY / 16384.0);
  az = (az - az_offset) * (GRAVITY / 16384.0);

  // Low-pass filter
  lp_ax = FILTER_ALPHA * lp_ax + (1 - FILTER_ALPHA) * ax;
  lp_ay = FILTER_ALPHA * lp_ay + (1 - FILTER_ALPHA) * ay;
  lp_az = FILTER_ALPHA * lp_az + (1 - FILTER_ALPHA) * az;

  // Calculate tilt angles
  float pitch = atan2(-lp_ax, sqrt(lp_ay * lp_ay + lp_az * lp_az)) * 180/PI;
  float roll = atan2(lp_ay, lp_az) * 180/PI;

  // Calculate normalized movement vectors
  float moveX = constrain(roll / MAX_ANGLE, -1, 1);
  float moveY = constrain(pitch / MAX_ANGLE, -1, 1);

  // Apply deadzone
  if (abs(moveX) < DEADZONE) moveX = 0;
  if (abs(moveY) < DEADZONE) moveY = 0;

  // Update velocities
  velocityX = DRIFT_COMPENSATION * velocityX + moveX * RESPONSIVENESS;
  velocityY = DRIFT_COMPENSATION * velocityY + moveY * RESPONSIVENESS;

  // Apply mouse movement
  if (abs(velocityX) > 0.01 || abs(velocityY) > 0.01) {
    int moveX_pixels = velocityX * 30; // Sensitivity adjustment
    int moveY_pixels = velocityY * 30;
    
    Mouse.move(moveX_pixels, REVERSE_MOUSE ? moveY_pixels : moveY_pixels*-1);
  }

  delay(10);
}

void readSensorData(int16_t &ax, int16_t &ay, int16_t &az, float &gx, float &gy, float &gz) {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6500_ADDRESS, 6);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6500_ADDRESS, 6);
  gx = (Wire.read() << 8 | Wire.read()) * (250.0 / 32768.0);
  gy = (Wire.read() << 8 | Wire.read()) * (250.0 / 32768.0);
  gz = (Wire.read() << 8 | Wire.read()) * (250.0 / 32768.0);
}

void calibrateMPU() {
  delay(500);
  Serial.println("Calibrating... Keep sensor flat!");
  delay(2000);

  const int samples = 500;
  float axSum = 0, aySum = 0, azSum = 0;
  float gxSum = 0, gySum = 0, gzSum = 0;

  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az;
    float gx, gy, gz;
    readSensorData(ax, ay, az, gx, gy, gz);
    
    axSum += ax;
    aySum += ay;
    azSum += az;
    gxSum += gx;
    gySum += gy;
    gzSum += gz;
    delay(5);
  }

  ax_offset = axSum / samples;
  ay_offset = aySum / samples;
  az_offset = azSum / samples - 16384.0; // Account for 1G
  gx_offset = gxSum / samples;
  gy_offset = gySum / samples;
  gz_offset = gzSum / samples;
}