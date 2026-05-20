#include <AFMotor.h>

// Define the 3 pumps connected to the M1, M2, and M3 blocks
AF_DCMotor pump1(1); 
AF_DCMotor pump2(2); 
AF_DCMotor pump3(4); 

void setup() {
  Serial.begin(9600);
  Serial.println("Starting 3-Pump Test...");

  // Set all pumps to maximum speed (0 to 255)
  pump1.setSpeed(180);
  pump2.setSpeed(180);
  pump3.setSpeed(180);

  // Ensure they are all off to start
  pump1.run(RELEASE);
  pump2.run(RELEASE);
  pump3.run(RELEASE);
}

void loop() {
  Serial.println("Pumps ON");
  // Turn all 3 pumps on in the forward direction
  pump1.run(FORWARD);
  pump2.run(FORWARD);
  pump3.run(FORWARD);
  
 
 
 
}