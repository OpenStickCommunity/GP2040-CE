#include "addonmanager.h"

void AddonManager::LoadAddon(GPAddon* addon, ADDON_PROCESS processAt, bool enabled) {
    if (addon->available()) {
        AddonBlock * block = new AddonBlock;
		addon->setup();
        block->ptr = addon;
        block->process = processAt;
        block->enabled = enabled;
        addons.push_back(block);
	} else {
        delete addon; // Don't use the memory if we don't have to
    }
}

void AddonManager::ProcessAddons(ADDON_PROCESS processType) {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->process == processType && (*it)->enabled == true)
            (*it)->ptr->process();
    }
}

// HACK : change this for NeoPicoLED
GPAddon * AddonManager::GetAddon(std::string name) { // hack for NeoPicoLED
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->ptr->name() == name )
            return (*it)->ptr;
    }
}
