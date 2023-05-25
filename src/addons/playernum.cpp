#include "addons/playernum.h"
#include "storagemanager.h"
#include "system.h"
#include "helper.h"
#include "config.pb.h"

bool PlayerNumAddon::available() {
    return Storage::getInstance().getAddonOptions().playerNumberOptions.enabled;
}

void PlayerNumAddon::setup() {
    const PlayerNumberOptions& options = Storage::getInstance().getAddonOptions().playerNumberOptions;
    xinputIDs[0] = XINPUT_PLED_ON1;
    xinputIDs[1] = XINPUT_PLED_ON2;
    xinputIDs[2] = XINPUT_PLED_ON3;
    xinputIDs[3] = XINPUT_PLED_ON4;
    playerNum = options.number;
    if ( playerNum < 1 || playerNum > 4 ) {
        playerNum = 1; // error checking, set to 1 if we're off
    }
    assigned = 0; // what player ID did we get assigned to
}

void PlayerNumAddon::process()
{
    if ( assigned == 0 ) {
        Gamepad * gamepad = Storage::getInstance().GetGamepad();
        InputMode inputMode = static_cast<InputMode>(gamepad->getOptions().inputMode);
        if ( inputMode == INPUT_MODE_XINPUT ) {
            uint8_t * featureData = Storage::getInstance().GetFeatureData();
            if (featureData[0] == 0x01) {
                XInputPLEDPattern ledAction = (XInputPLEDPattern)featureData[2];
                if ( ledAction == XINPUT_PLED_ON1 )
                    handleLED(1);
                else if ( ledAction == XINPUT_PLED_ON2 )
                    handleLED(2);
                else if ( ledAction == XINPUT_PLED_ON3 )
                    handleLED(3);
                else if ( ledAction == XINPUT_PLED_ON4 )
                    handleLED(4);
            }
        } else {
            assigned = 1;
        }
    }
}

void PlayerNumAddon::handleLED(int num) {
    if ( playerNum != num ) {
        tud_disconnect();
        sleep_ms(2000 * playerNum);
        System::reboot(System::BootMode::GAMEPAD);
    } else {
        assigned = 1;
    }
}
