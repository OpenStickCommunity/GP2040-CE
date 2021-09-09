#ifndef _CONFIG_STORAGE_H_
#define _CONFIG_STORAGE_H_

#include "Animation.hpp"
#include "AnimationStation.hpp"
#include "Effects/StaticColor.hpp"

class AnimationStorage
{
	public:
		void setup();
		void save(AnimationStation as);

		AnimationMode getBaseAnimation();
		void setBaseAnimation(AnimationMode mode);
		uint8_t getBrightness();
		void setBrightness(uint8_t brightness);
};

static AnimationStorage AnimationStore;

#endif
