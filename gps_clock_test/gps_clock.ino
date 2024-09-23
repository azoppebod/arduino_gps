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
  Connection must be set at 9600
*/
#include <Arduino.h>
#include <NeoSWSerial.h>
#include <TinyGPSPlus.h>

// Declare variables
// GPS pin. RX Arduino = TX GPS 
// TX Arduino = RX GPS
const int RXPin = 4, TXPin = 3;
const int GPSBaud = 9600, serialBaud = 9600;
const int offset = -3;
const int relayPin = 7; // Relay
const int ledPin = 13; // LED

int32_t lastValidHour = 0;
int32_t lastValidMinute = 0;
int32_t lastValidSecond = 0;

bool fixStatus, turnOn;
int32_t hour, minute, second, currentTimeSeconds;
int32_t startHour1, startMinute1, startSecond1, endHour1, endMinute1, endSecond1, startTimeSeconds1, endTimeSeconds1;
int32_t startHour2, startMinute2, startSecond2, endHour2, endMinute2, endSecond2, startTimeSeconds2, endTimeSeconds2;
int32_t startHour3, startMinute3, startSecond3, endHour3, endMinute3, endSecond3, startTimeSeconds3, endTimeSeconds3;

// Creates GPS object
TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);

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
        Serial.print("Fix status: ");
        Serial.println(fixStatus);
        
        // Save the current time when we get a valid fix
        lastValidHour = gps.time.hour();
        lastValidMinute = gps.time.minute();
        lastValidSecond = gps.time.second();
        
        // Adjust hour based on time zone offset
        lastValidHour = (lastValidHour + offset);
        if (lastValidHour < 0) {
          lastValidHour += 24;
        }
        if (lastValidHour > 23) {
          lastValidHour -= 24;
        }
        
        return fixStatus;
      }
    }
  }
  return 0;
}

void writeTime() {
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(second);
  Serial.print("//");
  Serial.println(currentTimeSeconds);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Turn off by default
  
  Serial.begin(serialBaud);
  neogps.begin(GPSBaud);
  
  Serial.println("Waiting for GPS...");
  while (!checkGPSFix())
  {
    ;
  }
  Serial.println("Valid GPS signal established.");
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Turn off by default
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Turn on after acquire signal
}

void loop() {
  while (neogps.available() > 0) {
    gps.encode(neogps.read());
  
    if (gps.time.isUpdated() && gps.satellites.value() > 0) {
      // Use the last valid time instead of current GPS time
      hour = lastValidHour;
      minute = lastValidMinute;
      second = lastValidSecond;

      // Update the last valid time
      lastValidHour = gps.time.hour();
      lastValidMinute = gps.time.minute();
      lastValidSecond = gps.time.second();

      // Adjust hour based on time zone offset
      lastValidHour = (lastValidHour + offset);
      if (lastValidHour < 0) {
        lastValidHour += 24;
      }
      if (lastValidHour > 23) {
        lastValidHour -= 24;
      }

      // Convert current time to seconds
      currentTimeSeconds = (hour * 3600) + (minute * 60) + second;
      
      //
      // SCHEDULE 1
      //
      // Define time windows for turning on and off
      startHour1 = 8, startMinute1 = 0, startSecond1 = 0;
      endHour1 = 8, endMinute1 = 0, endSecond1 = 30;
      
      // Convert start and end times to seconds from start of the day
      startTimeSeconds1 = ((startHour1 * 3600) + (startMinute1 * 60) + startSecond1);
      endTimeSeconds1 = ((endHour1 * 3600) + (endMinute1 * 60) + endSecond1);
  
      //
      // SCHEDULE 2
      //
      startHour2 = 12, startMinute2 = 0, startSecond2 = 0;
      endHour2 = 12, endMinute2 = 1, endSecond2 = 0;
      
      // Convert start and end times to seconds from start of the day
      startTimeSeconds2 = ((startHour2 * 3600) + (startMinute2 * 60) + startSecond2);
      endTimeSeconds2 = ((endHour2 * 3600) + (endMinute2 * 60) + endSecond2);

      //
      // SCHEDULE 3
      //
      startHour3 = 21, startMinute3 = 0, startSecond3 = 0;
      endHour3 = 21, endMinute3 = 1, endSecond3 = 0;
      
       // Convert start and end times to seconds from start of the day
      startTimeSeconds3 = ((startHour3 * 3600) + (startMinute3 * 60) + startSecond3);
      endTimeSeconds3 = ((endHour3 * 3600) + (endMinute3 * 60) + endSecond3);
      
      // Check if the current time is within any turn-on window
      turnOn = false;
  
      // Check if the interval crosses midnight
      if (startTimeSeconds1 < endTimeSeconds1) {
        // Interval does not cross midnight
        if (currentTimeSeconds >= startTimeSeconds1 && currentTimeSeconds < endTimeSeconds1) {
          turnOn = true;
        }
      } else {
        // Interval crosses midnight
        if (currentTimeSeconds >= startTimeSeconds1 || currentTimeSeconds < endTimeSeconds1) {
          turnOn = true;
        }
      }

      // Check if the interval crosses midnight
      if (startTimeSeconds2 < endTimeSeconds2) {
        // Interval does not cross midnight
        if (currentTimeSeconds >= startTimeSeconds2 && currentTimeSeconds < endTimeSeconds2) {
          turnOn = true;
        }
      } else {
        // Interval crosses midnight
        if (currentTimeSeconds >= startTimeSeconds2 || currentTimeSeconds < endTimeSeconds2) {
          turnOn = true;
        }
      }

      // Check if the interval crosses midnight
      if (startTimeSeconds3 < endTimeSeconds3) {
        // Interval does not cross midnight
        if (currentTimeSeconds >= startTimeSeconds3 && currentTimeSeconds < endTimeSeconds3) {
          turnOn = true;
        }
      } else {
        // Interval crosses midnight
        if (currentTimeSeconds >= startTimeSeconds3 || currentTimeSeconds < endTimeSeconds3) {
          turnOn = true;
        }
      }
 
      // Control the relay based on the time check
      if (turnOn) {
        digitalWrite(relayPin, HIGH); // Turn on
        Serial.println("Relay set to HIGH - On");
        writeTime();
      } else {
        digitalWrite(relayPin, LOW); // Turn off
        Serial.println("Relay set to LOW - Off");
        writeTime();
      }
    }
    delay(5000); 
  }
}