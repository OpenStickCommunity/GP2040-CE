#include "addons/i2c_gpio_pcf8575.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

bool PCF8575Addon::available() {
    const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
    const PCF8575Options& options = Storage::getInstance().getAddonOptions().pcf8575Options;
    if (options.enabled) {
        pcf = new PCF8575();
        PeripheralI2CScanResult result = PeripheralManager::getInstance().scanForI2CDevice(pcf->getDeviceAddresses());
        if (result.address > -1) {
            pcf->setAddress(result.address);
            pcf->setI2C(PeripheralManager::getInstance().getI2C(result.block));
            return true;
        } else {
            delete pcf;
        }
    }
    return false;
}

void PCF8575Addon::setup() {
    const PCF8575Options& options = Storage::getInstance().getAddonOptions().pcf8575Options;
    const GpioMappingInfo* gpioMappings = options.pins;

    // check if pins have actions defined
    uint16_t pinMask = 0xFFFF;
    for (uint8_t i = 0; i < options.pins_count; i++) {
        GpioMappingInfo pin = gpioMappings[i];
        if ((pin.action != GpioAction::NONE) && (pin.action != GpioAction::RESERVED) && (pin.action != GpioAction::ASSIGNED_TO_ADDON)) {
            pinRef.insert({i,pin});
        }
    }

    // at least one pin is defined with an action
    if (pinRef.size() > 0) {
        pcf->begin();
        // set default mask
        pcf->send(pinMask);
    }
}

void PCF8575Addon::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    for (std::map<uint8_t, GpioMappingInfo>::iterator pin = pinRef.begin(); pin != pinRef.end(); ++pin) {
        if (pin->second.direction == GpioDirection::GPIO_DIRECTION_INPUT) {
            uint8_t pinRaw = pcf->getPin(pin->first);
            bool pinValue = (bool)(!(pinRaw == 1));
            switch (pin->second.action) {
                case GpioAction::BUTTON_PRESS_UP:    inputButtonUp = pinValue; break;
                case GpioAction::BUTTON_PRESS_DOWN:  inputButtonDown = pinValue; break;
                case GpioAction::BUTTON_PRESS_LEFT:  inputButtonLeft = pinValue; break;
                case GpioAction::BUTTON_PRESS_RIGHT: inputButtonRight = pinValue; break;
                case GpioAction::BUTTON_PRESS_B1:    inputButtonB1 = pinValue; break;
                case GpioAction::BUTTON_PRESS_B2:    inputButtonB2 = pinValue; break;
                case GpioAction::BUTTON_PRESS_B3:    inputButtonB3 = pinValue; break;
                case GpioAction::BUTTON_PRESS_B4:    inputButtonB4 = pinValue; break;
                case GpioAction::BUTTON_PRESS_L1:    inputButtonL1 = pinValue; break;
                case GpioAction::BUTTON_PRESS_R1:    inputButtonR1 = pinValue; break;
                case GpioAction::BUTTON_PRESS_L2:    inputButtonL2 = pinValue; break;
                case GpioAction::BUTTON_PRESS_R2:    inputButtonR2 = pinValue; break;
                case GpioAction::BUTTON_PRESS_S1:    inputButtonS1 = pinValue; break;
                case GpioAction::BUTTON_PRESS_S2:    inputButtonS2 = pinValue; break;
                case GpioAction::BUTTON_PRESS_L3:    inputButtonL3 = pinValue; break;
                case GpioAction::BUTTON_PRESS_R3:    inputButtonR3 = pinValue; break;
                case GpioAction::BUTTON_PRESS_A1:    inputButtonA1 = pinValue; break;
                case GpioAction::BUTTON_PRESS_A2:    inputButtonA2 = pinValue; break;
                case GpioAction::BUTTON_PRESS_FN:    inputButtonFN = pinValue; break;
                default:                             break;
            }
        } else if (pin->second.direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
            switch (pin->second.action) {
                case GpioAction::BUTTON_PRESS_UP:    pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_UP) == GAMEPAD_MASK_UP)); break;
                case GpioAction::BUTTON_PRESS_DOWN:  pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_DOWN) == GAMEPAD_MASK_DOWN)); break;
                case GpioAction::BUTTON_PRESS_LEFT:  pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_LEFT) == GAMEPAD_MASK_LEFT)); break;
                case GpioAction::BUTTON_PRESS_RIGHT: pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT)); break;
                case GpioAction::BUTTON_PRESS_B1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B1) == GAMEPAD_MASK_B1)); break;
                case GpioAction::BUTTON_PRESS_B2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B2) == GAMEPAD_MASK_B2)); break;
                case GpioAction::BUTTON_PRESS_B3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B3) == GAMEPAD_MASK_B3)); break;
                case GpioAction::BUTTON_PRESS_B4:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B4) == GAMEPAD_MASK_B4)); break;
                case GpioAction::BUTTON_PRESS_L1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_L1) == GAMEPAD_MASK_L1)); break;
                case GpioAction::BUTTON_PRESS_R1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_R1) == GAMEPAD_MASK_R1)); break;
                case GpioAction::BUTTON_PRESS_L2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_L2) == GAMEPAD_MASK_L2)); break;
                case GpioAction::BUTTON_PRESS_R2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_R2) == GAMEPAD_MASK_R2)); break;
                case GpioAction::BUTTON_PRESS_S1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_S1) == GAMEPAD_MASK_S1)); break;
                case GpioAction::BUTTON_PRESS_S2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_S2) == GAMEPAD_MASK_S2)); break;
                case GpioAction::BUTTON_PRESS_L3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_L3) == GAMEPAD_MASK_L3)); break;
                case GpioAction::BUTTON_PRESS_R3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_R3) == GAMEPAD_MASK_R3)); break;
                case GpioAction::BUTTON_PRESS_A1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_A1) == GAMEPAD_MASK_A1)); break;
                case GpioAction::BUTTON_PRESS_A2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_A2) == GAMEPAD_MASK_A2)); break;
                default:                             break;
            }
        } else {
            // NYI
        }
    }

    if (inputButtonUp) gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (inputButtonDown) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (inputButtonLeft) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (inputButtonRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
    if (inputButtonB1) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (inputButtonB2) gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (inputButtonB3) gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (inputButtonB4) gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (inputButtonL1) gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (inputButtonR1) gamepad->state.buttons |= GAMEPAD_MASK_R1;
    if (inputButtonL2) gamepad->state.buttons |= GAMEPAD_MASK_L2;
    if (inputButtonR2) gamepad->state.buttons |= GAMEPAD_MASK_R2;
    if (inputButtonS1) gamepad->state.buttons |= GAMEPAD_MASK_S1;
    if (inputButtonS2) gamepad->state.buttons |= GAMEPAD_MASK_S2;
    if (inputButtonL3) gamepad->state.buttons |= GAMEPAD_MASK_L3;
    if (inputButtonR3) gamepad->state.buttons |= GAMEPAD_MASK_R3;
    if (inputButtonA1) gamepad->state.buttons |= GAMEPAD_MASK_A1;
    if (inputButtonA2) gamepad->state.buttons |= GAMEPAD_MASK_A2;
}
