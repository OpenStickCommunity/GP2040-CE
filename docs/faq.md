# GP2040-CE FAQ

## General Questions

### Which input mode should I use?

This will depend on the platform you will be playing on:

* Use `XInput Mode` as the preferred mode for compatibility with PC games and 3rd party console adapters
* Use `PS4 Mode` on PS4, and on PS4 games on PS5 systems
* Use `PS3 Mode` on PS3, and on PS4 with games that support legacy controllers
* Use `Switch Mode` on Nintendo Switch
* Use `Keyboard Mode` for MAME cabinets, PC rhythm games, etc.

### Will GP2040-CE natively support PS5, Xbox One, or Xbox Series consoles?

These consoles implement security measures that prevent unauthorized accessories from being used. The process of cracking or bypassing that security may not be legal everywhere. These consoles could be supported in the future if a user-friendly and completely legal implementation method is found, such as the implementation of [PS4 Mode](add-ons/ps4-mode).

### Can I use multiple controllers with GP2040-CE on the same system?

Yes! Each GP2040-CE board is treated as a separate controller. Be sure to only run the embedded web configurator for each controller one at a time.

If you're installing boards with GP2040-CE in an arcade cabinet, check out the [Player Number add-on](add-ons/player-number.md) to force each board to a specific player number.

### Does GP2040-CE really have less than 1 ms of input latency?

Yes! If your platform supports 1000 Hz USB polling, input latency will be less than 1ms. GP2040-CE is configured for 1000 Hz / 1 ms polling by default in all modes, however some systems override or ignore the polling rate the controller requests. The 1000 Hz polling rate is confirmed to work on PC and MiSTer. Even if your platform doesn't support high rate USB polling, GP2040-CE is still reading and processing your inputs as fast as the target system will allow.

### Do the additional features like RGB LEDs, Player LEDs, and OLED displays affect performance?

Not at all! The RP2040 processor of the Pico has two cores. GP2040-CE dedicates one of these cores strictly to reading, processing and sending player inputs. All secondary functions such as LEDs and displays are run on the secondary core. No matter how crazy the feature set becomes, GP2040-CE is unlikely to introduce any additional input latency.

### Why do the buttons have weird labels like B3, A1, S2, etc.?

GP2040-CE uses a generic system for handling button inputs that resembles a traditional PlayStation controller layout with a few extra buttons. This means 4 face buttons (B1-B4), 4 shoulder buttons (L1, L2, R1, R2), Select and Start (S1, S2), 2 stick buttons (L3, R3) and 2 auxiliary buttons for things like Home and Capture (A1, A2) on the Switch. The GP2040-CE documentation and web configurator both provide a dropdown to change the button labels to more familiar controller layouts. You can refer to the button mapping table on the [GP2040-CE Usage](usage.md#buttons) page.

## Technical Questions

### What kind of voodoo is that built-in web configurator?

There's no magic here, just a few cool libraries working together:

* Single page application using React and Bootstrap is embedded in the GP2040-CE firmware
* TinyUSB library provides virtual network connection via RNDIS
* lwIP library provides an HTTP server which serves up the embedded React app and the web configuration API
* ArduinoJson library is used for serialization and deserialization of web API requests
