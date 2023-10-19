#include "addons/wiiext.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"
#include "config.pb.h"

bool WiiExtensionInput::available() {
    const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
    const WiiOptions& options = Storage::getInstance().getAddonOptions().wiiOptions;

    return (!displayOptions.enabled && (options.enabled && isValidPin(options.i2cSDAPin) && isValidPin(options.i2cSCLPin)));
}

void WiiExtensionInput::setup() {
    const WiiOptions& options = Storage::getInstance().getAddonOptions().wiiOptions;
    nextTimer = getMillis();

#if WII_EXTENSION_DEBUG==true
    stdio_init_all();
#endif

    uIntervalMS = 0;

    currentConfig = NULL;
    
    wii = new WiiExtension(
        options.i2cSDAPin,
        options.i2cSCLPin,
        options.i2cBlock == 0 ? i2c0 : i2c1,
        options.i2cSpeed,
        WII_EXTENSION_I2C_ADDR);
    wii->begin();
    wii->start();

    reloadConfig();

    // Run during setup to catch boot selection mode
    wii->poll();

    update();
}

void WiiExtensionInput::process() {
    if (nextTimer < getMillis()) {
        wii->poll();

        update();
              
        nextTimer = getMillis() + uIntervalMS;
    }

    if (currentConfig != NULL) {
        queueAnalogChange(WiiAnalogs::WII_ANALOG_LEFT_X, leftX, lastLeftX);
        queueAnalogChange(WiiAnalogs::WII_ANALOG_LEFT_Y, leftY, lastLeftY);
        queueAnalogChange(WiiAnalogs::WII_ANALOG_RIGHT_X, rightX, lastRightX);
        queueAnalogChange(WiiAnalogs::WII_ANALOG_RIGHT_Y, rightY, lastRightY);
        queueAnalogChange(WiiAnalogs::WII_ANALOG_LEFT_TRIGGER, triggerLeft, lastTriggerLeft);
        queueAnalogChange(WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER, triggerRight, lastTriggerRight);
        updateAnalogState();

        setButtonState(buttonC, WiiButtons::WII_BUTTON_C);
        setButtonState(buttonZ, WiiButtons::WII_BUTTON_Z);

        setButtonState(buttonA, WiiButtons::WII_BUTTON_A);
        setButtonState(buttonB, WiiButtons::WII_BUTTON_B);
        setButtonState(buttonX, WiiButtons::WII_BUTTON_X);
        setButtonState(buttonY, WiiButtons::WII_BUTTON_Y);
        setButtonState(buttonL, WiiButtons::WII_BUTTON_L);
        setButtonState(buttonZL, WiiButtons::WII_BUTTON_ZL);
        setButtonState(buttonR, WiiButtons::WII_BUTTON_R);
        setButtonState(buttonZR, WiiButtons::WII_BUTTON_ZR);
        setButtonState(buttonSelect, WiiButtons::WII_BUTTON_MINUS);
        setButtonState(buttonStart, WiiButtons::WII_BUTTON_PLUS);
        setButtonState(buttonHome, WiiButtons::WII_BUTTON_HOME);

        setButtonState(dpadUp, WiiButtons::WII_BUTTON_UP);
        setButtonState(dpadDown, WiiButtons::WII_BUTTON_DOWN);
        setButtonState(dpadLeft, WiiButtons::WII_BUTTON_LEFT);
        setButtonState(dpadRight, WiiButtons::WII_BUTTON_RIGHT);

        if (lastLeftX != leftX) lastLeftX = leftX;
        if (lastLeftY != leftY) lastLeftY = leftY;
        if (lastRightX != rightX) lastRightX = rightX;
        if (lastRightY != rightY) lastRightY = rightY;
        if (lastTriggerLeft != triggerLeft) lastTriggerLeft = triggerLeft;
        if (lastTriggerRight != triggerRight) lastTriggerRight = triggerRight;
    }
}

uint16_t WiiExtensionInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t WiiExtensionInput::bounds(uint16_t x, uint16_t out_min, uint16_t out_max) {
    if (x > out_max) x = out_max;
    if (x < out_min) x = out_min;
    return x;
}

