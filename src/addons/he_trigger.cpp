#include "addons/he_trigger.h"
#include "storagemanager.h"

#include <algorithm>
#include <cmath>
#include "hardware/adc.h"

bool HETriggerAddon::available() {
    return Storage::getInstance().getAddonOptions().heTriggerOptions.enabled;
}

void HETriggerAddon::setup() {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    this->muxTotal = 32 / options.muxChannels;
    if ( this->muxTotal > 4 ) this->muxTotal = 4;

    // Init the ADC options
    muxPinArray[0] = options.muxADCPin0;
    muxPinArray[1] = options.muxADCPin1;
    muxPinArray[2] = options.muxADCPin2;
    muxPinArray[3] = options.muxADCPin3;
    for(int i = 0; i < muxTotal; i++) {
        if ( muxPinArray[i] >= 26 && muxPinArray[i] < 29 ) {
            adc_gpio_init(muxPinArray[i]);
        }
    }

    // Init our select pins
    this->selectPins = log2(options.muxChannels);
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
}

void HETriggerAddon::selectChannel(uint8_t channel) {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    for(int i = 0; i < selectPins; i++) {
        if ( selectPinArray[i] != -1 ) {
            gpio_put(selectPinArray[i], (channel >> i) & 0x01);
        }   
    }
}

void HETriggerAddon::process() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    uint8_t mux;
    uint8_t channel;
    uint16_t value;
    for (int he = 0; he < 32; he++) {
        // Ignore triggers with no actions
        if (options.triggers[he].action == -10 )
            continue;

        mux = (he / options.muxChannels);
        channel = (he / options.muxChannels) + (he % options.muxChannels);
        selectChannel(channel);
        // allow the multiplexer output and ADC to settle
        sleep_us(5);
        adc_select_input(muxPinArray[mux]);
        // First conversion after switching channels can contain the
        // previous sample. Perform a dummy read to flush it.
        adc_read();
        sleep_us(2);
        value = adc_read();
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
                default: break;
            }
        }
    }
}
