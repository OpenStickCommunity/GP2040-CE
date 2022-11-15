# GP2040 FAQ

## General Questions

### Which input mode should I use?

Generally speaking, XInput will be the mode of choice for everything except Nintendo Switch and PlayStation 3. XInput mode is the most fully-featured, has the best compatibility with PC games and is compatible with console adapters like the Brook Wingman product line.

### What is the extent of PS4 support in GP2040?

GP2040 will work on PS4 games that implement support for legacy PS3 controllers. Many of the popular PS4 fighting games have this support.

### Does/can/will GP2040 natively support the PS4, PS5, Xbox One or Xbox Series consoles?

These consoles implement security to prevent unauthorized accessories from being used. The process of cracking or bypassing that security may not be legal everywhere. These consoles could be supported in the future if a user-friendly and completely legal implementation method is found.

### Can I use multiple controllers on the same system?

Yes! Each board with GP2040 is treated as a separate controller. The one thing to keep in mind would be to only run the web configurator for one controller at a time.

### Does GP2040 really have less than 1 ms of latency?

Yes...if you're platform supports 1000 Hz USB polling. GP2040 is configured for 1000 Hz / 1 ms polling, however some systems override or ignore the polling rate the controller requests. PC and MiSTer are confirmed to work with 1000 Hz polling. Even if your system doesn't support a USB polling rate that high, you can feel comfortable knowing GP2040 is still reading and processing your inputs as fast as the target system will allow.

### Do the additional features like RGB LEDs, Player LEDs and OLED displays affect performance?

No! The RP2040 chip contains two processing cores. GP2040 dedicates one core to reading inputs and sending them via USB, while the second core is used to handle any auxiliary modules like LEDs and display support. No matter how crazy the feature set of GP2040 becomes, it's unlikely your controller's input latency will be affected.

### Why do the buttons have weird labels like B3, A1, S2, etc.?

GP2040 uses a generic system for handling button inputs that most closely maps to a traditional PlayStation controller layout with a few extra buttons. This means 4 face buttons (B1-B4), 4 shoulder buttons (L1, L2, R1, R2), Select and Start (S1, S2), 2 stick buttons (L3, R3) and 2 auxiliary buttons for things like Home and Capture (A1, A2) on the Switch. The GP2040 documentation and web configurator have a dropdown to change the labels to more familiar controller layouts. You can also refer to the button mapping table on the [GP2040 Usage](https://www.gp2040-CE.info/#/usage?id=buttons) page.

## Technical Questions

### Why use PlatformIO instead of \<insert favorite project setup\>?

Setting up a development environment to build Pico SDK projects is a manual process which requires several components to be installed and configured. Using PlatformIO allows easy installation and updating of build and project dependencies, and makes for a less confusing experience for new developers and people that just want to make a few tweaks for a custom build.

### What kind of voodoo is that built-in web configurator?

There's no magic here, just a few cool libraries working together:

* Single page application using React and Bootstrap is embedded in the GP2040 firmware
* TinyUSB library provides virtual network connection via RNDIS
* lwIP library provides an HTTP server which serves up the embedded React app and the web configuration API
* ArduinoJson library is used for serialization and deserialization of web API requests