void WiiExtensionInput::update() {
    if (wii->extensionType != WII_EXTENSION_NONE) {
        uint16_t joystickMid = GetJoystickMidValue(Storage::getInstance().getGamepadOptions().inputMode);
        currentConfig = &extensionConfigs[wii->extensionType];

        //for (const auto& [extensionButton, value] : currentConfig->buttonMap) {
        //    WII_SET_MASK(buttonState, wii->getController()->buttons[extensionButton], value);
        //}

        isAnalogTriggers = false;

        if (wii->extensionType == WII_EXTENSION_NUNCHUCK) {
            buttonZ = wii->getController()->buttons[WiiButtons::WII_BUTTON_Z];
            buttonC = wii->getController()->buttons[WiiButtons::WII_BUTTON_C];

            leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightX = joystickMid;
            rightY = joystickMid;

            triggerLeft = 0;
            triggerRight = 0;
        } else if ((wii->extensionType == WII_EXTENSION_CLASSIC) || (wii->extensionType == WII_EXTENSION_CLASSIC_PRO)) {
            buttonA = wii->getController()->buttons[WiiButtons::WII_BUTTON_A];
            buttonB = wii->getController()->buttons[WiiButtons::WII_BUTTON_B];
            buttonX = wii->getController()->buttons[WiiButtons::WII_BUTTON_X];
            buttonY = wii->getController()->buttons[WiiButtons::WII_BUTTON_Y];
            buttonL = wii->getController()->buttons[WiiButtons::WII_BUTTON_L];
            buttonZL = wii->getController()->buttons[WiiButtons::WII_BUTTON_ZL];
            buttonR = wii->getController()->buttons[WiiButtons::WII_BUTTON_R];
            buttonZR = wii->getController()->buttons[WiiButtons::WII_BUTTON_ZR];
            buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
            buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];
            buttonHome = wii->getController()->buttons[WiiButtons::WII_BUTTON_HOME];
            dpadUp = wii->getController()->buttons[WiiButtons::WII_BUTTON_UP];
            dpadDown = wii->getController()->buttons[WiiButtons::WII_BUTTON_DOWN];
            dpadLeft = wii->getController()->buttons[WiiButtons::WII_BUTTON_LEFT];
            dpadRight = wii->getController()->buttons[WiiButtons::WII_BUTTON_RIGHT];

            if (wii->extensionType == WII_EXTENSION_CLASSIC) {
                triggerLeft  = wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_TRIGGER];
                triggerRight = wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER];
                isAnalogTriggers = true;
            }

            leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
        } else if (wii->extensionType == WII_EXTENSION_GUITAR) {
            buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
            buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];

            dpadUp = wii->getController()->buttons[WiiButtons::WII_BUTTON_UP];
            dpadDown = wii->getController()->buttons[WiiButtons::WII_BUTTON_DOWN];

            buttonB = wii->getController()->buttons[GuitarButtons::GUITAR_GREEN];
            buttonA = wii->getController()->buttons[GuitarButtons::GUITAR_RED];
            buttonX = wii->getController()->buttons[GuitarButtons::GUITAR_YELLOW];
            buttonY = wii->getController()->buttons[GuitarButtons::GUITAR_BLUE];
            buttonL = wii->getController()->buttons[GuitarButtons::GUITAR_ORANGE];

            // whammy currently maps to Joy2X in addition to the raw whammy value
            whammyBar = wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_X];
            buttonR = wii->getController()->buttons[GuitarButtons::GUITAR_PEDAL];

            leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_RIGHT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightY = joystickMid;

            triggerLeft = 0;
            triggerRight = 0;
        } else if (wii->extensionType == WII_EXTENSION_TAIKO) {
            buttonL = wii->getController()->buttons[TaikoButtons::TATA_KAT_LEFT];
            buttonR = wii->getController()->buttons[TaikoButtons::TATA_KAT_RIGHT];

            dpadLeft = wii->getController()->buttons[TaikoButtons::TATA_DON_LEFT];
            buttonA = wii->getController()->buttons[TaikoButtons::TATA_DON_RIGHT];
        } else if (wii->extensionType == WII_EXTENSION_DRUMS) {
            buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
            buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];

            buttonB = wii->getController()->buttons[DrumButtons::DRUM_RED];
            buttonA = wii->getController()->buttons[DrumButtons::DRUM_GREEN];
            buttonX = wii->getController()->buttons[DrumButtons::DRUM_YELLOW];
            buttonY = wii->getController()->buttons[DrumButtons::DRUM_BLUE];
            buttonL = wii->getController()->buttons[DrumButtons::DRUM_ORANGE];
            buttonZR = wii->getController()->buttons[DrumButtons::DRUM_PEDAL];

            leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightX = joystickMid;
            rightY = joystickMid;

            triggerLeft = 0;
            triggerRight = 0;
        } else if (wii->extensionType == WII_EXTENSION_TURNTABLE) {
            buttonSelect = wii->getController()->buttons[WiiButtons::WII_BUTTON_MINUS];
            buttonStart = wii->getController()->buttons[WiiButtons::WII_BUTTON_PLUS];

            dpadLeft = wii->getController()->buttons[TurntableButtons::TURNTABLE_LEFT_GREEN];
            dpadUp = wii->getController()->buttons[TurntableButtons::TURNTABLE_LEFT_RED];
            dpadRight = wii->getController()->buttons[TurntableButtons::TURNTABLE_LEFT_BLUE];

            buttonY = wii->getController()->buttons[TurntableButtons::TURNTABLE_RIGHT_GREEN];
            buttonX = wii->getController()->buttons[TurntableButtons::TURNTABLE_RIGHT_RED];
            buttonA = wii->getController()->buttons[TurntableButtons::TURNTABLE_RIGHT_BLUE];

            buttonZR = wii->getController()->buttons[TurntableButtons::TURNTABLE_EUPHORIA];

            leftX = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_X],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            leftY = map(wii->getController()->analogState[WiiAnalogs::WII_ANALOG_LEFT_Y],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightX = map(wii->getController()->analogState[TurntableAnalogs::TURNTABLE_RIGHT],0,WII_ANALOG_PRECISION_3,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
            rightY = map(wii->getController()->analogState[TurntableAnalogs::TURNTABLE_LEFT],WII_ANALOG_PRECISION_3,0,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);

            triggerLeft  = wii->getController()->analogState[TurntableAnalogs::TURNTABLE_EFFECTS];
            triggerRight = wii->getController()->analogState[TurntableAnalogs::TURNTABLE_CROSSFADE];

            isAnalogTriggers = true;
        }
    } else {
        currentConfig = NULL;
    }
}

