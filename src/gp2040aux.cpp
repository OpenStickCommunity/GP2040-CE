// GP2040 includes
#include "gp2040aux.h"
#include "gamepad.h"

#include "storagemanager.h" // Global Managers
#include "addonmanager.h"
#include "usbhostmanager.h"

#include "addons/board_led.h"
#include "addons/buzzerspeaker.h"
#include "addons/i2cdisplay.h" // Add-Ons
#include "addons/pleds.h"
#include "addons/ps4mode.h"
#include "addons/pspassthrough.h"
#include "addons/neopicoleds.h"
#include "addons/inputhistory.h"
#include "addons/xbonepassthrough.h"

#include <iterator>

GP2040Aux::GP2040Aux() {
}

GP2040Aux::~GP2040Aux() {
}

void GP2040Aux::setup() {
	InputHistoryAddon* inputHistoryAddon = new InputHistoryAddon();
	I2CDisplayAddon* i2CDisplayAddon = new I2CDisplayAddon();

	if(inputHistoryAddon->available() && i2CDisplayAddon->available())
		i2CDisplayAddon->attachInputHistoryAddon(inputHistoryAddon);

	// Setup Add-ons
	addons.LoadUSBAddon(new PSPassthroughAddon(), CORE1_LOOP);
	addons.LoadUSBAddon(new XBOnePassthroughAddon(), CORE1_LOOP);
	addons.LoadAddon(inputHistoryAddon, CORE1_LOOP);
	addons.LoadAddon(i2CDisplayAddon, CORE1_LOOP);
	addons.LoadAddon(new NeoPicoLEDAddon(), CORE1_LOOP);
	addons.LoadAddon(new PlayerLEDAddon(), CORE1_LOOP);
	addons.LoadAddon(new BoardLedAddon(), CORE1_LOOP);
	addons.LoadAddon(new BuzzerSpeakerAddon(), CORE1_LOOP);
	addons.LoadAddon(new PS4ModeAddon(), CORE1_LOOP);

	// Initialize our USB manager
	USBHostManager::getInstance().start();
}

void GP2040Aux::run() {
	while (1) {
		addons.ProcessAddons(CORE1_LOOP);
	}
}
