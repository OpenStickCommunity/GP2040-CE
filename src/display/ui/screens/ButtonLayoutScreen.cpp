#include "ButtonLayoutScreen.h"
#include "buttonlayouts.h"

void ButtonLayoutScreen::drawScreen() {
    getRenderer()->drawRectangle(0, 0, 128, 7, displayProfileBanner, displayProfileBanner);
    getRenderer()->drawText(0, 0, header, displayProfileBanner);
    getRenderer()->drawText(0, 7, footer);
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
        return drawLever(element.parameters.x1, element.parameters.y1-yOffset, element.parameters.x2, element.parameters.y2, element.parameters.stroke, element.parameters.fill, element.parameters.value);
    } else if ((element.elementType == GP_ELEMENT_BTN_BUTTON) || (element.elementType == GP_ELEMENT_DIR_BUTTON) || (element.elementType == GP_ELEMENT_PIN_BUTTON)) {
        GPButton* button = drawButton(element.parameters.x1, element.parameters.y1-yOffset, element.parameters.x2, element.parameters.y2, element.parameters.stroke, element.parameters.fill, element.parameters.value);

        // set type of button
        button->setInputType(element.elementType);
        button->setInputDirection(false);
        button->setShape((GPShape_Type)element.parameters.shape);
        button->setAngle(element.parameters.angleStart);
        button->setAngleEnd(element.parameters.angleEnd);
        button->setClosed(element.parameters.closed);

        if (element.elementType == GP_ELEMENT_DIR_BUTTON) button->setInputDirection(true);

        return (GPWidget*)button;
    } else if (element.elementType == GP_ELEMENT_SPRITE) {
        return drawSprite(element.parameters.x1, element.parameters.y1-yOffset, element.parameters.x2, element.parameters.y2);
    } else if (element.elementType == GP_ELEMENT_SHAPE) {
        GPShape* shape = drawShape(element.parameters.x1, element.parameters.y1-yOffset, element.parameters.x2, element.parameters.y2, element.parameters.stroke, element.parameters.fill);
        shape->setShape((GPShape_Type)element.parameters.shape);
        shape->setAngle(element.parameters.angleStart);
        shape->setAngleEnd(element.parameters.angleEnd);
        shape->setClosed(element.parameters.closed);
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
        LayoutManager::LayoutList currLayoutLeft = LayoutManager::getInstance().getLayoutA();
        LayoutManager::LayoutList currLayoutRight = LayoutManager::getInstance().getLayoutB();
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