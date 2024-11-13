#include <ESP8266WiFi.h>
#include <espnow.h>

#define NUM_SLAVES 4

#define y

// MAC-Adressen der vier Slaves (ersetze sie durch die tatsächlichen MACs deiner Slaves)
uint8_t slaveMacs[NUM_SLAVES][6] = {
  { 0x48, 0x3F, 0xDA, 0x57, 0xAF, 0x84 },  // Slave 1
  { 0xBC, 0xFF, 0x4D, 0x02, 0x05, 0x97 },  // Slave 2
  { 0xBC, 0xFF, 0x4D, 0x15, 0x96, 0xB8 },  // Slave 3
  { 0xC4, 0x5B, 0xBE, 0x4E, 0x94, 0xCC }   // Slave 4
};

// Struktur für die Steuerungsdaten
typedef struct struct_message_controls {
  int id;            // Nachrichtentyp-Kennung
  int speed;
  int nothalt;
  int langsamfahrt;
  bool fahrtrichtung;
} struct_message_controls;

// Struktur für die Sensordaten
typedef struct struct_message_sensors {
  int id;            // Nachrichtentyp-Kennung
  int station;
  bool sensor1;
  bool sensor2;
  bool sensor3;
  bool sensor4;
} struct_message_sensors;

struct_message_controls controlData;
struct_message_sensors sensorData0;
struct_message_sensors sensorData1;

// Empfangene Daten
union received_data {
  struct_message_controls control_data;
  struct_message_sensors sensor_data;
} receivedData;

// Callback für das Senden
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  digitalWrite(D4, HIGH);
  Serial.print("Daten gesendet an: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" Status: ");
  Serial.println(sendStatus == 0 ? "Erfolgreich" : "Fehlgeschlagen");
  digitalWrite(D4, LOW);
}

// Callback für den Empfang von Daten
void OnDataReceived(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  int id;
  memcpy(&id, incomingData, sizeof(int));
  if (id == 0 && len == sizeof(struct_message_sensors)) {
    memcpy(&receivedData.sensor_data, incomingData, sizeof(struct_message_sensors));
    if(receivedData.sensor_data.station == 0){
      sensorData0.id = receivedData.sensor_data.id;
      sensorData0.station = receivedData.sensor_data.station;
      sensorData0.sensor1 = receivedData.sensor_data.sensor1;
      sensorData0.sensor2 = receivedData.sensor_data.sensor2;
      sensorData0.sensor3 = receivedData.sensor_data.sensor3;
      sensorData0.sensor4 = receivedData.sensor_data.sensor4;
      esp_now_send(slaveMacs[0], (uint8_t *)&sensorData0, sizeof(sensorData0));
    }else if(receivedData.sensor_data.station == 1){
      sensorData1.id = receivedData.sensor_data.id;
      sensorData1.station = receivedData.sensor_data.station;
      sensorData1.sensor1 = receivedData.sensor_data.sensor1;
      sensorData1.sensor2 = receivedData.sensor_data.sensor2;
      sensorData1.sensor3 = receivedData.sensor_data.sensor3;
      sensorData1.sensor4 = receivedData.sensor_data.sensor4;
      esp_now_send(slaveMacs[2], (uint8_t *)&sensorData1, sizeof(sensorData1));
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(D4, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D3, OUTPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialisierung fehlgeschlagen");
    digitalWrite(D3, HIGH);
    return;
  }
  digitalWrite(D0, HIGH);

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
}


void loop() {
  //Serial.println(".");
}
