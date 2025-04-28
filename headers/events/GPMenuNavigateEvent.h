#ifndef _GPMENUNAVIGATEEVENT_H_
#define _GPMENUNAVIGATEEVENT_H_

#include "system.h"

class GPMenuNavigateEvent : public GPEvent {
    public:
        GPMenuNavigateEvent() {}
        GPMenuNavigateEvent(GpioAction action) {
            this->menuAction = action;
        }
        virtual ~GPMenuNavigateEvent() {}

        GPEventType eventType() { return this->_eventType; }

        GpioAction menuAction;
    private:
        GPEventType _eventType = GP_EVENT_MENU_NAVIGATE;
};

#endif