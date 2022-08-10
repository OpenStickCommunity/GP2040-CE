// GP2040 includes
#include "gp2040aux.h"
#include "gamepad.h"
#include "storagemanager.h" // Managers
#include "addons/i2cdisplay.h" // Add-Ons
#include "addons/neopicoleds.h"
#include "addons/playerleds.h"

#include <iterator>

GP2040Aux::GP2040Aux() {
}

GP2040Aux::~GP2040Aux() {
}

void GP2040Aux::setup() {
	setupAddon(new I2CDisplayAddon());
	setupAddon(new NeoPicoLEDAddon());
	setupAddon(new PlayerLEDAddon());
}

void GP2040Aux::run() {
	while (1) {
		for (std::vector<GPAddon*>::iterator it = Storage::getInstance().Addons.begin(); it != Storage::getInstance().Addons.end(); it++) {
			(*it)->process();
		}
	}
}

void GP2040Aux::setupAddon(GPAddon* addon) {
	if (addon->available()) {
		addon->setup();
		Storage::getInstance().Addons.push_back(addon);
	}
}
