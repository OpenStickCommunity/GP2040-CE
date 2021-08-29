#ifndef _NEO_PICO_H_
#define _NEO_PICO_H_ 

#include "ws2812.pio.h"

class NeoPico
{
public:
  NeoPico(int ledPin, int numPixels);
  void Show();
  void Clear();
  void SetPixel(int pixel, uint32_t color);
  uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);
private:
  void PutPixel(uint32_t pixel_grb);
  PIO pio = pio0;
  int numPixels = 0;
  uint8_t frame[100];
};


#endif