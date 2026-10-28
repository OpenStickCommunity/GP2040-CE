#include "addons/he_trigger.h"
#include "addons/analog.h"
#include "storagemanager.h"

#include "hardware/adc.h"

#define ADC_MAX ((1 << 12) - 1) // 4095

static Pin_t getMuxADCPin(const HETriggerOptions & options, uint32_t mux) {
    switch (mux) {
        case 0: return options.muxADCPin0;
        case 1: return options.muxADCPin1;
        case 2: return options.muxADCPin2;
        case 3: return options.muxADCPin3;
        default: return -1;
    }
}

int HETriggerAddon::getMuxCount(int32_t muxChannels) {
    int count = HETRIGGER_COUNT / muxChannels;
    return count > HETRIGGER_MUX_COUNT ? HETRIGGER_MUX_COUNT : count;
}

bool HETriggerAddon::isTriggerScanned(const HETriggerOptions & options, uint32_t index) {
    uint32_t mux = index / options.muxChannels;
    return mux < (uint32_t)getMuxCount(options.muxChannels) && AnalogInput::isAdcPin(getMuxADCPin(options, mux));
}

bool HETriggerAddon::available() {
    return Storage::getInstance().getAddonOptions().heTriggerOptions.enabled;
}

void HETriggerAddon::setup() {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    this->muxTotal = getMuxCount(options.muxChannels);

    // Init the ADC options
    for(int mux = 0; mux < HETRIGGER_MUX_COUNT; mux++) {
        muxPinArray[mux] = getMuxADCPin(options, mux);
    }
    for(int i = 0; i < muxTotal; i++) {
        if (AnalogInput::isAdcPin(muxPinArray[i])) {
            adc_gpio_init(muxPinArray[i]);
        }
    }

    // Init our select pins
    switch(options.muxChannels) {
        case 4:
            this->selectPins = 2;
            break;
        case 8:
            this->selectPins = 3;
            break;
        case 16:
            this->selectPins = 4;
            break;
        case 1:
        default:
            this->selectPins = 0;
            break;
    }

    for(int mux = 0; mux < HETRIGGER_MUX_COUNT; mux++) {
        int src = options.separateSelectPins ? mux : 0;
        selectPinArray[mux][0] = options.muxes[src].selectPin0;
        selectPinArray[mux][1] = options.muxes[src].selectPin1;
        selectPinArray[mux][2] = options.muxes[src].selectPin2;
        selectPinArray[mux][3] = options.muxes[src].selectPin3;
    }

    for(int mux = 0; mux < muxTotal; mux++) {
        for(int i = 0; i < selectPins; i++) {
            if ( selectPinArray[mux][i] != -1 ) {
                gpio_init(selectPinArray[mux][i]);
                gpio_set_dir(selectPinArray[mux][i], GPIO_OUT);
                gpio_put(selectPinArray[mux][i], 0);
            }
        }
    }

    lastADCSelected = -1;

    if ( options.emaSmoothing == 1 ) {
        // Read all ADC values once
        for(int i = 0; i < 32; i++) {
            // Ignore triggers with no actions
            if (options.triggers[i].action == -10 )
                continue;
            mux = (i / options.muxChannels);
            // Ignore triggers past the configured multiplexers or on one without an ADC pin
            if ( mux >= (uint32_t)muxTotal || !AnalogInput::isAdcPin(muxPinArray[mux]) )
                continue;
            channel = (i % options.muxChannels);
            selectChannel(mux, channel);
            // Only Switch ADC if we are not currently on the mux ADC
            if ( lastADCSelected != muxPinArray[mux]) {
                adc_select_input(muxPinArray[mux] - ADC_BASE_PIN);
                lastADCSelected = muxPinArray[mux];
            }
            emaSmoothingReads[i] = adc_read();
            lastIncrement[i] = adc_read();
            triggerActive[i] = false;
        }
        emaSmoothingFactor = (float)options.smoothingFactor / 100.f; // 99 = max smoothing factor
    }
}

void HETriggerAddon::selectChannel(uint8_t mux, uint8_t channel) {
    for(int i = 0; i < selectPins; i++) {
        if ( selectPinArray[mux][i] != -1 ) {
            gpio_put(selectPinArray[mux][i], (channel >> i) & 0x01);
        }
    }
}

uint16_t HETriggerAddon::emaSmoothing(uint16_t value, uint16_t previous) {
    float ema_value = (float)value / ADC_MAX;
    float ema_previous = (float)previous / ADC_MAX;
    return ((emaSmoothingFactor*ema_value) + ((1.0f-emaSmoothingFactor) * ema_previous)) * ADC_MAX;
}

