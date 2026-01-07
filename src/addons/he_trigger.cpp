#include "addons/he_trigger.h"
#include "storagemanager.h"

#include "hardware/adc.h"

#define ADC_MAX ((1 << 12) - 1) // 4095

bool HETriggerAddon::available() {
    return Storage::getInstance().getAddonOptions().heTriggerOptions.enabled;
}

void HETriggerAddon::setup() {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    this->muxTotal = 32 / options.muxChannels;
    if ( this->muxTotal > 4 )
        this->muxTotal = 4; // Direct = 4, 4-Channel = 4, 8-Channel = 3, 16-Channel = 2

    // Init the ADC options
    muxPinArray[0] = options.muxADCPin0;
    muxPinArray[1] = options.muxADCPin1;
    muxPinArray[2] = options.muxADCPin2;
    muxPinArray[3] = options.muxADCPin3;
    for(int i = 0; i < muxTotal; i++) {
        if ( muxPinArray[i] >= 26 && muxPinArray[i] <= 29 ) {
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

    selectPinArray[0] = options.selectPin0;
    selectPinArray[1] = options.selectPin1;
    selectPinArray[2] = options.selectPin2;
    selectPinArray[3] = options.selectPin3;
    for(int i = 0; i < selectPins; i++) {
        if ( selectPinArray[i] != -1 ) {
            gpio_init(selectPinArray[i]);
            gpio_set_dir(selectPinArray[i], GPIO_OUT);
            gpio_put(selectPinArray[i], 0);
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
            channel = (i % options.muxChannels);
            selectChannel(channel);
            // Only Switch ADC if we are not currently on the mux ADC
            if ( lastADCSelected != muxPinArray[mux]) {
                adc_select_input(muxPinArray[mux]-26);
                lastADCSelected = muxPinArray[mux];
            }
            emaSmoothingReads[i] = adc_read();
        }
        emaSmoothingFactor = (float)options.smoothingFactor / 100.f; // 99 = max smoothing factor
    }
}

void HETriggerAddon::selectChannel(uint8_t channel) {
    for(int i = 0; i < selectPins; i++) {
        if ( selectPinArray[i] != -1 ) {
            gpio_put(selectPinArray[i], (channel >> i) & 0x01);
        }   
    }
}

uint16_t HETriggerAddon::emaSmoothing(uint16_t value, uint16_t previous) {
    float ema_value = (float)value / ADC_MAX;
    float ema_previous = (float)previous / ADC_MAX;
    return ((emaSmoothingFactor*ema_value) + ((1.0f-emaSmoothingFactor) * ema_previous)) * ADC_MAX;
}

void HETriggerAddon::preprocess() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    for (uint8_t he = 0; he < 32; he++) {
        // Ignore triggers with no actions
        if (options.triggers[he].action == -10 )
            continue;
        mux = (he / options.muxChannels);
        channel = (he % options.muxChannels);
        selectChannel(channel);
        // Only Switch ADC if we are not currently on the mux ADC
        if ( lastADCSelected != muxPinArray[mux]) {
            adc_select_input(muxPinArray[mux]-26);
            lastADCSelected = muxPinArray[mux];
        }
        value = adc_read();

        // EMA Smoothing
        if ( options.emaSmoothing == 1 ) {
            value = emaSmoothing(value, emaSmoothingReads[he]);
            emaSmoothingReads[he] = value;
        }

        if (value >= options.triggers[he].active) {
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
                case GpioAction::MENU_NAVIGATION_UP: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_UP)); break;
                case GpioAction::MENU_NAVIGATION_DOWN: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_DOWN)); break;
                case GpioAction::MENU_NAVIGATION_LEFT: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_LEFT)); break;
                case GpioAction::MENU_NAVIGATION_RIGHT: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_RIGHT)); break;
                case GpioAction::MENU_NAVIGATION_SELECT: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_SELECT)); break;
                case GpioAction::MENU_NAVIGATION_BACK: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_BACK)); break;
                case GpioAction::MENU_NAVIGATION_TOGGLE: EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_TOGGLE)); break;
                default: break;
            }
        }
    }
}
