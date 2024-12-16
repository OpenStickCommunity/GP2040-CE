#include "ButtonLayoutScreen.h"
#include "buttonlayouts.h"
#include "drivermanager.h"
#include "drivers/ps4/PS4Driver.h"
#include "drivers/xbone/XBOneDriver.h"
#include "drivers/xinput/XInputDriver.h"

void ButtonLayoutScreen::init() {
    const InputHistoryOptions& inputHistoryOptions = Storage::getInstance().getAddonOptions().inputHistoryOptions;
    isInputHistoryEnabled = inputHistoryOptions.enabled;
    inputHistoryX = inputHistoryOptions.row;
    inputHistoryY = inputHistoryOptions.col;
    inputHistoryLength = inputHistoryOptions.length;
    profileDelayStart = getMillis();
    gamepad = Storage::getInstance().GetGamepad();
    inputMode = DriverManager::getInstance().getInputMode();
    
    footer = "";
    historyString = "";
    inputHistory.clear();

    setViewport((isInputHistoryEnabled ? 8 : 0), 0, (isInputHistoryEnabled ? 56 : getRenderer()->getDriver()->getMetrics()->height), getRenderer()->getDriver()->getMetrics()->width);

	// load layout (drawElement pushes element to the display list)
    uint16_t elementCtr = 0;
    LayoutManager::LayoutList currLayoutLeft = LayoutManager::getInstance().getLayoutA();
    LayoutManager::LayoutList currLayoutRight = LayoutManager::getInstance().getLayoutB();
    for (elementCtr = 0; elementCtr < currLayoutLeft.size(); elementCtr++) {
        pushElement(currLayoutLeft[elementCtr]);
    }
    for (elementCtr = 0; elementCtr < currLayoutRight.size(); elementCtr++) {
        pushElement(currLayoutRight[elementCtr]);
    }

	// start with profile mode displayed
    prevGamepadProfileNumber = -1;
    prevLEDAnimationProfileNumber = -1;
    prevSpecialMoveProfileNumber = -1;
    prevLayoutLeft = Storage::getInstance().getDisplayOptions().buttonLayout;
    prevLayoutRight = Storage::getInstance().getDisplayOptions().buttonLayoutRight;
    prevLeftOptions = Storage::getInstance().getDisplayOptions().buttonLayoutCustomOptions.paramsLeft;
    prevRightOptions = Storage::getInstance().getDisplayOptions().buttonLayoutCustomOptions.paramsRight;

    // we cannot look at macro options enabled, pull the pins
    
    // macro display now uses our pin functions, so we need to check if pins are enabled...
    macroEnabled = false;
    // Macro Button initialized by void Gamepad::setup()
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch( pinMappings[pin].action ) {
            case GpioAction::BUTTON_PRESS_MACRO:
            case GpioAction::BUTTON_PRESS_MACRO_1:
            case GpioAction::BUTTON_PRESS_MACRO_2:
            case GpioAction::BUTTON_PRESS_MACRO_3:
            case GpioAction::BUTTON_PRESS_MACRO_4:
            case GpioAction::BUTTON_PRESS_MACRO_5:
            case GpioAction::BUTTON_PRESS_MACRO_6:
                macroEnabled = true;
                break;
            default:
                break;
        }
    }

    getRenderer()->clearScreen();
}

void ButtonLayoutScreen::shutdown() {
    clearElements();
}

void ButtonLayoutScreen::addCustomHeader(std::string newStr){
    profileDelayStart = getMillis();
    profileModeString = newStr;
    profileModeDisplay = true;
}

