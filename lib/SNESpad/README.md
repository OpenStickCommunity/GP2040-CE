# SNESpad for Arduino and PicoSDK

This is a library for the Raspberry Pi Pico that allows it to interface with a SNES controller. This library was adapted from the Arduino SNESpad library, with changes and optimizations made for better performance and compatibility with the Pico SDK.

## Original Library

This library is based on the [nespad](https://github.com/rahji/nespad) library developed by rahji. The original library was designed for the Arduino platform.

## Features

This library allows for the reading of button states from a SNES controller connected to a Pico or Arduino. It supports standard SNES controller, the NES controller, and the SNES Mouse. Each of these devices can be identified and read by this library.

## Usage

1. Include the SNESpad.h header in your program.
2. Create a SNESpad object, specifying the clock, latch, and data pin numbers.
3. Use the `begin()` and `start()` function to initialize the SNESpad.
4. Use the `poll()` function to update the state of the SNESpad.
5. You can then read the state of the buttons and the direction pad using the appropriate members of the SNESpad class.

## Button Variables

Here are the members of the SNESpad class that hold the state of the buttons:

- `buttonA`
- `buttonB`
- `buttonX`
- `buttonY`
- `buttonStart`
- `buttonSelect`
- `buttonL`
- `buttonR`
- `directionUp`
- `directionDown`
- `directionLeft`
- `directionRight`

For the SNES Mouse, these additional variables are available:

- `mouseX`
- `mouseY`

## Example

Here is an example of how to create a SNESpad object and read the state of the buttons:

```cpp
#include "SNESpad.h"

#define CLOCK 0
#define LATCH 1
#define DATA 2

SNESpad snespad(CLOCK, LATCH, DATA);

void setup() {
    snespad.begin();
    snespad.start();
}

void loop() {
    snespad.poll();

    if (snespad.buttonA) {
        printf("Button A is pressed.\n");
    }

    if (snespad.buttonB) {
        printf("Button B is pressed.\n");
    }

    // Add more button checks as needed.
}
```

This example will print a message to the console whenever button A or B is pressed.

## Author

This library was ported and substantially rewritten by Robert Dale Smith.

Email: <robert@robertdalesmith.com>

## Examples

Included in this repository are three examples that demonstrate different use cases for the SNESpad library.

1. [serial.ino](examples/serial/serial.ino) - Demonstrates how to use the SNESpad library to monitor a SNES controller over a USB serial connection.

2. [hid.ino](examples/hid/hid.ino) - Demonstrates how to use the SNESpad library to create HID mouse and HID joystick USB devices using a SNES controller.

3. [xinput.ino](examples/xinput/xinput.ino) - Demonstrates how to use the SNESpad library to create an XInput USB controller using a SNES controller or mouse.

## Changes and Enhancements

This version of the library is optimized for performance and compatibility. Notable changes from the original Arduino version include:

- The control polling methods have been optimized to closely match the specifications of the original console.
- Added support for the Super Nintendo mouse and the original NES controllers.

## Version History

- Version: 2.0 (2023)
    - Extended to Pico SDK (Robert Dale Smith)
    - Total refactor of class structure. (Robert Dale Smith)
    - Mouse and NES controller support (Robert Dale Smith)

Below is a history of the original SNESpad library:

- Version: 1.3 (11/12/2010) - Removed shortcut constructor which was causing issues.
- Version: 1.2 (05/25/2009) - Put pin numbers in constructor (Pascal Hahn)
- Version: 1.1 (09/22/2008) - Fixed compilation errors in Arduino 0012 (Rob Duarte)
- Version: 1.0 (09/20/2007) - Created (Rob Duarte)

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
