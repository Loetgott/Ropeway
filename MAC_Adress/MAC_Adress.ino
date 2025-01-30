#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // Setze den Modus auf Station (Client)
  Serial.println();
  
  // Zeigt die MAC-Adresse des Wemos D1 Mini an
  Serial.print("MAC-Adresse: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Leere Schleife
}