void WiiExtensionInput::setControllerButton(uint16_t controllerID, uint16_t buttonID, uint32_t buttonMask) {
    extensionConfigs[controllerID].buttonMap[buttonID] = buttonMask;
}

void WiiExtensionInput::setControllerAnalog(uint16_t controllerID, uint16_t analogID, uint32_t axisType) {
    extensionConfigs[controllerID].analogMap[analogID].axisType = axisType;
}

void WiiExtensionInput::reloadConfig() {
    const WiiOptions& wiiOptions = Storage::getInstance().getAddonOptions().wiiOptions;

    // digital mapping
    setControllerButton(WII_EXTENSION_NUNCHUCK, WiiButtons::WII_BUTTON_C, wiiOptions.controllers.nunchuk.buttonC);
    setControllerButton(WII_EXTENSION_NUNCHUCK, WiiButtons::WII_BUTTON_Z, wiiOptions.controllers.nunchuk.buttonZ);
    
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_A, wiiOptions.controllers.classic.buttonA);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_B, wiiOptions.controllers.classic.buttonB);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_X, wiiOptions.controllers.classic.buttonX);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_Y, wiiOptions.controllers.classic.buttonY);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_L, wiiOptions.controllers.classic.buttonL);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_ZL, wiiOptions.controllers.classic.buttonZL);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_R, wiiOptions.controllers.classic.buttonR);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_ZR, wiiOptions.controllers.classic.buttonZR);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_MINUS, wiiOptions.controllers.classic.buttonMinus);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_PLUS, wiiOptions.controllers.classic.buttonPlus);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_HOME, wiiOptions.controllers.classic.buttonHome);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_UP, wiiOptions.controllers.classic.buttonUp);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_DOWN, wiiOptions.controllers.classic.buttonDown);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_LEFT, wiiOptions.controllers.classic.buttonLeft);
    setControllerButton(WII_EXTENSION_CLASSIC, WiiButtons::WII_BUTTON_RIGHT, wiiOptions.controllers.classic.buttonRight);

    setControllerButton(WII_EXTENSION_TAIKO, TaikoButtons::TATA_KAT_LEFT, wiiOptions.controllers.taiko.buttonKatLeft);
    setControllerButton(WII_EXTENSION_TAIKO, TaikoButtons::TATA_KAT_RIGHT, wiiOptions.controllers.taiko.buttonKatRight);
    setControllerButton(WII_EXTENSION_TAIKO, TaikoButtons::TATA_DON_LEFT, wiiOptions.controllers.taiko.buttonDonLeft);
    setControllerButton(WII_EXTENSION_TAIKO, TaikoButtons::TATA_DON_RIGHT, wiiOptions.controllers.taiko.buttonDonRight);

    setControllerButton(WII_EXTENSION_GUITAR, GuitarButtons::GUITAR_RED, wiiOptions.controllers.guitar.buttonRed);
    setControllerButton(WII_EXTENSION_GUITAR, GuitarButtons::GUITAR_GREEN, wiiOptions.controllers.guitar.buttonGreen);
    setControllerButton(WII_EXTENSION_GUITAR, GuitarButtons::GUITAR_YELLOW, wiiOptions.controllers.guitar.buttonYellow);
    setControllerButton(WII_EXTENSION_GUITAR, GuitarButtons::GUITAR_BLUE, wiiOptions.controllers.guitar.buttonBlue);
    setControllerButton(WII_EXTENSION_GUITAR, GuitarButtons::GUITAR_ORANGE, wiiOptions.controllers.guitar.buttonOrange);
    setControllerButton(WII_EXTENSION_GUITAR, GuitarButtons::GUITAR_PEDAL, wiiOptions.controllers.guitar.buttonPedal);
    setControllerButton(WII_EXTENSION_GUITAR, WiiButtons::WII_BUTTON_MINUS, wiiOptions.controllers.guitar.buttonMinus);
    setControllerButton(WII_EXTENSION_GUITAR, WiiButtons::WII_BUTTON_PLUS, wiiOptions.controllers.guitar.buttonPlus);
    setControllerButton(WII_EXTENSION_GUITAR, WiiButtons::WII_BUTTON_UP, wiiOptions.controllers.guitar.strumUp);
    setControllerButton(WII_EXTENSION_GUITAR, WiiButtons::WII_BUTTON_DOWN, wiiOptions.controllers.guitar.strumDown);

    setControllerButton(WII_EXTENSION_DRUMS, DrumButtons::DRUM_RED, wiiOptions.controllers.drum.buttonRed);
    setControllerButton(WII_EXTENSION_DRUMS, DrumButtons::DRUM_GREEN, wiiOptions.controllers.drum.buttonGreen);
    setControllerButton(WII_EXTENSION_DRUMS, DrumButtons::DRUM_YELLOW, wiiOptions.controllers.drum.buttonYellow);
    setControllerButton(WII_EXTENSION_DRUMS, DrumButtons::DRUM_BLUE, wiiOptions.controllers.drum.buttonBlue);
    setControllerButton(WII_EXTENSION_DRUMS, DrumButtons::DRUM_ORANGE, wiiOptions.controllers.drum.buttonOrange);
    setControllerButton(WII_EXTENSION_DRUMS, DrumButtons::DRUM_PEDAL, wiiOptions.controllers.drum.buttonPedal);
    setControllerButton(WII_EXTENSION_DRUMS, WiiButtons::WII_BUTTON_MINUS, wiiOptions.controllers.drum.buttonMinus);
    setControllerButton(WII_EXTENSION_DRUMS, WiiButtons::WII_BUTTON_PLUS, wiiOptions.controllers.drum.buttonPlus);

    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_LEFT_RED, wiiOptions.controllers.turntable.buttonLeftRed);
    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_LEFT_GREEN, wiiOptions.controllers.turntable.buttonLeftGreen);
    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_LEFT_BLUE, wiiOptions.controllers.turntable.buttonLeftBlue);
    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_RIGHT_RED, wiiOptions.controllers.turntable.buttonRightRed);
    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_RIGHT_GREEN, wiiOptions.controllers.turntable.buttonRightGreen);
    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_RIGHT_BLUE, wiiOptions.controllers.turntable.buttonRightBlue);
    setControllerButton(WII_EXTENSION_TURNTABLE, TurntableButtons::TURNTABLE_EUPHORIA, wiiOptions.controllers.turntable.buttonEuphoria);
    setControllerButton(WII_EXTENSION_TURNTABLE, WiiButtons::WII_BUTTON_MINUS, wiiOptions.controllers.turntable.buttonMinus);
    setControllerButton(WII_EXTENSION_TURNTABLE, WiiButtons::WII_BUTTON_PLUS, wiiOptions.controllers.turntable.buttonPlus);

    // analog mapping
    setControllerAnalog(WII_EXTENSION_NUNCHUCK, WiiAnalogs::WII_ANALOG_LEFT_X, wiiOptions.controllers.nunchuk.stick.x.axisType);
    setControllerAnalog(WII_EXTENSION_NUNCHUCK, WiiAnalogs::WII_ANALOG_LEFT_Y, wiiOptions.controllers.nunchuk.stick.y.axisType);

    setControllerAnalog(WII_EXTENSION_CLASSIC, WiiAnalogs::WII_ANALOG_LEFT_X, wiiOptions.controllers.classic.leftStick.x.axisType);
    setControllerAnalog(WII_EXTENSION_CLASSIC, WiiAnalogs::WII_ANALOG_LEFT_Y, wiiOptions.controllers.classic.leftStick.y.axisType);
    setControllerAnalog(WII_EXTENSION_CLASSIC, WiiAnalogs::WII_ANALOG_RIGHT_X, wiiOptions.controllers.classic.rightStick.x.axisType);
    setControllerAnalog(WII_EXTENSION_CLASSIC, WiiAnalogs::WII_ANALOG_RIGHT_Y, wiiOptions.controllers.classic.rightStick.y.axisType);
    setControllerAnalog(WII_EXTENSION_CLASSIC, WiiAnalogs::WII_ANALOG_LEFT_TRIGGER, wiiOptions.controllers.classic.leftTrigger.axisType);
    setControllerAnalog(WII_EXTENSION_CLASSIC, WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER, wiiOptions.controllers.classic.rightTrigger.axisType);

    setControllerAnalog(WII_EXTENSION_GUITAR, WiiAnalogs::WII_ANALOG_LEFT_X, wiiOptions.controllers.guitar.stick.x.axisType);
    setControllerAnalog(WII_EXTENSION_GUITAR, WiiAnalogs::WII_ANALOG_LEFT_Y, wiiOptions.controllers.guitar.stick.y.axisType);
    setControllerAnalog(WII_EXTENSION_GUITAR, WiiAnalogs::WII_ANALOG_RIGHT_X, wiiOptions.controllers.guitar.whammyBar.axisType);

    setControllerAnalog(WII_EXTENSION_DRUMS, WiiAnalogs::WII_ANALOG_LEFT_X, wiiOptions.controllers.drum.stick.x.axisType);
    setControllerAnalog(WII_EXTENSION_DRUMS, WiiAnalogs::WII_ANALOG_LEFT_Y, wiiOptions.controllers.drum.stick.y.axisType);

    setControllerAnalog(WII_EXTENSION_TURNTABLE, WiiAnalogs::WII_ANALOG_LEFT_X, wiiOptions.controllers.turntable.stick.x.axisType);
    setControllerAnalog(WII_EXTENSION_TURNTABLE, WiiAnalogs::WII_ANALOG_LEFT_Y, wiiOptions.controllers.turntable.stick.y.axisType);
    setControllerAnalog(WII_EXTENSION_TURNTABLE, WiiAnalogs::WII_ANALOG_RIGHT_X, wiiOptions.controllers.turntable.leftTurntable.axisType);
    setControllerAnalog(WII_EXTENSION_TURNTABLE, WiiAnalogs::WII_ANALOG_RIGHT_Y, wiiOptions.controllers.turntable.rightTurntable.axisType);
    setControllerAnalog(WII_EXTENSION_TURNTABLE, WiiAnalogs::WII_ANALOG_LEFT_TRIGGER, wiiOptions.controllers.turntable.effects.axisType);
    setControllerAnalog(WII_EXTENSION_TURNTABLE, WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER, wiiOptions.controllers.turntable.fader.axisType);
}

