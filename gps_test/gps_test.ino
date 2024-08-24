#include <TinyGPSPlus.h>
#include <NeoSWSerial.h>

// Definir los pines para el GPS
const int RXPin = 4, TXPin = 3;
const uint32_t GPSBaud = 9600;

// Crear el objeto TinyGPS++
TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  neogps.begin(GPSBaud);
  Serial.println("Esperando datos del GPS...");
}

void loop() {
  while (neogps.available() > 0) {
    gps.encode(neogps.read());

    if (gps.time.isUpdated()) {
      // Obtener la hora actual
      Serial.print("Hora (UTC): ");
      Serial.print(gps.time.hour());
      Serial.print(":");
      Serial.print(gps.time.minute());
      Serial.print(":");
      Serial.println(gps.time.second());
    }
  }
}
