// GP2040 includes
#include "gp2040aux.h"
#include "gamepad.h"

#include "storagemanager.h" // Global Managers
#include "addonmanager.h"

#include "addons/i2cdisplay.h" // Add-Ons
#include "addons/neopicoleds.h"
#include "addons/pleds.h"
#include "addons/board_led.h"
#include "addons/buzzerspeaker.h"

GP2040Aux::GP2040Aux() {
}

GP2040Aux::~GP2040Aux() {
}

void GP2040Aux::setup() {
	addons.LoadAddon(new I2CDisplayAddon(), CORE1_LOOP);
	addons.LoadAddon(new NeoPicoLEDAddon(), CORE1_LOOP);
	addons.LoadAddon(new PlayerLEDAddon(), CORE1_LOOP);
	addons.LoadAddon(new BoardLedAddon(), CORE1_LOOP);
	addons.LoadAddon(new BuzzerSpeakerAddon(), CORE1_LOOP);
}

void GP2040Aux::run() {
	while (1) {
		addons.ProcessAddons(CORE1_LOOP);
	}
}
