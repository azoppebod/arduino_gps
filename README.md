# Automatic Offline Light Control Based on GPS Time

## Overview

This Arduino project controls the turning on and off of a device, such as a light, at multiple times during the day based on the time retrieved from a GPS module (offline, no internet required). The project uses an Arduino Uno and a NEO-6M GPS module to get the current UTC time, converts it to the local time according to the specified time zone offset, and controls a relay to turn the device on or off based on predefined time windows.

## Features

- **Automatic Light Control**: The relay is turned on or off at specific times of the day based on the GPS time.
- **Multiple Time Windows**: Supports multiple time windows for controlling the device.
- **Time Zone Adjustment**: Adjusts the UTC time retrieved from the GPS module to the local time using a configurable time zone offset.
- **Serial Output for Debugging**: Prints the current time and relay state to the serial monitor for easy debugging.

## Hardware Requirements

- Arduino Uno
- NEO-6M GPS Module
- Relay Module
- Connecting wires

## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)
- [TinyGPS++ Library](https://github.com/mikalhart/TinyGPSPlus)
- [NeoSWSerial Library](https://github.com/SlashDevin/NeoSWSerial)

## Circuit Connections

- **GPS Module**:
  - TX (GPS) → RX (Arduino Pin 4)
  - RX (GPS) → TX (Arduino Pin 3)
  - VCC → 5V
  - GND → GND

- **Relay Module**:
  - IN (Relay) → Pin 7 (Arduino)
  - VCC → 5V
  - GND → GND

## Installation

1. Clone the repository or download the `.zip` file.
   ```bash
   git clone https://github.com/yourusername/automatic-light-control.git
2. Open the project in the Arduino IDE.

3. Install the required libraries:

- TinyGPS++
- NeoSWSerial
You can install these libraries via the Arduino Library Manager (Sketch > Include Library > Manage Libraries...).

4. Adjust the time zone offset according to your location in the code:
   ```bash
   const int offset = -3; // Example for UTC-3
5. Upload the code to your Arduino Uno.

## How It Works
1. The Arduino continuously reads the time from the GPS module.
2. The UTC time from the GPS is adjusted according to the offset to reflect the local time.
3. The code checks if the current time falls within any of the predefined time windows.
4. If the current time is within a time window, the relay is turned on; otherwise, it is turned off.
5. The current time and the relay state are printed to the serial monitor for debugging purposes.

## Customization
- Time Windows: You can define up to three time windows for controlling the relay by adjusting the startHour, startMinute, startSecond, endHour, endMinute, and endSecond variables.
   ```bash
   int startHour1 = 16, startMinute1 = 0, startSecond1 = 0;
   int endHour1 = 18, endMinute1 = 0, endSecond1 = 30;
- Time Zone Offset: Adjust the offset variable to set your local time zone.

## Example Output
Here is an example of the serial output:

**Time: 18:10:15
TurnOn: 1 | Relay set to HIGH - On**

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Author
Andrés Zoppelletto - azoppe

## Acknowledgments
Libraries used: TinyGPS++, NeoSWSerial
