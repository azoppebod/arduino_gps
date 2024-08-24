#include <NeoSWSerial.h>

// Definir los pines para el módulo GPS
const int RXPin = 4, TXPin = 3;
const uint32_t GPSBaud = 9600;

// Crear un objeto NeoSWSerial para la comunicación serial con el GPS
NeoSWSerial neogps(RXPin, TXPin);

void setup() {
  // Iniciar la comunicación serial con el monitor
  Serial.begin(9600);
  
  // Iniciar la comunicación serial con el GPS
  neogps.begin(GPSBaud);

  // Mensaje inicial en el monitor serial
  Serial.println("Hello, GPS World!");
}

void loop() {
  // Verificar si hay datos disponibles del GPS
  while (neogps.available()) {
    // Leer un carácter del GPS y enviarlo al monitor serial
    char c = neogps.read();
    Serial.print(c);
  }
}
