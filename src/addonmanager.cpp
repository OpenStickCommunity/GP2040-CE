#include "addonmanager.h"
#include "usbhostmanager.h"

bool AddonManager::LoadAddon(GPAddon* addon, ADDON_PROCESS processAt) {
    if (addon->available()) {
        AddonBlock * block = new AddonBlock;
        addon->setup();
        block->ptr = addon;
        block->process = processAt;
        addons.push_back(block);
        return true;
    } else {
        delete addon; // Don't use the memory if we don't have to   
    }

    return false;
}

bool AddonManager::LoadUSBAddon(GPAddon* addon, ADDON_PROCESS processAt) {
    bool ret = LoadAddon(addon, processAt);
    if ( ret == true )
        USBHostManager::getInstance().pushListener(addon->getListener());
    return ret;
}

void AddonManager::ReinitializeAddons(ADDON_PROCESS processType) {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->process == processType )
            (*it)->ptr->reinit();
    }
}

void AddonManager::PreprocessAddons(ADDON_PROCESS processType) {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->process == processType )
            (*it)->ptr->preprocess();
    }
}

void AddonManager::ProcessAddons(ADDON_PROCESS processType) {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->process == processType )
            (*it)->ptr->process();
    }
}

// HACK : change this for NeoPicoLED
GPAddon * AddonManager::GetAddon(std::string name) { // hack for NeoPicoLED
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->ptr->name() == name )
            return (*it)->ptr;
    }
    return nullptr;
}
