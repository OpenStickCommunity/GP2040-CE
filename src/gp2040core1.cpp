// GP2040 includes
#include "gp2040core1.h"
#include "gamepad.h"
#include "storage.h"

#include <iterator>

GP2040Core1::GP2040Core1() {
}

GP2040Core1::~GP2040Core1() {
}

void GP2040Core1::setup() {
	I2CDisplayModule * i2c = new I2CDisplayModule();
	setupModule(i2c);
	NeoPicoLEDModule * neopico = new NeoPicoLEDModule();
	setupModule(neopico);
	PLEDModule * pled = new PLEDModule();
	setupModule(pled);
}

void GP2040Core1::run() {
	bool configMode = Storage::getInstance().GetConfigMode();
	if(configMode == true) {
		configLoop();
	} else {
		loop();
	}
}

void GP2040Core1::configLoop() {
	Gamepad * gamepad;
	while(1) { // Note: this will change when we display config on the I2C
		gamepad = Storage::getInstance().GetGamepad();
		for (std::vector<GPModule*>::iterator it = Storage::getInstance().Modules.begin(); it != Storage::getInstance().Modules.end(); it++) {
			(*it)->process(gamepad);
			(*it)->loop();
		}
	}
}

void GP2040Core1::loop() {	
	Gamepad * gamepad;
	while (1) {
		gamepad = Storage::getInstance().GetGamepad();
		for (std::vector<GPModule*>::iterator it = Storage::getInstance().Modules.begin(); it != Storage::getInstance().Modules.end(); it++) {
			(*it)->process(gamepad);
			(*it)->loop();
		}
	}
}

void GP2040Core1::setupModule(GPModule* module) {
	if (module->available()) {
		module->setup();
		Storage::getInstance().Modules.push_back(module);
	}
}
