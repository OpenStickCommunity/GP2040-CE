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

#include "enums.pb.h"

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

#define MAX_CUSTOM_COLORS 16
inline std::vector<RGB> customColors { };

typedef enum
{
  BUTTONCASELIGHTTYPE_BUTTON_ONLY,
  BUTTONCASELIGHTTYPE_CASE_ONLY,
  BUTTONCASELIGHTTYPE_BUTTON_AND_CASE,
} EButtonCaseEffectType;

class Animation {
public:
  Animation(Lights& InRGBLights, EButtonCaseEffectType InButtonCaseEffectType);
  virtual ~Animation(){};

  //Which buttons are held at the moment
  virtual void UpdatePressed(std::vector<int32_t> InPressedPins);
  void ClearPressed();

  static LEDFormat format;

  virtual void Animate(RGB (&frame)[FRAME_MAX]) = 0;
  
  //param adjustment
  virtual void CycleParameterChange() {};

  virtual void SetOptionalParams(uint32_t OptionalParams) {};

  virtual bool IsFinished() { return false; } //ready for delete? Only applicable to special move anims really

protected:

  //gets current frame time
  void UpdateTime();

  //Update timers for pressed buttons this frame
  void UpdatePresses();
  void DecrementFadeCounters();

  //notifies
  virtual void NewPressForPin(int lightIndex) {};

  RGB BlendColor(RGB start, RGB end, float alpha);
  RGB FadeColor(RGB start, RGB end, uint32_t TimeLeft);

  virtual int32_t GetFadeTime();

  //Type Helpers
  bool LightTypeIsForAnimation(LightType Type);

  //Get color helpers
  virtual RGB GetNonPressedColorForLight(uint32_t LightIndex);
  virtual RGB GetPressedColorForLight(uint32_t LightIndex);
  virtual RGB GetColorForIndex(uint32_t ColorIndex);

  //Light data
  Lights* RGBLights;

  //Is this running as a button pressed animation
  bool isButtonAnimation = false;

  //Pins currently pressed
  std::vector<int32_t> pressedPins;

  // Color fade 
  std::vector<int32_t> fadeTimes;

  absolute_time_t lastUpdateTime = nil_time;

  uint32_t holdTimeInMs = 1000;
  uint32_t fadeoutTimeInMs = 1000;

  int64_t updateTimeInMs = 20;

  EButtonCaseEffectType ButtonCaseEffectType = EButtonCaseEffectType::BUTTONCASELIGHTTYPE_BUTTON_AND_CASE;

  int16_t cycleTime = -1;
};

#endif
