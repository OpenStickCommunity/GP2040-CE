#ifndef _ADDONMANAGER_H_
#define _ADDONMANAGER_H_

#include "gpaddon.h"

#include <vector>

enum ADDON_PROCESS {
    CORE0_INPUT,
    CORE0_USBREPORT,
    CORE1_ALWAYS,
    CORE1_LOOP
};

struct AddonBlock {
    GPAddon * ptr;
    ADDON_PROCESS process;
};

class AddonManager {
public:
    AddonManager() {}
    ~AddonManager() {}
    bool LoadAddon(GPAddon*);
    bool LoadUSBAddon(GPAddon*);
    void ReinitializeAddons();
    void PreprocessAddons();
    void ProcessAddons();
    void PostprocessAddons(bool);
    GPAddon * GetAddon(std::string); // hack for NeoPicoLED
private:
    std::vector<AddonBlock*> addons;    // addons currently loaded
};

#endif
