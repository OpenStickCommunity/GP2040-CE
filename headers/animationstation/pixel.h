#ifndef PIXEL_HPP_
#define PIXEL_HPP_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "enums.pb.h"

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
  Light(uint8_t InFirstLedIndex, uint8_t InNumLedsPerLight, LightPosition InPosition, uint8_t InGIPOPin, LightType InType)
  {
    FirstLedIndex = InFirstLedIndex;
    Position = InPosition;
    Type = InType;
    LedsPerLight = InNumLedsPerLight;
    //GamePadMask = GamePadMask;
    if(InType != LightType::LightType_Case)
      GIPOPin = InGIPOPin;
    else
      CaseLightIndex = InGIPOPin;
  }

  // index of first LED
  uint32_t FirstLedIndex; 

  // Approximate grid position of Light on the device
  LightPosition Position; 

  // Type of light, used in animations to allow users to seperate off lights for different anims
  LightType Type; 

  //How many leds make up this light.
  uint8_t LedsPerLight;

  //Game pad mask (if applicaple) (Needed to do SOCD on Lights)
 // uint32_t GamePadMask;

  //GIPO pin this action (if applicaple) is on
  int32_t GIPOPin = -1;

  //Sequential index of this case light
  int32_t CaseLightIndex = -1;
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
