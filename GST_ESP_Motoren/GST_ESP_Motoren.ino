#include <ESP8266WiFi.h>
#include <espnow.h>
#include <millisDelay.h>
#include <SpeedStepper.h>

#define UFRStepperPin D4
#define UFLStepperPin D0
#define UFHStepperPin D7
#define BLStepperPin D6
#define BRStepperPin D5
#define dirPin D1

#define myMaxSpeed 3000
#define haltDecel 400
#define nothaltDecel 1500
#define langsamfahrt1Speed 1000
#define langsamfahrt2Speed 600
#define langsamfahrt3Speed 200
#define abfahrtAccel 500

#define VDelayTime 3000
#define UFSpeedFactor 0.2
#define BSpeedFactor 2
#define BVLength 15000

int speed = 3000;
bool fahrtrichtung = true;
bool accelerating;
bool decelerating;
int isLangsamfahrt = 0;
int isNothalt = 0;

millisDelay VDelay;

millisDelay runDelay;
const uint32_t RUN_DELAY_MS = 10000;

typedef struct struct_message_sensors {
  int id;  // Nachrichtentyp-Kennung
  int station;
  bool sensor1;
  bool sensor2;
  bool sensor3;
  bool sensor4;
} struct_message_sensors;

typedef struct struct_message_controls {
  int id;
  bool abfahrt;
  int nothalt;
  int langsamfahrt;
  bool fahrtrichtung;
} struct_message_controls;

typedef struct struct_message_speed {
  int id;
  int speed;
} struct_message_speed;

struct_message_controls receivedControlData;
struct_message_sensors receivedSensorData;
struct_message_speed receivedSpeedData;

bool hasAbfahrt = false;
bool starting = false;

void onReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  int id;
  memcpy(&id, incomingData, sizeof(int));
  if (id == 0 && len == sizeof(struct_message_sensors)) {
    memcpy(&receivedSensorData, incomingData, sizeof(struct_message_sensors));
    Serial.println("Sensor Daten empfangen!");
    Serial.print(receivedSensorData.sensor1);
    Serial.print(" | ");
    Serial.print(receivedSensorData.sensor2);
    Serial.print(" | ");
    Serial.print(receivedSensorData.sensor3);
    Serial.print(" | ");
    Serial.println(receivedSensorData.sensor4);
    if (fahrtrichtung) {  //der ganze Sensoren Schmarrn
      if (!receivedSensorData.sensor1 && receivedSensorData.sensor2 && !accelerating) {
        startAcceleration();
      } else if (receivedSensorData.sensor1 && !receivedSensorData.sensor2 && accelerating) {
        stopAcceleration();
      }
      if (!receivedSensorData.sensor4 && receivedSensorData.sensor3 && !decelerating) {
        startDeceleration();
      } else if (receivedSensorData.sensor4 && !receivedSensorData.sensor3 && decelerating) {
        stopDeceleration();
      }
    } else {
      if (receivedSensorData.sensor1 && !receivedSensorData.sensor2 && !decelerating) {
        startDeceleration();
      } else if (!receivedSensorData.sensor1 && receivedSensorData.sensor2 && decelerating) {
        stopDeceleration();
      }
      if (receivedSensorData.sensor4 && !receivedSensorData.sensor3 && !accelerating) {
        startAcceleration();
      } else if (!receivedSensorData.sensor4 && receivedSensorData.sensor3 && accelerating) {
        stopAcceleration();
      }
    }
  } else if (id == 1 && len == sizeof(struct_message_controls)) {
    memcpy(&receivedControlData, incomingData, sizeof(struct_message_controls));
    Serial.print(receivedControlData.abfahrt);
    Serial.print(" | ");
    Serial.print(receivedControlData.nothalt);
    Serial.print(" | ");
    Serial.print(receivedControlData.langsamfahrt);
    Serial.print(" | ");
    Serial.println(receivedControlData.fahrtrichtung);
    if (receivedControlData.langsamfahrt != 0) {
      isLangsamfahrt = receivedControlData.langsamfahrt;
    }
    if (receivedControlData.fahrtrichtung != fahrtrichtung) {
      Serial.println("fahrtrichtung:" + (String)receivedControlData.fahrtrichtung);
      fahrtrichtung = receivedControlData.fahrtrichtung;
      setStepperDirection(fahrtrichtung);
    }
    Serial.println("nothalt: " + (String)isNothalt);
    if (receivedControlData.nothalt != isNothalt) {
      isNothalt = receivedControlData.nothalt;
      if (isNothalt == 1) {
        //Serial.println("halt");
        halt();
      } else if (isNothalt == 3) {
        //Serial.println("nothalt");
        nothalt();
      }
    }
    if (receivedControlData.abfahrt) {
      setStepperDirection(receivedControlData.fahrtrichtung);
      abfahrt();
      starting = true;
    }
  } else if (id == 2 && len == sizeof(struct_message_speed)) {
    memcpy(&receivedSpeedData, incomingData, sizeof(struct_message_speed));
    //Serial.print("speed :");
    //Serial.println(receivedSpeedData.speed);
    if (isNothalt == 0 && receivedSpeedData.speed != speed && hasAbfahrt) {
      speed = receivedSpeedData.speed;
      setStepperSpeed(speed);
    }
  }
}

