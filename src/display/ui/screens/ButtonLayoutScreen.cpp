#include "ButtonLayoutScreen.h"
#include "buttonlayouts.h"

void ButtonLayoutScreen::drawScreen() {
    getRenderer()->drawRectangle(0, 0, 128, 7, displayProfileBanner, displayProfileBanner);
    getRenderer()->drawText(0, 0, header, displayProfileBanner);
    getRenderer()->drawText(0, 7, footer);
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawButtonLayoutLeft()
{
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;
    ButtonLayoutParamsLeft leftOptions = buttonLayoutCustomOptions.paramsLeft;
    return adjustByCustomSettings(GPButtonLayouts_LeftLayouts.at(leftOptions.layout)(), leftOptions.common);
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawButtonLayoutRight()
{
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;
    ButtonLayoutParamsRight rightOptions = buttonLayoutCustomOptions.paramsRight;
    return adjustByCustomSettings(GPButtonLayouts_RightLayouts.at(rightOptions.layout)(), rightOptions.common, 64);
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::adjustByCustomSettings(ButtonLayoutScreen::LayoutList layout, ButtonLayoutParamsCommon common, uint16_t originX, uint16_t originY) {
    if (layout.size() > 0) {
        int32_t startX = layout[0].parameters[0];
        int32_t startY = layout[0].parameters[1];
        int32_t customX = common.startX;
        int32_t customY = common.startY;
        int32_t offsetX = customX-startX;
        int32_t offsetY = customY-startY;
        for (uint16_t elementCtr = 0; elementCtr < layout.size(); elementCtr++) {
            if (layout[elementCtr].elementType == GP_ELEMENT_BTN_BUTTON) {
                layout[elementCtr].parameters[0] += originX+(offsetX+common.buttonPadding);
                layout[elementCtr].parameters[1] += originY+(offsetY+common.buttonPadding);
            } else {
                layout[elementCtr].parameters[0] += originX+offsetX;
                layout[elementCtr].parameters[1] += originY+offsetY;
            }

            if (((GPShape_Type)layout[elementCtr].parameters[7] == GP_SHAPE_ELLIPSE) || ((GPShape_Type)layout[elementCtr].parameters[7] == GP_SHAPE_POLYGON)) {
                // radius
                layout[elementCtr].parameters[2] = common.buttonRadius;
                layout[elementCtr].parameters[3] = common.buttonRadius;
            }
        }
    }
    return layout;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless()
{
	return BUTTON_GROUP_STICKLESS;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawWasdBox()
{
    return BUTTON_GROUP_WASD_BOX;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawUDLR()
{
    return BUTTON_GROUP_UDLR;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawArcadeStick()
{
    return BUTTON_GROUP_ARCADE_STICK;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVLXA()
{
    return BUTTON_GROUP_VLXA;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboardMirrored()
{
    return BUTTON_GROUP_FIGHTBOARD_MIRRORED;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboardStick()
{
    return BUTTON_GROUP_FIGHTBOARD_STICK;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboardStickMirrored()
{
    return BUTTON_GROUP_FIGHTBOARD_STICK_MIRRORED;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawTwinStickA()
{
    return BUTTON_GROUP_TWINSTICK_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawTwinStickB()
{
    return BUTTON_GROUP_TWINSTICK_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawMAMEA()
{
    return BUTTON_GROUP_MAME_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawOpenCore0WASDA()
{
    return BUTTON_GROUP_OPEN_CORE_WASD_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawMAMEB()
{
    return BUTTON_GROUP_MAME_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawMAME8B()
{
    return BUTTON_GROUP_MAME_8B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawOpenCore0WASDB()
{
    return BUTTON_GROUP_OPEN_CORE_WASD_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawKeyboardAngled()
{
    return BUTTON_GROUP_KEYBOARD_ANGLED;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVewlix()
{
    return BUTTON_GROUP_VEWLIX;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVLXB()
{
    return BUTTON_GROUP_VLXB;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboard()
{
    return BUTTON_GROUP_FIGHTBOARD;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVewlix7()
{
    return BUTTON_GROUP_VEWLIX7;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSega2p()
{
    return BUTTON_GROUP_SEGA_2P;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawNoir8()
{
    return BUTTON_GROUP_NOIR8;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawCapcom()
{
    return BUTTON_GROUP_CAPCOM;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawCapcom6()
{
    return BUTTON_GROUP_CAPCOM6;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawWasdButtons()
{
    return BUTTON_GROUP_WASD_BUTTONS;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawArcadeButtons()
{
    return BUTTON_GROUP_ARCADE_BUTTONS;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless13A()
{
    return BUTTON_GROUP_STICKLESS13A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons13B()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS13B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless16A()
{
    return BUTTON_GROUP_STICKLESS16A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons16B()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS16B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless14A()
{
    return BUTTON_GROUP_STICKLESS14A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons14B()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS14B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadA()
{
    return BUTTON_GROUP_DANCEPAD_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadB()
{
    return BUTTON_GROUP_DANCEPAD_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadDDRLeft()
{
    return BUTTON_GROUP_DANCEPAD_DDR_LEFT;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadDDRSolo()
{
    return BUTTON_GROUP_DANCEPAD_DDR_SOLO;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadPIULeft()
{
    return BUTTON_GROUP_DANCEPAD_PIU_LEFT;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawPopnA()
{
    return BUTTON_GROUP_POPN_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawTaikoA()
{
    return BUTTON_GROUP_TAIKO_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBMTurntableA()
{
    return BUTTON_GROUP_BM_TURNTABLE_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBM5KeyA()
{
    return BUTTON_GROUP_BM_5KEY_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBM7KeyA()
{
    return BUTTON_GROUP_BM_7KEY_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawGitadoraFretA()
{
    return BUTTON_GROUP_GITADORA_FRET_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawGitadoraStrumA()
{
    return BUTTON_GROUP_GITADORA_STRUM_A;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadDDRRight()
{
    return BUTTON_GROUP_DANCEPAD_DDR_RIGHT;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadPIURight()
{
    return BUTTON_GROUP_DANCEPAD_PIU_RIGHT;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawPopnB()
{
    return BUTTON_GROUP_POPN_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawTaikoB()
{
    return BUTTON_GROUP_TAIKO_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBMTurntableB()
{
    return BUTTON_GROUP_BM_TURNTABLE_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBM5KeyB()
{
    return BUTTON_GROUP_BM_5KEY_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBM7KeyB()
{
    return BUTTON_GROUP_BM_7KEY_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawGitadoraFretB()
{
    return BUTTON_GROUP_GITADORA_FRET_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawGitadoraStrumB()
{
    return BUTTON_GROUP_GITADORA_STRUM_B;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBlankA()
{
    return {};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBlankB()
{
    return {};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBoardDefinedA() {
#ifdef DEFAULT_BOARD_LAYOUT_A
    return DEFAULT_BOARD_LAYOUT_A;
#else
    return drawBlankA();
#endif
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBoardDefinedB() {
#ifdef DEFAULT_BOARD_LAYOUT_B
    return DEFAULT_BOARD_LAYOUT_B;
#else
    return drawBlankA();
#endif
}

GPLever* ButtonLayoutScreen::drawLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType) {
    GPLever* lever = new GPLever();
    lever->setRenderer(getRenderer());
    
    lever->setPosition(startX, startY);
    lever->setStrokeColor(strokeColor);
    lever->setFillColor(fillColor);
    lever->setRadius(sizeX);
    lever->setInputType(inputType);

    return (GPLever*)addElement(lever);
}

GPButton* ButtonLayoutScreen::drawButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask) {
    GPButton* button = new GPButton();
    button->setRenderer(getRenderer());

    button->setPosition(startX, startY);
    button->setStrokeColor(strokeColor);
    button->setFillColor(fillColor);
    button->setSizeX(sizeX);
    button->setSizeY(sizeY);
    button->setInputMask(inputMask);

    return (GPButton*)addElement(button);
}

GPShape* ButtonLayoutScreen::drawShape(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor) {
    GPShape* shape = new GPShape();
    shape->setRenderer(getRenderer());

    shape->setPosition(startX, startY);
    shape->setStrokeColor(strokeColor);
    shape->setFillColor(fillColor);
    shape->setSizeX(sizeX);
    shape->setSizeY(sizeY);

    return (GPShape*)addElement(shape);
}

GPSprite* ButtonLayoutScreen::drawSprite(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY) {
    GPSprite* sprite = new GPSprite();
    sprite->setRenderer(getRenderer());

    sprite->setPosition(startX, startY);
    sprite->setSizeX(sizeX);
    sprite->setSizeY(sizeY);

    return (GPSprite*)addElement(sprite);
}

GPWidget* ButtonLayoutScreen::drawElement(GPButtonLayout element) {
    uint16_t yOffset = 0;

    if (isInputHistoryEnabled) {
        yOffset = 5;
    }

    if (element.elementType == GP_ELEMENT_LEVER) {
        return drawLever(element.parameters[0], element.parameters[1]-yOffset, element.parameters[2], element.parameters[3], element.parameters[4], element.parameters[5], element.parameters[6]);
    } else if ((element.elementType == GP_ELEMENT_BTN_BUTTON) || (element.elementType == GP_ELEMENT_DIR_BUTTON) || (element.elementType == GP_ELEMENT_PIN_BUTTON)) {
        GPButton* button = drawButton(element.parameters[0], element.parameters[1]-yOffset, element.parameters[2], element.parameters[3], element.parameters[4], element.parameters[5], element.parameters[6]);

        // set type of button
        button->setInputType(element.elementType);
        button->setInputDirection(false);
        button->setShape((GPShape_Type)element.parameters[7]);
        button->setAngle(element.parameters[8]);
        button->setAngleEnd(element.parameters[9]);
        button->setClosed(element.parameters[10]);

        if (element.elementType == GP_ELEMENT_DIR_BUTTON) button->setInputDirection(true);

        return (GPWidget*)button;
    } else if (element.elementType == GP_ELEMENT_SPRITE) {
        return drawSprite(element.parameters[0], element.parameters[1]-yOffset, element.parameters[2], element.parameters[3]);
    } else if (element.elementType == GP_ELEMENT_SHAPE) {
        GPShape* shape = drawShape(element.parameters[0], element.parameters[1]-yOffset, element.parameters[2], element.parameters[3], element.parameters[4], element.parameters[5]);
        shape->setShape((GPShape_Type)element.parameters[7]);
        shape->setAngle(element.parameters[8]);
        shape->setAngleEnd(element.parameters[9]);
        shape->setClosed(element.parameters[10]);
        return shape;
    }
    return NULL;
}

int8_t ButtonLayoutScreen::update() {
    GPWidget::update();
    bool configMode = getConfigMode();

    if (!hasInitialized) {
        const InputHistoryOptions& inputHistoryOptions = Storage::getInstance().getAddonOptions().inputHistoryOptions;
        isInputHistoryEnabled = inputHistoryOptions.enabled;
        inputHistoryX = inputHistoryOptions.row;
        inputHistoryY = inputHistoryOptions.col;
        inputHistoryLength = inputHistoryOptions.length;

        // load layout
        uint16_t elementCtr = 0;
        uint16_t layoutLeft = Storage::getInstance().getDisplayOptions().buttonLayout;
        uint16_t layoutRight = Storage::getInstance().getDisplayOptions().buttonLayoutRight;
        LayoutList currLayoutLeft = GPButtonLayouts_LeftLayouts.at(layoutLeft)();
        LayoutList currLayoutRight = GPButtonLayouts_RightLayouts.at(layoutRight)();
        for (elementCtr = 0; elementCtr < currLayoutLeft.size(); elementCtr++) {
            drawElement(currLayoutLeft[elementCtr]);
        }
        for (elementCtr = 0; elementCtr < currLayoutRight.size(); elementCtr++) {
            drawElement(currLayoutRight[elementCtr]);
        }

        profileDelayStart = getMillis();

        hasInitialized = true;
    }

    // main logic loop
    showProfileBanner();
    if (isInputHistoryEnabled) processInputHistory();

    // check for exit/screen change
    if (!configMode) {
        return DisplayMode::BUTTONS;
    } else {
        uint16_t buttonState = getGamepad()->state.buttons;

        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B1) {
                prevButtonState = 0;
                return -1;
            }
        }

        prevButtonState = buttonState;
  
        return DisplayMode::BUTTONS;
    }
}

void ButtonLayoutScreen::processInputHistory() {
	std::deque<std::string> pressed;

	// Get key states
	std::array<bool, INPUT_HISTORY_MAX_INPUTS> currentInput = {

		pressedUp(),
		pressedDown(),
		pressedLeft(),
		pressedRight(),

		pressedUpLeft(),
		pressedUpRight(),
		pressedDownLeft(),
		pressedDownRight(),

		getGamepad()->pressedB1(),
		getGamepad()->pressedB2(),
		getGamepad()->pressedB3(),
		getGamepad()->pressedB4(),
		getGamepad()->pressedL1(),
		getGamepad()->pressedR1(),
		getGamepad()->pressedL2(),
		getGamepad()->pressedR2(),
		getGamepad()->pressedS1(),
		getGamepad()->pressedS2(),
		getGamepad()->pressedL3(),
		getGamepad()->pressedR3(),
		getGamepad()->pressedA1(),
		getGamepad()->pressedA2(),
	};

	uint8_t mode = ((displayModeLookup.count(getGamepad()->getOptions().inputMode) > 0) ? displayModeLookup.at(getGamepad()->getOptions().inputMode) : 0);

	// Check if any new keys have been pressed
	if (lastInput != currentInput) {
		// Iterate through array
		for (uint8_t x=0; x<INPUT_HISTORY_MAX_INPUTS; x++) {
			// Add any pressed keys to deque
			if (currentInput[x] && (displayNames[mode][x] != "")) pressed.push_back(displayNames[mode][x]);
		}
		// Update the last keypress array
		lastInput = currentInput;
	}

	if (pressed.size() > 0) {
		std::string newInput;
		for(const auto &s : pressed) {
				if(!newInput.empty())
						newInput += "+";
				newInput += s;
		}

		inputHistory.push_back(newInput);
	}

	if (inputHistory.size() > (inputHistoryLength / 2) + 1) {
		inputHistory.pop_front();
	}

	std::string ret;

	for (auto it = inputHistory.crbegin(); it != inputHistory.crend(); ++it) {
		std::string newRet = ret;
		if (!newRet.empty())
			newRet = " " + newRet;

		newRet = *it + newRet;
		ret = newRet;

		if (ret.size() >= inputHistoryLength) {
			break;
		}
	}

	if(ret.size() >= inputHistoryLength) {
		historyString = ret.substr(ret.size() - inputHistoryLength);
	} else {
		historyString = ret;
	}

    footer = historyString;
}

void ButtonLayoutScreen::showProfileBanner() {
    int profileDelayCheck = getMillis();

    if (((profileDelayCheck - profileDelayStart) / 1000) >= profileDelay) {
        // stop displaying
        displayProfileBanner = false;
    } else {
        // display
        header = "     Profile #" + std::to_string(getGamepad()->getOptions().profileNumber);
        displayProfileBanner = true;
    }
}

bool ButtonLayoutScreen::pressedUp()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_UP);
		case DPAD_MODE_LEFT_ANALOG:  return getGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN;
		case DPAD_MODE_RIGHT_ANALOG: return getGamepad()->state.ry == GAMEPAD_JOYSTICK_MIN;
	}

	return false;
}

bool ButtonLayoutScreen::pressedDown()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_DOWN);
		case DPAD_MODE_LEFT_ANALOG:  return getGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX;
		case DPAD_MODE_RIGHT_ANALOG: return getGamepad()->state.ry == GAMEPAD_JOYSTICK_MAX;
	}

	return false;
}

bool ButtonLayoutScreen::pressedLeft()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_LEFT);
		case DPAD_MODE_LEFT_ANALOG:  return getGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN;
		case DPAD_MODE_RIGHT_ANALOG: return getGamepad()->state.rx == GAMEPAD_JOYSTICK_MIN;
	}

	return false;
}

bool ButtonLayoutScreen::pressedRight()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_RIGHT);
		case DPAD_MODE_LEFT_ANALOG:  return getGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX;
		case DPAD_MODE_RIGHT_ANALOG: return getGamepad()->state.rx == GAMEPAD_JOYSTICK_MAX;
	}

	return false;
}

bool ButtonLayoutScreen::pressedUpLeft()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT));
		case DPAD_MODE_LEFT_ANALOG:  return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN);
		case DPAD_MODE_RIGHT_ANALOG: return (getGamepad()->state.rx == GAMEPAD_JOYSTICK_MIN) && (getGamepad()->state.ry == GAMEPAD_JOYSTICK_MIN);
	}

	return false;
}

bool ButtonLayoutScreen::pressedUpRight()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT));
		case DPAD_MODE_LEFT_ANALOG:  return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN);
		case DPAD_MODE_RIGHT_ANALOG: return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN);
	}

	return false;
}

bool ButtonLayoutScreen::pressedDownLeft()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT));
		case DPAD_MODE_LEFT_ANALOG:  return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
		case DPAD_MODE_RIGHT_ANALOG: return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
	}

	return false;
}

bool ButtonLayoutScreen::pressedDownRight()
{
	switch (getGamepad()->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((getGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT));
		case DPAD_MODE_LEFT_ANALOG:  return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
		case DPAD_MODE_RIGHT_ANALOG: return (getGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
	}

	return false;
}