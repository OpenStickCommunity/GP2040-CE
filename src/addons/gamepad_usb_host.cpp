#include "addons/gamepad_usb_host.h"
#include "addons/gamepad_usb_host_listener.h"
#include "storagemanager.h"
#include "drivermanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"
#include "class/hid/hid_host.h"

bool GamepadUSBHostAddon::available()
{
    const GamepadUSBHostOptions& gamepadUSBHostOptions = Storage::getInstance().getAddonOptions().gamepadUSBHostOptions;
    return gamepadUSBHostOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void GamepadUSBHostAddon::setup()
{
    //stdio_init_all();
    //printf("setup function\n");
    listener = new GamepadUSBHostListener();
    ((GamepadUSBHostListener*)listener)->setup();
}

void GamepadUSBHostAddon::preprocess() {
    ((GamepadUSBHostListener*)listener)->process();
}