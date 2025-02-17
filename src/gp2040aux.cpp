// GP2040 includes
#include "gp2040aux.h"
#include "gamepad.h"

#include "drivermanager.h"
#include "storagemanager.h"
#include "usbhostmanager.h"

#include "addons/board_led.h"  // Add-Ons
#include "addons/buzzerspeaker.h"
#include "addons/display.h"
#include "addons/pleds.h"
#include "addons/neopicoleds.h"
#include "addons/reactiveleds.h"
#include "addons/drv8833_rumble.h"

#include <iterator>

GP2040Aux::GP2040Aux() : isReady(false), inputDriver(nullptr) {
}

GP2040Aux::~GP2040Aux() {
}

// GP2040Aux will always come after GP2040 setup(), so we can rely on the
// GP2040 setup function for certain setup functions.
void GP2040Aux::setup() {
	PeripheralManager::getInstance().initI2C();
	PeripheralManager::getInstance().initSPI();
	PeripheralManager::getInstance().initUSB();

	// Initialize our input driver's auxilliary functions
	inputDriver = DriverManager::getInstance().getDriver();
	if ( inputDriver != nullptr ) {
		inputDriver->initializeAux();

		// Check if we have a USB listener
		USBListener * listener = inputDriver->get_usb_auth_listener();
		if (listener != nullptr) {
			USBHostManager::getInstance().pushListener(listener);
		}
	}

	// Setup Add-ons
	addons.LoadAddon(new DisplayAddon());
	addons.LoadAddon(new NeoPicoLEDAddon());
	addons.LoadAddon(new PlayerLEDAddon());
	addons.LoadAddon(new BoardLedAddon());
	addons.LoadAddon(new BuzzerSpeakerAddon());
	addons.LoadAddon(new DRV8833RumbleAddon());
	addons.LoadAddon(new ReactiveLEDAddon());

	// Initialize our USB manager
	USBHostManager::getInstance().start();

	// Ready to sync Core0 and Core1
	isReady = true;
}

void GP2040Aux::run() {
	while (1) {
		// Pre, Process, and Post
		addons.PreprocessAddons();
		addons.ProcessAddons();

		// Run auxiliary functions for input driver on Core1
		if ( inputDriver != nullptr ) {
			inputDriver->processAux();
		}
	}
}
