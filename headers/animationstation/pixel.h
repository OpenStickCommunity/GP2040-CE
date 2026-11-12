#ifndef PIXEL_HPP_
#define PIXEL_HPP_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "pico.h" // NUM_BANK0_GPIOS
#include "enums.pb.h"

// Hall Effect sensors that can drive a light; matches ext*StaticColors max_size in config.proto
#define MAX_EXT_INPUT_LIGHT_COLOR_INDEXES 32

// GPIO keys equal the pin number
inline int32_t MakeLightInputKey(LightInputSource source, uint8_t index) { return ((int32_t)source << 8) | index; }

// Board config LIGHT_DATA_* entries: firstLedIndex, ledCount, x, y, GPIO pin / case index / HE sensor index, type, input source
#define LIGHT_DATA_ENTRY_SIZE 7

constexpr LightInputSource LightDataInputSource(uint8_t type, uint8_t source)
{
  return (type == LightType::LightType_ActionButton && source == LightInputSource::LightInputSource_HallEffect)
    ? LightInputSource::LightInputSource_HallEffect : LightInputSource::LightInputSource_GPIO;
}

inline bool IsValidLightInput(LightInputSource source, uint32_t index)
{
  switch(source)
  {
    case LightInputSource::LightInputSource_GPIO:       return index < NUM_BANK0_GPIOS;
    case LightInputSource::LightInputSource_HallEffect: return index < MAX_EXT_INPUT_LIGHT_COLOR_INDEXES;
    default:                                            return false;
  }
}

struct Pixel {
  Pixel(int index, uint32_t mask = 0) : index(index), mask(mask) { }
  Pixel(int index, std::vector<uint8_t> positions) : index(index), positions(positions) { }
  Pixel(int index, uint32_t mask, std::vector<uint8_t> positions) : index(index), mask(mask), positions(positions) { }

  int index;                      // The pixel index
  uint32_t mask;                  // Used to detect per-pixel lighting
  std::vector<uint8_t> positions; // The actual LED indexes on the chain
};

inline const Pixel NO_PIXEL(-1);

inline bool operator==(const Pixel &lhs, const Pixel &rhs) {
  return lhs.index == rhs.index;
}

// Enums //////////////////////////////////////////////////////////////////////////

// Structs //////////////////////////////////////////////////////////////////////////

//Grid position of a single RGB Light
struct LightPosition
{
  LightPosition() {}

  LightPosition(uint32_t xCoord, uint32_t yCoord)
  {
    XPosition = xCoord;
    YPosition = yCoord;
  }
  
  int XPosition = 0;
  int YPosition = 0;
};

//A single RGB light on the device. Replaced Pixel
struct Light 
{
  Light(uint8_t InFirstLedIndex, uint8_t InNumLedsPerLight, LightPosition InPosition, uint8_t InGPIOPinOrNonButtonIndex, LightType InType, LightInputSource InSource = LightInputSource::LightInputSource_GPIO)
  {
    FirstLedIndex = InFirstLedIndex;
    Position = InPosition;
    Type = InType;
    LedsPerLight = InNumLedsPerLight;
    //GamePadMask = GamePadMask;
    if(InType == LightType::LightType_Case)
      NonButtonIndex = InGPIOPinOrNonButtonIndex;
    else if(InType == LightType::LightType_Player1Light)
    {
      NonButtonIndex = InGPIOPinOrNonButtonIndex;
      PlayerLightIndex = 0;
    }
    else if(InType == LightType::LightType_Player2Light)
    {
      NonButtonIndex = InGPIOPinOrNonButtonIndex;
      PlayerLightIndex = 1;
    }
    else if(InType == LightType::LightType_Player3Light)
    {
      NonButtonIndex = InGPIOPinOrNonButtonIndex;
      PlayerLightIndex = 2;
    }
    else if(InType == LightType::LightType_Player4Light)
    {
      NonButtonIndex = InGPIOPinOrNonButtonIndex;
      PlayerLightIndex = 3;
    }
    else if(InType == LightType::LightType_ActionButton || InType == LightType::LightType_Turbo)
    {
      if(InType == LightType::LightType_Turbo)
        InSource = LightInputSource::LightInputSource_GPIO;

      if(IsValidLightInput(InSource, InGPIOPinOrNonButtonIndex))
      {
        InputSource = InSource;
        InputIndex = InGPIOPinOrNonButtonIndex;
        InputKey = MakeLightInputKey(InSource, InGPIOPinOrNonButtonIndex);
      }
    }
  }

  // index of first LED
  uint32_t FirstLedIndex; 

  // Approximate grid position of Light on the device
  LightPosition Position; 

  // Type of light, used in animations to allow users to seperate off lights for different anims
  LightType Type; 

  //How many leds make up this light.
  uint8_t LedsPerLight;

  LightInputSource InputSource = LightInputSource::LightInputSource_GPIO;
  uint8_t InputIndex = 0;

  //Game pad mask (if applicaple) (Needed to do SOCD on Lights)
 // uint32_t GamePadMask;

  //-1 if unbound
  int32_t InputKey = -1;

  //Index into NonButtonIndex array in a led profile
  int32_t NonButtonIndex = -1;

  //Player ID
  int32_t PlayerLightIndex = -1;
};

//All RGB lights on the device. Replaced PixelMatrix
struct Lights
{
public:
  Lights() {}

  void Setup(std::vector<Light> InLights)
  {
    AllLights.clear();
    AllLights = InLights;
  }

  inline uint8_t GetLedCount() const
  {
    int highestLedSoFar = 0;
    for(const Light& thisLight : AllLights )
    {
      int ledIndexValue = (int)thisLight.FirstLedIndex + (int)thisLight.LedsPerLight;
      if(ledIndexValue > highestLedSoFar)
        highestLedSoFar = ledIndexValue;
    }
    return highestLedSoFar;
  }

  inline uint16_t GetLightsCount() const
  {
    return AllLights.size();
  }

  //Array of all the lights
  std::vector<Light> AllLights;
};

#endif