void ButtonLayoutScreen::updateCustomHeaders()
{
	Storage& storage = Storage::getInstance();

    // Check to see if gamepad profile has changed
    if(!ledAnimationProfileModeDisplay && !specialMoveProfileModeDisplay){
        uint8_t profileNumber = getGamepad()->getOptions().profileNumber;
        if (prevGamepadProfileNumber != profileNumber) {
            prevGamepadProfileNumber = profileNumber;

            std::string profileStr;
            profileStr.assign(storage.currentProfileLabel(), strlen(storage.currentProfileLabel()));
            if (profileStr.empty()) {
                profileStr = "     Profile #";
                profileStr +=  std::to_string(profileNumber);
            } else {
                profileStr.insert(profileStr.begin(), (21-profileStr.length())/2, ' ');
            }

            gamepadProfileModeDisplay = true;
            addCustomHeader(profileStr);
        }
    }

    // Check to see if LED animation profile has changed
    if(!gamepadProfileModeDisplay && !specialMoveProfileModeDisplay){
        uint8_t profileNumber = AnimationStation::options.baseProfileIndex;
        if (prevLEDAnimationProfileNumber != profileNumber) {
            prevLEDAnimationProfileNumber = profileNumber;

            std::string profileStr;
            profileStr = "    LED Profile #";
            profileStr +=  std::to_string(profileNumber);

            ledAnimationProfileModeDisplay = true;
            addCustomHeader(profileStr);
        }
    }
    
    // Check to see if special move profile has changed
    if(!gamepadProfileModeDisplay && !ledAnimationProfileModeDisplay){
        uint8_t profileNumber = SpecialMoveSystem::Options.CurrentProfileIndex;
        if (prevSpecialMoveProfileNumber != profileNumber) {
            prevSpecialMoveProfileNumber = profileNumber;

        	const SpecialMoveOptions_Proto& optionsProto = storage.getSpecialMoveOptions();
            std::string profileStr;
            profileStr.assign(optionsProto.profiles[profileNumber].Label, strlen(optionsProto.profiles[profileNumber].Label));
            if (profileStr.empty()) {
                profileStr = "Special Profile #";
                profileStr +=  std::to_string(profileNumber);
            } else {
                profileStr.insert(profileStr.begin(), (21-profileStr.length())/2, ' ');
            }

            specialMoveProfileModeDisplay = true;
            addCustomHeader(profileStr);
        }
    }
}

int8_t ButtonLayoutScreen::update() {
	Storage& storage = Storage::getInstance();
    bool configMode = storage.GetConfigMode();
    
    // Check if we've updated button layouts while in config mode
    if (configMode) {
        uint8_t layoutLeft = storage.getDisplayOptions().buttonLayout;
        uint8_t layoutRight = storage.getDisplayOptions().buttonLayoutRight;
        bool inputHistoryEnabled = storage.getAddonOptions().inputHistoryOptions.enabled;
        if ((prevLayoutLeft != layoutLeft) || (prevLayoutRight != layoutRight) || (isInputHistoryEnabled != inputHistoryEnabled) || compareCustomLayouts()) {
            shutdown();
            init();
        }
    }

    updateCustomHeaders();

    // main logic loop
	generateHeader();
    if (isInputHistoryEnabled)
		processInputHistory();

    // check for exit/screen change
    if (Storage::getInstance().GetConfigMode()) {
        uint16_t buttonState = getGamepad()->state.buttons;
        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B1) {
                prevButtonState = 0;
                return DisplayMode::CONFIG_INSTRUCTION;
            }
        }
        prevButtonState = buttonState;
    }

	return -1;
}

