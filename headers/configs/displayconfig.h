#ifndef _DISPLAYCONFIG_H_
#define _DISPLAYCONFIG_H_

#include "gpconfig.h"

#include <vector>

class DisplayState {
public:
private:
    std::vector<char*> options;
};

class DisplayMenu {
public:
    
private:
};

class DisplayConfig : public GPConfig
{
public:
    virtual void setup();
    virtual void loop();
private:
    std::vector<char **> menu_data;
};

#endif