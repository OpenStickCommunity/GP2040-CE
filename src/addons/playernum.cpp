#include "addons/playernum.h"
#include "storagemanager.h"
#include "system.h"

bool PlayerNumAddon::available() {
    AddonOptions options = Storage::getInstance().getAddonOptions();
    return options.PlayerNumAddonEnabled;
}

void PlayerNumAddon::setup() {
    AddonOptions options = Storage::getInstance().getAddonOptions();
    xinputIDs[0] = XINPUT_PLED_ON1;
    xinputIDs[1] = XINPUT_PLED_ON2;
    xinputIDs[2] = XINPUT_PLED_ON3;
    xinputIDs[3] = XINPUT_PLED_ON4;
    playerNum = options.playerNumber;
    if ( playerNum < 1 || playerNum > 4 ) {
        playerNum = 1; // error checking, set to 1 if we're off
    }
    assigned = 0; // what player ID did we get assigned to
}

void PlayerNumAddon::process()
{
    if ( assigned == 0 ) {
        Gamepad * gamepad = Storage::getInstance().GetGamepad();
        InputMode inputMode = gamepad->options.inputMode;
        if ( inputMode == INPUT_MODE_XINPUT ) {
            uint8_t * featureData = Storage::getInstance().GetFeatureData();
            if (featureData[0] == 0x01) {
                if ( (featureData[2] != xinputIDs[playerNum-1]) ) {
                    sleep_ms(1000 * (playerNum-1)); // Wait Player_Num * 1-second for reboot
                    System::reboot(System::BootMode::GAMEPAD);
                } else {
                    assigned = 1;
                }
            }
        }
    }
}
