#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_

#include "AnimationStation.hpp"
#include "SpecialMoveSystem.hpp"

class AnimationStorage
{
  public:
    void save();
    AnimationOptions getAnimationOptions();
    void getSpecialMoveOptions(SpecialMoveOptions& options);
};

static AnimationStorage AnimationStore;

#endif
