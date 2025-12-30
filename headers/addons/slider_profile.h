#ifndef _SliderProfile_H
#define _SliderProfile_H

#include "gpaddon.h"
#include "types.h"

#ifndef SLIDER_PROFILE_ENABLED
#define SLIDER_PROFILE_ENABLED 0
#endif

#ifndef SLIDER_PROFILE_NUM_POSITIONS
#define SLIDER_PROFILE_NUM_POSITIONS 2
#endif

#ifndef SLIDER_PROFILE_DEFAULT_PROFILE
#define SLIDER_PROFILE_DEFAULT_PROFILE 1
#endif

#define MAX_PROFILE_SLIDER_POSITIONS 8

// Slider Module Name
#define SliderProfileName "SliderProfile"

class SliderProfileInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // SliderProfile Button Setup
    virtual void reinit();
    virtual void preprocess() {}
    virtual void process();     // SliderProfile process
    virtual void postprocess(bool sent) {}
    virtual std::string name() { return SliderProfileName; }
private:
    uint32_t read();

    Mask_t positionMasks[MAX_PROFILE_SLIDER_POSITIONS] = {0};
    uint32_t numPositions = 0;
};

#endif  // _SliderProfile_H
