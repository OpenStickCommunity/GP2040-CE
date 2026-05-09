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
        GPEncoderChangeEvent(uint8_t id, int8_t dir, uint8_t mag, uint8_t encMode) {
            this->encoder = id;
            this->direction = dir;
            this->magnitude = mag;
            this->mode = encMode;
        }
        virtual ~GPEncoderChangeEvent() {}

        uint8_t encoder = 0;
        int8_t direction = 0;
        // Number of logical steps this event represents. Always >= 1.
        // Lets producers collapse a burst (e.g. a fast spin between input ticks)
        // into a single event instead of allocating one event per step.
        uint8_t magnitude = 1;
        // RotaryEncoderPinMode value the encoder is currently configured as.
        // Lets consumers (e.g. KeyboardDriver volume control) filter out
        // events from encoders mapped to non-volume outputs while still
        // letting display widgets animate on any rotation.
        uint8_t mode = 0;

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_ENCODER_CHANGE;
};

#endif