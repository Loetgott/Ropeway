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

bool halt = false;
bool nothalt = false;
bool anwurf = false;
bool abfahrt = false;
bool powerSwitch = false;
bool vorwaerts = false;
bool rueckwaerts = false;
bool umlauf = false;
bool beschicken = false;
bool cis = false;
bool buttonsPushed[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

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
  
  Serial.begin(115200);  // Serielle Schnittstelle für Debugging
  Serial1.begin(115200);    // Serielle Schnittstelle für Kommunikation mit ESP8266
  Serial.println("Arduino Mega bereit.");
}

void loop() {
  for (int i = 38; i < 54; i++) {
    if(digitalRead(i) == LOW && !buttonsPushed[i - 38]){
      buttonsPushed[i - 38] = true;
      Serial1.println((String)i + ":true");
      //Serial.println((String)i + ":true");
    }else if(digitalRead(i) == HIGH && buttonsPushed[i - 38]){
      buttonsPushed[i - 38] = false;
      Serial1.println((String)i + ":false");
      //Serial.println((String)i + ":false");
    }
  }
}
