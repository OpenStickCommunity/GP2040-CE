#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include "config.pb.h"
#include <string>

namespace ConfigUtils {
    void load(Config& config);
    bool save(Config& config);
    
    void initUnsetPropertiesWithDefaults(Config& config);

    std::string toJSON(const Config& config);
    bool fromJSON(Config& config, const char* data, size_t dataLen);
    bool fromLegacyStorage(Config& config);

    bool isPinInverted(const GpioMappings& mappings, uint32_t pin);
    void setPinInverted(GpioMappings& mappings, uint32_t pin, bool inverted);
    void clearInvertedPins(GpioMappings& mappings);
}

#endif
