// GP2040 includes
#include "gp2040.h"
#include "helper.h"
#include "modules/pleds.h"
#include "storage.h"
#include "usb_driver.h"

// Pico includes
#include "pico/bootrom.h"
#include "pico/util/queue.h"

// TinyUSB
#include "tusb.h"

#define GAMEPAD_DEBOUNCE_MILLIS 5 // make this a class object

GP2040::GP2040() : nextRuntime(0) {
	Storage::getInstance().SetGamepad(new Gamepad(GAMEPAD_DEBOUNCE_MILLIS));
}

GP2040::~GP2040() {
}

void GP2040::setup() {	
    // Setup Gamepad and Gamepad Storage
	GamepadStore.start();
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->setup();

	// Initialize Cross-Core Communications
	queue_init(Storage::getInstance().GetGamepadQueue(), sizeof(Gamepad), 1);
	queue_init(Storage::getInstance().GetFeatureQueue(), PLED_REPORT_SIZE, 20);
	
	// Check for Config or Regular Input (w/ Button Combos)
	InputMode inputMode = gamepad->options.inputMode;
	gamepad->read();
	if (gamepad->pressedF1() && gamepad->pressedUp()) {
		reset_usb_boot(0, 0); // RESET everything
	} else if (gamepad->pressedS2()) { // Config Mode
		Storage::getInstance().SetConfigMode(true);
		inputMode = INPUT_MODE_CONFIG; // force config
        initialize_driver(inputMode);
		ConfigManager::getInstance().setup(CONFIG_TYPE_WEB);
	} else { // Gamepad Mode
		Storage::getInstance().SetConfigMode(false);
		if (gamepad->pressedB3())
			inputMode = INPUT_MODE_HID;
		else if (gamepad->pressedB1())
			inputMode = INPUT_MODE_SWITCH;
		else if (gamepad->pressedB2())
			inputMode = INPUT_MODE_XINPUT;
		else if (gamepad->pressedF1() && gamepad->pressedUp())
			reset_usb_boot(0, 0); // RESET everything
		// Save gamepad option if we selected something new
		if (inputMode != gamepad->options.inputMode ) {
			gamepad->options.inputMode = inputMode;
			gamepad->save();
		}
		initialize_driver(inputMode);
	}
}

void GP2040::run() {
	bool configMode = Storage::getInstance().GetConfigMode();
	if(configMode == true) { 	// Configuration Mode
		configLoop();
	} else {					// Standard Gamepad
		loop();
	}
}

void GP2040::loop() {
	Gamepad * gamepad;
	while (1) {
		if (nextRuntime > getMicro()) { // fix for unsigned
			sleep_us(50); // Give some time back to our CPU (lower power consumption)
			continue;
		}
		gamepad = Storage::getInstance().GetGamepad();
		// gpio reads
		gamepad->read();
	#if GAMEPAD_DEBOUNCE_MILLIS > 0
		gamepad->debounce();
	#endif
		gamepad->hotkey();
		gamepad->process(); // send to USB controller host

		// Get Player LED features on USB
		send_report(gamepad->getReport(), gamepad->getReportSize());
		memset(featureData, 0, sizeof(featureData));
		receive_report(featureData);
		tud_task();

		// Setup Queue for our Module Data (Player LEDs)
		if (featureData[0])
			queue_try_add(Storage::getInstance().GetFeatureQueue(), featureData);
		
		// Setup Queue for our Module Data (Gamepad Input)
		if (queue_is_empty(Storage::getInstance().GetGamepadQueue())) {
			memcpy(&snapshot, gamepad, sizeof(Gamepad));
			queue_try_add(Storage::getInstance().GetGamepadQueue(), &snapshot);
		}

		nextRuntime = getMicro() + GAMEPAD_POLL_MICRO;
	}
}

void GP2040::configLoop() {
	while(1) {
		ConfigManager::getInstance().loop();
	}
}
