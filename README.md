# Automatic Light Control Based on GPS Time

### Project Overview
This project is designed to automate the control of a relay (e.g., for a light) based on time, using an Arduino Uno and a NEO-6M GPS module. The system retrieves accurate time from the GPS satellites, adjusts it to the local time zone, and turns the relay on or off according to multiple pre-defined time intervals.

The use of a GPS module ensures that the system remains synchronized with the correct time, even after power loss or resets, making it perfect for time-sensitive automation tasks.

### Features
- **GPS Time Synchronization**: Retrieves accurate UTC time from GPS satellites.
- **Local Time Conversion**: Adjusts the time to the local time zone with a customizable offset.
- **Relay Control**: Automatically switches the relay on or off based on defined time intervals.
- **LCD Display**: Shows current time, satellite count, and system status on a 16x2 LCD.
- **Multiple Schedules**: Configure multiple time intervals during the day for controlling the relay.

### Hardware Requirements
- Arduino Uno
- NEO-6M GPS Module
- 16x2 LCD with I2C interface
- Relay Module
- Jumper Wires
- Breadboard (optional)

### Software Requirements
- Arduino IDE (version 1.8.19 or later)
- Libraries:
  - [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)
  - [NeoSWSerial](https://github.com/SlashDevin/NeoSWSerial)
  - [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)

### Wiring Diagram
- **GPS Module**:
  - TX (GPS) -> RX (Arduino Pin 4)
  - RX (GPS) -> TX (Arduino Pin 3)
  - VCC -> 5V
  - GND -> GND
- **LCD Display**:
  - SDA -> A4
  - SCL -> A5
  - VCC -> 5V
  - GND -> GND
- **Relay Module**:
  - IN -> Arduino Pin 7
  - VCC -> 5V
  - GND -> GND

### Installation
1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/automatic-light-control-gps.git
   cd automatic-light-control-gps
2. Open the project in the Arduino IDE:
  - Launch Arduino IDE and open the .ino file.
  - Make sure to install the necessary libraries through the Arduino Library Manager or by downloading them from GitHub.
3. Upload the code to your Arduino:
  - Select the correct board (Arduino Uno) and port.
  - Click "Upload" to transfer the sketch to the Arduino.

### Configuration
  - Time Zone Offset: Adjust the time zone by changing the offset variable in the code. For example, for UTC-3, set offset = -3;.
  - Schedules: Modify the schedules array to set different time intervals for turning the relay on and off. Each entry defines a start time and an end time in the format {hour, minute, second}.

### Usage
Once the code is uploaded, the system will start:

  - It will wait for a valid GPS signal.
  - Once the GPS time is obtained and the local time is calculated, the system will check if the current time falls within any of the predefined time windows.
  - The relay will turn on or off based on the schedule, and the current time and system status will be displayed on the LCD.

### Troubleshooting
  - GPS not connecting: Make sure the GPS module has a clear view of the sky and is properly connected.
  - LCD not displaying correctly: Verify that the correct I2C address is set in the code (usually 0x27 or 0x3F).
  - Relay not working: Check the wiring and ensure the relay is powered correctly.