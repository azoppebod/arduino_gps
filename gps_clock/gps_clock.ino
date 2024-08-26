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

#include <NeoSWSerial.h>
#include <TinyGPSPlus.h>

// Declare variables
// GPS pin. RX Arduino = TX GPS 
// TX Arduino = RX GPS
const int RXPin = 4, TXPin = 3;
const uint32_t GPSBaud = 9600, serialBaud = 9600;
const int offset = -3;
bool fixStatus, turnOn;
uint32_t hour, minute, second, currentTimeSeconds;
uint32_t startHour1, startMinute1, startSecond1, endHour1, endMinute1, endSecond1, startTimeSeconds1, endTimeSeconds1;
uint32_t startHour2, startMinute2, startSecond2, endHour2, endMinute2, endSecond2, startTimeSeconds2, endTimeSeconds2;
uint32_t startHour3, startMinute3, startSecond3, endHour3, endMinute3, endSecond3, startTimeSeconds3, endTimeSeconds3;

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
        Serial.print("Fix status: ");
        Serial.println(fixStatus);
        return fixStatus;
      }
    }
  }
  return 0;
}

void setup() {
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
}

void loop() {
  while (neogps.available() > 0) {
    gps.encode(neogps.read());
  
    if (gps.time.isUpdated() && gps.satellites.value() > 0) {
      hour = gps.time.hour(); // UTC current Time 
      minute = gps.time.minute();      
      second = gps.time.second();      

      // Adjust hour based on time zone offset
      hour = (hour + offset);
      if (hour < 0) {
        hour += 24;
      }

      if (hour > 23) {
        hour -= 24;
      }

      // Convert current time and range times to minutes
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
//      delay(10000);              // for troubleshooting purposes
    }
  }
}

void writeTime() {
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.print(second);
  Serial.print(":");
  Serial.println(currentTimeSeconds);
}
