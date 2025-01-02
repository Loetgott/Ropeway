#include <AccelStepper.h>

#define seilStepperPin D8
#define UFRStepperPin D7
#define UFLStepperPin D6
#define UFHStepperPin D5
#define BLStepperPin D0
#define BRStepperPin D4
#define dirPin D3

#define initialSpeed 10000  // Anfangsgeschwindigkeit
#define reducedSpeed 500   // Reduzierte Geschwindigkeit, wenn das Ziel näher rückt
#define acceleration 400     // Beschleunigung

AccelStepper seilStepper(1, seilStepperPin, dirPin);
AccelStepper UFRStepper(1, UFRStepperPin, dirPin);
AccelStepper UFLStepper(1, UFLStepperPin, dirPin);
AccelStepper UFHStepper(1, UFHStepperPin, dirPin);
AccelStepper BLStepper(1, BLStepperPin, dirPin);
AccelStepper BRStepper(1, BRStepperPin, dirPin);

void setup() {
  // Maximalgeschwindigkeit und Beschleunigung für alle Motoren einstellen
  seilStepper.setMaxSpeed(initialSpeed);
  seilStepper.setAcceleration(acceleration);
  
  UFRStepper.setMaxSpeed(initialSpeed);
  UFRStepper.setAcceleration(acceleration);
  
  UFLStepper.setMaxSpeed(initialSpeed);
  UFLStepper.setAcceleration(acceleration);
  
  UFHStepper.setMaxSpeed(initialSpeed);
  UFHStepper.setAcceleration(acceleration);
  
  BLStepper.setMaxSpeed(initialSpeed);
  BLStepper.setAcceleration(acceleration);
  
  BRStepper.setMaxSpeed(initialSpeed);
  BRStepper.setAcceleration(acceleration);

  // Zielpositionen festlegen, um die Bewegung zu starten
  seilStepper.move(100000);
  UFRStepper.move(100000);
  UFLStepper.move(100000);
  UFHStepper.move(100000);
  BLStepper.move(100000);
  BRStepper.move(100000);
}

void loop() {
  // Motoren bewegen
  seilStepper.run();
  UFRStepper.run();
  UFLStepper.run();
  UFHStepper.run();
  BLStepper.run();
  BRStepper.run();
  
  // Überprüfen, ob die Motoren das Ziel annähern, um die Geschwindigkeit anzupassen
  if (seilStepper.distanceToGo() < 10000) {
    seilStepper.setMaxSpeed(reducedSpeed);
  }
  if (UFRStepper.distanceToGo() < 10000) {
    UFRStepper.setMaxSpeed(reducedSpeed);
  }
  if (UFLStepper.distanceToGo() < 10000) {
    UFLStepper.setMaxSpeed(reducedSpeed);
  }
  if (UFHStepper.distanceToGo() < 10000) {
    UFHStepper.setMaxSpeed(reducedSpeed);
  }
  if (BLStepper.distanceToGo() < 10000) {
    BLStepper.setMaxSpeed(reducedSpeed);
  }
  if (BRStepper.distanceToGo() < 10000) {
    BRStepper.setMaxSpeed(reducedSpeed);
  }

  // Wenn das Ziel erreicht ist, eine neue Bewegung starten
  if (seilStepper.distanceToGo() == 0) {
    seilStepper.move(-100000); // Bewege zurück zum Ausgangspunkt
    seilStepper.setMaxSpeed(initialSpeed); // Setze die maximale Geschwindigkeit zurück
  }
  if (UFRStepper.distanceToGo() == 0) {
    UFRStepper.move(-100000);
    UFRStepper.setMaxSpeed(initialSpeed);
  }
  if (UFLStepper.distanceToGo() == 0) {
    UFLStepper.move(-100000);
    UFLStepper.setMaxSpeed(initialSpeed);
  }
  if (UFHStepper.distanceToGo() == 0) {
    UFHStepper.move(-100000);
    UFHStepper.setMaxSpeed(initialSpeed);
  }
  if (BLStepper.distanceToGo() == 0) {
    BLStepper.move(-100000);
    BLStepper.setMaxSpeed(initialSpeed);
  }
  if (BRStepper.distanceToGo() == 0) {
    BRStepper.move(-100000);
    BRStepper.setMaxSpeed(initialSpeed);
  }
}