void ButtonLayoutScreen::generateHeader() {
	// Limit to 21 chars with 6x8 font for now
	statusBar.clear();
	Storage& storage = Storage::getInstance();

 	// Display Profile # banner
	if ( profileModeDisplay ) {
		if (((getMillis() - profileDelayStart) / 1000) < profileDelay) {
			statusBar = profileModeString;
			return;
		} else {
			profileModeDisplay = false;
            gamepadProfileModeDisplay = false;
            ledAnimationProfileModeDisplay = false;
            specialMoveProfileModeDisplay = false;           
		}
	}

	// Display standard header
	switch (inputMode)
	{
		case INPUT_MODE_PS3:    statusBar += "PS3"; break;
		case INPUT_MODE_GENERIC: statusBar += "USBHID"; break;
		case INPUT_MODE_SWITCH: statusBar += "SWITCH"; break;
		case INPUT_MODE_MDMINI: statusBar += "GEN/MD"; break;
		case INPUT_MODE_NEOGEO: statusBar += "NGMINI"; break;
		case INPUT_MODE_PCEMINI: statusBar += "PCE/TG"; break;
		case INPUT_MODE_EGRET: statusBar += "EGRET"; break;
		case INPUT_MODE_ASTRO: statusBar += "ASTRO"; break;
		case INPUT_MODE_PSCLASSIC: statusBar += "PSC"; break;
		case INPUT_MODE_XBOXORIGINAL: statusBar += "OGXBOX"; break;
		case INPUT_MODE_PS4:
			statusBar += "PS4";
			if(((PS4Driver*)DriverManager::getInstance().getDriver())->getAuthSent() == true )
				statusBar += ":AS";
			else
				statusBar += "   ";
			break;
		case INPUT_MODE_PS5:
			statusBar += "PS5";
			if(((PS4Driver*)DriverManager::getInstance().getDriver())->getAuthSent() == true )
				statusBar += ":AS";
			else
				statusBar += "   ";
			break;
		case INPUT_MODE_XBONE:
			statusBar += "XBON";
			if(((XBOneDriver*)DriverManager::getInstance().getDriver())->getAuthSent() == true )
				statusBar += "E";
			else
				statusBar += "*";
			break;
		case INPUT_MODE_XINPUT:
            statusBar += "X";
            if(((XInputDriver*)DriverManager::getInstance().getDriver())->getAuthEnabled() == true )
                statusBar += "B360";
            else
                statusBar += "INPUT";
            break;
		case INPUT_MODE_KEYBOARD: statusBar += "HID-KB"; break;
		case INPUT_MODE_CONFIG: statusBar += "CONFIG"; break;
	}

	const TurboOptions& turboOptions = storage.getAddonOptions().turboOptions;
	if ( turboOptions.enabled ) {
		statusBar += " T";
		if ( turboOptions.shotCount < 10 ) // padding
			statusBar += "0";
		statusBar += std::to_string(turboOptions.shotCount);
	} else {
		statusBar += "    "; // no turbo, don't show Txx setting
	}

	const GamepadOptions & options = gamepad->getOptions();

	switch (gamepad->getActiveDpadMode())
	{
		case DPAD_MODE_DIGITAL:      statusBar += " D"; break;
		case DPAD_MODE_LEFT_ANALOG:  statusBar += " L"; break;
		case DPAD_MODE_RIGHT_ANALOG: statusBar += " R"; break;
	}

	switch (Gamepad::resolveSOCDMode(gamepad->getOptions()))
	{
		case SOCD_MODE_NEUTRAL:               statusBar += " SOCD-N"; break;
		case SOCD_MODE_UP_PRIORITY:           statusBar += " SOCD-U"; break;
		case SOCD_MODE_SECOND_INPUT_PRIORITY: statusBar += " SOCD-L"; break;
		case SOCD_MODE_FIRST_INPUT_PRIORITY:  statusBar += " SOCD-F"; break;
		case SOCD_MODE_BYPASS:                statusBar += " SOCD-X"; break;
	}
	if (macroEnabled)
		statusBar += " M";
}

void ButtonLayoutScreen::drawScreen() {
    if (profileModeDisplay) {
        getRenderer()->drawRectangle(0, 0, 128, 7, true, true);
    	getRenderer()->drawText(0, 0, statusBar, true);
    } else {
		getRenderer()->drawText(0, 0, statusBar);
	}
    getRenderer()->drawText(0, 7, footer);

 	Storage& storage = Storage::getInstance();
   	getRenderer()->drawText(0, 0, AnimationStation::printfs[0]);
    getRenderer()->drawText(0, 1, AnimationStation::printfs[1]);
    getRenderer()->drawText(0, 2, AnimationStation::printfs[2]);
    getRenderer()->drawText(0, 3, AnimationStation::printfs[3]);
    getRenderer()->drawText(0, 4, storage.printfs[0]);
    getRenderer()->drawText(0, 5, storage.printfs[1]);
    getRenderer()->drawText(0, 6, storage.printfs[2]);
    getRenderer()->drawText(0, 7, storage.printfs[3]);
}

