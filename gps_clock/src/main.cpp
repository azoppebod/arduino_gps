/*
  Project: Automatic Device Control Based on GPS Time
  Date: August 23, 2024
  Author: Andrés Zoppelletto
  Description:
  This code controls the turning on and off of a device at multiple times during the day using an Arduino Uno and a NEO-6M GPS module.
  The program retrieves the time from the GPS, converts it to the local time based on the specified time zone offset, and turns a relay 
  connected to a device on or off based on the current time. Multiple time windows during the day are defined for automatic light control.
  It also shows the GPS state, time and schedule execution in a LCD. It has a button to trigger the manual execution.
  Libraries Used:
  - NeoSWSerial: For serial communication with the GPS module.
  - TinyGPS++: To decode data from the GPS module.
  - Wire: To control LCD comunication.
  - LiquidCrystal_I2C: For LCD controlling.
  Notes:
  Make sure to adjust the time zone offset according to your location. It can be automatically defined based on the physical location 
  using GPS, but it should also account for Daylight Saving Time (DST).
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
const int relayPin = 9, ledPin = 13, buttonPin = A1, auxOverridePin = A2, overridePin = A3;
const int PWMValue = 63;
const uint32_t GPSBaud = 9600;
const int offset = -3;
bool fixStatus, turnOn, manual;
bool backlightState = false; // Variable to track backlight state
int buttonState = 0, lastButtonState = 0, activeSchedule;
int overrideState = 0, lastOverrideState = 0;
unsigned long lastTime = millis();

// Creating objects
TinyGPSPlus gps;
NeoSWSerial neogps(RXPin, TXPin);
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Define schedules using arrays
const int NUM_SCHEDULES = 3;
const int schedules[NUM_SCHEDULES][6] = {
  // {hh24_ini, mm_ini, ss_ini, hh24_end, mm_end, ss_end}
  {8, 0, 0, 8, 0, 30},   
  {12, 0, 0, 12, 1, 0},
  {21, 0, 0, 21, 1, 0}
};

// LCD writting function
void writeToLCD(int line, const String& text) {
  lcd.setCursor(0, line);
  lcd.print(text);
  for (int i = text.length(); i < 16; i++) {
    lcd.print(" ");
  }
}

void handleButtonPress() {
  // Using an analog pin to avoid false positives. During testing, the digital pins were unreliable.
  int valorAnalogico = analogRead(buttonPin);  // read button 
  float voltage = valorAnalogico * (5.0 / 1023.0);  // convert into voltage

  if (voltage > 1) {
    buttonState = 1;
  } else {
    buttonState = 0;
  }

  // if button pressed, and wasn't pressed before
  if ((buttonState == 1) & (lastButtonState == 0)) {
      backlightState = !backlightState;
      // delay(10);
      if (backlightState) {
        lcd.backlight();
      } else {
        lcd.noBacklight();
      }
  }
  
  lastButtonState = buttonState;
}

void handleOverrideButtonPress() {
  int analogValue = analogRead(overridePin);  // read button 
  float voltageOverride = analogValue * (5.0 / 1023.0);  // convert into voltage

  if (voltageOverride > 1) {
    overrideState = 1;
  } else {
    overrideState = 0;
  }
  
  if (overrideState != lastOverrideState) {
    manual = !manual;

    if (!turnOn) { // if schedule is off
      if (manual) { // but button is pressed, then turn on
        digitalWrite(ledPin, HIGH);
        analogWrite(relayPin, PWMValue);
        writeToLCD(1, "ON: Manual");
      } else { // if schedule is off, and button is not pressed anymore, then turn off
        digitalWrite(ledPin, LOW);
        digitalWrite(relayPin, LOW);
        writeToLCD(1, "OFF");
      }
    }
  }

  lastOverrideState = overrideState;
}

// GPS availability control
bool checkGPSFix()
{
  handleButtonPress(); // It checks if button was pushed while checking for GPS
  handleOverrideButtonPress(); // It checks if override button was pushed while checking for GPS
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

// GPS global control
void initializeGPS() {
  digitalWrite(ledPin, LOW);
  digitalWrite(relayPin, LOW);

  lcd.backlight(); // Turn on backlight to see initialization messages

  writeToLCD(0, "Searching GPS");
  writeToLCD(1, "");
  while (!checkGPSFix()) {
    ;
  }
  writeToLCD(0, "GPS OK!");
  writeToLCD(1, "");

  delay(500);

  // Keep the backlight in the last state based on the button. If it was never pushed, the backlight will turn off by default
  if (backlightState) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}

void setup() {
  Serial.begin(9600);
  neogps.begin(GPSBaud);
  lcd.init();
  
  pinMode(buttonPin, INPUT_PULLUP);  // To set button as input
  analogWrite(buttonPin, LOW);  // No pressed as default

  pinMode(auxOverridePin, OUTPUT);
  digitalWrite(auxOverridePin, HIGH); // Sending voltage
  pinMode(overridePin, INPUT_PULLUP); // To set override button as input
  analogWrite(overridePin, LOW);  // No pressed as default

  pinMode(relayPin, OUTPUT); // To set relay as output
  digitalWrite(relayPin, LOW); // Turn off by default

  pinMode(ledPin, OUTPUT); // To set led as output
  digitalWrite(ledPin, LOW); // Turn off by default

  initializeGPS(); // Call to GPS init function
}

void loop() {

  handleButtonPress(); // Call to button press control
  handleOverrideButtonPress(); // Call to override button press control

  while (neogps.available() > 0) {
    gps.encode(neogps.read());

    if (gps.time.isUpdated()) {
      // UTC current Time 
      int month = gps.date.month();
      int day = gps.date.day();
      int hour = gps.time.hour(); 
      int minute = gps.time.minute();      
      int second = gps.time.second();      

      // Adjust hour based on time zone offset
      hour = (hour + offset) % 24;
      if (hour < 0) {
        hour += 24;
      }

      // Check if the current time is within any turn-on window
      activeSchedule = -1;
      turnOn = false;

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

      if ((month == 12 && day == 25 && hour == 0 && minute == 0) || // Dec 25th
          (month == 1 && day == 1 && hour == 0 && minute < 2)) { // Jan 1st
        turnOn = true;
        activeSchedule = (month == 12) ? 4 : 5; // 4 for Dec 25th, 5 for Jan 1st
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
        analogWrite(relayPin, PWMValue);
        writeToLCD(1, "ON: " + String(activeSchedule));
      } else if (!manual) { // if button is not pressed and schedule finished, then turn off
        digitalWrite(ledPin, LOW);
        digitalWrite(relayPin, LOW);
        writeToLCD(1, "OFF");
      }
    }
    lastTime = millis();
  }

  // If lose the GPS data for certain period of time, it goes to the GPS global control function
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) { // set to 1 second -> 1000 ms
    initializeGPS(); // Call to GPS initialization global function
  }
}