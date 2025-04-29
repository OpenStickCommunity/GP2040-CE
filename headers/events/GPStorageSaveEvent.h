#ifndef _GPSTORAGESAVEEVENT_H_
#define _GPSTORAGESAVEEVENT_H_

#include "system.h"

class GPStorageSaveEvent : public GPEvent {
    public:
        GPStorageSaveEvent() {}
        GPStorageSaveEvent(bool force, bool restart = false) {
            this->forceSave = force;
            this->restartAfterSave = restart;
        }
        virtual ~GPStorageSaveEvent() {}

        GPEventType eventType() { return this->_eventType; }

        bool forceSave = false;
        bool restartAfterSave = false;
    private:
        GPEventType _eventType = GP_EVENT_STORAGE_SAVE;
};

#endif