#include <ESP8266WiFi.h>
#include <espnow.h>

// Empfänger MAC-Adresse
uint8_t broadcastAddress[] = {0x98, 0xF4, 0xAB, 0xCC, 0xE1, 0xEF};

typedef struct struct_message_sensors {
  int id = 0;
  int station = 0;
  bool sensor1;
  bool sensor2;
  bool sensor3;
  bool sensor4;
} struct_message_sensors;

struct_message_sensors sensorData;

// Pins der digitalen Sensoren definieren
const int sensor1 = 0;
const int sensor2 = 1;
const int sensor3 = 2;
const int sensor4 = 3;

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
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);
    pinMode(sensor4, INPUT);
    
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
    if (digitalRead(sensor1) == LOW && !sensor1Triggered) {
        sensor1Triggered = true;
        sensorData.sensor1 = true;
        sendData = true;
    } else if (digitalRead(sensor1) == HIGH && sensor1Triggered) {
        sensor1Triggered = false;
        sensorData.sensor1 = false;
        sendData = true;
    }

    // Sensor 2 überprüfen
    if (digitalRead(sensor2) == LOW && !sensor2Triggered) {
        sensor2Triggered = true;
        sensorData.sensor2 = true;
        sendData = true;
    } else if (digitalRead(sensor2) == HIGH && sensor2Triggered) {
        sensor2Triggered = false;
        sensorData.sensor2 = false;
        sendData = true;
    }

    // Sensor 3 überprüfen
    if (digitalRead(sensor3) == LOW && !sensor3Triggered) {
        sensor3Triggered = true;
        sensorData.sensor3 = true;
        sendData = true;
    } else if (digitalRead(sensor3) == HIGH && sensor3Triggered) {
        sensor3Triggered = false;
        sensorData.sensor3 = false;
        sendData = true;
    }

    // Sensor 4 überprüfen
    if (digitalRead(sensor4) == LOW && !sensor4Triggered) {
        sensor4Triggered = true;
        sensorData.sensor4 = true;
        sendData = true;
    } else if (digitalRead(sensor4) == HIGH && sensor4Triggered) {
        sensor4Triggered = false;
        sensorData.sensor4 = false;
        sendData = true;
    }

    // Daten nur senden, wenn sich der Status eines Sensors geändert hat
    if (sendData) {
        esp_now_send(broadcastAddress, (uint8_t *)&sensorData, sizeof(sensorData));
        Serial.println("Daten gesendet aufgrund einer Statusänderung");
    }

    delay(50);  // Kurze Pause, um unnötige Abfragen zu vermeiden
}
