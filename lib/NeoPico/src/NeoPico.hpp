#ifndef _NEO_PICO_H_
#define _NEO_PICO_H_

#include "ws2812.pio.h"
#include <vector>

class NeoPico
{
public:
  NeoPico(int ledPin, int numPixels);
  void Show();
  void Clear();
  // void SetPixel(int pixel, uint32_t color);
  void SetFrame(uint32_t newFrame[100]);
private:
  void PutPixel(uint32_t pixel_grb);
  PIO pio = pio0;
  int numPixels = 0;
  uint32_t frame[100];
};

#endif
