#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_

#include "AnimationStation.hpp"

class AnimationStorage
{
  public:
    void save();

    AnimationOptions getAnimationOptions();
    void setAnimationOptions(AnimationOptions options);
};

static AnimationStorage AnimationStore;

#endif
