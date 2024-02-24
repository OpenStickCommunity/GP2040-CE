#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_

#include "AnimationStation.hpp"

class AnimationStorage
{
  public:
    AnimationStorage(const AnimationStorage&) = delete;
    AnimationStorage& operator=(const AnimationStorage&) = delete;

    static AnimationStorage& getInstance() {
      static AnimationStorage animationStore;
      return animationStore;
    }

    void save();
    AnimationOptions getAnimationOptions();
  private:
    AnimationStorage() = default;
    ~AnimationStorage() = default;
};

#endif
