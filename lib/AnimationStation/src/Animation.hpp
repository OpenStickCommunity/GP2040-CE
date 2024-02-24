#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "Pixel.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <vector>
#include "NeoPico.hpp"
#include <map>

struct RGB {
  // defaults allows trivial constructor, avoiding compiler complaints and avoiding unnessecary initialization
  // animation always memsets the frame before use, to this is safe.
  RGB() = default; 
  RGB(const RGB&) = default;


  constexpr RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), w(0) {}

  constexpr RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    : r(r), g(g), b(b), w(w) { }

  RGB(uint32_t c)
    : r((c >> 16) & 255), g((c >> 8) & 255), b((c >> 0) & 255) { }

  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;

  inline static RGB wheel(uint8_t pos) {
    pos = 255 - pos;
    if (pos < 85) {
      return RGB(255 - pos * 3, 0, pos * 3);
    } else if (pos < 170) {
      pos -= 85;
      return RGB(0, pos * 3, 255 - pos * 3);
    } else {
      pos -= 170;
      return RGB(pos * 3, 255 - pos * 3, 0);
    }
  }

  inline uint32_t value(LEDFormat format, float brightnessX = 1.0F) const {
    switch (format) {
      case LED_FORMAT_GRB:
        return ((uint32_t)(g * brightnessX) << 16)
            | ((uint32_t)(r * brightnessX) << 8)
            | (uint32_t)(b * brightnessX);

      case LED_FORMAT_RGB:
        return ((uint32_t)(r * brightnessX) << 16)
            | ((uint32_t)(g * brightnessX) << 8)
            | (uint32_t)(b * brightnessX);

      case LED_FORMAT_GRBW:
      {
        if ((r == g) && (r == b))
          return (uint32_t)(r * brightnessX);

        return ((uint32_t)(g * brightnessX) << 24)
            | ((uint32_t)(r * brightnessX) << 16)
            | ((uint32_t)(b * brightnessX) << 8)
            | (uint32_t)(w * brightnessX);
      }

      case LED_FORMAT_RGBW:
      {
        if ((r == g) && (r == b))
          return (uint32_t)(r * brightnessX);

        return ((uint32_t)(r * brightnessX) << 24)
            | ((uint32_t)(g * brightnessX) << 16)
            | ((uint32_t)(b * brightnessX) << 8)
            | (uint32_t)(w * brightnessX);
      }
    }

    assert(false);
    return 0;
  }
};

constexpr RGB ColorBlack(0, 0, 0);
constexpr RGB ColorWhite(255, 255, 255);
constexpr RGB ColorRed(255, 0, 0);
constexpr RGB ColorOrange(255, 128, 0);
constexpr RGB ColorYellow(255, 255, 0);
constexpr RGB ColorLimeGreen(128, 255, 0);
constexpr RGB ColorGreen(0, 255, 0);
constexpr RGB ColorSeafoam(0, 255, 128);
constexpr RGB ColorAqua(0, 255, 255);
constexpr RGB ColorSkyBlue(0, 128, 255);
constexpr RGB ColorBlue(0, 0, 255);
constexpr RGB ColorPurple(128, 0, 255);
constexpr RGB ColorPink(255, 0, 255);
constexpr RGB ColorMagenta(255, 0, 128);

inline const std::vector<RGB> colors {
    ColorBlack,     ColorWhite,  ColorRed,     ColorOrange, ColorYellow,
    ColorLimeGreen, ColorGreen,  ColorSeafoam, ColorAqua,   ColorSkyBlue,
    ColorBlue,      ColorPurple, ColorPink,    ColorMagenta };

struct LedRGBState {
    RGB HitColor;
    int32_t Time;
};

class Animation {
public:
  Animation(PixelMatrix &inMatrix);
  virtual void UpdatePixels(const std::vector<Pixel>& pixels);
  void ClearPixels();
  virtual ~Animation(){};

  static LEDFormat format;

  bool notInFilter(const Pixel& pixel);
  virtual void Animate(RGB (&frame)[100]) = 0;
  void UpdateTime();
  void UpdatePresses(RGB (&frame)[100]);
  void DecrementFadeCounter(int32_t index);

  virtual void ParameterUp() = 0;
  virtual void ParameterDown() = 0;

  virtual void FadeTimeUp();
  virtual void FadeTimeDown();

  virtual const LedRGBState& GetLedRGBStateAtIndex(uint32_t index);

  RGB BlendColor(const RGB& start, const RGB& end, uint32_t frame);

protected:
/* We track both the full matrix as well as individual pixels here to support
button press changes. Rather than adjusting the matrix to represent a subset of pixels,
we provide a subset of pixels to use as a filter. */
  PixelMatrix *matrix;
  std::vector<Pixel> pixels;
  bool filtered = false;

  // Color fade 
  RGB defaultColor = ColorBlack;  
  std::map<uint32_t, LedRGBState> ledRGBStates;
  absolute_time_t lastUpdateTime = nil_time;
  uint32_t coolDownTimeInMs = 1000;
  int64_t updateTimeInMs = 20;

};

#endif
