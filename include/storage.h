#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>
#include "NeoPico.hpp"
#include "enums.h"

#define BOARD_STORAGE_INDEX 1024
#define LED_STORAGE_INDEX 1536

struct BoardOptions
{
	bool useUserDefinedPins;
	uint8_t pinDpadUp;
	uint8_t pinDpadDown;
	uint8_t pinDpadLeft;
	uint8_t pinDpadRight;
	uint8_t pinButtonB1;
	uint8_t pinButtonB2;
	uint8_t pinButtonB3;
	uint8_t pinButtonB4;
	uint8_t pinButtonL1;
	uint8_t pinButtonR1;
	uint8_t pinButtonL2;
	uint8_t pinButtonR2;
	uint8_t pinButtonS1;
	uint8_t pinButtonS2;
	uint8_t pinButtonL3;
	uint8_t pinButtonR3;
	uint8_t pinButtonA1;
	uint8_t pinButtonA2;
};

struct LEDOptions
{
	uint8_t ledBrightnessMaximum;
	uint8_t ledBrightnessSteps;
	LEDFormat ledFormat;
	LedLayout ledLayout;
	uint8_t ledsPerPixel;

	uint8_t ledDpadUp;
	uint8_t ledDpadDown;
	uint8_t ledDpadLeft;
	uint8_t ledDpadRight;
	uint8_t ledButtonB1;
	uint8_t ledButtonB2;
	uint8_t ledButtonB3;
	uint8_t ledButtonB4;
	uint8_t ledButtonL1;
	uint8_t ledButtonR1;
	uint8_t ledButtonL2;
	uint8_t ledButtonR2;
	uint8_t ledButtonS1;
	uint8_t ledButtonS2;
	uint8_t ledButtonL3;
	uint8_t ledButtonR3;
	uint8_t ledButtonA1;
	uint8_t ledButtonA2;
};

BoardOptions getBoardOptions();
void setBoardOptions(BoardOptions options);

LEDOptions getLEDOptions();
void setLEDOptions(LEDOptions options);

#endif
