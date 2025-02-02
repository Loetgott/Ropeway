#include <millisDelay.h>

#define haltButton 40
#define nothaltButton 52
#define anwurfButton 42
#define abfahrtButton 38
#define powerSwitchButton 43
#define vorwaertsButton 45
#define rueckwaertsButton 47
#define umlaufButton 51
#define beschickenButton 41
#define cisButton 49

#define haltLED 25
#define nothaltLED 37
#define anwurfLED 27
#define abfahrtLED 23
#define powerSwitchLED 26
#define vorwaertsLED 28
#define rueckwaertsLED 30
#define umlaufLED 34
#define beschickenLED 24
#define cisLED 32

bool isHalt = false;
bool isNothalt = false;
bool isAnwurf = false;
bool isAbfahrt = false;
bool isPowerSwitch = false;
bool isVorwaerts = false;
bool isRueckwaerts = false;
bool isUmlauf = false;
bool isBeschicken = false;
bool isCIS = false;
bool buttonsPushed[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

void setup() {
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(40, INPUT_PULLUP);
  pinMode(41, INPUT_PULLUP);
  pinMode(42, INPUT_PULLUP);
  pinMode(43, INPUT_PULLUP);
  pinMode(44, INPUT_PULLUP);
  pinMode(45, INPUT_PULLUP);
  pinMode(46, INPUT_PULLUP);
  pinMode(47, INPUT_PULLUP);
  pinMode(48, INPUT_PULLUP);
  pinMode(49, INPUT_PULLUP);
  pinMode(50, INPUT_PULLUP);
  pinMode(51, INPUT_PULLUP);
  pinMode(52, INPUT_PULLUP);
  pinMode(53, INPUT_PULLUP);

  Serial.begin(9600);   // Serielle Schnittstelle für Debugging
  Serial1.begin(9600);  // Serielle Schnittstelle für Kommunikation mit ESP8266
  Serial.println("Arduino Mega bereit.");
}

millisDelay abfahrtBlinkDelay;

void loop() {
  for (int i = 38; i < 54; i++) {
    if (digitalRead(i) == LOW && !buttonsPushed[i - 38]) {
      buttonsPushed[i - 38] = true;
      Serial1.println((String)i + ":true");
      Serial.println((String)i + ":true");
    } else if (digitalRead(i) == HIGH && buttonsPushed[i - 38]) {
      buttonsPushed[i - 38] = false;
      Serial1.println((String)i + ":false");
      Serial.println((String)i + ":false");
    }
  }
  if (Serial1.available()) {
    String message = Serial1.readStringUntil('\n');
    Serial.println("Nachricht vom ESP empfangen: " + message);
    if (message == "anwurf") {
      anwurf();
    } else if (message == "abfahrt") {
      abfahrt();
    } else if (message == "rueckwaerts") {
      rueckwaerts();
    } else if (message == "vorwaerts") {
      vorwaerts();
    } else if (message == "halt") {
      halt();
    } else if (message == "nothalt") {
      nothalt();
    }
  }
  if (isAnwurf && !isAbfahrt) {
    if (abfahrtBlinkDelay.justFinished()) {
      if (digitalRead(abfahrtLED) == HIGH) {
        digitalWrite(abfahrtLED, LOW);
      } else {
        digitalWrite(abfahrtLED, HIGH);
      }
      abfahrtBlinkDelay.restart();
    } else if (!abfahrtBlinkDelay.isRunning()) {
      abfahrtBlinkDelay.start(500);
    }
  }
}


void anwurf() {
  isHalt = false;
  isNothalt = false;
  isAnwurf = true;
  digitalWrite(anwurfLED, HIGH);
}

void vorwaerts() {
  isRueckwaerts = false;
  isVorwaerts = true;
  digitalWrite(vorwaertsLED, HIGH);
}

void rueckwaerts() {
  isVorwaerts = false;
  isRueckwaerts = true;
  digitalWrite(rueckwaertsLED, HIGH);
}

void abfahrt() {
  isAbfahrt = true;
  digitalWrite(abfahrtLED, HIGH);
  digitalWrite(anwurfLED, LOW);
}

void halt() {
  isHalt = true;
  digitalWrite(haltLED, HIGH);
}

void nothalt() {
  isNothalt = true;
  digitalWrite(nothaltLED, HIGH);
}