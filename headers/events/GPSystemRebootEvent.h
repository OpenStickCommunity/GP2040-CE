#ifndef _GPSYSTEMREBOOTEVENT_H_
#define _GPSYSTEMREBOOTEVENT_H_

#include "system.h"

class GPSystemRebootEvent : public GPEvent {
    public:
        GPSystemRebootEvent() {}
        GPSystemRebootEvent(System::BootMode mode) {
            this->bootMode = mode;
        }
        virtual ~GPSystemRebootEvent() {}

        GPEventType eventType() { return this->_eventType; }

        System::BootMode bootMode = System::BootMode::DEFAULT;
    private:
        GPEventType _eventType = GP_EVENT_SYSTEM_REBOOT;
};

#endif