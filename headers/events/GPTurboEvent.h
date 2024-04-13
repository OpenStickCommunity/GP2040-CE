#ifndef _GPTURBOEVENT_H_
#define _GPTURBOEVENT_H_

class GPTurboEvent : public GPEvent {
    public:
        GPTurboEvent() {}
        ~GPTurboEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_TURBO;
};

class GPTurboReinitEvent : public GPTurboEvent {
    public:
        GPTurboReinitEvent() {}
        ~GPTurboReinitEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_TURBO_REINIT;
};

class GPTurboChangeEvent : public GPTurboEvent {
    public:
        GPTurboChangeEvent() {}
        GPTurboChangeEvent(uint8_t prev, uint8_t curr) {
            this->previousValue = prev;
            this->currentValue = curr;
        }
        ~GPTurboChangeEvent() {}

        GPEventType eventType() { return this->_eventType; }

        uint8_t previousValue;
        uint8_t currentValue;
    private:
        GPEventType _eventType = GP_EVENT_TURBO_CHANGE;
};

#endif