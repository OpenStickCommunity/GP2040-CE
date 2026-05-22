# GP2040 Configuration for Raspberry Pi Pico 2 W

![Raspberry Pi Pico 2 W](https://assets.raspberrypi.com/static/d3014f1a0c3e8cf43e9eaed64b1e37ef/pico-2-w.png)

Basic pin setup for a stock Raspberry Pi Pico 2 W with Bluetooth support. This is a drop-in replacement for the original Pico W, featuring the RP2350 chip with improved performance.

## Features
- RP2350 dual-core Arm Cortex-M33 @ 150MHz
- 520KB SRAM
- Bluetooth 5.2 (upgraded from 5.0)
- Pin-compatible with Pico W

## Building

```bash
cmake -DGPIO_BOARD_CONFIG=Pico2W ..
make
```
