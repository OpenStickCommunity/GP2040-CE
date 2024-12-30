#include "addons/psx.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h" /// ??

// Should the Addon be setup and called ?
bool PsxAddon::available() {
    return Storage::getInstance().getAddonOptions().psxOptions.enabled;
}

// Called once at startup, when GP2040 or GP2040aux registers the addon
void PsxAddon::setup() {};

// Called after the gamepad is read, but before the read values are processed
void preprocess() {}

// Called after the gamepad values avec been processed
void PsxAddon::process() {};


