#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <millisDelay.h>

#define NUM_SLAVES 4

SoftwareSerial megaSerial(D1, D2);  // RX, TX (D7 ist GPIO13 für RX, D6 ist GPIO12 für TX, falls später benötigt)

// MAC-Adressen der vier Slaves (ersetze sie durch die tatsächlichen MACs deiner Slaves)
uint8_t slaveMacs[NUM_SLAVES][6] = {
  { 0x48, 0x3F, 0xDA, 0x57, 0xAF, 0x84 },  // Slave 1
  { 0xBC, 0xFF, 0x4D, 0x02, 0x05, 0x97 },  // Slave 2
  { 0xBC, 0xFF, 0x4D, 0x15, 0x86, 0xB8 },  // Slave 3
  { 0xC4, 0x5B, 0xBE, 0x4E, 0x94, 0xCC }   // Slave 4
};

// Struktur für die Steuerungsdaten
typedef struct struct_message_controls {
  int id;
  bool abfahrt;
  int nothalt;
  int langsamfahrt;
  bool fahrtrichtung;
} struct_message_controls;

//Struktur für Geschwindigkeitsdaten
typedef struct struct_message_speed {
  int id;
  int speed;
} struct_message_speed;

// Struktur für die Sensordaten
typedef struct struct_message_sensors {
  int id;  // Nachrichtentyp-Kennung
  int station;
  bool sensor1;
  bool sensor2;
  bool sensor3;
  bool sensor4;
} struct_message_sensors;

struct_message_controls controlData;
struct_message_speed speedData;
struct_message_sensors sensorData0;
struct_message_sensors sensorData1;

// Empfangene Daten
union received_data {
  struct_message_controls control_data;
  struct_message_sensors sensor_data;
} receivedData;

// Callback für das Senden
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  digitalWrite(D3, LOW);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, LOW);
  //Serial.print("Daten gesendet an: ");
  //for (int i = 0; i < 6; i++) {
  //  Serial.print(mac_addr[i], HEX);
  //  if (i < 5) Serial.print(":");
  //}
  //Serial.print(" Status: ");
  //Serial.println(sendStatus == 0 ? "Erfolgreich" : "Fehlgeschlagen");
  if(sendStatus != 0){
    digitalWrite(D3,HIGH);
  }else{
    digitalWrite(D5,HIGH);
  }
  digitalWrite(D4, LOW);
}

// Callback für den Empfang von Daten
void OnDataReceived(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  
  Serial.print("Daten empfangen von MAC-Adresse: ");
  Serial.println(macStr);

  // Überprüft, ob die Nachricht die richtige Länge hat
  if (len == sizeof(struct_message_sensors)) {
    memcpy(&receivedData.sensor_data, incomingData, sizeof(struct_message_sensors));

    // MAC-Adresse überprüfen und Station zuordnen
    if (strcmp(macStr, "BC:FF:4D:02:05:97") == 0) {  // MAC-Adresse der Station 0
      Serial.println("Station 0 erkannt.");
      // Daten für Station 0 speichern
      sensorData0 = receivedData.sensor_data;
      Serial.println("Sensordaten von Station 0 empfangen: " + 
                     (String)sensorData0.sensor1 + ", " + 
                     (String)sensorData0.sensor2 + ", " + 
                     (String)sensorData0.sensor3 + ", " + 
                     (String)sensorData0.sensor4);
      esp_now_send(slaveMacs[0], (uint8_t *)&sensorData0, sizeof(sensorData0));
    } else if (strcmp(macStr, "C4:5B:BE:4E:94:CC") == 0) {  // MAC-Adresse der Station 1
      Serial.println("Station 1 erkannt.");
      // Daten für Station 1 speichern
      sensorData1 = receivedData.sensor_data;
      Serial.println("Sensordaten von Station 1 empfangen: " + 
                     (String)sensorData1.sensor1 + ", " + 
                     (String)sensorData1.sensor2 + ", " + 
                     (String)sensorData1.sensor3 + ", " + 
                     (String)sensorData1.sensor4);
      esp_now_send(slaveMacs[2], (uint8_t *)&sensorData1, sizeof(sensorData1));
    } else {
      Serial.println("Unbekannte Station. Daten verworfen.");
    }
  }
}


millisDelay speedDelay;
int speedDelayDuration = 200;

void setup() {
  megaSerial.begin(9600);
  Serial.begin(9600);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D3, OUTPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialisierung fehlgeschlagen");
    digitalWrite(D3, HIGH);
    return;
  }
  digitalWrite(D5, HIGH);

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataReceived);

  // Slaves hinzufügen und Ausgabe bei Erfolg
  for (int i = 0; i < NUM_SLAVES; i++) {
    if (esp_now_add_peer(slaveMacs[i], ESP_NOW_ROLE_SLAVE, 1, NULL, 0) == 0) {
      Serial.print("Slave ");
      Serial.print(i + 1);
      Serial.print(" mit MAC ");
      for (int j = 0; j < 6; j++) {
        Serial.print(slaveMacs[i][j], HEX);
        if (j < 5) Serial.print(":");
      }
      Serial.println(" erfolgreich hinzugefügt.");
    } else {
      Serial.print("Fehler beim Hinzufügen von Slave ");
      Serial.println(i + 1);
    }
  }

  speedData.id = 2;
  controlData.id = 1;
  sensorData0.id = 0;
  sensorData1.id = 0;

  speedDelay.start(speedDelayDuration);
}

