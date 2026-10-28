#include "drivermanager.h"

#include "drivers/net/NetDriver.h"
#include "drivers/astro/AstroDriver.h"
#include "drivers/egret/EgretDriver.h"
#include "drivers/hid/HIDDriver.h"
#include "drivers/keyboard/KeyboardDriver.h"
#include "drivers/mdmini/MDMiniDriver.h"
#include "drivers/neogeo/NeoGeoDriver.h"
#include "drivers/pcengine/PCEngineDriver.h"
#include "drivers/psclassic/PSClassicDriver.h"
#include "drivers/ps3/PS3Driver.h"
#include "drivers/ps4/PS4Driver.h"
#include "drivers/switch/SwitchDriver.h"
#include "drivers/switchpro/SwitchProDriver.h"
#include "drivers/switch2pro/Switch2ProDriver.h"
#include "drivers/xbone/XBOneDriver.h"
#include "drivers/xboxog/XboxOriginalDriver.h"
#include "drivers/xinput/XInputDriver.h"
#include "drivers/p5general/P5GeneralDriver.h"
#include "drivers/sinput/SInputDriver.h"

#include "usbhostmanager.h"
#include "storagemanager.h"

typedef GPDriver * (*DriverFactory)();

// The driver of each input mode. setup() and hasDriver() share it, so a
// mode added here needs nothing else.
static DriverFactory driverFactory(InputMode mode) {
    switch (mode) {
        case INPUT_MODE_CONFIG:
            return []() -> GPDriver * { return new NetDriver(); };
        case INPUT_MODE_ASTRO:
            return []() -> GPDriver * { return new AstroDriver(); };
        case INPUT_MODE_EGRET:
            return []() -> GPDriver * { return new EgretDriver(); };
        case INPUT_MODE_KEYBOARD:
            return []() -> GPDriver * { return new KeyboardDriver(); };
        case INPUT_MODE_GENERIC:
            return []() -> GPDriver * { return new HIDDriver(); };
        case INPUT_MODE_MDMINI:
            return []() -> GPDriver * { return new MDMiniDriver(); };
        case INPUT_MODE_NEOGEO:
            return []() -> GPDriver * { return new NeoGeoDriver(); };
        case INPUT_MODE_PSCLASSIC:
            return []() -> GPDriver * { return new PSClassicDriver(); };
        case INPUT_MODE_PCEMINI:
            return []() -> GPDriver * { return new PCEngineDriver(); };
        case INPUT_MODE_PS3:
            return []() -> GPDriver * { return new PS3Driver(); };
        case INPUT_MODE_PS4:
            return []() -> GPDriver * { return new PS4Driver(PS4_CONTROLLER); };
        case INPUT_MODE_PS5:
            return []() -> GPDriver * { return new PS4Driver(PS4_ARCADESTICK); };
        case INPUT_MODE_P5GENERAL:
            return []() -> GPDriver * { return new P5GeneralDriver(); };
        case INPUT_MODE_SWITCH:
            return []() -> GPDriver * { return new SwitchDriver(); };
        case INPUT_MODE_XBONE:
            return []() -> GPDriver * { return new XBOneDriver(); };
        case INPUT_MODE_XBOXORIGINAL:
            return []() -> GPDriver * { return new XboxOriginalDriver(); };
        case INPUT_MODE_XINPUT:
            return []() -> GPDriver * { return new XInputDriver(); };
        case INPUT_MODE_SWITCH_PRO:
            return []() -> GPDriver * {
                switch (Storage::getInstance().getGamepadOptions().switchProControllerType) {
                    case SWITCH_PRO_CONTROLLER_TYPE_SNES:
                        return new SwitchProDriver(SWITCH_TYPE_SNES);
                    case SWITCH_PRO_CONTROLLER_TYPE_SUPER_FAMICOM:
                        return new SwitchProDriver(SWITCH_TYPE_SNES, SWITCH_REGION_JAPAN);
                    case SWITCH_PRO_CONTROLLER_TYPE_N64:
                        return new SwitchProDriver(SWITCH_TYPE_N64);
                    case SWITCH_PRO_CONTROLLER_TYPE_GENESIS:
                        return new SwitchProDriver(SWITCH_TYPE_GENESIS);
                    case SWITCH_PRO_CONTROLLER_TYPE_MEGA_DRIVE:
                        return new SwitchProDriver(SWITCH_TYPE_GENESIS, SWITCH_REGION_JAPAN);
                    case SWITCH_PRO_CONTROLLER_TYPE_NES:
                        return new SwitchProDriver(SWITCH_TYPE_NES_LEFT_JOYCON);
                    case SWITCH_PRO_CONTROLLER_TYPE_FAMICOM_I:
                        return new SwitchProDriver(SWITCH_TYPE_FAMICOM_LEFT_JOYCON);
                    case SWITCH_PRO_CONTROLLER_TYPE_FAMICOM_II:
                        return new SwitchProDriver(SWITCH_TYPE_FAMICOM_RIGHT_JOYCON);
                    case SWITCH_PRO_CONTROLLER_TYPE_JOYCON_LEFT:
                        return new SwitchProDriver(SWITCH_TYPE_LEFT_JOYCON);
                    case SWITCH_PRO_CONTROLLER_TYPE_JOYCON_RIGHT:
                        return new SwitchProDriver(SWITCH_TYPE_RIGHT_JOYCON);
                    case SWITCH_PRO_CONTROLLER_TYPE_JOYCON_PAIR:
                        return new SwitchProDriver(SWITCH_TYPE_LEFT_JOYCON, SWITCH_REGION_USA, true);
                    default:
                        return new SwitchProDriver();
                }
            };
        case INPUT_MODE_SWITCH2_PRO:
            return []() -> GPDriver * { return new Switch2ProDriver(); };
        case INPUT_MODE_SINPUT:
            return []() -> GPDriver * { return new SInputDriver(); };
        default:
            return nullptr;
    }
}

void DriverManager::setup(InputMode mode) {
    DriverFactory factory = driverFactory(mode);
    if (factory == nullptr)
        return;
    driver = factory();

    // Initialize our chosen driver
    driver->initialize();
    inputMode = mode;
}

bool DriverManager::hasDriver(InputMode mode) {
    return driverFactory(mode) != nullptr;
}
