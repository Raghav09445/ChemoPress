// ─────────────────────────────────────────────────────────────
// ChemoPress — PI Pressure Controller (TRIPLE PUMP / 2x L298N)
// Hardware: Arduino Uno + 2x L298N Motor Drivers
//           + MPRLS I2C pressure sensor
//           + 3 DC air pumps
// ─────────────────────────────────────────────────────────────

#include <Wire.h>
#include "Adafruit_MPRLS.h"

// ── L298N Motor Driver #1 Pins ────────────────────────────────
// Pump 1 (Board 1, Channel A)
const int enA = 3;  // PWM Pin
const int in1 = 2;  // Direction 1
const int in2 = 4;  // Direction 2

// Pump 2 (Board 1, Channel B)
const int enB = 5;  // PWM Pin
const int in3 = 7;  // Direction 1
const int in4 = 8;  // Direction 2

// ── L298N Motor Driver #2 Pins ────────────────────────────────
// Pump 3 (Board 2, Channel A)
const int enC = 6;  // PWM Pin
const int in5 = 9;  // Direction 1
const int in6 = 10; // Direction 2

// ── MPRLS Pressure Sensor (I2C) ───────────────────────────────
Adafruit_MPRLS mprls(-1, -1);

// ── Valve pin ─────────────────────────────────────────────────
#define VALVE_PIN  A0

// ── PI Parameters — REPLACE WITH YOUR COHEN-COON VALUES ───────
const float KP       = 4.08;
const float TI       = 5.58;
const float SETPOINT = 26.5;   // hPa — middle of 20-33 window

// ── Safety and control limits ─────────────────────────────────
const float MAX_PRESSURE  = 40.0;  
const float BLEED_TRIGGER = 31.0;  // hPa — open valve above this
const float DEADBAND      =  0.5;  

// ── Variable for Auto-Calibrated Atmospheric Pressure ─────────
float LOCAL_ATMOS = 0.0; 

// ── PI state variables ────────────────────────────────────────
float integral            = 0;
float lastTime            = 0;
float rawPressure         = 0.0;
float lastValidPressure   = 0.0;

// ── Pressure filter ───────────────────────────────────────────
float buf[3] = {0, 0, 0};
int   bufIdx = 0;

// ─────────────────────────────────────────────────────────────
// Read filtered gauge pressure with EMI spike filter
// ─────────────────────────────────────────────────────────────
float readPressure() {
  rawPressure = mprls.readPressure() - LOCAL_ATMOS;

  // Spike filter — reject physically impossible jumps
  if (rawPressure > 200.0 ||
      rawPressure < -50.0 ||
      abs(rawPressure - lastValidPressure) > 50.0) {
    rawPressure = lastValidPressure;  // use last good value
  } else {
    lastValidPressure = rawPressure;
  }

  // 3-sample moving average
  buf[bufIdx] = rawPressure;
  bufIdx = (bufIdx + 1) % 3;
  return (buf[0] + buf[1] + buf[2]) / 3.0;
}

// ─────────────────────────────────────────────────────────────
// Set ALL 3 pumps to calculated PI speed
// ─────────────────────────────────────────────────────────────
void setPumpSpeed(int speed) {
  speed = constrain(speed, 0, 255);

  if (speed > 0) {
    // Pump 1 FORWARD
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, speed);
    
    // Pump 2 FORWARD
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, speed);

    // Pump 3 FORWARD
    digitalWrite(in5, HIGH);
    digitalWrite(in6, LOW);
    analogWrite(enC, speed);
  } else {
    // ALL PUMPS STOP
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(enA, 0);
    
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    analogWrite(enB, 0);

    digitalWrite(in5, LOW);
    digitalWrite(in6, LOW);
    analogWrite(enC, 0);
  }
}

// ─────────────────────────────────────────────────────────────
// Safety stop
// ─────────────────────────────────────────────────────────────
void safetyStop(const char* reason, float badPressure) {
  // Force pumps to stop immediately
  setPumpSpeed(0);
  
  // Open the bleed valve
  digitalWrite(VALVE_PIN, HIGH);
  
  Serial.print(F("\n[SAFETY HALT] "));
  Serial.println(reason);
  Serial.print(F("Trigger value: "));
  Serial.print(badPressure);
  Serial.println(F(" hPa"));
  Serial.println(F("Press RESET to restart."));
  while (1); // Freeze system safely
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  // --- L298N Motor Pins Setup ---
  pinMode(enA, OUTPUT); pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT); pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(enC, OUTPUT); pinMode(in5, OUTPUT); pinMode(in6, OUTPUT);

  // --- Valve Setup ---
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW);

  // Ensure pumps are off before initializing I2C
  setPumpSpeed(0);

  // --- Sensor Init ---
  if (!mprls.begin()) {
    Serial.println(F("[ERROR] MPRLS not found!"));
    safetyStop("MPRLS sensor not found", 0.0);
  }

  // --- Auto-Calibrate Baseline Pressure ---
  Serial.println(F("Calibrating baseline atmospheric pressure..."));
  delay(1000); // Let the sensor settle
  
  float totalAtm = 0;
  for(int i=0; i<10; i++){
    totalAtm += mprls.readPressure();
    delay(50);
  }
  LOCAL_ATMOS = totalAtm / 10.0;
  Serial.print(F("Baseline set to: "));
  Serial.print(LOCAL_ATMOS);
  Serial.println(F(" hPa"));

  Serial.println(F("# ChemoPress Ready — TRIPLE PUMP MODE"));

  delay(2000);
  lastTime = millis() / 1000.0;
}

// ─────────────────────────────────────────────────────────────
void loop() {
  float now      = millis() / 1000.0;
  float dt       = now - lastTime;
  lastTime       = now;

  float pressure = readPressure();

  // ── Soft bleed — keeps pressure below 33 hPa ─────────────
  if (pressure > BLEED_TRIGGER) {
    digitalWrite(VALVE_PIN, HIGH);
    if (integral > 0) integral = 0;  // prevent pump fighting valve
  } else {
    digitalWrite(VALVE_PIN, LOW);
  }

  // ── Hard safety cutoff ────────────────────────────────────
  if (pressure > MAX_PRESSURE) {
    safetyStop("Over-pressure limit exceeded", pressure);
  }

  // ── PI Controller ─────────────────────────────────────────
  float error = SETPOINT - pressure;

  if (abs(error) < DEADBAND) {
    error = 0;
  }

  // NOTE: Left clamp at 150 to prevent motor stall!
  integral += error * dt;
  integral  = constrain(integral, -150, 150);

  float output = KP * error + (KP / TI) * integral;
  output = constrain(output, 0, 255);

  setPumpSpeed((int)output);

  // ── Human-Readable Data Log ───────────────────────────────
  Serial.print(F("Time(ms): "));
  Serial.print(millis());
  
  Serial.print(F("  |  Raw: "));
  Serial.print(rawPressure);
  
  Serial.print(F("  |  Filt: "));
  Serial.print(pressure);
  
  Serial.print(F("  |  Err: "));
  Serial.print(error);
  
  Serial.print(F("  |  PWM: "));
  Serial.println((int)output);

  delay(100);
}