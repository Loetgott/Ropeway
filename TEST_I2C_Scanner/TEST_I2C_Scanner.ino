#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);  // Warten, bis die serielle Verbindung hergestellt ist
  Serial.println("I2C Scanner läuft...");
}

void loop() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Scanne nach I2C-Geräten...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C-Gerät gefunden an Adresse 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    } 
    else if (error == 4) {
      Serial.print("Unbekannter Fehler an Adresse 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  
  if (nDevices == 0)
    Serial.println("Keine I2C-Geräte gefunden.\n");
  else
    Serial.println("Scan abgeschlossen.\n");
  
  delay(5000);  // Warte 5 Sekunden, bevor du erneut scannst
}
