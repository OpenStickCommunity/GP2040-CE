#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_

#include "Animation.hpp"
#include "AnimationStation.hpp"

class AnimationStorage
{
  public:
    void setup(AnimationStation *as);
    void save();

    AnimationOptions getAnimationOptions();
    void setAnimationOptions(AnimationOptions options);
  protected:
    AnimationStation *as;
};

static AnimationStorage AnimationStore;

#endif
