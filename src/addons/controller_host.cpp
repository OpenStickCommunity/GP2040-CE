#include "addons/controller_host.h"
#include "addons/controller_host_listener.h"
#include "storagemanager.h"
#include "drivermanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"
#include "class/hid/hid_host.h"


bool ControllerHostAddon::available()
{
    const ControllerHostOptions& controllerHostOptions = Storage::getInstance().getAddonOptions().controllerHostOptions;
	return controllerHostOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void ControllerHostAddon::setup()
{
    // stdio_init_all();
    // printf("setup function.");
    listener = new ControllerHostListener();
    ((ControllerHostListener*)listener)->setup();
}

void ControllerHostAddon::preprocess() {
  ((ControllerHostListener*)listener)->process();
}