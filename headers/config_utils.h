#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include "config.pb.h"
#include <string>

namespace ConfigUtils {
    Config load();
    bool save(Config& config);
    
    void initUnsetPropertiesWithDefaults(Config& config);

    std::string toJSON(const Config& config);
    Config fromJSON(const char* data, size_t dataLen, bool& success);
    Config fromLegacyStorage(bool& success);
}

#endif