GPLever* ButtonLayoutScreen::addLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType) {
    GPLever* lever = new GPLever();
    lever->setRenderer(getRenderer());
    lever->setPosition(startX, startY);
    lever->setStrokeColor(strokeColor);
    lever->setFillColor(fillColor);
    lever->setRadius(sizeX);
    lever->setInputType(inputType);
    lever->setViewport(this->getViewport());
    return (GPLever*)addElement(lever);
}

GPButton* ButtonLayoutScreen::addButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask) {
    GPButton* button = new GPButton();
    button->setRenderer(getRenderer());
    button->setPosition(startX, startY);
    button->setStrokeColor(strokeColor);
    button->setFillColor(fillColor);
    button->setSize(sizeX, sizeY);
    button->setInputMask(inputMask);
    button->setViewport(this->getViewport());
    return (GPButton*)addElement(button);
}

GPShape* ButtonLayoutScreen::addShape(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor) {
    GPShape* shape = new GPShape();
    shape->setRenderer(getRenderer());
    shape->setPosition(startX, startY);
    shape->setStrokeColor(strokeColor);
    shape->setFillColor(fillColor);
    shape->setSize(sizeX,sizeY);
    shape->setViewport(this->getViewport());
    return (GPShape*)addElement(shape);
}

GPSprite* ButtonLayoutScreen::addSprite(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY) {
    GPSprite* sprite = new GPSprite();
    sprite->setRenderer(getRenderer());
    sprite->setPosition(startX, startY);
    sprite->setSize(sizeX,sizeY);
    sprite->setViewport(this->getViewport());
    return (GPSprite*)addElement(sprite);
}

GPWidget* ButtonLayoutScreen::pushElement(GPButtonLayout element) {
    if (element.elementType == GP_ELEMENT_LEVER) {
        return addLever(element.parameters.x1, element.parameters.y1, element.parameters.x2, element.parameters.y2, element.parameters.stroke, element.parameters.fill, element.parameters.value);
    } else if ((element.elementType == GP_ELEMENT_BTN_BUTTON) || (element.elementType == GP_ELEMENT_DIR_BUTTON) || (element.elementType == GP_ELEMENT_PIN_BUTTON)) {
        GPButton* button = addButton(element.parameters.x1, element.parameters.y1, element.parameters.x2, element.parameters.y2, element.parameters.stroke, element.parameters.fill, element.parameters.value);

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
        return addSprite(element.parameters.x1, element.parameters.y1, element.parameters.x2, element.parameters.y2);
    } else if (element.elementType == GP_ELEMENT_SHAPE) {
        GPShape* shape = addShape(element.parameters.x1, element.parameters.y1, element.parameters.x2, element.parameters.y2, element.parameters.stroke, element.parameters.fill);
        shape->setShape((GPShape_Type)element.parameters.shape);
        shape->setAngle(element.parameters.angleStart);
        shape->setAngleEnd(element.parameters.angleEnd);
        shape->setClosed(element.parameters.closed);
        return shape;
    }
    return NULL;
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

		getProcessedGamepad()->pressedB1(),
		getProcessedGamepad()->pressedB2(),
		getProcessedGamepad()->pressedB3(),
		getProcessedGamepad()->pressedB4(),
		getProcessedGamepad()->pressedL1(),
		getProcessedGamepad()->pressedR1(),
		getProcessedGamepad()->pressedL2(),
		getProcessedGamepad()->pressedR2(),
		getProcessedGamepad()->pressedS1(),
		getProcessedGamepad()->pressedS2(),
		getProcessedGamepad()->pressedL3(),
		getProcessedGamepad()->pressedR3(),
		getProcessedGamepad()->pressedA1(),
		getProcessedGamepad()->pressedA2(),
	};

	uint8_t mode = ((displayModeLookup.count(getGamepad()->getOptions().inputMode) > 0) ? displayModeLookup.at(getGamepad()->getOptions().inputMode) : 0);

	// Check if any new keys have been pressed
	if (lastInput != currentInput) {
		// Iterate through array
		for (uint8_t x=0; x<INPUT_HISTORY_MAX_INPUTS; x++) {
			// Add any pressed keys to deque
			std::string inputChar(displayNames[mode][x]);
			if (currentInput[x] && (inputChar != "")) pressed.push_back(inputChar);
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

bool ButtonLayoutScreen::compareCustomLayouts()
{
    ButtonLayoutParamsLeft leftOptions = Storage::getInstance().getDisplayOptions().buttonLayoutCustomOptions.paramsLeft;
    ButtonLayoutParamsRight rightOptions = Storage::getInstance().getDisplayOptions().buttonLayoutCustomOptions.paramsRight;

    bool leftChanged = ((leftOptions.layout != prevLeftOptions.layout) || (leftOptions.common.startX != prevLeftOptions.common.startX) || (leftOptions.common.startY != prevLeftOptions.common.startY) || (leftOptions.common.buttonPadding != prevLeftOptions.common.buttonPadding) || (leftOptions.common.buttonRadius != prevLeftOptions.common.buttonRadius));
    bool rightChanged = ((rightOptions.layout != prevRightOptions.layout) || (rightOptions.common.startX != prevRightOptions.common.startX) || (rightOptions.common.startY != prevRightOptions.common.startY) || (rightOptions.common.buttonPadding != prevRightOptions.common.buttonPadding) || (rightOptions.common.buttonRadius != prevRightOptions.common.buttonRadius));
    
    return (leftChanged || rightChanged);
}

bool ButtonLayoutScreen::pressedUp()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_UP);
        case DPAD_MODE_LEFT_ANALOG:  return getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN;
        case DPAD_MODE_RIGHT_ANALOG: return getProcessedGamepad()->state.ry == GAMEPAD_JOYSTICK_MIN;
    }

    return false;
}

