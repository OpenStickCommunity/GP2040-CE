#include "playerleds.h"
#include "pico/stdlib.h"
#include <hardware/pwm.h>

void PlayerLEDs::animate(PLEDAnimationState animationState)
{
	// Reset state and bypass timer check if animation changed
	if (animationState.animation != selectedAnimation)
	{
		reset();
		selectedAnimation = animationState.animation;
	}
	else if (!time_reached(nextAnimationTime))
	{
		return;
	}

	parseState(animationState.state);

	switch (selectedAnimation)
	{
		case PLED_ANIM_BLINK:
			handleBlink(animationState.speed);
			break;

		case PLED_ANIM_CYCLE:
			handleCycle(animationState.speed);
			break;

		case PLED_ANIM_FADE:
			handleFade();
			break;

		case PLED_ANIM_BLINK_CUSTOM:
			handleBlinkCustom(animationState.speedOn, animationState.speedOff);
			break;

		default:
			break;
	}

	for (int i = 0; i < PLED_COUNT; i++)
		ledLevels[i] = PLED_MAX_LEVEL - (currentPledState[i] ? (brightness * brightness) : 0);
}
