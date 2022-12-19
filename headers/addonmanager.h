#ifndef _ADDONMANAGER_H_
#define _ADDONMANAGER_H_

#include "gpaddon.h"

#include <vector>
#include <pico/mutex.h>

typedef enum ADDON_PROCESS {
    CORE0_INPUT,
    CORE1_LOOP
} _ADDON_PROCESS;

typedef struct AddonBlock {
    GPAddon * ptr;
    ADDON_PROCESS process;
    bool enabled;
};

class AddonManager {
public:
    AddonManager() {}
    ~AddonManager() {}
    void LoadAddon(GPAddon*, ADDON_PROCESS, bool enabled=true);
    void ProcessAddons(ADDON_PROCESS);
    GPAddon * GetAddon(std::string); // hack for NeoPicoLED
private:
    std::vector<AddonBlock*> addons;    // addons currently loaded
};

#endif
