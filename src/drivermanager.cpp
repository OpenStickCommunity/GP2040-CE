#include "drivermanager.h"

#include "drivers/net/NetDriver.h"
#include "drivers/astro/AstroDriver.h"
#include "drivers/egret/EgretDriver.h"
#include "drivers/hid/HIDDriver.h"
#include "drivers/keyboard/KeyboardDriver.h"
#include "drivers/mdmini/MDMiniDriver.h"
#include "drivers/neogeo/NeogeoDriver.h"
#include "drivers/pcengine/PCEngineDriver.h"
#include "drivers/ps4/PS4Driver.h"
#include "drivers/switch/SwitchDriver.h"
#include "drivers/xbone/XBOneDriver.h"
#include "drivers/xboxog/XboxOriginalDriver.h"

void DriverManager::setup(InputMode mode) {
    switch (mode) {
        case INPUT_MODE_CONFIG:
            driver = new NetDriver(); 
            break;
        case INPUT_MODE_ASTRO:
            driver = new AstroDriver();
            break;
        case INPUT_MODE_EGRET:
            driver = new EgretDriver();
            break;
        case INPUT_MODE_HID:
            driver = new HIDDriver();
            break;
        case INPUT_MODE_KEYBOARD:
            driver = new KeyboardDriver();
            break;
        case INPUT_MODE_MDMINI:
            driver = new MDMiniDriver();
            break;
        case INPUT_MODE_NEOGEO:
            driver = new NeoGeoDriver();
            break;
        case INPUT_MODE_PCEMINI:
            driver = new PCEngineDriver();
            break;
        case INPUT_MODE_PS4:
            driver = new PS4Driver();
            break;
        case INPUT_MODE_SWITCH:
            driver = new SwitchDriver();
            break;
        case INPUT_MODE_XBONE:
            driver = new XBOneDriver();
            break;
        case INPUT_MODE_XBOXORIGINAL:
            driver = new XboxOriginalDriver();
            break;
        default:
            return;
    }
    driver->initialize();
}
