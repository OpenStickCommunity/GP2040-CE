#ifndef _GPCONFIG_H_
#define _GPCONFIG_H_

class GPConfig {
public:
    virtual void setup() = 0;
    virtual void loop() = 0;
private:
};

#endif