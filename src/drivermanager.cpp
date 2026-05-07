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
#include "drivers/xbone/XBOneDriver.h"
#include "drivers/xboxog/XboxOriginalDriver.h"
#include "drivers/xinput/XInputDriver.h"
#include "drivers/p5general/P5GeneralDriver.h"

#include "usbhostmanager.h"

#include <new>

void DriverManager::setup(InputMode mode) {
    // Use std::nothrow consistently so OOM returns nullptr (which the rest of
    // the firmware tolerates) instead of throwing through code that is not
    // exception-aware. If even the XInput fallback fails we leave inputMode
    // unchanged and the caller can decide how to recover (typically: nothing
    // happens until the next setup call).
    switch (mode) {
        case INPUT_MODE_CONFIG:
            driver = new (std::nothrow) NetDriver();
            break;
        case INPUT_MODE_ASTRO:
            driver = new (std::nothrow) AstroDriver();
            break;
        case INPUT_MODE_EGRET:
            driver = new (std::nothrow) EgretDriver();
            break;
        case INPUT_MODE_KEYBOARD:
            driver = new (std::nothrow) KeyboardDriver();
            break;
        case INPUT_MODE_GENERIC:
            driver = new (std::nothrow) HIDDriver();
            break;
        case INPUT_MODE_MDMINI:
            driver = new (std::nothrow) MDMiniDriver();
            break;
        case INPUT_MODE_NEOGEO:
            driver = new (std::nothrow) NeoGeoDriver();
            break;
        case INPUT_MODE_PSCLASSIC:
            driver = new (std::nothrow) PSClassicDriver();
            break;
        case INPUT_MODE_PCEMINI:
            driver = new (std::nothrow) PCEngineDriver();
            break;
        case INPUT_MODE_PS3:
            driver = new (std::nothrow) PS3Driver();
            break;
        case INPUT_MODE_PS4:
            driver = new (std::nothrow) PS4Driver(PS4_CONTROLLER);
            break;
        case INPUT_MODE_PS5:
            driver = new (std::nothrow) PS4Driver(PS4_ARCADESTICK);
            break;
        case INPUT_MODE_P5GENERAL:
            driver = new (std::nothrow) P5GeneralDriver();
            break;
        case INPUT_MODE_SWITCH:
            driver = new (std::nothrow) SwitchDriver();
            break;
        case INPUT_MODE_XBONE:
            driver = new (std::nothrow) XBOneDriver();
            break;
        case INPUT_MODE_XBOXORIGINAL:
            driver = new (std::nothrow) XboxOriginalDriver();
            break;
        case INPUT_MODE_XINPUT:
            driver = new (std::nothrow) XInputDriver();
            break;
        case INPUT_MODE_SWITCH_PRO:
            driver = new (std::nothrow) SwitchProDriver();
            break;
        default:
            // A corrupted protobuf or future enum value must never leave 'driver' as a
            // dangling nullptr; the TinyUSB callback layer in usbdriver.cpp dereferences
            // getDriver()->... unconditionally and would otherwise hard-fault on first
            // USB request. Fall back to XInput, the project's documented default.
            driver = new (std::nothrow) XInputDriver();
            mode = INPUT_MODE_XINPUT;
            break;
    }

    // Initialize our chosen driver. If allocation failed (nullptr), only commit
    // the new inputMode if we successfully constructed a driver -- otherwise
    // we'd advertise a mode the firmware can't actually serve.
    if (driver != nullptr) {
        driver->initialize();
        inputMode = mode;
    }
}