bool ButtonLayoutScreen::pressedDown()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_DOWN);
        case DPAD_MODE_LEFT_ANALOG:  return getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX;
        case DPAD_MODE_RIGHT_ANALOG: return getProcessedGamepad()->state.ry == GAMEPAD_JOYSTICK_MAX;
    }

    return false;
}

bool ButtonLayoutScreen::pressedLeft()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_LEFT);
        case DPAD_MODE_LEFT_ANALOG:  return getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN;
        case DPAD_MODE_RIGHT_ANALOG: return getProcessedGamepad()->state.rx == GAMEPAD_JOYSTICK_MIN;
    }

    return false;
}

bool ButtonLayoutScreen::pressedRight()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_RIGHT);
        case DPAD_MODE_LEFT_ANALOG:  return getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX;
        case DPAD_MODE_RIGHT_ANALOG: return getProcessedGamepad()->state.rx == GAMEPAD_JOYSTICK_MAX;
    }

    return false;
}

bool ButtonLayoutScreen::pressedUpLeft()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT));
        case DPAD_MODE_LEFT_ANALOG:  return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN);
        case DPAD_MODE_RIGHT_ANALOG: return (getProcessedGamepad()->state.rx == GAMEPAD_JOYSTICK_MIN) && (getProcessedGamepad()->state.ry == GAMEPAD_JOYSTICK_MIN);
    }

    return false;
}

bool ButtonLayoutScreen::pressedUpRight()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT));
        case DPAD_MODE_LEFT_ANALOG:  return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN);
        case DPAD_MODE_RIGHT_ANALOG: return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MIN);
    }

    return false;
}

bool ButtonLayoutScreen::pressedDownLeft()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT));
        case DPAD_MODE_LEFT_ANALOG:  return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
        case DPAD_MODE_RIGHT_ANALOG: return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MIN) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
    }

    return false;
}

bool ButtonLayoutScreen::pressedDownRight()
{
    switch (getGamepad()->getActiveDpadMode())
    {
        case DPAD_MODE_DIGITAL:      return ((getProcessedGamepad()->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT));
        case DPAD_MODE_LEFT_ANALOG:  return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
        case DPAD_MODE_RIGHT_ANALOG: return (getProcessedGamepad()->state.lx == GAMEPAD_JOYSTICK_MAX) && (getProcessedGamepad()->state.ly == GAMEPAD_JOYSTICK_MAX);
    }

    return false;
}
