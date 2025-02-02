#include <Wire.h>
#include <Mouse.h>

#define MPU6500_ADDRESS  0x68
#define ACCEL_XOUT_H     0x3B

// Configuration
const float DEADZONE = 0.2;           // 20% tilt required to start moving
const float MAX_TILT = 45.0;          // Maximum tilt angle (degrees)
const int BASE_SPEED = 20;             // Base movement speed (pixels per update)
const int UPDATE_INTERVAL = 20;       // Milliseconds between movements

// Calibration offsets
float ax_offset = 0, ay_offset = 0, az_offset = 0;

// State tracking
float currentSpeedX = 0;
float currentSpeedY = 0;
unsigned long lastUpdate = 0;

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
  if (millis() - lastUpdate < UPDATE_INTERVAL) return;
  lastUpdate = millis();

  // Read accelerometer
  int16_t ax, ay, az;
  readAccel(ax, ay, az);

  // Convert to real-world units
  float ax_g = (ax - ax_offset) / 16384.0;
  float ay_g = (ay - ay_offset) / 16384.0;
  float az_g = (az - az_offset) / 16384.0;

  // Calculate tilt angles
  float pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180/PI;
  float roll = atan2(ay_g, az_g) * 180/PI;

  // Calculate movement vectors
  float targetX = constrain(roll / MAX_TILT, -1, 1);
  float targetY = constrain(pitch / MAX_TILT, -1, 1);

  // Apply deadzone
  if (abs(targetX) < DEADZONE) targetX = 0;
  if (abs(targetY) < DEADZONE) targetY = 0;

  // Smooth acceleration/deceleration
  currentSpeedX = lerp(currentSpeedX, targetX, 0.2);
  currentSpeedY = lerp(currentSpeedY, targetY, 0.2);

  // Calculate movement
  int moveX = round(currentSpeedX * BASE_SPEED);
  int moveY = round(currentSpeedY * BASE_SPEED);

  // Apply mouse movement
  if (moveX != 0 || moveY != 0) {
    Mouse.move(moveX, moveY);
  }
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
  float axSum = 0, aySum = 0, azSum = 0;

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
  az_offset = azSum / samples - 16384.0; // Account for 1G
}

float lerp(float a, float b, float t) {
  return a + t * (b - a);
}