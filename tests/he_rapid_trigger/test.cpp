// Host test for headers/addons/he_rapid_trigger.h:
//   c++ -std=c++17 -I headers tests/he_rapid_trigger/test.cpp -o /tmp/he_rt_test && /tmp/he_rt_test tests/he_rapid_trigger/vectors.txt
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "addons/he_rapid_trigger.h"

static std::string trim(const std::string& s) {
    const size_t b = s.find_first_not_of(" \t");
    const size_t e = s.find_last_not_of(" \t\r");
    return b == std::string::npos ? "" : s.substr(b, e - b + 1);
}

int main(int argc, char** argv) {
    std::ifstream file(argc > 1 ? argv[1] : "tests/he_rapid_trigger/vectors.txt");
    if (!file) {
        std::fprintf(stderr, "cannot open vectors file\n");
        return 2;
    }

    int failures = 0, cases = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (trim(line).empty() || line[0] == '#') continue;
        std::stringstream parts(line);
        std::string name, configText, depthsText, expected;
        std::getline(parts, name, '|');
        std::getline(parts, configText, '|');
        std::getline(parts, depthsText, '|');
        std::getline(parts, expected, '|');

        int rapid, continuous;
        HERapidTriggerConfig config;
        std::stringstream(configText) >> rapid >> continuous >> config.actuation >> config.pressSensitivity
            >> config.releaseSensitivity >> config.noise >> config.travel;
        config.rapidTrigger = rapid;
        config.continuous = continuous;

        HERapidTriggerState state;
        heRapidTriggerReset(state);
        std::string actual;
        std::stringstream depths(depthsText);
        int32_t depth;
        while (depths >> depth) actual += heRapidTriggerUpdate(state, config, depth) ? '1' : '0';

        cases++;
        if (actual != trim(expected)) {
            failures++;
            std::printf("FAIL %s: expected %s got %s\n", trim(name).c_str(), trim(expected).c_str(), actual.c_str());
        }
    }

    std::printf("%d/%d rapid trigger vectors passed\n", cases - failures, cases);
    return failures ? 1 : 0;
}