void HETriggerAddon::preprocess() {
    scan(true);
}

// Web config skips core0 add-ons, so only track sensor state for LEDs and the display
void HETriggerAddon::configModeProcess() {
    lastADCSelected = -1; // calibration reads may have switched the ADC input
    scan(false);
}

void HETriggerAddon::scan(bool applyActions) {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    gamepad->state.heTriggers = 0;
    for (uint8_t he = 0; he < 32; he++) {
        // Ignore triggers with no actions
        if (options.triggers[he].action == -10 )
            continue;
        mux = (he / options.muxChannels);
        // Ignore triggers past the configured multiplexers or on one without an ADC pin
        if ( mux >= (uint32_t)muxTotal || !AnalogInput::isAdcPin(muxPinArray[mux]) )
            continue;
        channel = (he % options.muxChannels);
        selectChannel(mux, channel);
        // Only Switch ADC if we are not currently on the mux ADC
        if ( lastADCSelected != muxPinArray[mux]) {
            adc_select_input(muxPinArray[mux] - ADC_BASE_PIN);
            lastADCSelected = muxPinArray[mux];
        }

        value = adc_read();
        activationThreshold = (uint16_t)options.triggers[he].active;
        releaseThreshold = (uint16_t)options.triggers[he].active;

        if ( options.triggers[he].rapidTrigger ) {
            releaseThreshold = (uint16_t)options.triggers[he].release;
        }

        // EMA Smoothing
        if ( options.emaSmoothing == 1 ) {
            value = emaSmoothing(value, emaSmoothingReads[he]);
            emaSmoothingReads[he] = value;
        }

        if (options.triggers[he].is_polarized) {
            // effectively inverting value and thresholds
            value = ADC_MAX - value;
            activationThreshold = ADC_MAX - activationThreshold;
            releaseThreshold = ADC_MAX - releaseThreshold;
        }

        bool wasActive = triggerActive[he];
        if (!options.triggers[he].rapidTrigger) {
            // no rapid trigger
            triggerActive[he] = value > activationThreshold;
        } else {
            // chad rapid trigger
            bool pressing = (value > lastIncrement[he]) && (value - lastIncrement[he]) > options.triggers[he].noise;
            bool releasing = (lastIncrement[he] > value) && (lastIncrement[he] - value) > options.triggers[he].noise;
            if (pressing || releasing) {
                lastIncrement[he] = value;
            }

            if ( !triggerActive[he] && pressing && value >= activationThreshold) {
                triggerActive[he] = true;
            } else if (triggerActive[he] && releasing && value <= releaseThreshold) {
                triggerActive[he] = false;
            }
        }
        if (triggerActive[he]) {
            gamepad->state.heTriggers |= (1u << he);
            if (!applyActions)
                continue;
            switch (options.triggers[he].action) {
                case GpioAction::BUTTON_PRESS_UP: gamepad->state.dpad |= GAMEPAD_MASK_UP; break;
                case GpioAction::BUTTON_PRESS_DOWN: gamepad->state.dpad |= GAMEPAD_MASK_DOWN; break;
                case GpioAction::BUTTON_PRESS_LEFT: gamepad->state.dpad |= GAMEPAD_MASK_LEFT; break;
                case GpioAction::BUTTON_PRESS_RIGHT: gamepad->state.dpad |= GAMEPAD_MASK_RIGHT; break;
                case GpioAction::BUTTON_PRESS_B1: gamepad->state.buttons |= GAMEPAD_MASK_B1; break;
                case GpioAction::BUTTON_PRESS_B2: gamepad->state.buttons |= GAMEPAD_MASK_B2; break;
                case GpioAction::BUTTON_PRESS_B3: gamepad->state.buttons |= GAMEPAD_MASK_B3; break;
                case GpioAction::BUTTON_PRESS_B4: gamepad->state.buttons |= GAMEPAD_MASK_B4; break;
                case GpioAction::BUTTON_PRESS_L1: gamepad->state.buttons |= GAMEPAD_MASK_L1; break;
                case GpioAction::BUTTON_PRESS_R1: gamepad->state.buttons |= GAMEPAD_MASK_R1; break;
                case GpioAction::BUTTON_PRESS_L2: gamepad->state.buttons |= GAMEPAD_MASK_L2; break;
                case GpioAction::BUTTON_PRESS_R2: gamepad->state.buttons |= GAMEPAD_MASK_R2; break;
                case GpioAction::BUTTON_PRESS_S1: gamepad->state.buttons |= GAMEPAD_MASK_S1; break;
                case GpioAction::BUTTON_PRESS_S2: gamepad->state.buttons |= GAMEPAD_MASK_S2; break;
                case GpioAction::BUTTON_PRESS_L3: gamepad->state.buttons |= GAMEPAD_MASK_L3; break;
                case GpioAction::BUTTON_PRESS_R3: gamepad->state.buttons |= GAMEPAD_MASK_R3; break;
                case GpioAction::BUTTON_PRESS_A1: gamepad->state.buttons |= GAMEPAD_MASK_A1; break;
                case GpioAction::BUTTON_PRESS_A2: gamepad->state.buttons |= GAMEPAD_MASK_A2; break;
                case GpioAction::BUTTON_PRESS_A3: gamepad->state.buttons |= GAMEPAD_MASK_A3; break;
                case GpioAction::BUTTON_PRESS_A4: gamepad->state.buttons |= GAMEPAD_MASK_A4; break;
                case GpioAction::BUTTON_PRESS_E1: gamepad->state.buttons |= GAMEPAD_MASK_E1; break;
                case GpioAction::BUTTON_PRESS_E2: gamepad->state.buttons |= GAMEPAD_MASK_E2; break;
                case GpioAction::BUTTON_PRESS_E3: gamepad->state.buttons |= GAMEPAD_MASK_E3; break;
                case GpioAction::BUTTON_PRESS_E4: gamepad->state.buttons |= GAMEPAD_MASK_E4; break;
                case GpioAction::BUTTON_PRESS_E5: gamepad->state.buttons |= GAMEPAD_MASK_E5; break;
                case GpioAction::BUTTON_PRESS_E6: gamepad->state.buttons |= GAMEPAD_MASK_E6; break;
                case GpioAction::BUTTON_PRESS_E7: gamepad->state.buttons |= GAMEPAD_MASK_E7; break;
                case GpioAction::BUTTON_PRESS_E8: gamepad->state.buttons |= GAMEPAD_MASK_E8; break;
                case GpioAction::BUTTON_PRESS_E9: gamepad->state.buttons |= GAMEPAD_MASK_E9; break;
                case GpioAction::BUTTON_PRESS_E10: gamepad->state.buttons |= GAMEPAD_MASK_E10; break;
                case GpioAction::BUTTON_PRESS_E11: gamepad->state.buttons |= GAMEPAD_MASK_E11; break;
                case GpioAction::BUTTON_PRESS_E12: gamepad->state.buttons |= GAMEPAD_MASK_E12; break;                
                case GpioAction::ANALOG_DIRECTION_LS_X_NEG:	gamepad->state.lx = GAMEPAD_JOYSTICK_MIN; break;
			    case GpioAction::ANALOG_DIRECTION_LS_X_POS:	gamepad->state.lx = GAMEPAD_JOYSTICK_MAX; break;
			    case GpioAction::ANALOG_DIRECTION_LS_Y_NEG:	gamepad->state.ly = GAMEPAD_JOYSTICK_MIN; break;
			    case GpioAction::ANALOG_DIRECTION_LS_Y_POS:	gamepad->state.ly = GAMEPAD_JOYSTICK_MAX; break;
			    case GpioAction::ANALOG_DIRECTION_RS_X_NEG:	gamepad->state.rx = GAMEPAD_JOYSTICK_MIN; break;
			    case GpioAction::ANALOG_DIRECTION_RS_X_POS:	gamepad->state.rx = GAMEPAD_JOYSTICK_MAX; break;
                case GpioAction::ANALOG_DIRECTION_RS_Y_NEG:	gamepad->state.ry = GAMEPAD_JOYSTICK_MIN; break;
                case GpioAction::ANALOG_DIRECTION_RS_Y_POS:	gamepad->state.ry = GAMEPAD_JOYSTICK_MAX; break;
                case GpioAction::BUTTON_PRESS_FN:	gamepad->state.aux |= AUX_MASK_FUNCTION; break;
                case GpioAction::MENU_NAVIGATION_UP:
                case GpioAction::MENU_NAVIGATION_DOWN:
                case GpioAction::MENU_NAVIGATION_LEFT:
                case GpioAction::MENU_NAVIGATION_RIGHT:
                case GpioAction::MENU_NAVIGATION_SELECT:
                case GpioAction::MENU_NAVIGATION_BACK:
                case GpioAction::MENU_NAVIGATION_TOGGLE:
                    if (!wasActive) {
                        EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(options.triggers[he].action));
                    }
                    break;
                default: break;
            }
        }
    }
}
