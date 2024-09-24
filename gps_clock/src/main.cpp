/*
  Project: Automatic Light Control Based on GPS Time
  Date: August 23, 2024
  Author: Andrés Zoppelletto
  Description:
  This code controls the turning on and off of a device at multiple times during the day using an Arduino Uno and a NEO-6M GPS module.
  The program retrieves the time from the GPS, converts it to the local time based on the specified time zone offset, and turns a relay 
  connected to a light on or off based on the current time. Multiple time windows during the day are defined for automatic light control.
  Libraries Used:
  - TinyGPS++: To decode data from the GPS module.
  - NeoSWSerial: For serial communication with the GPS module.
  Notes:
  Make sure to adjust the time zone offset according to your location.
*/

#include <Arduino.h>
#include <NeoSWSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GPS pin. RX Arduino = TX GPS 
// TX Arduino = RX GPS
const int RXPin = 4, TXPin = 3;
const uint32_t GPSBaud = 9600;
const int offset = -3;
bool fixStatus, turnOn;

// Creates objects
TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);
LiquidCrystal_I2C lcd(0x27, 16, 2);  

int relayPin = 7; // Relay

// Define schedules using arrays
const int NUM_SCHEDULES = 5;
const int schedules[NUM_SCHEDULES][6] = {
  // {hh_ini, mm_ini, ss_ini, hh_end, mm_end, ss_end}
  {9, 0, 0, 9, 0, 30},   
  {12, 0, 0, 12, 1, 0},
  {21, 0, 0, 21, 1, 0}, 
  {20, 27, 0, 20, 27, 30}, 
  {20, 27, 35, 20, 27, 40} 
};

bool checkGPSFix()
{
  while ((neogps.available() > 0))
  {
    if (gps.encode(neogps.read()))
    {
      if ((gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) &&
          (gps.location.isUpdated() && gps.date.isUpdated() && gps.time.isUpdated()) &&
          (gps.satellites.value() > 0))
      {
        fixStatus = true;
        return fixStatus;
      }
    }
  }
  return 0;
}
void writeToLCD(int line, const String& text) {
  lcd.setCursor(0, line);
  lcd.print(text);
  for (int i = text.length(); i < 16; i++) {
    lcd.print(" ");
  }
}

void setup() {
  Serial.begin(9600);
  neogps.begin(GPSBaud);
  lcd.init();
  lcd.backlight();

  Serial.println("Waiting for GPS...");
  writeToLCD(0, "Buscando GPS");
    while (!checkGPSFix())
    {
      ;
    }
  Serial.println("Valid GPS signal established.");
  writeToLCD(0, "GPS Ok");
  writeToLCD(1, "");

  delay(1000);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Turn off by default
}

void loop() {
  while (neogps.available() > 0) {
    gps.encode(neogps.read());

    if (gps.time.isUpdated()) {
      int hour = gps.time.hour(); // UTC current Time 
      int minute = gps.time.minute();      
      int second = gps.time.second();      

      // Adjust hour based on time zone offset
      hour = (hour + offset) % 24;
      if (hour < 0) {
        hour += 24;
      }

      // Check if the current time is within any turn-on window
      bool turnOn = false;
      int activeSchedule = -1;

      for (int i = 0; i < NUM_SCHEDULES; i++) {
        int startHour = schedules[i][0], startMinute = schedules[i][1], startSecond = schedules[i][2];
        int endHour = schedules[i][3], endMinute = schedules[i][4], endSecond = schedules[i][5];

        if ((hour > startHour || (hour == startHour && minute > startMinute) || 
            (hour == startHour && minute == startMinute && second >= startSecond)) &&
            (hour < endHour || (hour == endHour && minute < endMinute) || 
            (hour == endHour && minute == endMinute && second <= endSecond))) {
          turnOn = true;
          activeSchedule = i + 1;
          Serial.print("TurnOn: ");
          Serial.print(activeSchedule);
          Serial.print(" |");
          break;
        }
      }

      // Create char arrays for time and satellites
      char timeStr[9];
      char satStr[3];

      // Format time string
      sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);

      // Format satellites string
      sprintf(satStr, "%d", gps.satellites.value());

      // Print to Serial
      Serial.print("Time: ");
      Serial.print(timeStr);
      Serial.print(" // Sat: ");
      Serial.println(satStr);

      // Write to LCD
      writeToLCD(0, String(timeStr) + "/Sat:" + String(satStr));

      // Control the relay based on the time check
      if (turnOn) {
        digitalWrite(relayPin, HIGH); // Turn on
        Serial.print("Relay set to HIGH - On // ");
        writeToLCD(1, "Prendido: " + String(activeSchedule));
      } else {
        digitalWrite(relayPin, LOW); // Turn off
        Serial.print("Relay set to LOW - Off // ");
        writeToLCD(1, "Apagado");
      }
    }
  }
}