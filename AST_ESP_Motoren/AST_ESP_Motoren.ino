#include <millisDelay.h>
#include <SpeedStepper.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define seilStepperPin D8
#define UFRStepperPin D7
#define UFLStepperPin D6
#define UFHStepperPin D5
#define BLStepperPin D0
#define BRStepperPin D4
#define dirPin D3

SpeedStepper seilStepper(seilStepperPin, dirPin);
SpeedStepper UFRStepper(UFRStepperPin, dirPin);
SpeedStepper UFLStepper(UFLStepperPin, dirPin);
SpeedStepper UFHStepper(UFHStepperPin, dirPin);
SpeedStepper BLStepper(BLStepperPin, dirPin);
SpeedStepper BRStepper(BRStepperPin, dirPin);

typedef struct struct_message_sensors {
  int id;  // Nachrichtentyp-Kennung
  int station;
  bool sensor1;
  bool sensor2;
  bool sensor3;
  bool sensor4;
} struct_message_sensors;

typedef struct struct_message_controls {
  int id;  // Nachrichtentyp-Kennung
  int speed;
  int nothalt;
  int langsamfahrt;
  bool fahrtrichtung;
} struct_message_controls;

struct_message_controls receivedControlData;
struct_message_sensors receivedSensorData;

void onReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  int id;
  memcpy(&id, incomingData, sizeof(int));
  if (id == 0 && len == sizeof(struct_message_sensors)) {
    memcpy(&receivedSensorData, incomingData, sizeof(struct_message_sensors));
    //Serial.println("Sensor Daten empfangen!");
    //Serial.print(receivedSensorData.sensor1);
    //Serial.print(" | ");
    //Serial.print(receivedSensorData.sensor2);
    //Serial.print(" | ");
    //Serial.print(receivedSensorData.sensor3);
    //Serial.print(" | ");
    //Serial.println(receivedSensorData.sensor4);
    if (fahrtrichtung) {
      if (!receivedSensorData.sensor1 && receivedSensorData.sensor2) {
        accelerating = true;
        startAcceleration();
      } else if (receivedSensorData.sensor1 && !receivedSensorData.sensor2 && accelerating) {
        stopAcceleration();
      }
      if (!receivedSensorData.sensor4 && receivedSensorData.sensor3) {
        decelerating = true;
        startDeceleration();
      } else if (receivedSensorData.sensor4 && !receivedSensorData.sensor3 && decelerating) {
        stopDeceleration();
      }
    } else {
      if (receivedSensorData.sensor1 && !receivedSensorData.sensor2) {
        decelerating = true;
        startDeceleration();
      } else if (!receivedSensorData.sensor1 && receivedSensorData.sensor2 && decelerating) {
        stopDeceleration();
      }
      if (receivedSensorData.sensor4 && !receivedSensorData.sensor3) {
        accelerating = true;
        startAcceleration();
      } else if (!receivedSensorData.sensor4 && receivedSensorData.sensor3 && accelerating) {
        stopAcceleration();
      }
    }
  } else if (id == 1 && len == sizeof(struct_message_controls)) {
    memcpy(&receivedControlData, incomingData, sizeof(struct_message_controls));
    //Serial.println("Steuerung Daten empfangen!");
    //Serial.print(receivedControlData.speed);
    //Serial.print(" | ");
    //Serial.print(receivedControlData.nothalt);
    //Serial.print(" | ");
    //Serial.print(receivedControlData.langsamfahrt);
    //Serial.print(" | ");
    //Serial.println(receivedControlData.fahrtrichtung);
    if (receivedControlData.langsamfahrt != 0) {
      isLangsamfahrt = true;
      langsamfahrt(receivedControlData.langsamfahrt);
    } else {
      speed = receivedControlData.speed;
      isLangsamfahrt = false;
    }
    if (speed == 0 && receivedControlData.fahrtrichtung != fahrtrichtung) {
      fahrtrichtung = receivedControlData.fahrtrichtung;
    }
    if (receivedControlData.nothalt != isNothalt) {
      isNothalt = receivedControlData.nothalt;
      nothalt(receivedControlData.nothalt);
    }
    if (receivedControlData.speed != speed) {
      speed = receivedControlData.speed;
      updateSpeed;
    }
  }
}

millisDelay runDelay;
const uint32_t RUN_DELAY_MS = 10000;

uint32_t start_ms;

Stream* serialIO;
int newMaxSpeed = 3000;

void setup() {
  // Initialisiere jeden Stepper
  initializeStepper(seilStepper, newMaxSpeed, 500);
  initializeStepper(UFRStepper, newMaxSpeed, 500);
  initializeStepper(UFLStepper, newMaxSpeed, 500);
  initializeStepper(UFHStepper, newMaxSpeed, 500);
  initializeStepper(BLStepper, newMaxSpeed, 500);
  initializeStepper(BRStepper, newMaxSpeed, 500);
  runDelay.start(RUN_DELAY_MS);
  start_ms = millis();
  seilStepper.setSpeed(newMaxSpeed);
  UFRStepper.setSpeed(newMaxSpeed);
  UFLStepper.setSpeed(newMaxSpeed);
  UFHStepper.setSpeed(newMaxSpeed);
  BLStepper.setSpeed(newMaxSpeed);
  BRStepper.setSpeed(newMaxSpeed);
}

// Initialisierungsfunktion für jeden Stepper
void initializeStepper(SpeedStepper& stepper, float maxSpeed, float acceleration) {
  stepper.setMaxSpeed(maxSpeed);
  stepper.setMinSpeed(1);
  stepper.setAcceleration(acceleration);
  stepper.stopAndSetHome();
  stepper.hardStop();
}

void loop() {
  // Alle Motoren laufen lassen
  seilStepper.run();
  UFRStepper.run();
  UFLStepper.run();
  UFHStepper.run();
  BLStepper.run();
  BRStepper.run();

  if (runDelay.justFinished()) {
    seilStepper.setSpeed(120);
    UFRStepper.setSpeed(120);
    UFLStepper.setSpeed(120);
    UFHStepper.setSpeed(120);
    BLStepper.setSpeed(120);
    BRStepper.setSpeed(120);
    runDelay.stop();
  }
  //serialIO->println("Max Speed: " + String(seilStepper.getSpeed()));

}