SpeedStepper UFRStepper(UFRStepperPin, dirPin);
SpeedStepper UFLStepper(UFLStepperPin, dirPin);
SpeedStepper UFHStepper(UFHStepperPin, dirPin);
SpeedStepper BRStepper(BLStepperPin, dirPin);
SpeedStepper BLStepper(BRStepperPin, dirPin);

void setup() {
  pinMode(D3, OUTPUT);
  Serial.begin(9600);
  // WLAN im STA-Modus starten
  WiFi.mode(WIFI_STA);

  // ESP-NOW Initialisierung
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialisierung fehlgeschlagen");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onReceive);

  initializeStepper(UFRStepper, myMaxSpeed * UFSpeedFactor, 500);
  initializeStepper(UFLStepper, myMaxSpeed * UFSpeedFactor, 500);
  initializeStepper(UFHStepper, myMaxSpeed * UFSpeedFactor, 500);
  initializeStepper(BRStepper, myMaxSpeed * BVSpeedFactor, 500);
  initializeStepper(BLStepper, myMaxSpeed * BVSpeedFactor, 500);

  runDelay.start(RUN_DELAY_MS);
  setStepperSpeed(0);
}

void initializeStepper(SpeedStepper &stepper, float maxSpeed, float acceleration) {
  stepper.setMaxSpeed(maxSpeed);
  stepper.setMinSpeed(1);
  stepper.setAcceleration(acceleration);
  stepper.stopAndSetHome();
  stepper.hardStop();
}

void setStepperAcceleration(float acceleration) {
  UFRStepper.setAcceleration(acceleration * UFSpeedFactor);
  UFLStepper.setAcceleration(acceleration * UFSpeedFactor);
  UFHStepper.setAcceleration(acceleration * UFSpeedFactor);
  BRStepper.setAcceleration(acceleration * BVSpeedFactor);
  BLStepper.setAcceleration(acceleration * BVSpeedFactor);
}

void setStepperMaxSpeed(float speed){
  UFRStepper.setAcceleration(speed * UFSpeedFactor);
  UFLStepper.setAcceleration(speed * UFSpeedFactor);
  UFHStepper.setAcceleration(speed * UFSpeedFactor);
  BRStepper.setAcceleration(speed * BVSpeedFactor);
  BLStepper.setAcceleration(speed * BVSpeedFactor);
}

void setStepperSpeed(float speed) {
  UFRStepper.setSpeed(speed * UFSpeedFactor);
  UFLStepper.setSpeed(speed * UFSpeedFactor);
  UFHStepper.setSpeed(speed * UFSpeedFactor);
  if (fahrtrichtung) {
    if (!decelerating) {
      BRStepper.setSpeed(speed * BSpeedFactor);
    } else {
      BRStepper.setSpeed(speed * UFSpeedFactor);
    }
    if (!accelerating) {
      BLStepper.setSpeed(speed * UFSpeedFactor);
    } else {
      BLStepper.setSpeed(speed * BSpeedFactor);
    }
  } else {
    if (!accelerating) {
      BRStepper.setSpeed(speed * UFSpeedFactor);
    } else {
      BRStepper.setSpeed(speed * BSpeedFactor);
    }
    if (!decelerating) {
      BLStepper.setSpeed(speed * BSpeedFactor);
    } else {
      BLStepper.setSpeed(speed * UFSpeedFactor);
    }
  }
}

