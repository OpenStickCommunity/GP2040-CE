#ifndef _GPENCODEREVENT_H_
#define _GPENCODEREVENT_H_

class GPEncoderChangeEvent : public GPEvent {
    public:
        GPEncoderChangeEvent() {}
        GPEncoderChangeEvent(uint8_t id, int8_t dir) {
            this->encoder = id;
            this->direction = dir;
        }
        virtual ~GPEncoderChangeEvent() {}

        uint8_t encoder = 0;
        int8_t direction = 0;

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_ENCODER_CHANGE;
};

#endif