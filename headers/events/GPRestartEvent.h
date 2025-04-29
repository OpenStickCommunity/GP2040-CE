#ifndef _GPRESTARTEVENT_H_
#define _GPRESTARTEVENT_H_

#include "system.h"

class GPRestartEvent : public GPEvent {
    public:
        GPRestartEvent() {}
        GPRestartEvent(System::BootMode mode) {
            this->bootMode = mode;
        }
        virtual ~GPRestartEvent() {}

        GPEventType eventType() { return this->_eventType; }

        System::BootMode bootMode;
    private:
        GPEventType _eventType = GP_EVENT_RESTART;
};

#endif