#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include "config.pb.h"
#include <cstdint>
#include <string>

namespace ConfigUtils {
    void load(Config& config);
    bool save(Config& config);
    
    void initUnsetPropertiesWithDefaults(Config& config);

    // headroom: room to prepend an HTTP header in place
    // maxSize: return "" rather than allocate more than this
    std::string toJSON(const Config& config, size_t headroom = 0, size_t maxSize = SIZE_MAX);
    bool fromJSON(Config& config, const char* data, size_t dataLen);
    bool fromLegacyStorage(Config& config);
}

#endif
