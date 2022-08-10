/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <string>
#include <hardware/i2c.h>
#include "OneBitDisplay.h"
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"

#ifndef DISPLAY_I2C_ADDR
#define DISPLAY_I2C_ADDR 0x3C
#endif

#ifndef DISPLAY_SIZE
#define DISPLAY_SIZE OLED_128x64
#endif

#ifndef DISPLAY_FLIP
#define DISPLAY_FLIP 0
#endif

#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 0
#endif

#ifndef DISPLAY_USEWIRE
#define DISPLAY_USEWIRE 1
#endif

// i2c Display Module
#define I2CDisplayName "I2CDisplay"

// i2C OLED Display
class I2CDisplayAddon : public GPAddon
{
public:
	virtual bool available();  // GPAddon
	virtual void setup();
	virtual void process();
	virtual std::string name() { return I2CDisplayName; }
	void clearScreen(int render); // DisplayModule
	void drawHitbox(int startX, int startY, int buttonRadius, int buttonPadding, Gamepad *gamepad);
	void drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding, Gamepad *gamepad);
	void drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding, Gamepad *gamepad);
	void drawStatusBar();

	void drawText(int startX, int startY, std::string text);
	
	void initMenu(char**);

	void setStatusBar(Gamepad *gamepad);
	uint8_t ucBackBuffer[1024];
	OBDISP obd;
	std::string statusBar;
};

#endif
