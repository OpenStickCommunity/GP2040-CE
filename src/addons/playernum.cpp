#include "addons/playernum.h"
#include "storagemanager.h"
#include "system.h"
#include "helper.h"
#include "config.pb.h"
#include "device/usbd.h"

// Move to Proto Enums
typedef enum
{
	XINPUT_PLED_OFF       = 0x00, // All off
	XINPUT_PLED_BLINKALL  = 0x01, // All blinking
	XINPUT_PLED_FLASH1    = 0x02, // 1 flashes, then on
	XINPUT_PLED_FLASH2    = 0x03, // 2 flashes, then on
	XINPUT_PLED_FLASH3    = 0x04, // 3 flashes, then on
	XINPUT_PLED_FLASH4    = 0x05, // 4 flashes, then on
	XINPUT_PLED_ON1       = 0x06, // 1 on
	XINPUT_PLED_ON2       = 0x07, // 2 on
	XINPUT_PLED_ON3       = 0x08, // 3 on
	XINPUT_PLED_ON4       = 0x09, // 4 on
	XINPUT_PLED_ROTATE    = 0x0A, // Rotating (e.g. 1-2-4-3)
	XINPUT_PLED_BLINK     = 0x0B, // Blinking*
	XINPUT_PLED_SLOWBLINK = 0x0C, // Slow blinking*
	XINPUT_PLED_ALTERNATE = 0x0D, // Alternating (e.g. 1+4-2+3), then back to previous*
} XInputPLEDPattern;

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
            if (gamepad->auxState.playerID.enabled && gamepad->auxState.playerID.active) {
                if ( gamepad->auxState.playerID.value != 0 )
                    handleLED(gamepad->auxState.playerID.value);
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