void WiiExtensionInput::setButtonState(bool buttonState, uint16_t buttonMask) {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    if (buttonState) {
        if (currentConfig->buttonMap[buttonMask] > GAMEPAD_MASK_A2) {
            if (((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_UP) == GAMEPAD_MASK_UP) gamepad->state.dpad    |= ((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_UP);
            if (((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_DOWN) == GAMEPAD_MASK_DOWN) gamepad->state.dpad    |= ((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_DOWN);
            if (((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_LEFT) == GAMEPAD_MASK_LEFT) gamepad->state.dpad    |= ((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_LEFT);
            if (((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT) gamepad->state.dpad    |= ((currentConfig->buttonMap[buttonMask] >> 16) & GAMEPAD_MASK_RIGHT);
        } else {
            gamepad->state.buttons |= currentConfig->buttonMap[buttonMask];
        }
    }
}

void WiiExtensionInput::queueAnalogChange(uint16_t analogInput, uint16_t analogValue, uint16_t lastAnalogValue) {
    if (analogInput != lastAnalogValue) analogChanges[currentConfig->analogMap[analogInput].axisType].push_back({analogInput, analogValue});
}

void WiiExtensionInput::updateAnalogState() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = isAnalogTriggers;

    uint16_t joystickMid = GetJoystickMidValue(Storage::getInstance().getGamepadOptions().inputMode);

    uint16_t axisType;
    uint16_t analogInput;
    uint16_t analogValue;

    uint16_t axisToChange;
    uint16_t adjustedValue;

    uint16_t minValue = GAMEPAD_JOYSTICK_MIN;
    uint16_t midValue = joystickMid;
    uint16_t maxValue = GAMEPAD_JOYSTICK_MAX;

    std::map<uint16_t, std::vector<uint16_t>> axesOfChange = {
        {WII_ANALOG_TYPE_LEFT_STICK_X,{}},
        {WII_ANALOG_TYPE_LEFT_STICK_Y,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_X,{}},
        {WII_ANALOG_TYPE_RIGHT_STICK_Y,{}},
        {WII_ANALOG_TYPE_LEFT_TRIGGER,{}},
        {WII_ANALOG_TYPE_RIGHT_TRIGGER,{}}
    };

    for (auto currChange = analogChanges.begin(); currChange != analogChanges.end(); ++currChange) {
        if (!currChange->second.empty()) {
            // this analog type has changes. get the last one, use it, and clear the list.            
            axisType = currChange->first;
            analogInput = currChange->second.front().analogInput;
            analogValue = getAverage(currChange->second);
            currChange->second.clear();

            axisToChange = WII_ANALOG_TYPE_NONE;
            adjustedValue = 0;

            if (!isAnalogTriggers && ((analogInput == WiiAnalogs::WII_ANALOG_LEFT_TRIGGER) || (analogInput == WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER))) continue;

            // define ranges
            switch (analogInput) {
                case WiiAnalogs::WII_ANALOG_LEFT_X:
                case WiiAnalogs::WII_ANALOG_LEFT_Y:
                case WiiAnalogs::WII_ANALOG_RIGHT_X:
                case WiiAnalogs::WII_ANALOG_RIGHT_Y:
                    minValue = GAMEPAD_JOYSTICK_MIN;
                    midValue = joystickMid;
                    maxValue = GAMEPAD_JOYSTICK_MAX;
                    break;
                case WiiAnalogs::WII_ANALOG_LEFT_TRIGGER:
                case WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER:
                    minValue = GAMEPAD_TRIGGER_MIN;
                    midValue = GAMEPAD_TRIGGER_MID;
                    maxValue = GAMEPAD_TRIGGER_MAX;
                    break;
            }

            switch (axisType) {
                case WII_ANALOG_TYPE_LEFT_STICK_X:
                    axisToChange = WII_ANALOG_TYPE_LEFT_STICK_X;
                    adjustedValue = bounds(analogValue,minValue,maxValue);
                    break;
                case WII_ANALOG_TYPE_LEFT_STICK_Y:
                    axisToChange = WII_ANALOG_TYPE_LEFT_STICK_Y;
                    adjustedValue = bounds(analogValue,minValue,maxValue);
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_X:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_STICK_X;
                    adjustedValue = bounds(analogValue,minValue,maxValue);
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_Y:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_STICK_Y;
                    adjustedValue = bounds(analogValue,minValue,maxValue);
                    break;
                case WII_ANALOG_TYPE_DPAD_X:
                    if (analogValue < midValue/2) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
                    if (analogValue > midValue+(midValue/2)) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
                    break;
                case WII_ANALOG_TYPE_DPAD_Y:
                    if (analogValue < midValue/2) gamepad->state.dpad |= GAMEPAD_MASK_UP;
                    if (analogValue > midValue+(midValue/2)) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
                    break;
                case WII_ANALOG_TYPE_LEFT_TRIGGER:
                    axisToChange = WII_ANALOG_TYPE_LEFT_TRIGGER;
                    adjustedValue = bounds(analogValue,minValue,maxValue);
                    break;
                case WII_ANALOG_TYPE_RIGHT_TRIGGER:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_TRIGGER;
                    adjustedValue = bounds(analogValue,minValue,maxValue);
                    break;
                // advanced types
                case WII_ANALOG_TYPE_LEFT_STICK_X_PLUS:
                    axisToChange = WII_ANALOG_TYPE_LEFT_STICK_X;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MAX);
                    minValue = joystickMid;
                    maxValue = GAMEPAD_JOYSTICK_MAX;
                    break;
                case WII_ANALOG_TYPE_LEFT_STICK_X_MINUS:
                    axisToChange = WII_ANALOG_TYPE_LEFT_STICK_X;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MIN);
                    minValue = GAMEPAD_JOYSTICK_MIN;
                    maxValue = joystickMid;
                    break;
                case WII_ANALOG_TYPE_LEFT_STICK_Y_PLUS:
                    axisToChange = WII_ANALOG_TYPE_LEFT_STICK_Y;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MAX);
                    minValue = joystickMid;
                    maxValue = GAMEPAD_JOYSTICK_MAX;
                    break;
                case WII_ANALOG_TYPE_LEFT_STICK_Y_MINUS:
                    axisToChange = WII_ANALOG_TYPE_LEFT_STICK_Y;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MIN);
                    minValue = GAMEPAD_JOYSTICK_MIN;
                    maxValue = joystickMid;
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_X_PLUS:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_STICK_X;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MAX);
                    minValue = joystickMid;
                    maxValue = GAMEPAD_JOYSTICK_MAX;
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_X_MINUS:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_STICK_X;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MIN);
                    minValue = GAMEPAD_JOYSTICK_MIN;
                    maxValue = joystickMid;
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_Y_PLUS:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_STICK_Y;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MAX);
                    minValue = joystickMid;
                    maxValue = GAMEPAD_JOYSTICK_MAX;
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_Y_MINUS:
                    axisToChange = WII_ANALOG_TYPE_RIGHT_STICK_Y;
                    adjustedValue = map(analogValue,minValue,maxValue,joystickMid,GAMEPAD_JOYSTICK_MIN);
                    minValue = GAMEPAD_JOYSTICK_MIN;
                    maxValue = joystickMid;
                    break;
            }

            if (axisToChange != WII_ANALOG_TYPE_NONE) axesOfChange[axisToChange].push_back(bounds(adjustedValue,minValue,maxValue));
        }
    }

    for (auto currAxis = axesOfChange.begin(); currAxis != axesOfChange.end(); ++currAxis) {
        if (!currAxis->second.empty()) {
            axisType = currAxis->first;

            switch (axisType) {
                case WII_ANALOG_TYPE_LEFT_STICK_X:
                    gamepad->state.lx = getDelta(currAxis->second, joystickMid);
                    break;
                case WII_ANALOG_TYPE_LEFT_STICK_Y:
                    gamepad->state.ly = getDelta(currAxis->second, joystickMid);
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_X:
                    gamepad->state.rx = getDelta(currAxis->second, joystickMid);
                    break;
                case WII_ANALOG_TYPE_RIGHT_STICK_Y:
                    gamepad->state.ry = getDelta(currAxis->second, joystickMid);
                    break;
                case WII_ANALOG_TYPE_LEFT_TRIGGER:
                    gamepad->state.lt = getDelta(currAxis->second, GAMEPAD_TRIGGER_MID);
                    break;
                case WII_ANALOG_TYPE_RIGHT_TRIGGER:
                    gamepad->state.rt = getDelta(currAxis->second, GAMEPAD_TRIGGER_MID);
                    break;
            }
        }       
    }
}

uint16_t WiiExtensionInput::getAverage(std::vector<WiiAnalogChange> const& changes) {
    uint16_t values = 0;

    if (changes.empty()) {
        return 0;
    }

    for (auto currVal = changes.begin(); currVal != changes.end(); ++currVal) {
        values += currVal->analogValue;
    }

    return values / changes.size();
}

uint16_t WiiExtensionInput::getDelta(std::vector<uint16_t> const& changes, uint16_t baseValue) {
    uint16_t value = baseValue;

    if (changes.empty()) {
        return baseValue;
    }

    for (auto currVal = changes.begin(); currVal != changes.end(); ++currVal) {
        if (*currVal != baseValue) {
            if (*currVal < baseValue) {
                value -= (baseValue - *currVal);
            } else if (*currVal > baseValue) {
                value += (*currVal - baseValue);
            }
        }
    }

    return value;
}