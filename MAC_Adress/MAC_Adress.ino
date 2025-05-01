#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // Setze den Modus auf Station

  // Zeige die MAC-Adresse des WLAN-Interfaces an
  Serial.print("MAC-Adresse (WLAN): ");
  Serial.println(WiFi.macAddress());
  
  Serial.println();
}

void loop() {
  // Leere Schleife
}
