#include "configmanager.h"

#include "addonmanager.h"
#include "configs/webconfig.h"
#include "addons/neopicoleds.h"

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

void ConfigManager::setLedOptions(LEDOptions ledOptions) {
	Storage::getInstance().setLEDOptions(ledOptions);
}

void ConfigManager::setBoardOptions(BoardOptions boardOptions) {
	Storage::getInstance().setBoardOptions(boardOptions);

	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->mapDpadUp->setPin(boardOptions.pinDpadUp);
	gamepad->mapDpadDown->setPin(boardOptions.pinDpadDown);
	gamepad->mapDpadLeft->setPin(boardOptions.pinDpadLeft);
	gamepad->mapDpadRight->setPin(boardOptions.pinDpadRight);
	gamepad->mapButtonB1->setPin(boardOptions.pinButtonB1);
	gamepad->mapButtonB2->setPin(boardOptions.pinButtonB2);
	gamepad->mapButtonB3->setPin(boardOptions.pinButtonB3);
	gamepad->mapButtonB4->setPin(boardOptions.pinButtonB4);
	gamepad->mapButtonL1->setPin(boardOptions.pinButtonL1);
	gamepad->mapButtonR1->setPin(boardOptions.pinButtonR1);
	gamepad->mapButtonL2->setPin(boardOptions.pinButtonL2);
	gamepad->mapButtonR2->setPin(boardOptions.pinButtonR2);
	gamepad->mapButtonS1->setPin(boardOptions.pinButtonS1);
	gamepad->mapButtonS2->setPin(boardOptions.pinButtonS2);
	gamepad->mapButtonL3->setPin(boardOptions.pinButtonL3);
	gamepad->mapButtonR3->setPin(boardOptions.pinButtonR3);
	gamepad->mapButtonA1->setPin(boardOptions.pinButtonA1);
	gamepad->mapButtonA2->setPin(boardOptions.pinButtonA2);

	GamepadStore.save();
}

void ConfigManager::setPreviewBoardOptions(BoardOptions boardOptions) {
	Storage::getInstance().setPreviewBoardOptions(boardOptions);
}

void ConfigManager::setSplashImage(const SplashImage& image) {
	Storage::getInstance().setSplashImage(image);
}
