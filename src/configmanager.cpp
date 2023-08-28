#include "configmanager.h"

#include "addonmanager.h"
#include "configs/webconfig.h"
#include "addons/neopicoleds.h"

void ConfigManager::setup(ConfigType config) {
	if (config == CONFIG_TYPE_WEB)
		setupConfig(new WebConfig());

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
