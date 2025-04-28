#ifndef _GPPROFILEEVENT_H_
#define _GPPROFILEEVENT_H_

class GPProfileChangeEvent : public GPEvent {
    public:
        GPProfileChangeEvent() {}
        GPProfileChangeEvent(uint8_t prev, uint8_t curr) {
            this->previousValue = prev;
            this->currentValue = curr;
        }
        virtual ~GPProfileChangeEvent() {}

        GPEventType eventType() { return this->_eventType; }

        uint8_t previousValue;
        uint8_t currentValue;
    private:
        GPEventType _eventType = GP_EVENT_PROFILE_CHANGE;
};

#endif