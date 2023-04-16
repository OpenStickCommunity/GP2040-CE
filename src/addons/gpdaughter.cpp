#include "addons/gpdaughter.h"
#include "storagemanager.h"

#include <string>

bool GPDaughterInput::available() {
    AddonOptions options = Storage::getInstance().getAddonOptions();
	return (options.GPDaughterInputEnabled &&
        options.gpDaughterSDAPin != (uint8_t)-1 &&
        options.gpDaughterSCLPin != (uint8_t)-1);
}

void GPDaughterInput::decode_adc_map(const std::string& cfg)
{
    for (size_t i = 0; (i + 1) < cfg.size(); i += 3)
    {
             if (cfg[i] == 'l' && cfg[i+1] == 'x') adc_map.push_back(GAMEPAD_MASK_LX);
        else if (cfg[i] == 'l' && cfg[i+1] == 'y') adc_map.push_back(GAMEPAD_MASK_LY);
        else if (cfg[i] == 'r' && cfg[i+1] == 'x') adc_map.push_back(GAMEPAD_MASK_RX);
        else if (cfg[i] == 'r' && cfg[i+1] == 'y') adc_map.push_back(GAMEPAD_MASK_RY);
        else if (cfg[i] == 'l' && cfg[i+1] == '2') adc_map.push_back(GAMEPAD_MASK_L2);
        else if (cfg[i] == 'r' && cfg[i+1] == '2') adc_map.push_back(GAMEPAD_MASK_R2);
        else
            break;

        // Ensure map is comma-separated
        if ((i + 2) >= cfg.size() && cfg[i+2] != ',')
            break;
    }
}

void GPDaughterInput::decode_pin_map(const std::string& cfg)
{
    for (size_t i = 0; (i + 1) < cfg.size(); i += 3)
    {
             if (cfg[i] == 'b' && cfg[i+1] == '1') pin_map.push_back(GAMEPAD_MASK_B1);
        else if (cfg[i] == 'b' && cfg[i+1] == '2') pin_map.push_back(GAMEPAD_MASK_B2);
        else if (cfg[i] == 'b' && cfg[i+1] == '3') pin_map.push_back(GAMEPAD_MASK_B3);
        else if (cfg[i] == 'b' && cfg[i+1] == '4') pin_map.push_back(GAMEPAD_MASK_B4);
        else if (cfg[i] == 'l' && cfg[i+1] == '1') pin_map.push_back(GAMEPAD_MASK_L1);
        else if (cfg[i] == 'r' && cfg[i+1] == '1') pin_map.push_back(GAMEPAD_MASK_R1);
        else if (cfg[i] == 's' && cfg[i+1] == '1') pin_map.push_back(GAMEPAD_MASK_S1);
        else if (cfg[i] == 's' && cfg[i+1] == '2') pin_map.push_back(GAMEPAD_MASK_S2);
        else if (cfg[i] == 'l' && cfg[i+1] == '3') pin_map.push_back(GAMEPAD_MASK_L3);
        else if (cfg[i] == 'r' && cfg[i+1] == '3') pin_map.push_back(GAMEPAD_MASK_R3);
        else if (cfg[i] == 'a' && cfg[i+1] == '1') pin_map.push_back(GAMEPAD_MASK_A1);
        else if (cfg[i] == 'a' && cfg[i+1] == '2') pin_map.push_back(GAMEPAD_MASK_A2);
        else if (cfg[i] == 'd' && cfg[i+1] == 'u') pin_map.push_back(GAMEPAD_MASK_DU);
        else if (cfg[i] == 'd' && cfg[i+1] == 'd') pin_map.push_back(GAMEPAD_MASK_DD);
        else if (cfg[i] == 'd' && cfg[i+1] == 'l') pin_map.push_back(GAMEPAD_MASK_DL);
        else if (cfg[i] == 'd' && cfg[i+1] == 'r') pin_map.push_back(GAMEPAD_MASK_DR);
        else
            break;

        // Ensure map is comma-separated
        if ((i + 2) >= cfg.size() && cfg[i+2] != ',')
            break;
    }
}

void GPDaughterInput::setup() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    AddonOptions options = Storage::getInstance().getAddonOptions();

    gamepad->hasAnalogTriggers = true;
    gamepad->hasLeftAnalogStick = true;
    gamepad->hasRightAnalogStick = true;

    const std::string adc_str = options.gpDaughterADCMap.data();
    const std::string pin_str = options.gpDaughterPinMap.data();

    decode_adc_map(adc_str);
    decode_pin_map(pin_str);

    uIntervalMS = 1;
    nextTimer = getMillis();

    // Init the daughter board library
    daughter = new GPDaughter(1,
        options.gpDaughterSDAPin,
        options.gpDaughterSCLPin,
        options.gpDaughterBlock == 0 ? i2c0 : i2c1,
        options.gpDaughterSpeed,
        options.gpDaughterAddress);
    daughter->begin();
}

void GPDaughterInput::process()
{
    if (nextTimer < getMillis())
    {
        daughter->read();
        nextTimer = getMillis() + uIntervalMS; // throttle down updates
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    unsigned int channel;

    for (channel = 0; channel < adc_map.size(); channel++)
    {
        const auto adc_value = daughter->get_value(channel);

        switch (adc_map[channel])
        {
        case GAMEPAD_MASK_LX: gamepad->state.lx = adc_value; break;
        case GAMEPAD_MASK_LY: gamepad->state.ly = adc_value; break;
        case GAMEPAD_MASK_RX: gamepad->state.rx = adc_value; break;
        case GAMEPAD_MASK_RY: gamepad->state.ry = adc_value; break;
        case GAMEPAD_MASK_L2: gamepad->state.lt = (uint8_t)(adc_value / 256); break;
        case GAMEPAD_MASK_R2: gamepad->state.rt = (uint8_t)(adc_value / 256); break;
        }
    }

    unsigned int pin = 0;

    while (pin < pin_map.size())
    {
        const auto pin_channel = channel + (pin / 16);
        const auto value = daughter->get_value(pin_channel);

        for (unsigned int pin_mask = 1;
            pin_mask != 0 && pin < pin_map.size();
            pin_mask <<= 1, pin++)
        {
            const auto pin_value = (value & pin_mask) != 0;
            auto state_mask = pin_map[pin];

            switch(state_mask)
            {
            case GAMEPAD_MASK_B1:
            case GAMEPAD_MASK_B2:
            case GAMEPAD_MASK_B3:
            case GAMEPAD_MASK_B4:
            case GAMEPAD_MASK_L1:
            case GAMEPAD_MASK_R1:
            case GAMEPAD_MASK_S1:
            case GAMEPAD_MASK_S2:
            case GAMEPAD_MASK_L3:
            case GAMEPAD_MASK_R3:
            case GAMEPAD_MASK_A1:
            case GAMEPAD_MASK_A2:
                if (pin_value)
                    gamepad->state.buttons |= (uint16_t)state_mask;
                else
                    gamepad->state.buttons &= (uint16_t)~state_mask;
                break;
            case GAMEPAD_MASK_DU:
            case GAMEPAD_MASK_DD:
            case GAMEPAD_MASK_DL:
            case GAMEPAD_MASK_DR:
                state_mask >>= 16; // Convert the representation back
                if (pin_value)
                    gamepad->state.dpad |= (uint8_t)state_mask;
                else
                    gamepad->state.dpad &= (uint8_t)~state_mask;
                break;
            default:
                continue;
            }
        }
    }
}
