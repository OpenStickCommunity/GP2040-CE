#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "pixel.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <vector>
#include "NeoPico.h"
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

// Also defined in Enums.proto
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
constexpr RGB ColorIndigo(75, 0, 130);
constexpr RGB ColorViolet(238, 130, 238);

inline const std::vector<RGB> colors {
    ColorBlack,     ColorWhite,  ColorRed,     ColorOrange, ColorYellow,
    ColorLimeGreen, ColorGreen,  ColorSeafoam, ColorAqua,   ColorSkyBlue,
    ColorBlue,      ColorPurple, ColorPink,    ColorMagenta, ColorIndigo,
    ColorViolet
};

class Animation {
public:
  Animation(PixelMatrix &matrix);
  virtual void UpdatePixels(std::vector<Pixel> pixels);
  void ClearPixels();
  virtual ~Animation(){};

  static LEDFormat format;

  bool notInFilter(Pixel pixel);
  virtual bool Animate(RGB (&frame)[100]) = 0;
  void UpdateTime();
  void UpdatePresses(RGB (&frame)[100]);
  void DecrementFadeCounter(int32_t index);

  virtual void ParameterUp() = 0;
  virtual void ParameterDown() = 0;

  virtual void FadeTimeUp();
  virtual void FadeTimeDown();

  RGB BlendColor(RGB start, RGB end, uint32_t frame);

protected:
/* We track both the full matrix as well as individual pixels here to support
button press changes. Rather than adjusting the matrix to represent a subset of pixels,
we provide a subset of pixels to use as a filter. */
  PixelMatrix *matrix;
  std::vector<Pixel> pixels;
  bool filtered = false;

  // Color fade 
  RGB defaultColor = ColorBlack;  
  static std::map<uint32_t, int32_t> times;
  static std::map<uint32_t, RGB> hitColor;    
  absolute_time_t lastUpdateTime = nil_time;
  uint32_t coolDownTimeInMs = 1000;
  int64_t updateTimeInMs = 20;

};

#endif
