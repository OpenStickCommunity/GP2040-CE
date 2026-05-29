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

	// Ready to sync Core0 and Core1
	isReady = true;
}

void GP2040Aux::run() {
	while (1) {
		addons.PreprocessAddons();

		// Run auxiliary input driver functions (e.g. P5General auth USB
		// listener servicing hash_pending round-trips to the dongle)
		// BEFORE ProcessAddons() so the auth pipeline gets a turn at the
		// start of every Core 1 iteration. Otherwise a long display I2C
		// write inside ProcessAddons() (~10 ms at 1 MHz, ~25 ms at 400 kHz)
		// stalls the auth listener, causing hash_pending to stay set and
		// delaying HID reports to the PS5 by 1-2+ frames. Drivers without
		// an auxiliary step provide a no-op processAux(), so this ordering
		// is safe for all driver modes.
		if ( inputDriver != nullptr ) {
			inputDriver->processAux();
		}

		addons.ProcessAddons();
	}
}
