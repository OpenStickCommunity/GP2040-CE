#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "enums.h"
#include "gpconfig.h"
#include "storagemanager.h"

class ConfigManager {
public:
	ConfigManager(ConfigManager const&) = delete;
	void operator=(ConfigManager const&)  = delete;
    static ConfigManager& getInstance() {// Thread-safe storage ensures cross-thread talk
		static ConfigManager instance; // Guaranteed to be destroyed. // Instantiated on first use.
		return instance;
	}
    void setup(ConfigType);
    void loop(); // If anything needs to update in the gpconfig driver
    void setGamepadOptions(Gamepad*);
private:
    ConfigManager() {}
    void setupConfig(GPConfig*);
    ConfigType cType;
    GPConfig * config;
};

#endif