This example wiring diagram is applicable to both the PS Passthrough and the Keyboard Host Configuration Add-ons as both require a USB host port to be set up and connected to the board.

![USB Host Wiring Diagram](@site/docs/assets/images/gpc-add-ons-keyboard-host-configuration-wiring-diagram.png)

* `VCC` - Connects to 5V power (Example: VBUS on the Raspberry Pi Pico)
* `D+` - Connects to the `D+` GPIO Pin above, set in the Web Configurator. (Example: GPIO0 on the Raspberry Pi Pico)
* `D-` - Connects to the `D-` GPIO Pin above, automatically set based on D+. (Example: GPIO1 on the Raspberry Pi Pico)
* `GND` - Connects to a ground pin, any `GND` pin will work. (Example: GND on the Raspberry Pi Pico)
