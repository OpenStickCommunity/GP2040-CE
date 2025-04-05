#ifndef _DRIVERMANAGER_H
#define _DRIVERMANAGER_H

#include "enums.pb.h"
#include "gpdriver.h"

class GPDriver;

class DriverManager {
public:
    DriverManager(DriverManager const&) = delete;
    void operator=(DriverManager const&)  = delete;
    static DriverManager& getInstance() {// Thread-safe storage ensures cross-thread talk
        static DriverManager instance; // Guaranteed to be destroyed. // Instantiated on first use.
        return instance;
    }
    GPDriver * getDriver() { return driver; }
    void setup(InputMode);
    InputMode getInputMode(){ return inputMode; }
    bool isConfigMode(){ return (inputMode == INPUT_MODE_CONFIG); }
private:
    DriverManager() {}
    GPDriver * driver;
    InputMode inputMode;
};

#endif