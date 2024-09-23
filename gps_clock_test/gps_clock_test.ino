#include <NeoSWSerial.h>
#include <TinyGPSPlus.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// Declaración de variables
const int RXPin = 4, TXPin = 3;  // Pines RX y TX para el GPS
const int GPSBaud = 9600, serialBaud = 9600;  // Velocidades de comunicación
const int offset = -3;  // Desfase horario
const int relayPin = 7;  // Pin del relé
const int ledPin = 13;   // Pin del LED

bool fixStatus = 0;
float blinks;
unsigned long lastCalibrationTime = 0;
const unsigned long calibrationInterval = 10 * 1000;  // Recalibración cada 1 hora en milisegundos

TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);

void blink(float blinksPerSecond) {
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  static int ledState = LOW;
  unsigned long interval = (unsigned long)(1000.0 / (2 * blinksPerSecond));
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

bool checkGPSFix() {
  gps.encode(neogps.read());
  Serial.println(gps.location.isValid());
  Serial.println(gps.date.isValid());
  Serial.println(gps.time.isValid());
  Serial.println(gps.location.isUpdated());
  Serial.println(gps.date.isUpdated());
  Serial.println(gps.time.isUpdated());
  Serial.println(gps.satellites.value());
  if (neogps.available() > 0) {
    if (gps.encode(neogps.read())) {
      if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid() &&
          gps.location.isUpdated() && gps.date.isUpdated() && gps.time.isUpdated() &&
          gps.satellites.value() > 0) {
        fixStatus = true;
        Serial.println("checksi");
        return fixStatus;
      }
    }
  }
          Serial.println("checkno");
  return false;
}

void syncTimeWithGPS() {
  if (checkGPSFix()) {
    Serial.println("si");
    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();
    int hour = gps.time.hour() + offset;  // Ajuste de zona horaria
    int minute = gps.time.minute();
    int second = gps.time.second();
    
    if (hour < 0) hour += 24;
    if (hour > 23) hour -= 24;

    setTime(hour, minute, second, day, month, year);  // Ajusta la hora en la librería Time

    // Corregido: Uso de Serial.print() en lugar de printf
    Serial.print("Hora sincronizada: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.print(second);
  }
  else {
      Serial.println("no");
    blinks = 0.1;
  }
}

void setup() {
  Serial.begin(serialBaud);
  neogps.begin(GPSBaud);
  
  Serial.println("Waiting for GPS...");
  while (!checkGPSFix()) {
  }
  Serial.println("Valid GPS signal established.");
  blinks = 0.5;
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Apaga el relé por defecto
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Apaga el LED por defecto
  
  syncTimeWithGPS();  // Sincroniza la hora con el GPS al inicio
  lastCalibrationTime = millis();  // Establece el tiempo inicial de calibración
  
  // Configuración de las alarmas
  Alarm.alarmRepeat(8, 0, 0, turnRelayOn);   // Enciende el relé a las 8:00:00
  Alarm.alarmRepeat(8, 0, 30, turnRelayOff); // Apaga el relé a las 8:00:30
  Alarm.alarmRepeat(12, 0, 0, turnRelayOn);  // Enciende el relé a las 12:00:00
  Alarm.alarmRepeat(12, 1, 0, turnRelayOff); // Apaga el relé a las 12:01:00
  Alarm.alarmRepeat(22, 12, 0, turnRelayOn);  // Enciende el relé a las 21:00:00
  Alarm.alarmRepeat(22, 13, 0, turnRelayOff); // Apaga el relé a las 21:01:00
}

void turnRelayOn() {
  digitalWrite(relayPin, HIGH);  // Enciende el relé
  Serial.println("Relay set to HIGH - On");
}

void turnRelayOff() {
  digitalWrite(relayPin, LOW);  // Apaga el relé
  Serial.println("Relay set to LOW - Off");
}

void loop() {
  blink(blinks);

  if (millis() - lastCalibrationTime >= calibrationInterval) {
    syncTimeWithGPS();  // Recalibra el tiempo con el GPS
    lastCalibrationTime = millis();
  }
  
  Alarm.delay(5000);  // Utiliza Alarm.delay() en lugar de delay() para que funcione la librería TimeAlarms
}
