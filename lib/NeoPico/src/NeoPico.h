#ifndef _NEO_PICO_H_
#define _NEO_PICO_H_

#include "ws2812.pio.h"
#include <vector>

// Hard upper bound on LED chain length. All LED-related fixed-size buffers across the firmware
// (NeoPico::frame, AnimationStation::frame/linkageFrame, NeoPicoLEDAddon::frame) MUST use this
// value so they stay in lock-step. Any caller that computes an LED count from configuration
// must clamp the result to NEOPICO_MAX_LEDS before passing it into NeoPico::Setup, otherwise
// Show() / SetFrame() / button-position writes can run off the end of these arrays.
#ifndef NEOPICO_MAX_LEDS
#define NEOPICO_MAX_LEDS 100
#endif

typedef enum
{
  LED_FORMAT_GRB = 0,
  LED_FORMAT_RGB = 1,
  LED_FORMAT_GRBW = 2,
  LED_FORMAT_RGBW = 3,
} LEDFormat;

class NeoPico
{
public:
  NeoPico();
  void Setup(int ledPin, int inNumPixels, LEDFormat inFormat, PIO inPio, int inState);
  void Show();
  void Clear();
  void Off();
  LEDFormat GetFormat();
  void SetFrame(uint32_t * newFrame);
private:
  void PutPixel(uint32_t pixel_grb);
  LEDFormat format;
  PIO pio = pio1;
  int stateMachine = 0;
  int numPixels = 0;
  uint32_t frame[NEOPICO_MAX_LEDS];
};

#endif
