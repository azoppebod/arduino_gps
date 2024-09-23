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

// GPS pin. RX Arduino = TX GPS 
// TX Arduino = RX GPS
const int RXPin = 4, TXPin = 3;
const uint32_t GPSBaud = 9600;
const int offset = -3;
bool fixStatus, turnOn;

// Creates GPS object
TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);

int relayPin = 7; // Relay

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

void setup() {
  Serial.begin(9600);
  neogps.begin(GPSBaud);

  Serial.println("Waiting for GPS...");
    while (!checkGPSFix())
    {
      ;
    }
  Serial.println("Valid GPS signal established.");

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

      // Define time windows for turning on and off
      int startHour1 = 0, startMinute1 = 18, startSecond1 = 0;
      int endHour1 = 0, endMinute1 = 18, endSecond1 = 5;

      int startHour2 = 23, startMinute2 = 12, startSecond2 = 10;
      int endHour2 = 23, endMinute2 = 12, endSecond2 = 23;

      int startHour3 = 22, startMinute3 = 44, startSecond3 = 20;
      int endHour3 = 22, endMinute3 = 44, endSecond3 = 30;

      // Check if the current time is within any turn-on window
      bool turnOn = false;

      if ((hour >= startHour1 && minute >= startMinute1 && second >= startSecond1) && 
          (hour <= endHour1 && minute <= endMinute1 && second <= endSecond1)) {
        turnOn = true;
        Serial.print("TurnOn: 1 |");
      }

      if ((hour >= startHour2 && minute >= startMinute2 && second >= startSecond2) && 
          (hour <= endHour2 && minute <= endMinute2 && second <= endSecond2)) {
        turnOn = true;
        Serial.print("TurnOn: 2 |");
      }

      if ((hour >= startHour3 && minute >= startMinute3 && second >= startSecond3) && 
          (hour <= endHour3 && minute <= endMinute3 && second <= endSecond3)) {
        turnOn = true;
        Serial.print("TurnOn: 3 |");
      }

      // Control the relay based on the time check
      if (turnOn) {
        digitalWrite(relayPin, HIGH); // Turn on
        Serial.print("Relay set to HIGH - On // ");
      } else {
        digitalWrite(relayPin, LOW); // Turn off
        Serial.print("Relay set to LOW - Off // ");
      }

      // Print current time for debugging
      Serial.print("Time: ");
      Serial.print(hour < 10 ? "0" : "");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute < 10 ? "0" : "");
      Serial.print(minute);
      Serial.print(":");
      Serial.print(second < 10 ? "0" : "");
      Serial.print(second);
      Serial.print(" // Sat: ");
      Serial.println(gps.satellites.value());  
    }
  }
}