bool anwurf = false;
bool abfahrt = false;
int nothalt = 0;
int speed = 3000;

String split(String data, char delimiter, int index) {
  int found = 0;
  int start = 0;
  int end = -1;

  for (int i = 0; i <= index; i++) {
    start = end + 1;
    end = data.indexOf(delimiter, start);
    if (end == -1) {
      end = data.length();
    }
    if (i == index) {
      return data.substring(start, end);
    }
  }
  return "";
}

void loop() {
  if (megaSerial.available()) {
    digitalWrite(D5, HIGH);
    String message = megaSerial.readStringUntil('\n');
    Serial.println("Nachricht vom Mega empfangen: " + message);
    String command = split(message, ':', 0);
    String value = split(message, ':', 1);
    //Serial.println("command: " + command);
    //Serial.println("value: " + value);
    if (command == "42") { // anwurf
      if (value == "true") {
        Serial.println("anwurf");
        anwurf = true;
        abfahrt = false;
        controlData.abfahrt = false;
        controlData.nothalt = 0;
      }
    } else if (command == "38") { // abfahrt
      if (anwurf && value == "true") {
        abfahrt = true;
        Serial.println("abfahrt");
        controlData.abfahrt = true;
      }
    } else if (command == "45") { //fahrtrichtung vorwaerts
      if (!abfahrt && value == "true") {
        Serial.println("vorwaerts");
        controlData.fahrtrichtung = true;
      } else {
        controlData.nothalt = 3;
        anwurf = false;
        controlData.abfahrt = false;
      }
    } else if (command == "47") { // fahrtrichtung rueckwaerts
      if (!abfahrt && value == "true") {
        Serial.println("rueckwaerts");
        controlData.fahrtrichtung = false;
      } else {
        controlData.nothalt = 3;
        anwurf = false;
        controlData.abfahrt = false;
      }   
    } else if (command == "40") { //halt
      if (nothalt == 0 && value == "true") {
        Serial.println("halt");
        controlData.nothalt = 1;
        controlData.abfahrt = false;
        anwurf = false;
      }
    } else if (command == "52") { //nothalt
      if (value == "true") {
        Serial.println("nothalt");
        controlData.nothalt = 3;
        controlData.abfahrt = false;
        anwurf = false;
      }
    } else if (command == "speed") {
      if (value != (String)speed && !value.isEmpty()) {
        speed = value.toInt();
      }
    }
    esp_now_send(slaveMacs[0], (uint8_t *)&controlData, sizeof(controlData));
    esp_now_send(slaveMacs[2], (uint8_t *)&controlData, sizeof(controlData));
    Serial.println("---------------------------------------------------------------------------------------------------------------------------------------------------------");
    speedData.speed = speed;
  }
  if (speedDelay.justFinished()) {
    speedData.id = 2;
    esp_now_send(slaveMacs[0], (uint8_t *)&speedData, sizeof(speedData));
    esp_now_send(slaveMacs[2], (uint8_t *)&speedData, sizeof(speedData));
    speedDelay.restart();
  }
  if (Serial.available()) {
    digitalWrite(D5, HIGH);
    String message = Serial.readStringUntil('\n');
    Serial.println("Nachricht vom Mega empfangen: " + message);

    String command = split(message, ':', 0);
    String value = split(message, ':', 1);

    if (command == "42") { // anwurf
      if (value == "true") {
        Serial.println("anwurf");
        anwurf = true;
        abfahrt = false;
        controlData.abfahrt = false;
        controlData.nothalt = 0;
      }
    } else if (command == "38") { // abfahrt
      if (anwurf && value == "true") {
        abfahrt = true;
        Serial.println("abfahrt");
        controlData.abfahrt = true;
      }
    } else if (command == "45") { //fahrtrichtung vorwaerts
      if (!abfahrt && value == "true") {
        Serial.println("vorwaerts");
        controlData.fahrtrichtung = true;
      } else {
        controlData.nothalt = 3;
        anwurf = false;
        controlData.abfahrt = false;
      }
    } else if (command == "47") { // fahrtrichtung rueckwaerts
      if (!abfahrt && value == "true") {
        Serial.println("rueckwaerts");
        controlData.fahrtrichtung = false;
      } else {
        controlData.nothalt = 3;
        anwurf = false;
        controlData.abfahrt = false;
      }   
    } else if (command == "40") { //halt
      if (nothalt == 0 && value == "true") {
        Serial.println("halt");
        controlData.nothalt = 1;
        controlData.abfahrt = false;
        anwurf = false;
      }
    } else if (command == "52") { //nothalt
      if (value == "true") {
        Serial.println("nothalt");
        controlData.nothalt = 3;
        controlData.abfahrt = false;
        anwurf = false;
      }
    } else if (command == "speed") {
      if (value != (String)speed && !value.isEmpty()) {
        speed = value.toInt();
      }
    }
    esp_now_send(slaveMacs[0], (uint8_t *)&controlData, sizeof(controlData));
    esp_now_send(slaveMacs[2], (uint8_t *)&controlData, sizeof(controlData));
    Serial.println("---------------------------------------------------------------------------------------------------------------------------------------------------------");
    speedData.speed = speed;
  }
  if (speedDelay.justFinished()) {
    speedData.id = 2;
    esp_now_send(slaveMacs[0], (uint8_t *)&speedData, sizeof(speedData));
    esp_now_send(slaveMacs[2], (uint8_t *)&speedData, sizeof(speedData));
    speedDelay.restart();
  }
  digitalWrite(D5,LOW);
}