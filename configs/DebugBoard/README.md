# GP2040 Configuration for Debugging

![Debug setup](assets/DebugBoard.jpg)

Feral's debugging setup. This is what peak performance looks like.

But seriously, it contains (from left to right):

- Custom Hat #1: "Fightstick" with 17 total inputs
  - 1x [10x10x9mm 5 Way Tact Switch](https://www.amazon.com/Bestol-Direction-Switch-10109mm-Navigation/dp/B07F71N26Z/)
  - 12x [3x6x4.3mm Tact Switch](https://www.amazon.com/gp/product/B008DS188Y/)
- Custom Hat #2: Aux function module
  - 0.96" 128x32 SSD1306 I2C OLED
  - 4x Player LEDs
  - 1x DIP switch
- [Pimoroni Pico LiPo](https://shop.pimoroni.com/products/pimoroni-pico-lipo) (USB-C FTW!)
- [Waveshare RGB 16x10 LED Matrix](https://www.waveshare.com/pico-rgb-led.htm)
- Custom Hat #3: I2C expansion board
  - 256k EEPROM module
  - 2x JST-XH 4-pin connectors

All connected to a [Waveshare Quad GPIO Expander](https://www.waveshare.com/pico-quad-expander.htm). Custom hats are built on [Pimoroni Pico Proto boards](https://shop.pimoroni.com/products/pico-proto).

An [Adafruit QT Py RP2040](https://www.adafruit.com/product/4900) is flashed with a [custom Picoprobe build](assets/picoprobe_adafruit_qtpy_rp2040.uf2) and attached to the Pico LiPo's JST-SH connectors for debugging.
