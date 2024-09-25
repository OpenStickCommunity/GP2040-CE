#ifndef PLAYER_LEDS_H_
#define PLAYER_LEDS_H_

#include <cstring>
#include <cstdio>
#include <stdint.h>

#include "pico/time.h"

#define PLED_COUNT 4
#define PLED_MAX_BRIGHTNESS 0xFF
#define PLED_MAX_LEVEL 0xFFFF

typedef enum
{
	PLED_STATE_LED1 = (1 << 0),
	PLED_STATE_LED2 = (1 << 1),
	PLED_STATE_LED3 = (1 << 2),
	PLED_STATE_LED4 = (1 << 3),
} PLEDStateMask;

typedef enum
{
	PLED_ANIM_NONE,
	PLED_ANIM_OFF,
	PLED_ANIM_SOLID,
	PLED_ANIM_BLINK,
	PLED_ANIM_CYCLE,
	PLED_ANIM_FADE,
	PLED_ANIM_BLINK_CUSTOM,
} PLEDAnimationType;

const PLEDAnimationType ANIMATION_TYPES[] =
{
	PLED_ANIM_NONE,
	PLED_ANIM_OFF,
	PLED_ANIM_SOLID,
	PLED_ANIM_BLINK,
	PLED_ANIM_CYCLE,
	PLED_ANIM_FADE,
	PLED_ANIM_BLINK_CUSTOM,
};

typedef enum
{
	PLED_SPEED_OFF        = 0,
	PLED_SPEED_PLAID      = 10,
	PLED_SPEED_LUDICROUS  = 20,
	PLED_SPEED_RIDICULOUS = 50,
	PLED_SPEED_FASTER     = 100,
	PLED_SPEED_FAST       = 250,
	PLED_SPEED_NORMAL     = 500,
	PLED_SPEED_SLOW       = 1000,
} PLEDAnimationSpeed;

const PLEDAnimationSpeed ANIMATION_SPEEDS[] =
{
	PLED_SPEED_OFF,
	PLED_SPEED_PLAID,
	PLED_SPEED_LUDICROUS,
	PLED_SPEED_RIDICULOUS,
	PLED_SPEED_FASTER,
	PLED_SPEED_FAST,
	PLED_SPEED_NORMAL,
	PLED_SPEED_SLOW,
};

struct PLEDAnimationState
{
	uint8_t state = 0;
	PLEDAnimationType animation;
	PLEDAnimationSpeed speed;
	uint32_t speedOn = 0;
	uint32_t speedOff = 0;
};

class PlayerLEDs
{
	public:
		virtual void setup() = 0;
		virtual void display() = 0;
		void animate(PLEDAnimationState animationState);

	protected:
		void parseState(uint8_t state)
		{
			memcpy(lastPledState, currentPledState, sizeof(currentPledState));
			for (int i = 0; i < PLED_COUNT; i++)
				currentPledState[i] = (state & (1 << i)) == (1 << i);
		}

		inline void reset()
		{
			memset(lastPledState, 0, sizeof(lastPledState));
			memset(currentPledState, 0, sizeof(currentPledState));
			nextAnimationTime = get_absolute_time();
			brightness = PLED_MAX_BRIGHTNESS;
			fadeIn = false;
		}

		inline void handleBlink(PLEDAnimationSpeed speed)
		{
			for (int i = 0; i < PLED_COUNT; i++)
			{
				if (lastPledState[i])
					currentPledState[i] = false;
			}
			nextAnimationTime = make_timeout_time_ms(speed);
		}

		inline void handleBlinkCustom(uint32_t speed, uint32_t speedOff)
		{
			uint32_t nextSpeed = 0;
			for (int i = 0; i < PLED_COUNT; i++)
			{
				if (speed > 0 && speedOff == 0) {
					if (lastPledState[i]) {
						currentPledState[i] = true;
						nextSpeed = speed;
					}
				} else {
					if (lastPledState[i]) {
						currentPledState[i] = false;
						nextSpeed = speed;
					} else {
						currentPledState[i] = true;
						nextSpeed = speedOff;
					}
				}
			}
			nextAnimationTime = make_timeout_time_ms(nextSpeed);
		}

		inline void handleCycle(PLEDAnimationSpeed speed)
		{
			for (int i = 0; i < PLED_COUNT; i++)
			{
				if (lastPledState[i] != 0)
				{
					memset(currentPledState, 0, sizeof(currentPledState));
					for (int j = 0; j < PLED_COUNT; j++)
					{
						if (lastPledState[j])
						{
							currentPledState[(j + 1) % PLED_COUNT] = true;
							break;
						}
					}
					break;
				}
			}
			nextAnimationTime = make_timeout_time_ms(speed);
		}

		inline void handleFade()
		{
			if (fadeIn)
			{
				brightness += 5;
				if (brightness == PLED_MAX_BRIGHTNESS)
					fadeIn = false;
			}
			else
			{
				brightness -= 5;
				if (brightness == 0)
					fadeIn = true;
			}

			nextAnimationTime = make_timeout_time_ms(PLED_SPEED_LUDICROUS);
		}

		uint16_t ledLevels[PLED_COUNT] = {PLED_MAX_LEVEL, PLED_MAX_LEVEL, PLED_MAX_LEVEL, PLED_MAX_LEVEL};
		absolute_time_t nextAnimationTime;
		PLEDAnimationType selectedAnimation = PLED_ANIM_NONE;
		bool lastPledState[PLED_COUNT] = { };
		bool currentPledState[PLED_COUNT] = { };
		uint8_t brightness = PLED_MAX_BRIGHTNESS;
		bool fadeIn = false;
};

#endif
