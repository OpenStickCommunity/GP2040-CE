#include "addons/sidewindergp.h"
#include "storagemanager.h"


uint32_t SidewinderGPInputMapping[14] = {
    GAMEPAD_MASK_DU,
    GAMEPAD_MASK_DD,
    GAMEPAD_MASK_DR,
    GAMEPAD_MASK_DL,
    GAMEPAD_MASK_B1,
    GAMEPAD_MASK_B2,
    GAMEPAD_MASK_L3,
    GAMEPAD_MASK_B3,
    GAMEPAD_MASK_B4,
    GAMEPAD_MASK_R3,
    GAMEPAD_MASK_L1,
    GAMEPAD_MASK_R1,
    GAMEPAD_MASK_S2,
    GAMEPAD_MASK_S1
};

bool SidewinderGPInput::available() {
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
	sidewinderGPPinTrigger = Storage::getInstance().getAddonOptions().sidewinderGPPinTrigger;
	sidewinderGPPinClock = Storage::getInstance().getAddonOptions().sidewinderGPPinClock;
	sidewinderGPPinData = Storage::getInstance().getAddonOptions().sidewinderGPPinData;
    return options.SidewinderGPEnabled;
}

void SidewinderGPInput::setup() {
    gpio_init(sidewinderGPPinTrigger);
    gpio_set_dir(sidewinderGPPinTrigger, GPIO_OUT);
    gpio_put(sidewinderGPPinTrigger, 1);

    gpio_init(sidewinderGPPinClock);
    gpio_set_dir(sidewinderGPPinClock, GPIO_IN);
    gpio_pull_up(sidewinderGPPinClock);

    gpio_init(sidewinderGPPinData);
    gpio_set_dir(sidewinderGPPinData, GPIO_IN);
    gpio_pull_up(sidewinderGPPinData);
}

void SidewinderGPInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    gpio_put(sidewinderGPPinTrigger, 0);
    sleep_us(225);
    gpio_put(sidewinderGPPinTrigger, 1);

    bool btns[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    bool checksum = 0;
    bool ok = 0;

    for (int i=0; i < 15; i++) {
        // Wait for clock falling edge
        uint16_t timeout = 10000;
        while (timeout-- && !gpio_get(sidewinderGPPinClock)) {}
        while (timeout-- && gpio_get(sidewinderGPPinClock)) {}
        if (!timeout)
            break;
        if (i < 14) {
            btns[i] = !gpio_get(sidewinderGPPinData);
            checksum ^= btns[i];
        } else {
            ok = (checksum == gpio_get(sidewinderGPPinData));
        }
    }
    if (!ok)
        return;

    gamepad->state.lx = 0x8000;
    gamepad->state.ly = 0x8000;
    for (int i=0; i < 14; i++) {
        if (!btns[i])
            continue;
        uint32_t btnMap = SidewinderGPInputMapping[i];
        if (btnMap > (GAMEPAD_MASK_A2)) {
            switch (btnMap) {
                case (GAMEPAD_MASK_DU):
                    gamepad->state.ly = 0x0;
                    break;
                case (GAMEPAD_MASK_DD):
                    gamepad->state.ly = 0xffff;
                    break;
                case (GAMEPAD_MASK_DL):
                    gamepad->state.lx = 0x0;
                    break;
                case (GAMEPAD_MASK_DR):
                    gamepad->state.lx = 0xffff;
                    break;
            }
        }
        else gamepad->state.buttons |= btnMap;
    }
}
