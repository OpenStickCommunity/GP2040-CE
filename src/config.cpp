#include "config.h"
#include "configs/WebConfig.h"
#include "modules/neopicoleds.h"

void ConfigManager::setup(ConfigType config) {
	switch(config) {
		case CONFIG_TYPE_WEB:
			setupConfig(new WebConfig());
			break;
	}
    this->cType = config;
}

void ConfigManager::loop() {
    config->loop();
}

void ConfigManager::setupConfig(GPConfig * gpconfig) {
    gpconfig->setup();
    this->config = gpconfig;
}

void ConfigManager::setGamepadOptions(Gamepad* gamepad) {
	gamepad->save();
}

void ConfigManager::setLedOptions(LEDOptions * ledoptions) {
	Storage::getInstance().setLEDOptions(*ledoptions);

    // Configure LEDs in the neopico
	for (std::vector<GPModule*>::iterator it = Storage::getInstance().Modules.begin(); it != Storage::getInstance().Modules.end(); it++) {
		if ((*it)->name() == NeoPicoLEDName) {
			NeoPicoLEDModule * neopicoled = (NeoPicoLEDModule*)(*it);
			neopicoled->configureLEDs();
			break; // trigger LED configuration
		}
	} 
}

void ConfigManager::setBoardOptions(BoardOptions * boardoptions) {
	Storage::getInstance().setBoardOptions(*boardoptions);

	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->mapDpadUp->setPin(boardoptions->pinDpadUp);
	gamepad->mapDpadDown->setPin(boardoptions->pinDpadDown);
	gamepad->mapDpadLeft->setPin(boardoptions->pinDpadLeft);
	gamepad->mapDpadRight->setPin(boardoptions->pinDpadRight);
	gamepad->mapButtonB1->setPin(boardoptions->pinButtonB1);
	gamepad->mapButtonB2->setPin(boardoptions->pinButtonB2);
	gamepad->mapButtonB3->setPin(boardoptions->pinButtonB3);
	gamepad->mapButtonB4->setPin(boardoptions->pinButtonB4);
	gamepad->mapButtonL1->setPin(boardoptions->pinButtonL1);
	gamepad->mapButtonR1->setPin(boardoptions->pinButtonR1);
	gamepad->mapButtonL2->setPin(boardoptions->pinButtonL2);
	gamepad->mapButtonR2->setPin(boardoptions->pinButtonR2);
	gamepad->mapButtonS1->setPin(boardoptions->pinButtonS1);
	gamepad->mapButtonS2->setPin(boardoptions->pinButtonS2);
	gamepad->mapButtonL3->setPin(boardoptions->pinButtonL3);
	gamepad->mapButtonR3->setPin(boardoptions->pinButtonR3);
	gamepad->mapButtonA1->setPin(boardoptions->pinButtonA1);
	gamepad->mapButtonA2->setPin(boardoptions->pinButtonA2);

	GamepadStore.save();
}
