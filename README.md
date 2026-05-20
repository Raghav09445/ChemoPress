# ChemoPress
### Automated Pressure-Controlled Compressive Glove System


## About
ChemoPress prevents chemotherapy-induced peripheral 
neuropathy (CIPN) by applying controlled compression 
to the patient's hand during oxaliplatin infusion.

## Hardware
- Arduino Uno
- L298N Motor Drivers
- Adafruit MPRLS Pressure Sensor (I2C)
- 3x DC Air Pumps (6V-12V)
- Brass Air Release Valve

## Control System
- System modeled as FOPTD: P(s) = Ke^(-Ls) / (τs+1)
- PI controller tuned using Cohen-Coon method
- Target pressure range: 20-33 hPa

## Repository Structure
- /code     — Arduino sketches
- /hardware — Wiring and pin configuration

## Reference
Kerdsaeng et al., ECTI-CON 2025, IEEE
"Modeling of Pressure Control in Compressive 
Gloves for Neuropathy Prevention"
