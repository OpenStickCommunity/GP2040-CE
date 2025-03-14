#ifndef _ANIMATION_STORAGE_H_
#define _ANIMATION_STORAGE_H_

#include "AnimationStation.h"

class AnimationStorage
{
  public:
    void save();
    AnimationOptions getAnimationOptions();
};

static AnimationStorage AnimationStore;

#endif
