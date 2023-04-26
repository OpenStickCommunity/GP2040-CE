# GP2040-CE FAQ

## General Questions

### Which input mode should I use?

Generally speaking, XInput will be the mode of choice for everything except Nintendo Switch and PlayStation 3. XInput mode is the most fully-featured, has the best compatibility with PC games and is compatible with console adapters like the Brook Wingman product line.

### What is the extent of PS4 support in GP2040-CE?

GP2040-CE will work on PS4 games that implement support for legacy PS3 controllers. Many of the popular PS4 fighting games have this support.

### Will GP2040-CE natively support the PS4, PS5, Xbox One or Xbox Series consoles?

These consoles implement security measures that prevent unauthorized accessories from being used. The process of cracking or bypassing that security may not be legal everywhere. These consoles could be supported in the future if a user-friendly and completely legal implementation method is found.

### Does GP2040-CE support wireless connectivity?

Not yet! But the team is working on a proof of concept for the Raspberry Pi Pico W.  We cannot provice any timeline on when support will be implimented into the firmware, or any info regarding latency.  Plese also note that the normal Pico configuration will not work on the Pico W.

### Can I use multiple controllers on the same system?

Yes! Each GP2040-CE device is treated as a separate controller. However, you will only be able to run the embedded web configurator on one device at a time.

### Does GP2040-CE really have less than 1 ms of latency?

Yes! If your platform supports 1000 Hz USB polling, input latency will be less than 1ms. GP2040-CE is configured for 1000 Hz / 1 ms polling by default in all modes, however some systems override or ignore the polling rate the controller requests. The 1000 Hz polling rate is confirmed to work on PC and MiSTer. Even if your platform doesn't support high rate USB polling, GP2040-CE is still reading and processing your inputs as fast as the target system will allow.

### Do the additional features like RGB LEDs, Player LEDs and OLED displays affect performance?

No! GP2040-CE dedicates a processing core to just reading and writing player inputs. All secondary functions such as LEDs and displays are controlled by the secondary processor core. No matter how crazy the feature set becomes, GP2040-CE is unlikely to introduce any additional input latency.

### Why do the buttons have weird labels like B3, A1, S2, etc.?

GP2040-CE uses a generic system for handling button inputs that resembles a traditional PlayStation controller layout with a few extra buttons. This means 4 face buttons (B1-B4), 4 shoulder buttons (L1, L2, R1, R2), Select and Start (S1, S2), 2 stick buttons (L3, R3) and 2 auxiliary buttons for things like Home and Capture (A1, A2) on the Switch. The GP2040-CE documentation and web configurator both provide a dropdown to change the button labels to more familiar controller layouts. You can refer to the button mapping table on the [GP2040 Usage](http://gp2040-ce.info/#/usage?id=buttons) page.

## Technical Questions

### What kind of voodoo is that built-in web configurator?

There's no magic here, just a few cool libraries working together:

* Single page application using React and Bootstrap is embedded in the GP2040 firmware
* TinyUSB library provides virtual network connection via RNDIS
* lwIP library provides an HTTP server which serves up the embedded React app and the web configuration API
* ArduinoJson library is used for serialization and deserialization of web API requests
