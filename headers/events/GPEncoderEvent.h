#ifndef _GPENCODEREVENT_H_
#define _GPENCODEREVENT_H_

class GPEncoderChangeEvent : public GPEvent {
    public:
        GPEncoderChangeEvent() {}
        GPEncoderChangeEvent(uint8_t id, int8_t dir) {
            this->encoder = id;
            this->direction = dir;
            this->magnitude = 1;
        }
        GPEncoderChangeEvent(uint8_t id, int8_t dir, uint8_t mag) {
            this->encoder = id;
            this->direction = dir;
            this->magnitude = mag;
        }
        virtual ~GPEncoderChangeEvent() {}

        uint8_t encoder = 0;
        int8_t direction = 0;
        // Number of logical steps this event represents. Always >= 1.
        // Lets producers collapse a burst (e.g. a fast spin between input ticks)
        // into a single event instead of allocating one event per step.
        uint8_t magnitude = 1;

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_ENCODER_CHANGE;
};

#endif