#include <Wire.h>

#define MPU6500_ADDRESS  0x68
#define ACCEL_XOUT_H     0x3B
#define GYRO_XOUT_H      0x43

void setup() {
    Serial.begin(115200);
    Serial.println("Setting up MPU6500");

    Wire.begin();
    
    Wire.beginTransmission(MPU6500_ADDRESS);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    Serial.println("MPU6500 Ready");
}

void loop() {
    int16_t ax, ay, az, gx, gy, gz;

    // Read accelerometer data
    Wire.beginTransmission(MPU6500_ADDRESS);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6500_ADDRESS, 6, true);
    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();

    // Read gyroscope data
    Wire.beginTransmission(MPU6500_ADDRESS);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6500_ADDRESS, 6, true);
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();

    // Print values as CSV (comma-separated)
    Serial.print(ax); Serial.print(",");
    Serial.print(ay); Serial.print(",");
    Serial.print(az); Serial.print(",");
    Serial.print(gx); Serial.print(",");
    Serial.print(gy); Serial.print(",");
    Serial.println(gz);

    delay(10);
}