void setStepperDirection(bool direction) {
  fahrtrichtung = direction; 
  if (UFHStepper.getSpeed() == 0 && direction) {
    Serial.println("Fahrtrichtung vorwaerts");
    digitalWrite(D3, HIGH);
    UFRStepper.setDirForward();
    UFLStepper.setDirForward();
    UFHStepper.setDirForward();
    BRStepper.setDirForward();
    BLStepper.setDirForward();
  } else if (UFHStepper.getSpeed() == 0) {
    Serial.println("Fahrtrichtung rueckwaerts");
    digitalWrite(D3, LOW);
    UFRStepper.setDirReverse();
    UFLStepper.setDirReverse();
    UFHStepper.setDirReverse();
    BRStepper.setDirReverse();
    BLStepper.setDirReverse();
  }
}

void halt() {
  Serial.println("halt");
  setStepperAcceleration(haltDecel);
  setStepperSpeed(0);
}

void nothalt() {
  Serial.println("nothalt");
  setStepperAcceleration(nothaltDecel);
  setStepperSpeed(0);
}

void abfahrt() {
  Serial.println("abfahrt");
  hasAbfahrt = true;
  setStepperAcceleration(abfahrtAccel);
  setStepperSpeed(speed);
}

void startAcceleration() {
  accelerating = true;
  Serial.println("beschleunige...");
  Serial.println(fahrtrichtung);
  if (fahrtrichtung) {
    int acceleration = (speed * speed * (BSpeedFactor * BSpeedFactor - 1)) / (2 * BVLength);
    Serial.println("berechnete Beschleunigung:" + (String)acceleration);
    BLStepper.setAcceleration(acceleration);
    BLStepper.setSpeed(speed * BSpeedFactor);
  } else {
    int acceleration = (speed * speed * (BSpeedFactor * BSpeedFactor - 1)) / (2 * BVLength);
    Serial.println("berechnete Beschleunigung:" + (String)acceleration);
    BRStepper.setAcceleration(acceleration);
    BRStepper.setSpeed(speed * BSpeedFactor);
  }
}

void stopAcceleration() {
  Serial.println("beschleunige nicht mehr...");
  Serial.println(fahrtrichtung);
  if (fahrtrichtung) {
    BLStepper.setAcceleration(10000);
    BLStepper.setSpeed(speed * UFSpeedFactor);
  } else {
    BRStepper.setAcceleration(10000);
    BRStepper.setSpeed(speed * UFSpeedFactor);
  }
  accelerating = false;
}

void startDeceleration() {
  decelerating = true;
  Serial.println("verzögere...");
  Serial.println(fahrtrichtung);
  if (fahrtrichtung) {
    BRStepper.setAcceleration((speed * speed * (BSpeedFactor * BSpeedFactor - 1)) / (2 * BVLength));
    BRStepper.setSpeed(speed * UFSpeedFactor);
  } else {
    BLStepper.setAcceleration((speed * speed * (BSpeedFactor * BSpeedFactor - 1)) / (2 * BVLength));
    BLStepper.setSpeed(speed * UFSpeedFactor);
  }
}

void stopDeceleration() {
  VDelay.start(VDelayTime);
}

void loop() {
  if (VDelay.justFinished()) {
    VDelay.stop();
    Serial.println("verzögere nicht mehr...");
    Serial.println(fahrtrichtung);
    if (fahrtrichtung) {
      BRStepper.setAcceleration(10000);
      BRStepper.setSpeed(speed * BSpeedFactor);
    } else {
      BLStepper.setAcceleration(10000);
      BLStepper.setSpeed(speed * BSpeedFactor);
    }
    decelerating = false;
  }
  UFRStepper.run();
  UFLStepper.run();
  UFHStepper.run();
  BRStepper.run();
  BLStepper.run();
}
