#ifndef _GPSYSTEMERROREVENT_H_
#define _GPSYSTEMERROREVENT_H_

#include "system.h"

class GPSystemErrorEvent : public GPEvent {
    public:
        GPSystemErrorEvent() {}
        GPSystemErrorEvent(std::string message) {
            this->errorMessage = message;
        }
        virtual ~GPSystemErrorEvent() {}

        GPEventType eventType() { return this->_eventType; }

        std::string errorMessage;
    private:
        GPEventType _eventType = GP_EVENT_SYSTEM_ERROR;
};

#endif