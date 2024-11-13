#include <Wire.h>

#define SLAVE_ADDRESS 0x08  // Adresse des Slave-Geräts
String receivedMessage = "";  // Variable zum Speichern der empfangenen Nachricht

void setup() {
  Wire.begin(SLAVE_ADDRESS);  // Wemos D1 Mini als Slave mit Adresse initialisieren
  Wire.onReceive(receiveEvent);  // Funktion aufrufen, wenn Daten empfangen werden
  Serial.begin(115200);
}

void loop() {
  if (receivedMessage.length() > 0) {
    Serial.println("Nachricht empfangen: " + receivedMessage);
    receivedMessage = "";  // Nachricht zurücksetzen
  }
}

void receiveEvent(int bytes) {
  Serial.println("Empfang gestartet.");  // Debug-Ausgabe, um den Empfang zu überprüfen
  while (Wire.available()) {
    char c = Wire.read();  // Daten byteweise lesen
    receivedMessage += c;
  }
}

