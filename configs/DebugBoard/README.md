# GP2040 Configuration for Debugging

![Debug setup](assets/DebugBoard.jpg)

This is my personal debugging setup. It contains:

- 2x Raspberry Pi Pico (2nd is for picoprobe debugging via a custom proto board)
- 1x [Waveshare Quad GPIO Expander](https://www.waveshare.com/pico-quad-expander.htm)
- 1x [Waveshare 1.3inch LCD Display Module](https://www.waveshare.com/pico-lcd-1.3.htm) (used for HAT switch and buttons)
- 1x [Waveshare RGB 16x10 LED Matrix](https://www.waveshare.com/pico-rgb-led.htm)

The `BoardConfig.h` file is set up to work with these modules. The four buttons on the LCD module are `B1`, `B2`, `S1`, and `S2`. The HAT switch can be clicked, and that button is bound to `R3`.
