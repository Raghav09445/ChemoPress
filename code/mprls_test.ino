#include <Wire.h>
#include "Adafruit_MPRLS.h"

// -1 for reset pin, -1 for EOC pin
Adafruit_MPRLS mprls = Adafruit_MPRLS(-1, -1);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting simple sensor test...");

  // Try to connect to the sensor
  if (!mprls.begin()) {
    Serial.println("Error: Sensor not found! Check 3.3V, GND, A4, and A5.");
    while (1); // Freeze here if it fails
  }
  
  Serial.println("Sensor connected successfully!");
}

void loop() {
  // Read and print the pressure
  float pressure = mprls.readPressure();
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  delay(500); // Wait half a second before reading again
}