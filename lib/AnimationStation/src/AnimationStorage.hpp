#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_

#include "Animation.hpp"
#include "AnimationStation.hpp"

class AnimationStorage
{
  public:
    void setup(AnimationStation *as);
    void save();

    uint8_t getMode();
    void setMode(uint8_t mode);
    uint8_t getBrightness();
    void setBrightness(uint8_t brightness);
  protected:
    AnimationStation *as;
};

static AnimationStorage AnimationStore;

#endif
