#include <ESP8266WiFi.h>
#include <espnow.h>
#include <millisDelay.h>

millisDelay sensor1Delay;
millisDelay sensor2Delay;
millisDelay sensor3Delay;
millisDelay sensor4Delay;

int sensorDelay = 2000;

int sensor1 = D3;
int sensor2 = D4;
int sensor3 = D2;
int sensor4 = D1;

// Empfänger MAC-Adresse
uint8_t broadcastAddress[] = { 0x50, 0x02, 0x91, 0xFA, 0xF5, 0x67 };

typedef struct struct_message_sensors {
  int id = 0;
  int station = 1;
  bool sensor1;
  bool sensor2;
  bool sensor3;
  bool sensor4;
} struct_message_sensors;

struct_message_sensors sensorData;

// Trigger-Status für jeden Sensor definieren
bool sensor1Triggered = false;
bool sensor2Triggered = false;
bool sensor3Triggered = false;
bool sensor4Triggered = false;

void onSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Sendestatus: ");
  Serial.println(sendStatus == 0 ? "Erfolgreich" : "Fehlgeschlagen");
}

void setup() {
  Serial.begin(115200);

  // Sensoren als Eingänge definieren
  pinMode(sensor1, INPUT_PULLUP);
  pinMode(sensor2, INPUT_PULLUP);
  pinMode(sensor3, INPUT_PULLUP);
  pinMode(sensor4, INPUT_PULLUP);

  // WLAN im STA-Modus starten
  WiFi.mode(WIFI_STA);

  // ESP-NOW Initialisierung
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialisierung fehlgeschlagen");
    return;
  }

  // Callback hinzufügen
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(onSent);

  // Empfänger hinzufügen
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  bool sendData = false;

  // Sensor 1 überprüfen
  if (sensor1Delay.justFinished()) {
    sensor1Delay.stop();  // Verzögerung beenden
  }
  if (digitalRead(sensor1) == LOW && !sensor1Triggered && !sensor1Delay.isRunning()) {
    sensor1Triggered = true;
    sensorData.sensor1 = true;
    sendData = true;
  } else if (digitalRead(sensor1) == HIGH && sensor1Triggered) {
    sensor1Triggered = false;
    sensorData.sensor1 = false;
    sendData = true;
    sensor1Delay.start(sensorDelay);
  }

  // Sensor 2 überprüfen
  if (sensor2Delay.justFinished()) {
    sensor2Delay.stop();  // Verzögerung beenden
  }
  if (digitalRead(sensor2) == LOW && !sensor2Triggered && !sensor2Delay.isRunning()) {
    sensor2Triggered = true;
    sensorData.sensor2 = true;
    sendData = true;
  } else if (digitalRead(sensor2) == HIGH && sensor2Triggered) {
    sensor2Triggered = false;
    sensorData.sensor2 = false;
    sendData = true;
    sensor2Delay.start(sensorDelay);
  }

  // Sensor 3 überprüfen
  if (sensor3Delay.justFinished()) {
    sensor3Delay.stop();  // Verzögerung beenden
  }
  if (digitalRead(sensor3) == LOW && !sensor3Triggered && !sensor3Delay.isRunning()) {
    sensor3Triggered = true;
    sensorData.sensor3 = true;
    sendData = true;
  } else if (digitalRead(sensor3) == HIGH && sensor3Triggered) {
    sensor3Triggered = false;
    sensorData.sensor3 = false;
    sendData = true;
    sensor3Delay.start(sensorDelay);
  }

  // Sensor 4 überprüfen
  if (sensor4Delay.justFinished()) {
    sensor4Delay.stop();  // Verzögerung beenden
  }
  if (digitalRead(sensor4) == LOW && !sensor4Triggered && !sensor4Delay.isRunning()) {
    sensor4Triggered = true;
    sensorData.sensor4 = true;
    sendData = true;
  } else if (digitalRead(sensor4) == HIGH && sensor4Triggered) {
    sensor4Triggered = false;
    sensorData.sensor4 = false;
    sendData = true;
    sensor4Delay.start(sensorDelay);
  }

  // Daten nur senden, wenn sich der Status eines Sensors geändert hat
  if (sendData) {
    esp_now_send(broadcastAddress, (uint8_t *)&sensorData, sizeof(sensorData));
    Serial.println("Daten gesendet aufgrund einer Statusänderung");
  }

  delay(50);  // Kurze Pause, um unnötige Abfragen zu vermeiden
}

