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
  - LiquidCrystal_I2C: For LED controlling.
  Notes:
  Make sure to adjust the time zone offset according to your location.
*/

#include <Arduino.h>
#include <NeoSWSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GPS pin:
// RX Arduino = TX GPS 
// TX Arduino = RX GPS
const int RXPin = 4, TXPin = 3;
const int relayPin = 7, ledPin = 13, buttonPin = A1;
const uint32_t GPSBaud = 9600;
const int offset = -3;
bool fixStatus, turnOn;
bool backlightState = false; // Variable to track backlight state
int buttonState = 0, lastButtonState = 0;

// Creates objects
TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Define schedules using arrays
const int NUM_SCHEDULES = 5;
const int schedules[NUM_SCHEDULES][6] = {
  // {hh_ini, mm_ini, ss_ini, hh_end, mm_end, ss_end}
  {9, 0, 0, 9, 0, 30},   
  {12, 0, 0, 12, 1, 0},
  {21, 0, 0, 21, 1, 0}
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
  
  pinMode(buttonPin, INPUT_PULLUP);  // To set button as input
  analogWrite(buttonPin, LOW);  // No pressed as default
  pinMode(relayPin, OUTPUT); // To set relay as output
  digitalWrite(relayPin, LOW); // Turn off by default
  pinMode(ledPin, OUTPUT); // To set led as output
  digitalWrite(ledPin, LOW); // Turn off by default

  lcd.backlight(); // Turn on backlight to see initialization messages

  writeToLCD(0, "Buscando GPS");
    while (!checkGPSFix())
    {
      ;
    }
  writeToLCD(0, "GPS Ok");
  writeToLCD(1, "");

  delay(1000);

  lcd.noBacklight(); // Turn off backlight after initialization
}

void loop() {
  int valorAnalogico = analogRead(buttonPin);  // read button 
  float voltage = valorAnalogico * (5.0 / 1023.0);  // convert into voltage

  if (voltage > 1) {
    buttonState = 1;
  } else {
    buttonState = 0;
  }
  delay(10);

  // if button pressed, and wasn't pressed before
  if ((buttonState == 1) & (lastButtonState == 0)) {
      backlightState = !backlightState;
      delay(10);
      if (backlightState) {
        lcd.backlight();
      } else {
        lcd.noBacklight();
      }
  }
  
  lastButtonState = buttonState;

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
          break;
        }
      }

      // Create char arrays for time and satellites
      char timeStr[9];
      char satStr[3];

      // Format strings
      sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
      sprintf(satStr, "%lu", gps.satellites.value());

      // Write to LCD
      writeToLCD(0, String(timeStr) + "/Sat:" + String(satStr));

      // Control the relay based on the time check
      if (turnOn) {
        digitalWrite(ledPin, HIGH);
        digitalWrite(relayPin, HIGH);
        writeToLCD(1, "Encendido: " + String(activeSchedule));
      } else {
        digitalWrite(ledPin, LOW);
        digitalWrite(relayPin, LOW);
        writeToLCD(1, "Apagado");
      }
    }
  }
}